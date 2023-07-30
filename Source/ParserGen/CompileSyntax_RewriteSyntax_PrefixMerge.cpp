#include "Compiler.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace compile_syntax;

			extern void CalculateFirstSet_IndirectStartRules(VisitorContext& context);
			extern void CalculateFirstSet_IndirectSimpleUseRules(VisitorContext& context);

			namespace rewritesyntax_prefixmerge
			{
				struct RewritingPrefixConflict
				{
					// all clauses are simple use clauses, RuleSymbol* in values are the rule referenced by keys
					SortedList<GlrClause*>									unaffectedClauses;		// clauses that are not affected by prefix extraction
					SortedList<GlrClause*>									prefixClauses;			// simple use clauses that are prefix themselves
					Group<GlrClause*, GlrClause*>							conflictedClauses;		// c1 -> c2 if c1's prefix is prefix clause c2
				};

				struct PrefixRuleWithClause
				{
					RuleSymbol*												ruleSymbol = nullptr;
					GlrClause*												clause = nullptr;

					bool operator==(PrefixRuleWithClause& p) const
					{
						return ruleSymbol == p.ruleSymbol && clause == p.clause;
					}
				};

				struct RewritingContext
				{
					List<RuleSymbol*>										pmRules;				// all rules that need to be rewritten
					Dictionary<RuleSymbol*, GlrRule*>						skippedRules;			// skipped RuleSymbol -> GlrRule
					Dictionary<RuleSymbol*, GlrRule*>						originRules;			// rewritten RuleSymbol -> GlrRule ends with _LRI_Original, which is the same GlrRule object before rewritten
					Dictionary<RuleSymbol*, GlrRule*>						lriRules;				// rewritten RuleSymbol -> GlrRule containing left_recursion_inject clauses
					Dictionary<RuleSymbol*, GlrRule*>						fixedAstRules;			// RuleSymbol -> GlrRule relationship after rewritten

					Group<RuleSymbol*, PrefixRuleWithClause>				extractPrefixClauses;	// RuleSymbol -> {rule to be extracted, clause begins with rule}
					Dictionary<Pair<RuleSymbol*, RuleSymbol*>, GlrRule*>	extractedPrefixRules;	// {rewritten RuleSymbol, prefix RuleSymbol} -> GlrRule ends with _LRI_Prefix
					Dictionary<RuleSymbol*, Ptr<RewritingPrefixConflict>>	extractedConflicts;		// rewritten RuleSymbol -> all needed information if prefix extraction affects how it generates left_recursion_inject clauses
				};

				Ptr<RewritingPrefixConflict> GetConflict(const RewritingContext& rContext, RuleSymbol* ruleSymbol)
				{
					vint indexConflict = rContext.extractedConflicts.Keys().IndexOf(ruleSymbol);
					if (indexConflict == -1)
					{
						return nullptr;
					}
					else
					{
						return rContext.extractedConflicts.Values()[indexConflict];
					}
				}

				Ptr<RewritingPrefixConflict> EnsureGetConflict(RewritingContext& rContext, RuleSymbol* ruleSymbol)
				{
					auto conflict = GetConflict(rContext, ruleSymbol);
					if (!conflict)
					{
						conflict = Ptr(new RewritingPrefixConflict);
						rContext.extractedConflicts.Add(ruleSymbol, conflict);
					}
					return conflict;
				}

/***********************************************************************
FillMissingPrefixMergeClauses
***********************************************************************/

				void RemoveDirectReferences(RulePathDependencies& references, RuleSymbol* ruleSymbol, GlrClause* clause)
				{
					vint index = references.Keys().IndexOf(ruleSymbol);
					if (index != -1)
					{
						auto&& values = const_cast<List<RuleClausePath>&>(references.GetByIndex(index));
						// TODO: (enumerable) foreach:indexed(alterable(reversed))
						for (vint i = values.Count() - 1; i >= 0; i--)
						{
							if (values[i].clause == clause)
							{
								values.RemoveAt(i);
							}
						}

						if (values.Count() == 0)
						{
							references.Remove(ruleSymbol);
						}
					}
				}

				void FillMissingPrefixMergeClauses(VisitorContext& vContext, SyntaxSymbolManager& syntaxManager, Ptr<GlrSyntaxFile> rewritten)
				{
					// find position of thses clauses in rules
					for (auto clauseRaw : vContext.clauseToConvertedToPrefixMerge)
					{
						auto ruleSymbol = vContext.clauseToRules[clauseRaw];
						auto ruleRaw = vContext.astRules[ruleSymbol];
						vint ruleIndex = rewritten->rules.IndexOf(ruleRaw);
						vint clauseIndex = ruleRaw->clauses.IndexOf(clauseRaw);
						auto clause = ruleRaw->clauses[clauseIndex];

						// create new rule and replace the clause with prefix_merge
						auto newRule = Ptr(new GlrRule);
						rewritten->rules.Insert(ruleIndex, newRule);
						newRule->codeRange = clauseRaw->codeRange;
						newRule->name = ruleRaw->name;
						newRule->name.value += L"_LRI_Isolated_" + itow(clauseIndex);
						newRule->clauses.Add(clause);

						auto newPM = Ptr(new GlrPrefixMergeClause);
						newPM->codeRange = clauseRaw->codeRange;
						ruleRaw->clauses[clauseIndex] = newPM;
						{
							auto startRule = Ptr(new GlrRefSyntax);
							newPM->rule = startRule;

							startRule->refType = GlrRefType::Id;
							startRule->literal.value = newRule->name.value;
						}

						// remove direct references
						RemoveDirectReferences(vContext.directStartRules, ruleSymbol, clause.Obj());
						RemoveDirectReferences(vContext.directSimpleUseRules, ruleSymbol, clause.Obj());

						// fix rule and clause symbols
						auto newRuleSymbol = syntaxManager.CreateRule(
							newRule->name.value,
							ruleSymbol->fileIndex,
							true,
							ruleSymbol->isParser,
							ruleRaw->name.codeRange
							);
						newRuleSymbol->isPartial = ruleSymbol->isPartial;
						newRuleSymbol->ruleType = vContext.clauseTypes[clause.Obj()];
						vContext.astRules.Add(newRuleSymbol, newRule.Obj());
						vContext.clauseTypes.Add(newPM.Obj(), newRuleSymbol->ruleType);
						vContext.clauseToRules.Set(clause.Obj(), newRuleSymbol);
						vContext.clauseToRules.Add(newPM.Obj(), ruleSymbol);
						vContext.clauseToStartRules.Add(newPM.Obj(), newRuleSymbol);

						// fix	directPmClauses
						//		indirectPmClauses
						//		directStartRules
						vContext.directPmClauses.Add(ruleSymbol, newPM.Obj());
						vContext.indirectPmClauses.Add(ruleSymbol, newPM.Obj());
						vContext.directStartRules.Add(ruleSymbol, { newRuleSymbol,newPM.Obj() });
						for (auto key : syntaxManager.Rules().Values())
						{
							if (vContext.indirectStartPathToLastRules.Contains({ key,ruleSymbol }))
							{
								vContext.indirectPmClauses.Add(key, newPM.Obj());
							}
						}
					}

					// fix	indirectStartRules
					//		indirectSimpleUseRules
					//		indirectStartPathToLastRules
					//		indirectSimpleUsePathToLastRules
					vContext.indirectStartRules.Clear();
					vContext.indirectSimpleUseRules.Clear();
					vContext.indirectStartPathToLastRules.Clear();
					vContext.indirectSimpleUsePathToLastRules.Clear();
					CalculateFirstSet_IndirectStartRules(vContext);
					CalculateFirstSet_IndirectSimpleUseRules(vContext);
				}

/***********************************************************************
CollectRewritingTargets
***********************************************************************/

				void CollectRewritingTargets(const VisitorContext& vContext, RewritingContext& rContext, Ptr<GlrSyntaxFile> rewritten)
				{
					for (auto rule : rewritten->rules)
					{
						auto ruleSymbol = vContext.syntaxManager.Rules()[rule->name.value];
						if (vContext.indirectPmClauses.Keys().Contains(ruleSymbol))
						{
							// when a rule has
							//   no direct prefix_merge clause
							//   only one clause which starts with prefix_merge clause
							//   such clause is not a simple use clause
							// skip rewriting it
							if (!vContext.directPmClauses.Keys().Contains(ruleSymbol))
							{
								bool found = false;
								for (auto clause : rule->clauses)
								{
									vint index = vContext.clauseToStartRules.Keys().IndexOf(clause.Obj());
									if (index == -1) continue;
							
									auto&& startRules = vContext.clauseToStartRules.GetByIndex(index);
									for (auto startRule : startRules)
									{
										if (vContext.indirectPmClauses.Keys().Contains(startRule))
										{
											if (vContext.simpleUseClauseToReferencedRules.Keys().Contains(clause.Obj()))
											{
												goto DO_NOT_SKIP;
											}

											if (!found)
											{
												found = true;
												break;
											}
											else
											{
												goto DO_NOT_SKIP;
											}
											break;
										}
									}
								}

								rContext.skippedRules.Add(ruleSymbol, rule.Obj());
								continue;
							DO_NOT_SKIP:;
							}

							rContext.pmRules.Add(ruleSymbol);

							vint indexStart = vContext.directStartRules.Keys().IndexOf(ruleSymbol);
							vint indexSimpleUse = vContext.directSimpleUseRules.Keys().IndexOf(ruleSymbol);

							if (indexStart != -1 && indexSimpleUse != -1)
							{
								// all clauses should be simple use to enable prefix detection
								if (vContext.directSimpleUseRules.GetByIndex(indexSimpleUse).Count() != rule->clauses.Count()) continue;
								Ptr<RewritingPrefixConflict> conflict;

								for (auto [startRule, startClause] : vContext.directStartRules.GetByIndex(indexStart))
								{
									// prefix_merge clauses and left recursive clauses are not involved in prefix detection/extraction
									if (dynamic_cast<GlrPrefixMergeClause*>(startClause)) continue;
									if (vContext.leftRecursiveClauses.Contains(ruleSymbol, startClause)) continue;

									// find all clause pair "!X" and "Y ...", see if X is a prefix of Y
									for (auto [simpleUseRule, simpleUseClause] : vContext.directSimpleUseRules.GetByIndex(indexSimpleUse))
									{
										// ignore if X is Y
										if (startRule == simpleUseRule) continue;

										// ignore if X is not a prefix of Y
										vint indexExtract = vContext.indirectStartPathToLastRules.Keys().IndexOf({ startRule,simpleUseRule });
										if (indexExtract == -1) continue;

										// ignore if Y ::= X directly or indirectly
										for (auto [extractRule, extractClause] : vContext.indirectStartPathToLastRules.GetByIndex(indexExtract))
										{
											// ignore if Y ::= X directly or indirectly
											if (vContext.directSimpleUseRules.Contains(extractRule, { simpleUseRule,extractClause })) continue;

											// prefix extraction needed
											if (!rContext.extractPrefixClauses.Contains(extractRule, { simpleUseRule,extractClause }))
											{
												rContext.extractPrefixClauses.Add(extractRule, { simpleUseRule,extractClause });
											}

											// fill conflict information for ruleSymbol
											if (!conflict)
											{
												conflict = EnsureGetConflict(rContext, ruleSymbol);
											}

											if (!conflict->prefixClauses.Contains(simpleUseClause))
											{
												conflict->prefixClauses.Add(simpleUseClause);
											}
											if (!conflict->conflictedClauses.Contains(startClause, simpleUseClause))
											{
												conflict->conflictedClauses.Add(startClause, simpleUseClause);
											}
										}
									}
								}

								if (conflict)
								{
									for (auto clause : rule->clauses)
									{
										if (!conflict->prefixClauses.Contains(clause.Obj()) && !conflict->conflictedClauses.Contains(clause.Obj()))
										{
											conflict->unaffectedClauses.Add(clause.Obj());
										}
									}
								}
							}
						}
					}
				}

/***********************************************************************
CreateRewrittenRules
***********************************************************************/

				void CreateRewrittenRules(const VisitorContext& vContext, RewritingContext& rContext, Ptr<GlrSyntaxFile> rewritten)
				{
					for (auto ruleSymbol : rContext.pmRules)
					{
						auto originRule = vContext.astRules[ruleSymbol];
						rContext.originRules.Add(ruleSymbol, originRule);

						auto lri = Ptr(new GlrRule);
						rewritten->rules.Add(lri);
						rContext.lriRules.Add(ruleSymbol, lri.Obj());

						lri->codeRange = originRule->codeRange;
						lri->attPublic = originRule->attPublic;
						lri->attParser = originRule->attParser;
						lri->codeRange = originRule->name.codeRange;
						lri->name = originRule->name;

						originRule->attPublic = {};
						originRule->attParser = {};
						originRule->name.value += L"_LRI_Original";
					}

					// TODO: (enumerable) foreach
					for (auto [ruleSymbol, index] : indexed(rContext.extractPrefixClauses.Keys()))
					{
						auto originRule = vContext.astRules[ruleSymbol];
						auto&& prefixClauses = rContext.extractPrefixClauses.GetByIndex(index);
						for (auto [prefixRuleSymbol, prefixClause] : From(prefixClauses)
							.OrderByKey([](auto&& p) {return p.ruleSymbol->Name(); }))
						{
							if (!rContext.extractedPrefixRules.Keys().Contains({ ruleSymbol,prefixRuleSymbol }))
							{
								auto ep = Ptr(new GlrRule);
								rewritten->rules.Insert(rewritten->rules.IndexOf(originRule), ep);
								rContext.extractedPrefixRules.Add({ ruleSymbol,prefixRuleSymbol }, ep.Obj());

								ep->codeRange = originRule->codeRange;
								ep->name.codeRange = originRule->name.codeRange;
								ep->name.value = ruleSymbol->Name() + L"_" + prefixRuleSymbol->Name() + L"_LRI_Prefix";
							}
						}
					}
				}

/***********************************************************************
FixRuleTypes
***********************************************************************/

				void FixRuleTypes(const VisitorContext& vContext, RewritingContext& rContext, SyntaxSymbolManager& syntaxManager)
				{
					CopyFrom(rContext.fixedAstRules, vContext.astRules);
					for (auto ruleSymbol : rContext.pmRules)
					{
						auto originRule = rContext.originRules[ruleSymbol];
						auto lriRule = rContext.lriRules[ruleSymbol];
						auto originSymbol = syntaxManager.CreateRule(
							originRule->name.value,
							ruleSymbol->fileIndex,
							ruleSymbol->isPublic,
							ruleSymbol->isParser,
							originRule->codeRange
							);

						originSymbol->isPartial = ruleSymbol->isPartial;
						originSymbol->ruleType = ruleSymbol->ruleType;
						rContext.fixedAstRules.Set(originSymbol, originRule);
						rContext.fixedAstRules.Set(ruleSymbol, lriRule);
					}

					for (auto [pair, epRule] : rContext.extractedPrefixRules)
					{
						auto originRule = rContext.originRules[pair.key];
						auto epRuleSymbol = syntaxManager.CreateRule(
							epRule->name.value,
							pair.key->fileIndex,
							false,
							false,
							originRule->codeRange
							);
						epRuleSymbol->ruleType = pair.value->ruleType;
						rContext.fixedAstRules.Add(epRuleSymbol, epRule);
					}

					for (auto [ruleSymbol, rule] : rContext.fixedAstRules)
					{
						if (!rule->type)
						{
							rule->type.value = ruleSymbol->ruleType->Name();
						}
					}

					for (auto ruleSymbol : syntaxManager.Rules().Values())
					{
						ruleSymbol->assignedNonArrayField = false;
						ruleSymbol->lrFlags.Clear();
					}
				}

/***********************************************************************
RewriteExtractedPrefixRules
***********************************************************************/

				void RewriteExtractedPrefixRules(const VisitorContext& vContext, RewritingContext& rContext, SyntaxSymbolManager& syntaxManager)
				{
					for (auto [pair, epRule] : rContext.extractedPrefixRules)
					{
						{
							auto lrpClause = Ptr(new GlrLeftRecursionPlaceholderClause);
							lrpClause->codeRange = epRule->codeRange;
							epRule->clauses.Add(lrpClause);

							auto lrp = Ptr(new GlrLeftRecursionPlaceholder);
							lrpClause->flags.Add(lrp);
							lrp->flag.value = L"LRIP_" + pair.key->Name() + L"_" + pair.value->Name();
							syntaxManager.lrpFlags.Add(lrp->flag.value);
						}
						{
							auto reuseClause = Ptr(new GlrReuseClause);
							reuseClause->codeRange = epRule->codeRange;
							epRule->clauses.Add(reuseClause);

							auto useSyntax = Ptr(new GlrUseSyntax);
							reuseClause->syntax = useSyntax;
							useSyntax->name.value = rContext.originRules[pair.value]->name.value;
						}
					}
				}

/***********************************************************************
RewriteRules (Common)
***********************************************************************/

				using PMClauseRecord = GenericRuleClausePath<GlrPrefixMergeClause>;

				struct PMInjectRecord
				{
					RuleSymbol*					pmRule = nullptr;
					RuleSymbol*					injectIntoRule = nullptr;
				};

				void RewriteRules_AddPmClause(
					const VisitorContext& vContext,
					RuleSymbol* ruleSymbol,
					GlrPrefixMergeClause* pmClause,
					Group<WString, PMClauseRecord>& pmClauses
				)
				{
					auto pmRule = vContext.clauseToRules[pmClause];
					if (ruleSymbol == pmRule)
					{
						if (!pmClauses.Contains(pmClause->rule->literal.value, { ruleSymbol,pmClause }))
						{
							pmClauses.Add(pmClause->rule->literal.value, { ruleSymbol,pmClause });
						}
					}
					else
					{
						for (auto [indirectStartRule, indirectClause] : vContext.indirectStartPathToLastRules[{ ruleSymbol, pmRule }])
						{
							if (!pmClauses.Contains(pmClause->rule->literal.value, { ruleSymbol,pmClause }))
							{
								pmClauses.Add(pmClause->rule->literal.value, { ruleSymbol,pmClause });
							}
						}
					}
				}

				Ptr<RewritingPrefixConflict> RewriteRules_CollectUnaffectedIndirectPmClauses(
					const VisitorContext& vContext,
					const RewritingContext& rContext,
					RuleSymbol* initiatedRuleSymbol,
					RuleSymbol* ruleSymbol,
					SortedList<RuleSymbol*>& visited,
					Group<WString, PMClauseRecord>& pmClauses
				)
				{
					auto conflict = initiatedRuleSymbol == ruleSymbol ? GetConflict(rContext, ruleSymbol) : nullptr;
					if (!visited.Contains(ruleSymbol))
					{
						visited.Add(ruleSymbol);

						if (conflict)
						{
							for (auto [simpleUseRule, simpleUseClause] : vContext.directSimpleUseRules[ruleSymbol])
							{
								if (conflict->unaffectedClauses.Contains(simpleUseClause))
								{
									RewriteRules_CollectUnaffectedIndirectPmClauses(vContext, rContext, initiatedRuleSymbol, simpleUseRule, visited, pmClauses);
								}
								else
								{
									vint indexConflicted = conflict->conflictedClauses.Keys().IndexOf(simpleUseClause);
									if (indexConflicted == -1) continue;

									auto&& prefixClauses = conflict->conflictedClauses.GetByIndex(indexConflicted);
									for (auto pmClause :
										From(vContext.indirectPmClauses[ruleSymbol]).Except(
											From(prefixClauses)
											.SelectMany([&](GlrClause* prefixClause)
											{
												return From(vContext.indirectPmClauses[vContext.simpleUseClauseToReferencedRules[prefixClause]]);
											})
										))
									{
										RewriteRules_AddPmClause(vContext, simpleUseRule, pmClause, pmClauses);
									}
								}
							}
						}
						else
						{
							for (auto pmClause : vContext.indirectPmClauses[ruleSymbol])
							{
								RewriteRules_AddPmClause(vContext, ruleSymbol, pmClause, pmClauses);
							}
						}
					}
					return conflict;
				}

				void RewriteRules_CheckPath(
					const VisitorContext& vContext,
					RuleSymbol* startSymbol,
					RuleSymbol* endSymbol,
					bool& hasSimpleUseTransition,
					bool& hasNonSimpleUseTransition
				)
				{
					// check the path from startSymbol to endSymbol
					// hasSimpleUseTransition and hasNonSimpleUseTransition must be false before non-recursive calling from outside
					// if a transition is found
					// either of one will be set to true
					// meaning such transition is counted

					bool isEndSymbolLeftRecursive = vContext.leftRecursiveClauses.Keys().Contains(endSymbol);
					if (isEndSymbolLeftRecursive)
					{
						// if endSymbol is left recursive
						// then we find a non-simple-use transition from startSymbol to endSymbol
						hasNonSimpleUseTransition = true;
					}

					if (startSymbol == endSymbol)
					{
						// if startSymbol is endSymbol
						// then we find a simple-use transition
						// which does not actually transit
						hasSimpleUseTransition = true;
					}
					else
					{
						// otherwise we look through all possible path from startSymbol to endSymbol
						vint index = vContext.indirectStartPathToLastRules.Keys().IndexOf({ startSymbol,endSymbol });
						if (index != -1)
						{
							for (auto [lastRuleSymbol, clause] : vContext.indirectStartPathToLastRules.GetByIndex(index))
							{
								if (vContext.simpleUseClauseToReferencedRules.Keys().Contains(clause))
								{
									// if we find a simple-use clause
									// we check how all transitions look like from startSymbol to lastRuleSymbol
									RewriteRules_CheckPath(vContext, startSymbol, lastRuleSymbol, hasSimpleUseTransition, hasNonSimpleUseTransition);
								}
								else
								{
									// if we find a non-simple-use clause
									// then we find a non-simple-use transition from startSymbol to endSymbol
									hasNonSimpleUseTransition = true;
								}
							}
						}
					}
				}

				void RewriteRules_CollectFlags(
					const VisitorContext& vContext,
					RuleSymbol* ruleSymbol,
					const List<PMClauseRecord>& pmClauses,
					Dictionary<WString, PMInjectRecord>& flags,
					bool& generateOptionalLri
				)
				{
					for (auto [injectIntoRule, pmClause] : pmClauses)
					{
						auto pmRule = vContext.clauseToRules[pmClause];
						bool hasSimpleUseTransition = false;
						bool hasNonSimpleUseTransition = false;
						RewriteRules_CheckPath(vContext, ruleSymbol, pmRule, hasSimpleUseTransition, hasNonSimpleUseTransition);

						if (hasSimpleUseTransition)
						{
							generateOptionalLri = true;
						}

						if (hasNonSimpleUseTransition)
						{
							flags.Add(L"LRI_" + pmRule->Name(), { pmRule,injectIntoRule });
						}
					}
				}

				bool RewriteRules_HasMultiplePaths(
					const VisitorContext& vContext,
					RuleSymbol* fromRule,
					RuleSymbol* toRule,
					Dictionary<Pair<RuleSymbol*, RuleSymbol*>, vint>& pathCounter
				)
				{
					Pair<RuleSymbol*, RuleSymbol*> key = { fromRule, toRule };
					vint index = pathCounter.Keys().IndexOf(key);
					if (index != -1) return pathCounter.Values()[index];

					RuleSymbol* currentRule = toRule;
					bool hasMultiplePaths = false;

					while (currentRule != fromRule)
					{
						index = vContext.indirectStartPathToLastRules.Keys().IndexOf({ fromRule,currentRule });
						if (index == -1) goto FINISHED;

						auto&& lastRules = vContext.indirectStartPathToLastRules.GetByIndex(index);
						if (lastRules.Count() > 1) {
							hasMultiplePaths = true;
							goto FINISHED;
						}
						currentRule = lastRules[0].ruleSymbol;
					}

				FINISHED:
					pathCounter.Add(key, hasMultiplePaths);
					return hasMultiplePaths;
				}

/***********************************************************************
RewriteRules (AST Creation)
***********************************************************************/

				Ptr<GlrLeftRecursionInjectClause> CreateLriClause(
					const WString& ruleName
				)
				{
					auto lriClause = Ptr(new GlrLeftRecursionInjectClause);

					auto lriStartRule = Ptr(new GlrRefSyntax);
					lriClause->rule = lriStartRule;
					lriStartRule->refType = GlrRefType::Id;
					lriStartRule->literal.value = ruleName;

					return lriClause;
				}

				Ptr<GlrLeftRecursionInjectContinuation> CreateLriContinuation(
					const VisitorContext& vContext,
					const RewritingContext& rContext,
					RuleSymbol* ruleSymbol,
					const PMInjectRecord& pmInjectRecord,
					const WString& flag,
					Dictionary<Pair<RuleSymbol*, RuleSymbol*>, vint>& pathCounter,
					bool generateOptionalLri
				)
				{
					auto lriCont = Ptr(new GlrLeftRecursionInjectContinuation);

					if (RewriteRules_HasMultiplePaths(vContext, ruleSymbol, pmInjectRecord.pmRule, pathCounter))
					{
						lriCont->configuration = GlrLeftRecursionConfiguration::Multiple;
					}
					else
					{
						lriCont->configuration = GlrLeftRecursionConfiguration::Single;
					}

					if (generateOptionalLri)
					{
						lriCont->type = GlrLeftRecursionInjectContinuationType::Optional;
					}
					else
					{
						lriCont->type = GlrLeftRecursionInjectContinuationType::Required;
					}

					auto lriContFlag = Ptr(new GlrLeftRecursionPlaceholder);
					lriCont->flags.Add(lriContFlag);
					lriContFlag->flag.value = flag;

					vint injectIntoOriginIndex = rContext.originRules.Keys().IndexOf(pmInjectRecord.injectIntoRule);
					auto lriContTargetName = injectIntoOriginIndex == -1
						? pmInjectRecord.injectIntoRule->Name()
						: rContext.originRules.Values()[injectIntoOriginIndex]->name.value
						;
					auto lriContTarget = CreateLriClause(lriContTargetName);
					lriCont->injectionTargets.Add(lriContTarget);

					return lriCont;
				}

/***********************************************************************
RewriteRules (Unaffected)
***********************************************************************/

				void RewriteRules_GenerateUnaffectedLRIClauses(
					const VisitorContext& vContext,
					const RewritingContext& rContext,
					RuleSymbol* ruleSymbol,
					GlrRule* lriRule,
					Dictionary<Pair<RuleSymbol*, RuleSymbol*>, vint>& pathCounter,
					Group<WString, PMClauseRecord>& pmClauses,
					SortedList<WString>& knownOptionalStartRules
				)
				{
					// TODO: (enumerable) foreach on group
					for (auto [pmName, pmIndex] : indexed(pmClauses.Keys()))
					{
						//   if originRule is not left recursive
						//     do not generate lriClause for the flag created for originRule, because there is no continuation
						//     if a pmName does generate some lriClause
						//       it becomes GLRICT::Optional
						//     otherwise
						//       it becomse GLRICT::Required
						//       generate useSyntax instead of lriClause

						Dictionary<WString, PMInjectRecord> flags;
						bool generateOptionalLri = false;
						RewriteRules_CollectFlags(
							vContext,
							ruleSymbol,
							pmClauses.GetByIndex(pmIndex),
							flags,
							generateOptionalLri
							);

						if (generateOptionalLri && flags.Count() == 0)
						{
							auto reuseClause = Ptr(new GlrReuseClause);
							reuseClause->codeRange = lriRule->codeRange;
							lriRule->clauses.Add(reuseClause);

							auto useSyntax = Ptr(new GlrUseSyntax);
							reuseClause->syntax = useSyntax;
							useSyntax->name.value = pmName;
						}

						for (auto [flag, pmInjectRecord] : flags)
						{
							auto lriClause = CreateLriClause(pmName);
							lriRule->clauses.Add(lriClause);

							auto lriCont = CreateLriContinuation(
								vContext,
								rContext,
								ruleSymbol,
								pmInjectRecord,
								flag,
								pathCounter,
								generateOptionalLri
								);
							lriClause->continuation = lriCont;

							if (generateOptionalLri)
							{
								generateOptionalLri = false;
								knownOptionalStartRules.Add(pmName);
							}
						}
					}
				}

/***********************************************************************
RewriteRules (Affected)
***********************************************************************/

				void RewriteRules_GenerateAffectedLRIClausesSubgroup(
					const VisitorContext& vContext,
					const RewritingContext& rContext,
					RuleSymbol* conflictedRuleSymbol,
					RuleSymbol* prefixRuleSymbol,
					SortedList<WString>& lripFlags,
					GlrRule* lriRule,
					Dictionary<Pair<RuleSymbol*, RuleSymbol*>, vint>& pathCounter,
					SortedList<WString>& knownOptionalStartRules
				)
				{
					Group<WString, PMClauseRecord> pmClauses;
					{
						SortedList<RuleSymbol*> visited;
						auto conflict = RewriteRules_CollectUnaffectedIndirectPmClauses(
							vContext,
							rContext,
							prefixRuleSymbol,
							prefixRuleSymbol,
							visited,
							pmClauses
							);

						if (conflict)
						{
							vContext.syntaxManager.AddError(
								ParserErrorType::PrefixExtractionAffectedRuleReferencedAnother,
								rContext.originRules[vContext.syntaxManager.Rules()[lriRule->name.value]]->codeRange,
								lriRule->name.value,
								conflictedRuleSymbol->Name(),
								prefixRuleSymbol->Name()
								);
							return;
						}
					}

					// TODO: (enumerable) foreach on group
					for (auto [pmName, pmIndex] : indexed(pmClauses.Keys()))
					{
						//   if originRule is not left recursive
						//     left_recursion_inject directly into conflictedRuleSymbol
						//     if a pmName does generate some lriClause
						//       it becomes GLRICT::Optional
						//     otherwise
						//       it becomse GLRICT::Required
						//       generate useSyntax instead of lriClause

						auto&& pmClauseRecords = pmClauses.GetByIndex(pmIndex);
				
						Dictionary<WString, PMInjectRecord> flags;
						bool omittedSelf = false;
						bool generateOptionalLri = false;
						RewriteRules_CollectFlags(
							vContext,
							prefixRuleSymbol,
							pmClauseRecords,
							flags,
							generateOptionalLri
							);

						if (generateOptionalLri)
						{
							// TODO: add test case for omittedSelf == true
							for (auto lripFlag : lripFlags)
							{
								auto lriClause = CreateLriClause(pmName);
								lriRule->clauses.Add(lriClause);

								auto lriCont = CreateLriContinuation(
									vContext,
									rContext,
									conflictedRuleSymbol,
									{ prefixRuleSymbol,conflictedRuleSymbol },
									lripFlag,
									pathCounter,
									false
									);
								lriClause->continuation = lriCont;
							}
						}

						if (knownOptionalStartRules.Contains(pmName))
						{
							generateOptionalLri = false;
						}

						for (auto [flag, pmInjectRecord] : flags)
						{
							{
								// Since there is no switches in pmClauses
								// So there is no switches in pmInjectRecord as well
								// because flags comes from pmClauses, collected by RewriteRules_CollectFlags above
								auto lriClause = CreateLriClause(pmName);
								lriRule->clauses.Add(lriClause);
					
								auto lriCont = CreateLriContinuation(
									vContext,
									rContext,
									prefixRuleSymbol,
									{ pmInjectRecord.pmRule,prefixRuleSymbol },
									flag,
									pathCounter,
									generateOptionalLri
									);
								lriClause->continuation = lriCont;

								for (auto lripFlag : lripFlags)
								{
									auto lriCont2 = CreateLriContinuation(
										vContext,
										rContext,
										conflictedRuleSymbol,
										{ prefixRuleSymbol,conflictedRuleSymbol },
										lripFlag,
										pathCounter,
										true
										);

									auto branchStart = lriCont->injectionTargets[0];
									if (branchStart->continuation)
									{
										// TODO: add test case
										auto newBranchStart = CreateLriClause(branchStart->rule->literal.value);
										newBranchStart->continuation = lriCont2;
										lriCont->injectionTargets.Add(newBranchStart);
									}
									else
									{
										branchStart->continuation = lriCont2;
									}
								}

								if (generateOptionalLri)
								{
									generateOptionalLri = false;
								}
							}
						}
					}
				}

				void RewriteRules_GenerateAffectedLRIClauses(
					const VisitorContext& vContext,
					const RewritingContext& rContext,
					RuleSymbol* ruleSymbol,
					GlrRule* lriRule,
					Ptr<RewritingPrefixConflict> conflict,
					Dictionary<Pair<RuleSymbol*, RuleSymbol*>, vint>& pathCounter,
					SortedList<WString>& knownOptionalStartRules
				)
				{
					// TODO: (enumerable) foreach on group
					for (auto [conflictedClause, conflictedIndex] : indexed(conflict->conflictedClauses.Keys()))
					{
						auto conflictedRuleSymbol = vContext.simpleUseClauseToReferencedRules[conflictedClause];
						auto&& prefixClauses = conflict->conflictedClauses.GetByIndex(conflictedIndex);
						for (auto prefixClause : prefixClauses)
						{
							auto prefixRuleSymbol = vContext.simpleUseClauseToReferencedRules[prefixClause];
							SortedList<WString> lripFlags;
							for (auto extracted : vContext.indirectStartPathToLastRules[{conflictedRuleSymbol, prefixRuleSymbol}])
							{
								lripFlags.Add(L"LRIP_" + extracted.ruleSymbol->Name() + L"_" + prefixRuleSymbol->Name());
							}
							RewriteRules_GenerateAffectedLRIClausesSubgroup(
								vContext,
								rContext,
								conflictedRuleSymbol,
								prefixRuleSymbol,
								lripFlags,
								lriRule,
								pathCounter,
								knownOptionalStartRules
								);
						}
					}
				}

/***********************************************************************
RewriteRules
***********************************************************************/

				bool CompareLri(Ptr<GlrLeftRecursionInjectClause> c1, Ptr<GlrLeftRecursionInjectClause> c2);

				bool CompareLriTargest(List<Ptr<GlrLeftRecursionInjectClause>>& targets1, List<Ptr<GlrLeftRecursionInjectClause>>& targets2)
				{
					if (targets1.Count() != targets2.Count()) return false;
					for (auto [target, i] : indexed(targets1))
					{
						if (!CompareLri(targets1[i], targets2[i])) return false;
					}
					return true;
				}

				bool CompareLri(Ptr<GlrLeftRecursionInjectClause> c1, Ptr<GlrLeftRecursionInjectClause> c2)
				{
					if (c1->rule->literal.value != c2->rule->literal.value) return false;
					if ((c1->continuation == nullptr) != (c2->continuation == nullptr)) return false;
					if (!c1->continuation) return true;

					auto cont1 = c1->continuation;
					auto cont2 = c2->continuation;
					if (cont1->flags.Count() != cont2->flags.Count()) return false;
					// TODO: (enumerable) foreach:indexed, flag not used, considering Linq:Zip, Any
					for (auto [flag, i] : indexed(cont1->flags))
					{
						if (cont1->flags[i]->flag.value != cont2->flags[i]->flag.value) return false;
					}

					return CompareLriTargest(cont1->injectionTargets, cont2->injectionTargets);
				}

				void OptimizeLri(List<Ptr<GlrLeftRecursionInjectClause>>& lriClauses)
				{
					for (auto lriClause : lriClauses)
					{
						if (lriClause->continuation)
						{
							OptimizeLri(lriClause->continuation->injectionTargets);
						}
					}

					List<Ptr<GlrLeftRecursionInjectClause>> results, candidates;
					for (auto [prefixRuleName, subLriClauses] : From(lriClauses)
						.GroupBy([](auto lriClause) {return lriClause->rule->literal.value; })
						)
					{
						CopyFrom(candidates, From(subLriClauses).Reverse());
						while (candidates.Count() > 0)
						{
							auto candidate = candidates[candidates.Count() - 1];
							candidates.RemoveAt(candidates.Count() - 1);
							// TODO: (enumerable) foreach:indexed(alterable(reversed))
							for (vint i = candidates.Count() - 1; i >= 0; i--)
							{
								auto compare = candidates[i];
								if (CompareLriTargest(candidate->continuation->injectionTargets, compare->continuation->injectionTargets))
								{
									List<Ptr<GlrLeftRecursionPlaceholder>> flags;
									CopyFrom(
										flags,
										From(candidate->continuation->flags)
											.Concat(compare->continuation->flags)
									);
									CopyFrom(
										candidate->continuation->flags,
										From(flags)
											.GroupBy([](auto flag) { return flag->flag.value; })
											.Select([](auto pair) { return pair.value.First(); })
									);

									if (compare->continuation->configuration == GlrLeftRecursionConfiguration::Multiple)
									{
										candidate->continuation->configuration = GlrLeftRecursionConfiguration::Multiple;
									}

									if(compare->continuation->type==GlrLeftRecursionInjectContinuationType::Optional)
									{
										candidate->continuation->type = GlrLeftRecursionInjectContinuationType::Optional;
									}

									candidates.RemoveAt(i);
								}
							}
							results.Add(candidate);
						}
					}

					CopyFrom(lriClauses, results);
				}

				void RewriteRules(const VisitorContext& vContext, const RewritingContext& rContext, SyntaxSymbolManager& syntaxManager)
				{
					Dictionary<Pair<RuleSymbol*, RuleSymbol*>, vint> pathCounter;
					for (auto [ruleSymbol, originRule] : rContext.originRules)
					{
						auto lriRule = rContext.lriRules[ruleSymbol];
						Ptr<RewritingPrefixConflict> conflict;
						Group<WString, PMClauseRecord> pmClauses;
						{
							SortedList<RuleSymbol*> visited;
							conflict = RewriteRules_CollectUnaffectedIndirectPmClauses(
								vContext,
								rContext,
								ruleSymbol,
								ruleSymbol,
								visited,
								pmClauses
								);
						}

						SortedList<WString> knownOptionalStartRules;
						RewriteRules_GenerateUnaffectedLRIClauses(
							vContext,
							rContext,
							ruleSymbol,
							lriRule,
							pathCounter,
							pmClauses,
							knownOptionalStartRules
							);

						if (conflict)
						{
							RewriteRules_GenerateAffectedLRIClauses(
								vContext,
								rContext,
								ruleSymbol,
								lriRule,
								conflict,
								pathCounter,
								knownOptionalStartRules
								);
						}

						List<Ptr<GlrClause>> otherClauses;
						List<Ptr<GlrLeftRecursionInjectClause>> lriClauses;
						for (auto clause : lriRule->clauses)
						{
							if (auto lriClause = clause.Cast<GlrLeftRecursionInjectClause>())
							{
								lriClauses.Add(lriClause);
							}
							else
							{
								otherClauses.Add(clause);
							}
						}

						if (lriClauses.Count() > 0)
						{
							OptimizeLri(lriClauses);
							CopyFrom(
								lriRule->clauses,
								From(otherClauses)
									.Concat(
										From(lriClauses).Select([](auto clause)->Ptr<GlrClause> {return clause; })
									)
								);
						}
					}
				}

/***********************************************************************
FixPrefixMergeClauses
***********************************************************************/

				void FixPrefixMergeClauses(const VisitorContext& vContext, const RewritingContext& rContext, SyntaxSymbolManager& syntaxManager)
				{
					for (auto ruleSymbol : vContext.directPmClauses.Keys())
					{
						auto originRule = rContext.originRules[ruleSymbol];

						auto lrpClause = Ptr(new GlrLeftRecursionPlaceholderClause);
						lrpClause->codeRange = originRule->codeRange;
						originRule->clauses.Insert(0, lrpClause);

						auto lrp = Ptr(new GlrLeftRecursionPlaceholder);
						lrp->flag.value = L"LRI_" + ruleSymbol->Name();
						lrpClause->flags.Add(lrp);
						syntaxManager.lrpFlags.Add(lrp->flag.value);

						for (vint i = 0; i < originRule->clauses.Count(); i++)
						{
							if (auto pmClause = originRule->clauses[i].Cast<GlrPrefixMergeClause>())
							{
								if (ruleSymbol->isPartial)
								{
									auto partialClause = Ptr(new GlrPartialClause);
									originRule->codeRange = originRule->codeRange;
									originRule->clauses[i] = partialClause;

									partialClause->type.value = ruleSymbol->ruleType->Name();

									auto refSyntax = Ptr(new GlrRefSyntax);
									refSyntax->refType = GlrRefType::Id;
									refSyntax->literal.value = pmClause->rule->literal.value;
									partialClause->syntax = refSyntax;
								}
								else
								{
									auto reuseClause = Ptr(new GlrReuseClause);
									originRule->codeRange = originRule->codeRange;
									originRule->clauses[i] = reuseClause;

									auto useSyntax = Ptr(new GlrUseSyntax);
									useSyntax->name.value = pmClause->rule->literal.value;
									reuseClause->syntax = useSyntax;
								}
							}
						}
					}
				}

/***********************************************************************
RenamePrefix
***********************************************************************/

				class RenamePrefixVisitor
					: public Object
					, protected virtual GlrSyntax::IVisitor
					, protected virtual GlrClause::IVisitor
				{
				protected:
					const RewritingContext&			rContext;
					RuleSymbol*						ruleSymbol;
					const SyntaxSymbolManager&		syntaxManager;

				public:
					RenamePrefixVisitor(
						const RewritingContext& _rContext,
						RuleSymbol* _ruleSymbol,
						const SyntaxSymbolManager& _syntaxManager
					)
						: rContext(_rContext)
						, ruleSymbol(_ruleSymbol)
						, syntaxManager(_syntaxManager)
					{
					}

					void FixClause(Ptr<GlrClause> clause)
					{
						clause->Accept(this);
					}

				protected:

					void FixStartRule(ParsingToken& ruleName)
					{
						auto startRuleSymbol = syntaxManager.Rules()[ruleName.value];
						vint index = rContext.extractedPrefixRules.Keys().IndexOf({ ruleSymbol,startRuleSymbol });
						if (index != -1)
						{
							auto epRule = rContext.extractedPrefixRules.Values()[index];
							ruleName.value = epRule->name.value;
							return;
						}

						index = rContext.originRules.Keys().IndexOf(startRuleSymbol);
						if (index != -1)
						{
							auto originRule = rContext.originRules.Values()[index];
							ruleName.value = originRule->name.value;
						}
					}

					////////////////////////////////////////////////////////////////////////
					// GlrSyntax::IVisitor
					////////////////////////////////////////////////////////////////////////

					void Visit(GlrRefSyntax* node) override
					{
						if (node->refType == GlrRefType::Id)
						{
							vint index = syntaxManager.Rules().Keys().IndexOf(node->literal.value);
							if (index != -1)
							{
								FixStartRule(node->literal);
							}
						}
					}

					void Visit(GlrUseSyntax* node) override
					{
						FixStartRule(node->name);
					}

					void Visit(GlrLoopSyntax* node) override
					{
					}

					void Visit(GlrOptionalSyntax* node) override
					{
					}

					void Visit(GlrSequenceSyntax* node) override
					{
						node->first->Accept(this);
					}

					void Visit(GlrAlternativeSyntax* node) override
					{
					}

					void Visit(GlrPushConditionSyntax* node) override
					{
						node->syntax->Accept(this);
					}

					void Visit(GlrTestConditionSyntax* node) override
					{
					}

					////////////////////////////////////////////////////////////////////////
					// GlrClause::IVisitor
					////////////////////////////////////////////////////////////////////////

					void Visit(GlrCreateClause* node) override
					{
						node->syntax->Accept(this);
					}

					void Visit(GlrPartialClause* node) override
					{
						node->syntax->Accept(this);
					}

					void Visit(GlrReuseClause* node) override
					{
						node->syntax->Accept(this);
					}

					void Visit(GlrLeftRecursionPlaceholderClause* node) override
					{
					}

					void Visit(GlrLeftRecursionInjectClause* node) override
					{
					}

					void Visit(GlrPrefixMergeClause* node) override
					{
					}
				};

				void RenamePrefix(RewritingContext& rContext, const SyntaxSymbolManager& syntaxManager)
				{
					for (auto [ruleSymbol, originRule] : From(rContext.originRules).Concat(rContext.skippedRules))
					{
						RenamePrefixVisitor visitor(rContext, ruleSymbol, syntaxManager);
						for (auto clause : originRule->clauses)
						{
							// !(a; b) should be moved from rule X_LRI_Original to left_recursion_inject clauses in rule X
							if (auto reuseClause = clause.Cast<GlrReuseClause>())
							{
								if (auto pushSyntax = reuseClause->syntax.Cast<GlrPushConditionSyntax>())
								{
									reuseClause->syntax = pushSyntax->syntax;
								}
							}
							visitor.FixClause(clause);
						}
					}
				}
			}

/***********************************************************************
RewriteSyntax
***********************************************************************/

			Ptr<GlrSyntaxFile> RewriteSyntax_PrefixMerge(VisitorContext& context, SyntaxSymbolManager& syntaxManager, Ptr<GlrSyntaxFile> syntaxFile)
			{
				using namespace rewritesyntax_prefixmerge;

				// merge files to single syntax file
				auto rewritten = Ptr(new GlrSyntaxFile);
				CopyFrom(rewritten->rules, syntaxFile->rules);

				// find clauses that need to be converted to prefix_merge and fix VisitorContext
				FillMissingPrefixMergeClauses(context, syntaxManager, rewritten);

				// find rules that need to be rewritten using left_recursion_inject
				RewritingContext rewritingContext;
				CollectRewritingTargets(context, rewritingContext, rewritten);

				// create rewritten rules, rename origin rules
				CreateRewrittenRules(context, rewritingContext, rewritten);

				// fix rule types (fix syntaxManager.rules, clear RuleSymbol fields)
				FixRuleTypes(context, rewritingContext, syntaxManager);

				// create clauses in rewritten X_Y_LRI_Prefix rules
				RewriteExtractedPrefixRules(context, rewritingContext, syntaxManager);

				// create left_recursion_inject clauses in rewritten rules
				RewriteRules(context, rewritingContext, syntaxManager);

				// convert prefix_merge to left_recursion_placeholder and reuse clauses (fix syntaxManager.lrpFlags)
				FixPrefixMergeClauses(context, rewritingContext, syntaxManager);

				// rename rule references in origin rules
				RenamePrefix(rewritingContext, syntaxManager);

				return rewritten;
			}
		}
	}
}
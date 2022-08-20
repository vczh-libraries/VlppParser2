#include "Compiler.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace compile_syntax;

			struct RewritingPrefixConflict
			{
				// all clauses are simple use clauses, RuleSymbol* in values are the rule referenced by keys
				SortedList<GlrClause*>									unaffectedClauses;		// clauses that are not affected by prefix extraction
				SortedList<GlrClause*>									prefixClauses;			// simple use clauses that are prefix themselves
				Group<GlrClause*, GlrClause*>							conflictedClauses;		// c1 -> c2 if c1's prefix is prefix clause c2
			};

			struct RewritingContext
			{
				List<RuleSymbol*>										pmRules;				// all rules that need to be rewritten
				Dictionary<RuleSymbol*, GlrRule*>						originRules;			// rewritten RuleSymbol -> GlrRule ends with _LRI_Original, which is the same GlrRule object before rewritten
				Dictionary<RuleSymbol*, GlrRule*>						lriRules;				// rewritten RuleSymbol -> GlrRule containing left_recursion_inject clauses
				Dictionary<RuleSymbol*, GlrRule*>						fixedAstRules;			// RuleSymbol -> GlrRule relationship after rewritten

				Group<RuleSymbol*, RuleClausePair>						extractPrefixClauses;	// RuleSymbol -> {rule to be extracted, clause begins with rule}
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
					conflict = MakePtr<RewritingPrefixConflict>();
					rContext.extractedConflicts.Add(ruleSymbol, conflict);
				}
				return conflict;
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
									// ignore if Y ::= X directly or indirectly
									if (startRule == simpleUseRule) continue;
									vint indexExtract = vContext.indirectStartPathToLastRules.Keys().IndexOf({ startRule,simpleUseRule });
									if (indexExtract == -1) continue;
									for (auto [extractRule, extractClause] : vContext.indirectStartPathToLastRules.GetByIndex(indexExtract))
									{
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

					auto lri = MakePtr<GlrRule>();
					rewritten->rules.Add(lri);
					rContext.lriRules.Add(ruleSymbol, lri.Obj());

					lri->name.value = originRule->name.value;
					originRule->name.value += L"_LRI_Original";
				}

				for (auto [ruleSymbol, index] : indexed(rContext.extractPrefixClauses.Keys()))
				{
					auto originRule = vContext.astRules[ruleSymbol];
					auto&& prefixClauses = rContext.extractPrefixClauses.GetByIndex(index);
					for (auto [prefixRuleSymbol, prefixClause] : From(prefixClauses)
						.OrderBy([](auto p1, auto p2) {return WString::Compare(p1.key->Name(), p2.key->Name()); }))
					{
						auto ep = MakePtr<GlrRule>();
						rewritten->rules.Insert(rewritten->rules.IndexOf(originRule), ep);
						rContext.extractedPrefixRules.Add({ ruleSymbol,prefixRuleSymbol }, ep.Obj());

						ep->name.value = ruleSymbol->Name() + L"_" + prefixRuleSymbol->Name() + L"_LRI_Prefix";
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
					auto originSymbol = syntaxManager.CreateRule(originRule->name.value, originRule->codeRange);

					originSymbol->ruleType = ruleSymbol->ruleType;
					rContext.fixedAstRules.Set(originSymbol, originRule);
					rContext.fixedAstRules.Set(ruleSymbol, lriRule);
				}

				for (auto [pair, epRule] : rContext.extractedPrefixRules)
				{
					auto originRule = rContext.originRules[pair.key];
					auto epRuleSymbol = syntaxManager.CreateRule(epRule->name.value, originRule->codeRange);
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
					ruleSymbol->isPartial = false;
					ruleSymbol->assignedNonArrayField = false;
					ruleSymbol->ruleType = nullptr;
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
						auto lrpClause = MakePtr<GlrLeftRecursionPlaceholderClause>();
						epRule->clauses.Add(lrpClause);

						auto lrp = MakePtr<GlrLeftRecursionPlaceholder>();
						lrpClause->flags.Add(lrp);
						lrp->flag.value = L"LRIP_" + pair.key->Name() + L"_" + pair.value->Name();
					}
					{
						auto reuseClause = MakePtr<GlrReuseClause>();
						epRule->clauses.Add(reuseClause);

						auto useSyntax = MakePtr<GlrUseSyntax>();
						reuseClause->syntax = useSyntax;
						useSyntax->name.value = rContext.originRules[pair.value]->name.value;
					}
				}
			}

/***********************************************************************
RewriteRules (Common)
***********************************************************************/

			Ptr<RewritingPrefixConflict> RewriteRules_CollectUnaffectedIndirectPmClauses(
				const VisitorContext& vContext,
				const RewritingContext& rContext,
				RuleSymbol* initiatedRuleSymbol,
				RuleSymbol* ruleSymbol,
				SortedList<RuleSymbol*>& visited,
				Group<WString, Pair<RuleSymbol*, GlrPrefixMergeClause*>>& pmClauses
			)
			{
				auto conflict = initiatedRuleSymbol == ruleSymbol ? GetConflict(rContext, ruleSymbol) : nullptr;
				if (!visited.Contains(ruleSymbol))
				{
					visited.Add(ruleSymbol);

					if (conflict)
					{
						for (auto pair : vContext.directSimpleUseRules[ruleSymbol])
						{
							if (conflict->unaffectedClauses.Contains(pair.value))
							{
								RewriteRules_CollectUnaffectedIndirectPmClauses(vContext, rContext, initiatedRuleSymbol, pair.key, visited, pmClauses);
							}
							else
							{
								vint indexConflicted = conflict->conflictedClauses.Keys().IndexOf(pair.value);
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
									if (!pmClauses.Contains(pmClause->rule->literal.value, { pair.key, pmClause }))
									{
										pmClauses.Add(pmClause->rule->literal.value, { pair.key,pmClause });
									}
								}
							}
						}
					}
					else
					{
						for (auto pmClause : vContext.indirectPmClauses[ruleSymbol])
						{
							if (!pmClauses.Contains(pmClause->rule->literal.value, { ruleSymbol, pmClause }))
							{
								pmClauses.Add(pmClause->rule->literal.value, { ruleSymbol,pmClause });
							}
						}
					}
				}
				return conflict;
			}

			void RewriteRules_CollectFlags(
				const VisitorContext& vContext,
				RuleSymbol* ruleSymbol,
				bool isLeftRecursive,
				const List<Pair<RuleSymbol*, GlrPrefixMergeClause*>>& pmClauses,
				Dictionary<WString, Pair<RuleSymbol*, RuleSymbol*>>& flags,
				bool& omittedSelf,
				bool& generateOptionalLri
			)
			{
				for (auto [injectIntoRule, pmClause] : pmClauses)
				{
					auto pmRule = vContext.clauseToRules[pmClause];
					if (ruleSymbol == pmRule)
					{
						if (isLeftRecursive)
						{
							generateOptionalLri = true;
						}
						else
						{
							omittedSelf = true;
							continue;
						}
					}
					else if (vContext.indirectSimpleUsePathToLastRules.Keys().Contains({ ruleSymbol,pmRule }))
					{
						generateOptionalLri = true;
					}

					flags.Add(L"LRI_" + pmRule->Name(), { pmRule,injectIntoRule });
				}

				if (omittedSelf)
				{
					if (flags.Count() > 0)
					{
						generateOptionalLri = true;
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
					currentRule = lastRules[0].key;
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
				auto lriClause = MakePtr<GlrLeftRecursionInjectClause>();

				auto lriStartRule = MakePtr<GlrRefSyntax>();
				lriClause->rule = lriStartRule;
				lriStartRule->refType = GlrRefType::Id;
				lriStartRule->literal.value = ruleName;

				return lriClause;
			}

			Ptr<GlrLeftRecursionInjectContinuation> CreateLriContinuation(
				const VisitorContext& vContext,
				const RewritingContext& rContext,
				RuleSymbol* ruleSymbol,
				RuleSymbol* pmRule,
				RuleSymbol* injectIntoRule,
				const WString& flag,
				Dictionary<Pair<RuleSymbol*, RuleSymbol*>, vint>& pathCounter,
				bool generateOptionalLri
			)
			{
				auto lriCont = MakePtr<GlrLeftRecursionInjectContinuation>();

				if (RewriteRules_HasMultiplePaths(vContext, ruleSymbol, pmRule, pathCounter))
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

				auto lriContFlag = MakePtr<GlrLeftRecursionPlaceholder>();
				lriCont->flag = lriContFlag;
				lriContFlag->flag.value = flag;

				auto lriContTarget = CreateLriClause(rContext.originRules[injectIntoRule]->name.value);
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
				bool isLeftRecursive,
				Dictionary<Pair<RuleSymbol*, RuleSymbol*>, vint>& pathCounter,
				Group<WString, Pair<RuleSymbol*, GlrPrefixMergeClause*>>& pmClauses,
				SortedList<WString>& knownOptionalStartRules
			)
			{
				for (auto [pmName, pmIndex] : indexed(pmClauses.Keys()))
				{
					//   if originRule is not left recursive
					//     do not generate lriClause for the flag created for originRule, because there is no continuation
					//     if a pmName does generate some lriClause
					//       it becomes GLRICT::Optional
					//     otherwise
					//       it becomse GLRICT::Required
					//       generate useSyntax instead of lriClause

					Dictionary<WString, Pair<RuleSymbol*, RuleSymbol*>> flags;
					bool omittedSelf = false;
					bool generateOptionalLri = false;
					RewriteRules_CollectFlags(
						vContext,
						ruleSymbol,
						isLeftRecursive,
						pmClauses.GetByIndex(pmIndex),
						flags,
						omittedSelf,
						generateOptionalLri
						);

					if (omittedSelf && flags.Count() == 0)
					{
						auto reuseClause = MakePtr<GlrReuseClause>();
						lriRule->clauses.Add(reuseClause);

						auto useSyntax = MakePtr<GlrUseSyntax>();
						reuseClause->syntax = useSyntax;
						useSyntax->name.value = pmName;
					}

					for (auto [flag, pmRulePair] : flags)
					{
						auto [pmRule, injectIntoRule] = pmRulePair;
						auto lriClause = CreateLriClause(pmName);
						lriRule->clauses.Add(lriClause);

						auto lriCont = CreateLriContinuation(
							vContext,
							rContext,
							ruleSymbol,
							pmRule,
							injectIntoRule,
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
				auto isLeftRecursive = vContext.leftRecursiveClauses.Contains(prefixRuleSymbol);
				Group<WString, Pair<RuleSymbol*, GlrPrefixMergeClause*>> pmClauses;
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

				for (auto [pmName, pmIndex] : indexed(pmClauses.Keys()))
				{
					//   if originRule is not left recursive
					//     left_recursion_inject directly into conflictedRuleSymbol
					//     if a pmName does generate some lriClause
					//       it becomes GLRICT::Optional
					//     otherwise
					//       it becomse GLRICT::Required
					//       generate useSyntax instead of lriClause
				
					Dictionary<WString, Pair<RuleSymbol*, RuleSymbol*>> flags;
					bool omittedSelf = false;
					bool generateOptionalLri = false;
					RewriteRules_CollectFlags(
						vContext,
						prefixRuleSymbol,
						isLeftRecursive,
						pmClauses.GetByIndex(pmIndex),
						flags,
						omittedSelf,
						generateOptionalLri
						);

					if (knownOptionalStartRules.Contains(pmName))
					{
						generateOptionalLri = false;
					}

					if (omittedSelf && flags.Count() == 0)
					{
						// TODO: add test case
						for (auto lripFlag : lripFlags)
						{
							auto lriClause = CreateLriClause(pmName);
							lriRule->clauses.Add(lriClause);

							auto lriCont = CreateLriContinuation(
								vContext,
								rContext,
								conflictedRuleSymbol,
								prefixRuleSymbol,
								conflictedRuleSymbol,
								lripFlag,
								pathCounter,
								false
								);
							lriClause->continuation = lriCont;
						}
					}

					for (auto [flag, pmRulePair] : flags)
					{
						auto [pmRule, injectIntoRule] = pmRulePair;
						{
							auto lriClause = CreateLriClause(pmName);
							lriRule->clauses.Add(lriClause);
					
							auto lriCont = CreateLriContinuation(
								vContext,
								rContext,
								prefixRuleSymbol,
								pmRule,
								prefixRuleSymbol,
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
									prefixRuleSymbol,
									conflictedRuleSymbol,
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
							lripFlags.Add(L"LRIP_" + extracted.key->Name() + L"_" + prefixRuleSymbol->Name());
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

			void RewriteRules(const VisitorContext& vContext, const RewritingContext& rContext, SyntaxSymbolManager& syntaxManager)
			{
				Dictionary<Pair<RuleSymbol*, RuleSymbol*>, vint> pathCounter;
				for (auto [ruleSymbol, originRule] : rContext.originRules)
				{
					auto lriRule = rContext.lriRules[ruleSymbol];
					auto isLeftRecursive = vContext.leftRecursiveClauses.Contains(ruleSymbol);
					Ptr<RewritingPrefixConflict> conflict;
					Group<WString, Pair<RuleSymbol*, GlrPrefixMergeClause*>> pmClauses;
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
						isLeftRecursive,
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

					auto lrpClause = MakePtr<GlrLeftRecursionPlaceholderClause>();
					originRule->clauses.Insert(0, lrpClause);

					auto lrp = MakePtr<GlrLeftRecursionPlaceholder>();
					lrp->flag.value = L"LRI_" + ruleSymbol->Name();
					lrpClause->flags.Add(lrp);
					syntaxManager.lrpFlags.Add(lrp->flag.value);

					for (vint i = 0; i < originRule->clauses.Count(); i++)
					{
						if (auto pmClause = originRule->clauses[i].Cast<GlrPrefixMergeClause>())
						{
							auto reuseClause = MakePtr<GlrReuseClause>();
							originRule->clauses[i] = reuseClause;

							auto useSyntax = MakePtr<GlrUseSyntax>();
							useSyntax->name.value = pmClause->rule->literal.value;
							reuseClause->syntax = useSyntax;
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

				void NotBeginWithARule(ParsingAstBase* node)
				{
					CHECK_FAIL(L"vl::glr::parsergen::RenamePrefix(RewritingContext, Ptr<GlrSyntaxFile>)#Internal error: should have been cought by RuleMixedPrefixMergeWithClauseNotSyntacticallyBeginWithARule.");
				}

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
					if (node->refType != GlrRefType::Id)
					{
						NotBeginWithARule(node);
					}
					else
					{
						vint index = syntaxManager.Rules().Keys().IndexOf(node->literal.value);
						if (index == -1)
						{
							NotBeginWithARule(node);
						}
						else
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
					NotBeginWithARule(node);
				}

				void Visit(GlrOptionalSyntax* node) override
				{
					NotBeginWithARule(node);
				}

				void Visit(GlrSequenceSyntax* node) override
				{
					node->first->Accept(this);
				}

				void Visit(GlrAlternativeSyntax* node) override
				{
					NotBeginWithARule(node);
				}

				void Visit(GlrPushConditionSyntax* node) override
				{
					NotBeginWithARule(node);
				}

				void Visit(GlrTestConditionSyntax* node) override
				{
					NotBeginWithARule(node);
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
					NotBeginWithARule(node);
				}

				void Visit(GlrPrefixMergeClause* node) override
				{
					NotBeginWithARule(node);
				}
			};

			void RenamePrefix(RewritingContext& rContext, const SyntaxSymbolManager& syntaxManager)
			{
				for (auto [ruleSymbol, originRule] : rContext.originRules)
				{
					RenamePrefixVisitor visitor(rContext, ruleSymbol, syntaxManager);
					for (auto clause : originRule->clauses)
					{
						visitor.FixClause(clause);
					}
				}
			}

/***********************************************************************
RewriteSyntax
***********************************************************************/

			Ptr<GlrSyntaxFile> RewriteSyntax(const VisitorContext& context, SyntaxSymbolManager& syntaxManager, collections::List<Ptr<GlrSyntaxFile>>& files)
			{
				// merge files to single syntax file

				auto rewritten = MakePtr<GlrSyntaxFile>();
				for (auto file : files)
				{
					CopyFrom(rewritten->switches, file->switches, true);
					CopyFrom(rewritten->rules, file->rules, true);
				}

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
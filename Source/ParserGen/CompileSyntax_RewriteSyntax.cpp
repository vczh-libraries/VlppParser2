#include "Compiler.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace compile_syntax;

			struct RewritingContext
			{
				List<RuleSymbol*>						pmRules;
				Dictionary<RuleSymbol*, GlrRule*>		originRules;
				Dictionary<RuleSymbol*, GlrRule*>		lriRules;
				Dictionary<RuleSymbol*, GlrRule*>		fixedAstRules;
			};

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
			}

/***********************************************************************
FixRuleTypes
***********************************************************************/

			void FixRuleTypes(const VisitorContext& vContext, RewritingContext& rContext, SyntaxSymbolManager& syntaxManager, Ptr<GlrSyntaxFile> rewritten)
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
RewriteRules
***********************************************************************/

			void RewriteRules(const VisitorContext& vContext, RewritingContext& rContext, SyntaxSymbolManager& syntaxManager, Ptr<GlrSyntaxFile> rewritten)
			{
				for (auto [ruleSymbol, originRule] : rContext.originRules)
				{
					auto lriRule = rContext.lriRules[ruleSymbol];
					auto isLeftRecursive = vContext.leftRecursiveClauses.Contains(ruleSymbol);

					Group<WString, GlrPrefixMergeClause*> pmClauses;
					for (auto pmClause : vContext.indirectPmClauses[ruleSymbol])
					{
						pmClauses.Add(pmClause->rule->literal.value, pmClause);
					}

					for (auto [pmName, pmIndex] : indexed(pmClauses.Keys()))
					{
						//   if originRule is not left recursive
						//     do not generate lriClause for the flag created for originRule, because there is no continuation
						//     if a pmName does generate some lriClause
						//       it becomes GLRICT::Optional
						//     otherwise
						//       it becomse GLRICT::Single
						//       generate useSyntax instead of lriClause

						SortedList<WString> flags;
						bool omittedSelf = false;
						bool generateOptionalLri = false;
						for (auto pmClause : pmClauses.GetByIndex(pmIndex))
						{
							auto pmRule = vContext.pmClauseToRules[pmClause];
							if (ruleSymbol == pmRule && !isLeftRecursive)
							{
								omittedSelf = true;
							}
							else
							{
								flags.Add(L"LRI_" + pmRule->Name());
							}
						}

						// TODO: determine if it needs GLRICT::Optional (generateOptionalLri)

						if (omittedSelf)
						{
							if (flags.Count() > 0)
							{
								generateOptionalLri = true;
							}
							else
							{
								auto reuseClause = MakePtr<GlrReuseClause>();
								lriRule->clauses.Add(reuseClause);

								auto useSyntax = MakePtr<GlrUseSyntax>();
								reuseClause->syntax = useSyntax;
								useSyntax->name.value = pmName;
							}
						}

						for (auto flag : flags)
						{
							auto lriClause = MakePtr<GlrLeftRecursionInjectClause>();
							lriRule->clauses.Add(lriClause);

							auto lriStartRule = MakePtr<GlrRefSyntax>();
							lriClause->rule = lriStartRule;
							lriStartRule->refType = GlrRefType::Id;
							lriStartRule->literal.value = pmName;

							auto lriCont = MakePtr<GlrLeftRecursionInjectContinuation>();
							lriClause->continuation = lriCont;
							// TODO: determine if it needs GLRC::Multiple
							lriCont->configuration = GlrLeftRecursionConfiguration::Single;
							if (generateOptionalLri)
							{
								lriCont->type = GlrLeftRecursionInjectContinuationType::Optional;
								generateOptionalLri = false;
							}
							else
							{
								lriCont->type = GlrLeftRecursionInjectContinuationType::Required;
							}

							auto lriContFlag = MakePtr<GlrLeftRecursionPlaceholder>();
							lriCont->flag = lriContFlag;
							lriContFlag->flag.value = flag;

							auto lriContTarget = MakePtr<GlrLeftRecursionInjectClause>();
							lriCont->injectionTargets.Add(lriContTarget);

							auto lriTargetRule = MakePtr<GlrRefSyntax>();
							lriContTarget->rule = lriTargetRule;
							lriTargetRule->refType = GlrRefType::Id;
							lriTargetRule->literal.value = originRule->name.value;
						}
					}
				}
			}

/***********************************************************************
FixPrefixMergeClauses
***********************************************************************/

			void FixPrefixMergeClauses(const VisitorContext& vContext, RewritingContext& rContext, SyntaxSymbolManager& syntaxManager, Ptr<GlrSyntaxFile> rewritten)
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
				RewritingContext&			rContext;
				const SyntaxSymbolManager&	syntaxManager;

			public:
				RenamePrefixVisitor(
					RewritingContext& _rContext,
					const SyntaxSymbolManager& _syntaxManager
				)
					: rContext(_rContext)
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
					auto ruleSymbol = syntaxManager.Rules()[ruleName.value];
					vint index = rContext.originRules.Keys().IndexOf(ruleSymbol);
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

			void RenamePrefix(RewritingContext& rContext, const SyntaxSymbolManager& syntaxManager, Ptr<GlrSyntaxFile> rewritten)
			{
				RenamePrefixVisitor visitor(rContext, syntaxManager);
				for (auto originRule : rContext.originRules.Values())
				{
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
				FixRuleTypes(context, rewritingContext, syntaxManager, rewritten);

				// create left_recursion_inject clauses in rewritten rules
				RewriteRules(context, rewritingContext, syntaxManager, rewritten);

				// convert prefix_merge to left_recursion_placeholder and reuse clauses (fix syntaxManager.lrpFlags)
				FixPrefixMergeClauses(context, rewritingContext, syntaxManager, rewritten);

				// rename rule references in origin rules
				RenamePrefix(rewritingContext, syntaxManager, rewritten);

				// TODO: delete rContext.fixedAstRules if unused

				return rewritten;
			}
		}
	}
}
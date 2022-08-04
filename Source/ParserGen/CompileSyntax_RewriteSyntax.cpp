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
Calculating
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
Rules
***********************************************************************/

			void CreateRewrittenRules(const VisitorContext& vContext, RewritingContext& rContext, Ptr<GlrSyntaxFile> rewritten)
			{
				for (auto ruleSymbol : rContext.pmRules)
				{
					rContext.originRules.Add(ruleSymbol, vContext.astRules[ruleSymbol]);
				}

				for (auto [ruleSymbol, origin] : rContext.originRules)
				{
					auto lri = MakePtr<GlrRule>();
					rewritten->rules.Add(lri);
					rContext.lriRules.Add(ruleSymbol, lri.Obj());

					lri->name.value = origin->name.value;
					origin->name.value += L"_LRI_Origin";
				}
			}

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
Clauses
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

				// convert prefix_merge to left_recursion_placeholder and reuse clauses (fix syntaxManager.lrpFlags)
				FixPrefixMergeClauses(context, rewritingContext, syntaxManager, rewritten);

				// rename rule references in origin rules

				// TODO: delete rContext.fixedAstRules if unused

				return rewritten;
			}
		}
	}
}
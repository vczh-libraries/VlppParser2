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
				List<RuleSymbol*>					pmRules;
				Dictionary<WString, GlrRule*>		originRules;
				Dictionary<WString, GlrRule*>		rewrittenRules;
			};

/***********************************************************************
Calculating
***********************************************************************/

			void CollectRewritingTargets(VisitorContext& vContext, RewritingContext& rContext, Ptr<GlrSyntaxFile> rewritten)
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

/***********************************************************************
Clauses
***********************************************************************/

/***********************************************************************
RewriteSyntax
***********************************************************************/

			Ptr<GlrSyntaxFile> RewriteSyntax(VisitorContext& context, SyntaxSymbolManager& syntaxManager, collections::List<Ptr<GlrSyntaxFile>>& files)
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

				// create rewritten rules

				// fix rule types

				return rewritten;
			}
		}
	}
}
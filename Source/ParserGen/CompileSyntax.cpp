#include "Compiler.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace compile_syntax;

			extern void ResolveName(VisitorContext& context, List<Ptr<GlrSyntaxFile>>& files);
			extern void ValidatePartialRules(VisitorContext& context, List<Ptr<GlrSyntaxFile>>& files);
			extern void CalculateRuleAndClauseTypes(VisitorContext& context);
			extern void ValidateTypes(VisitorContext& context, List<Ptr<GlrSyntaxFile>>& files);
			extern void ValidateStructure(VisitorContext& context, List<Ptr<GlrSyntaxFile>>& files);
			extern void CompileSyntax(VisitorContext& context, List<Ptr<GlrSyntaxFile>>& files);

/***********************************************************************
CompileSyntax
***********************************************************************/

			void CompileSyntax(AstSymbolManager& astManager, LexerSymbolManager& lexerManager, SyntaxSymbolManager& syntaxManager, Ptr<CppParserGenOutput> output, collections::List<Ptr<GlrSyntaxFile>>& files)
			{
				VisitorContext context(astManager, lexerManager, syntaxManager, output);

				for (auto file : files)
				{
					for (auto rule : file->rules)
					{
						if (context.lexerManager.Tokens().Keys().Contains(rule->name.value))
						{
							context.syntaxManager.Global().AddError(
								ParserErrorType::RuleNameConflictedWithToken,
								rule->name.value
							);
						}
						else
						{
							auto ruleSymbol = context.syntaxManager.CreateRule(rule->name.value);
							context.astRules.Add(ruleSymbol, rule.Obj());
						}
					}
				}

				ResolveName(context, files);
				if (syntaxManager.Global().Errors().Count() > 0) return;

				ValidatePartialRules(context, files);
				if (syntaxManager.Global().Errors().Count() > 0) return;

				CalculateRuleAndClauseTypes(context);
				if (syntaxManager.Global().Errors().Count() > 0) return;

				ValidateTypes(context, files);
				if (syntaxManager.Global().Errors().Count() > 0) return;

				ValidateStructure(context, files);
				if (syntaxManager.Global().Errors().Count() > 0) return;

				CompileSyntax(context, files);
			}
		}
	}
}
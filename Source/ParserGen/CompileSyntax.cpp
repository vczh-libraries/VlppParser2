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

			bool NeedRewritten(collections::List<Ptr<GlrSyntaxFile>>& files)
			{
				return false;
			}

			bool VerifySyntax(VisitorContext& context, collections::List<Ptr<GlrSyntaxFile>>& files)
			{
				for (auto file : files)
				{
					for (auto rule : file->rules)
					{
						if (context.lexerManager.Tokens().Keys().Contains(rule->name.value))
						{
							context.syntaxManager.AddError(
								ParserErrorType::RuleNameConflictedWithToken,
								rule->codeRange,
								rule->name.value
							);
						}
						else
						{
							auto ruleSymbol = context.syntaxManager.CreateRule(rule->name.value, rule->codeRange);
							context.astRules.Add(ruleSymbol, rule.Obj());
						}

						for (auto clause : rule->clauses)
						{
							if (auto lrpClause = clause.Cast<GlrLeftRecursionPlaceholderClause>())
							{
								for (auto flag : lrpClause->flags)
								{
									if (!context.syntaxManager.lrpFlags.Contains(flag->flag.value))
									{
										context.syntaxManager.lrpFlags.Add(flag->flag.value);
									}
								}
							}
						}
					}
				}
				if (context.syntaxManager.Global().Errors().Count() > 0) return false;

				ResolveName(context, files);
				if (context.syntaxManager.Global().Errors().Count() > 0) return false;

				ValidatePartialRules(context, files);
				if (context.syntaxManager.Global().Errors().Count() > 0) return false;

				CalculateRuleAndClauseTypes(context);
				if (context.syntaxManager.Global().Errors().Count() > 0) return false;

				ValidateTypes(context, files);
				if (context.syntaxManager.Global().Errors().Count() > 0) return false;

				ValidateStructure(context, files);
				if (context.syntaxManager.Global().Errors().Count() > 0) return false;

				return true;
			}

			Ptr<GlrSyntaxFile> CompileSyntax(AstSymbolManager& astManager, LexerSymbolManager& lexerManager, SyntaxSymbolManager& syntaxManager, Ptr<CppParserGenOutput> output, collections::List<Ptr<GlrSyntaxFile>>& files)
			{
				if (NeedRewritten(files))
				{
					CHECK_FAIL(L"Not Implemented!");
				}
				else
				{
					VisitorContext context(astManager, lexerManager, syntaxManager, output);
					if (VerifySyntax(context, files))
					{
						CompileSyntax(context, files);
					}
					return nullptr;
				}
			}
		}
	}
}
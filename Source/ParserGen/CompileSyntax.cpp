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
			extern void CalculateFirstSet(VisitorContext& context, List<Ptr<GlrSyntaxFile>>& files);
			extern void ValidateTypes(VisitorContext& context, List<Ptr<GlrSyntaxFile>>& files);
			extern void ValidateStructure(VisitorContext& context, List<Ptr<GlrSyntaxFile>>& files);
			extern void CompileSyntax(VisitorContext& context, Ptr<CppParserGenOutput> output, List<Ptr<GlrSyntaxFile>>& files);

/***********************************************************************
CompileSyntax
***********************************************************************/

			bool NeedRewritten(collections::List<Ptr<GlrSyntaxFile>>& files)
			{
				return !From(files)
					.SelectMany([](auto file) { return From(file->rules); })
					.SelectMany([](auto rule) { return From(rule->clauses); })
					.FindType<GlrPrefixMergeClause>()
					.IsEmpty();
			}

			void CreateSyntaxSymbols(LexerSymbolManager& lexerManager, SyntaxSymbolManager& syntaxManager, collections::List<Ptr<GlrSyntaxFile>>& files)
			{
				for (auto file : files)
				{
					for (auto rule : file->rules)
					{
						if (lexerManager.Tokens().Keys().Contains(rule->name.value))
						{
							syntaxManager.AddError(
								ParserErrorType::RuleNameConflictedWithToken,
								rule->codeRange,
								rule->name.value
							);
						}
						else
						{
							auto ruleSymbol = syntaxManager.CreateRule(rule->name.value, rule->codeRange);
						}

						for (auto clause : rule->clauses)
						{
							if (auto lrpClause = clause.Cast<GlrLeftRecursionPlaceholderClause>())
							{
								for (auto flag : lrpClause->flags)
								{
									if (!syntaxManager.lrpFlags.Contains(flag->flag.value))
									{
										syntaxManager.lrpFlags.Add(flag->flag.value);
									}
								}
							}
						}
					}
				}

				for (auto file : files)
				{
					for (auto switchItem : file->switches)
					{
						syntaxManager.CreateSwitch(
							switchItem->name.value,
							(switchItem->value == GlrSwitchValue::True),
							switchItem->name.codeRange
							);
					}
				}
			}

			bool VerifySyntax(VisitorContext& context, collections::List<Ptr<GlrSyntaxFile>>& files)
			{
				ResolveName(context, files);
				if (context.syntaxManager.Global().Errors().Count() > 0) return false;

				ValidatePartialRules(context, files);
				if (context.syntaxManager.Global().Errors().Count() > 0) return false;

				CalculateRuleAndClauseTypes(context);
				if (context.syntaxManager.Global().Errors().Count() > 0) return false;

				CalculateFirstSet(context, files);
				if (context.syntaxManager.Global().Errors().Count() > 0) return false;

				ValidateTypes(context, files);
				if (context.syntaxManager.Global().Errors().Count() > 0) return false;

				ValidateStructure(context, files);
				if (context.syntaxManager.Global().Errors().Count() > 0) return false;

				return true;
			}

			Ptr<GlrSyntaxFile> CompileSyntax(AstSymbolManager& astManager, LexerSymbolManager& lexerManager, SyntaxSymbolManager& syntaxManager, Ptr<CppParserGenOutput> output, collections::List<Ptr<GlrSyntaxFile>>& files)
			{
				CreateSyntaxSymbols(lexerManager, syntaxManager, files);
				if (syntaxManager.Global().Errors().Count() > 0) return nullptr;

				if (NeedRewritten(files))
				{
					{
						VisitorContext context(astManager, lexerManager, syntaxManager);
						if (!VerifySyntax(context, files)) return nullptr;
					}
					CHECK_FAIL(L"Not Implemented!");
				}
				else
				{
					VisitorContext context(astManager, lexerManager, syntaxManager);
					if (!VerifySyntax(context, files)) return nullptr;
					CompileSyntax(context, output, files);
					return nullptr;
				}
			}
		}
	}
}
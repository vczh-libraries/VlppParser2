#include "Compiler.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace compile_syntax;

			extern void					ResolveName(VisitorContext& context, List<Ptr<GlrSyntaxFile>>& files);
			extern void					ValidatePartialRules(VisitorContext& context, List<Ptr<GlrSyntaxFile>>& files);
			extern void					CalculateRuleAndClauseTypes(VisitorContext& context);
			extern void					CalculateFirstSet(VisitorContext& context, List<Ptr<GlrSyntaxFile>>& files);
			extern void					ValidateSwitchesAndConditions(VisitorContext& context, List<Ptr<GlrSyntaxFile>>& files);
			extern void					ValidateTypes(VisitorContext& context, List<Ptr<GlrSyntaxFile>>& files);
			extern void					ValidateStructure(VisitorContext& context, List<Ptr<GlrSyntaxFile>>& files);

			extern Ptr<GlrSyntaxFile>	RewriteSyntax_Switch(VisitorContext& context, SyntaxSymbolManager& syntaxManager, collections::List<Ptr<GlrSyntaxFile>>& files);
			extern Ptr<GlrSyntaxFile>	RewriteSyntax_PrefixMerge(VisitorContext& context, SyntaxSymbolManager& syntaxManager, collections::List<Ptr<GlrSyntaxFile>>& files);
			extern void					CompileSyntax(VisitorContext& context, Ptr<CppParserGenOutput> output, List<Ptr<GlrSyntaxFile>>& files);

/***********************************************************************
CompileSyntax
***********************************************************************/

			bool NeedRewritten_Switch(collections::List<Ptr<GlrSyntaxFile>>& files)
			{
				return !From(files)
					.Where([](auto file) { return file->switches.Count() > 0; })
					.IsEmpty();
			}

			bool NeedRewritten_PrefixMerge(collections::List<Ptr<GlrSyntaxFile>>& files)
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

			bool VerifySyntax_UntilSwitch(VisitorContext& context, collections::List<Ptr<GlrSyntaxFile>>& files)
			{
				ResolveName(context, files);
				if (context.syntaxManager.Global().Errors().Count() > 0) return false;

				ValidateSwitchesAndConditions(context, files);
				if (context.syntaxManager.Global().Errors().Count() > 0) return false;

				ValidatePartialRules(context, files);
				if (context.syntaxManager.Global().Errors().Count() > 0) return false;

				CalculateRuleAndClauseTypes(context);
				if (context.syntaxManager.Global().Errors().Count() > 0) return false;

				return true;
			}

			bool VerifySyntax_UntilPrefixMerge(VisitorContext& context, collections::List<Ptr<GlrSyntaxFile>>& files)
			{
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
				Ptr<GlrSyntaxFile> rewritten;
				CreateSyntaxSymbols(lexerManager, syntaxManager, files);
				if (syntaxManager.Global().Errors().Count() > 0) goto FINISHED_COMPILING;

				if (NeedRewritten_Switch(files))
				{
					VisitorContext context(astManager, lexerManager, syntaxManager);
					if (!VerifySyntax_UntilSwitch(context, files)) goto FINISHED_COMPILING;
					rewritten = RewriteSyntax_Switch(context, syntaxManager, files);
					files.Clear();
					files.Add(rewritten);
				}

				if (NeedRewritten_PrefixMerge(files))
				{
					VisitorContext context(astManager, lexerManager, syntaxManager);
					if (!VerifySyntax_UntilSwitch(context, files)) goto FINISHED_COMPILING;
					if (!VerifySyntax_UntilPrefixMerge(context, files)) goto FINISHED_COMPILING;
					rewritten = RewriteSyntax_PrefixMerge(context, syntaxManager, files);
					files.Clear();
					files.Add(rewritten);
				}

				{
					VisitorContext context(astManager, lexerManager, syntaxManager);
					if (!VerifySyntax_UntilSwitch(context, files)) goto FINISHED_COMPILING;
					if (!VerifySyntax_UntilPrefixMerge(context, files)) goto FINISHED_COMPILING;
					CompileSyntax(context, output, files);
				}
			FINISHED_COMPILING:
				return rewritten;
			}
		}
	}
}
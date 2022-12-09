#include "Compiler.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace compile_syntax;

			extern void					ResolveName(VisitorContext& context, VisitorSwitchContext& sContext, Ptr<GlrSyntaxFile> syntaxFile);
			extern void					ValidateSwitchesAndConditions(VisitorContext& context, VisitorSwitchContext& sContext, Ptr<GlrSyntaxFile> syntaxFile);
			extern void					ValidatePartialRules(VisitorContext& context, Ptr<GlrSyntaxFile> syntaxFile);
			extern void					CalculateRuleAndClauseTypes(VisitorContext& context);

			extern void					CalculateFirstSet(VisitorContext& context, Ptr<GlrSyntaxFile> syntaxFile);
			extern void					ValidateTypes(VisitorContext& context, Ptr<GlrSyntaxFile> syntaxFile);
			extern void					ValidateStructure(VisitorContext& context, Ptr<GlrSyntaxFile> syntaxFile);
			extern void					ValidatePrefixMerge(VisitorContext& context, Ptr<GlrSyntaxFile> syntaxFile);

			extern Ptr<GlrSyntaxFile>	RewriteSyntax_Switch(VisitorContext& context, VisitorSwitchContext& sContext, SyntaxSymbolManager& syntaxManager, Ptr<GlrSyntaxFile> syntaxFile);
			extern Ptr<GlrSyntaxFile>	RewriteSyntax_PrefixMerge(VisitorContext& context, SyntaxSymbolManager& syntaxManager, Ptr<GlrSyntaxFile> syntaxFile);
			extern void					CompileSyntax(VisitorContext& context, Ptr<CppParserGenOutput> output, Ptr<GlrSyntaxFile> syntaxFile);

/***********************************************************************
CompileSyntax
***********************************************************************/

			bool NeedRewritten_Switch(Ptr<GlrSyntaxFile> syntaxFile)
			{
				return syntaxFile->switches.Count() > 0;
			}

			bool NeedRewritten_PrefixMerge(Ptr<GlrSyntaxFile> syntaxFile)
			{
				return !From(syntaxFile->rules)
					.SelectMany([](auto rule) { return From(rule->clauses); })
					.FindType<GlrPrefixMergeClause>()
					.IsEmpty();
			}

			void CreateSyntaxSymbols(LexerSymbolManager& lexerManager, SyntaxSymbolManager& syntaxManager, Ptr<GlrSyntaxFile> syntaxFile)
			{
				for (auto rule : syntaxFile->rules)
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

			bool VerifySyntax_UntilSwitch(VisitorContext& context, VisitorSwitchContext& sContext, Ptr<GlrSyntaxFile> syntaxFile)
			{
				ResolveName(context, sContext, syntaxFile);
				if (context.syntaxManager.Global().Errors().Count() > 0) return false;

				ValidateSwitchesAndConditions(context, sContext, syntaxFile);
				if (context.syntaxManager.Global().Errors().Count() > 0) return false;

				ValidatePartialRules(context, syntaxFile);
				if (context.syntaxManager.Global().Errors().Count() > 0) return false;

				CalculateRuleAndClauseTypes(context);
				if (context.syntaxManager.Global().Errors().Count() > 0) return false;

				return true;
			}

			bool VerifySyntax_UntilPrefixMerge(VisitorContext& context, Ptr<GlrSyntaxFile> syntaxFile)
			{
				CalculateFirstSet(context, syntaxFile);
				if (context.syntaxManager.Global().Errors().Count() > 0) return false;

				ValidateTypes(context, syntaxFile);
				if (context.syntaxManager.Global().Errors().Count() > 0) return false;

				ValidateStructure(context, syntaxFile);
				if (context.syntaxManager.Global().Errors().Count() > 0) return false;

				ValidatePrefixMerge(context, syntaxFile);
				if (context.syntaxManager.Global().Errors().Count() > 0) return false;

				return true;
			}

			Ptr<GlrSyntaxFile> CompileSyntax(AstSymbolManager& astManager, LexerSymbolManager& lexerManager, SyntaxSymbolManager& syntaxManager, Ptr<CppParserGenOutput> output, collections::List<Ptr<GlrSyntaxFile>>& files)
			{
				// merge files to single syntax file
				auto syntaxFile = Ptr(new GlrSyntaxFile);
				for (auto file : files)
				{
					CopyFrom(syntaxFile->switches, file->switches, true);
					CopyFrom(syntaxFile->rules, file->rules, true);
				}

				auto rawSyntaxFile = syntaxFile;

				CreateSyntaxSymbols(lexerManager, syntaxManager, syntaxFile);
				if (syntaxManager.Global().Errors().Count() > 0) goto FINISHED_COMPILING;

				if (NeedRewritten_Switch(syntaxFile))
				{
					VisitorContext context(astManager, lexerManager, syntaxManager);
					VisitorSwitchContext sContext;
					if (!VerifySyntax_UntilSwitch(context, sContext, syntaxFile)) goto FINISHED_COMPILING;

					syntaxFile = RewriteSyntax_Switch(context, sContext, syntaxManager, syntaxFile);
					if (context.syntaxManager.Global().Errors().Count() > 0) goto FINISHED_COMPILING;
				}

				if (NeedRewritten_PrefixMerge(syntaxFile))
				{
					VisitorContext context(astManager, lexerManager, syntaxManager);
					VisitorSwitchContext sContext;
					if (!VerifySyntax_UntilSwitch(context, sContext, syntaxFile)) goto FINISHED_COMPILING;
					if (!VerifySyntax_UntilPrefixMerge(context, syntaxFile)) goto FINISHED_COMPILING;

					syntaxFile = RewriteSyntax_PrefixMerge(context, syntaxManager, syntaxFile);
					if (context.syntaxManager.Global().Errors().Count() > 0) goto FINISHED_COMPILING;
				}

				{
					VisitorContext context(astManager, lexerManager, syntaxManager);
					VisitorSwitchContext sContext;
					if (!VerifySyntax_UntilSwitch(context, sContext, syntaxFile)) goto FINISHED_COMPILING;
					if (!VerifySyntax_UntilPrefixMerge(context, syntaxFile)) goto FINISHED_COMPILING;
					CompileSyntax(context, output, syntaxFile);
				}
			FINISHED_COMPILING:
				return rawSyntaxFile == syntaxFile ? nullptr : syntaxFile;
			}
		}
	}
}
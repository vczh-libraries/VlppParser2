/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_PARSERGEN_COMPILER
#define VCZH_PARSER2_PARSERGEN_COMPILER

#include "../ParserGen_Generated/ParserGenTypeAst.h"
#include "../ParserGen_Generated/ParserGenRuleAst.h"
#include "../Ast/AstSymbol.h"
#include "../Lexer/LexerSymbol.h"
#include "../Syntax/SyntaxSymbol.h"
#include "../ParserGen/ParserCppGen.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			namespace compile_syntax
			{
				using GlrRuleMap = collections::Dictionary<RuleSymbol*, GlrRule*>;
				using LiteralTokenMap = collections::Dictionary<GlrRefSyntax*, vint32_t>;
				using RuleReuseDependencies = collections::Group<RuleSymbol*, RuleSymbol*>;
				using RuleKnownTypes = collections::Group<RuleSymbol*, AstClassSymbol*>;
				using ClauseReuseDependencies = collections::Group<GlrReuseClause*, RuleSymbol*>;
				using ClauseTypeMap = collections::Dictionary<GlrClause*, AstClassSymbol*>;

				using LeftRecursiveClauseSet = collections::SortedList<collections::Pair<RuleSymbol*, GlrClause*>>;
				using LeftRecursiveClauseMap = collections::Group<RuleSymbol*, GlrClause*>;
				using StartRuleSet = collections::SortedList<collections::Pair<RuleSymbol*, RuleSymbol*>>;
				using LeftRecursionPlaceholderClauseMap = collections::Group<RuleSymbol*, GlrLeftRecursionPlaceholderClause*>;
				using LeftRecursionPlaceholderReverseMap = collections::Dictionary<GlrLeftRecursionPlaceholderClause*, RuleSymbol*>;
				using LeftRecursionInjectClauseMap = collections::Group<RuleSymbol*, GlrLeftRecursionInjectClause*>;
				using LeftRecursionInjectReverseMap = collections::Dictionary<GlrLeftRecursionInjectClause*, RuleSymbol*>;
				using PrefixMergeClauseMap = collections::Group<RuleSymbol*, GlrPrefixMergeClause*>;
				using PrefixMergeClauseReverseMap = collections::Dictionary<GlrPrefixMergeClause*, RuleSymbol*>;

				struct VisitorContext
				{
					ParserSymbolManager&				global;
					AstSymbolManager&					astManager;
					LexerSymbolManager&					lexerManager;
					SyntaxSymbolManager&				syntaxManager;

					GlrRuleMap							astRules;
					LiteralTokenMap						literalTokens;
					RuleReuseDependencies				ruleReuseDependencies;
					RuleKnownTypes						ruleKnownTypes;
					ClauseReuseDependencies				clauseReuseDependencies;
					ClauseTypeMap						clauseTypes;
					Ptr<regex::RegexLexer>				cachedLexer;

					LeftRecursiveClauseSet				leftRecursiveClauseParis;
					LeftRecursiveClauseMap				leftRecursiveClauses;
					LeftRecursionInjectClauseMap		directLriClauses, indirectLriClauses;
					LeftRecursionPlaceholderClauseMap	directLrpClauses, indirectLrpClauses;
					PrefixMergeClauseMap				directPmClauses, indirectPmClauses;
					RuleReuseDependencies				directStartRules, indirectStartRules;
					StartRuleSet						indirectStartRulePairs;
					LeftRecursionInjectReverseMap		lriClauseToRules;
					LeftRecursionPlaceholderReverseMap	lrpClauseToRules;
					PrefixMergeClauseReverseMap			pmClauseToRules;

					VisitorContext(
						AstSymbolManager& _astManager,
						LexerSymbolManager& _lexerManager,
						SyntaxSymbolManager& _syntaxManager
					)
						: global(_syntaxManager.Global())
						, astManager(_astManager)
						, lexerManager(_lexerManager)
						, syntaxManager(_syntaxManager)
					{
					}

					regex::RegexLexer& GetCachedLexer()
					{
						if (!cachedLexer)
						{
							auto tokens = From(lexerManager.TokenOrder())
								.Select([&](const WString& name) { return lexerManager.Tokens()[name]->regex; });
							cachedLexer = new regex::RegexLexer(tokens);
						}
						return *cachedLexer.Obj();
					}
				};
			}

/***********************************************************************
Compiler
***********************************************************************/

			extern WString						UnescapeLiteral(const WString& literal, wchar_t quot);
			extern void							CompileAst(AstSymbolManager& astManager, AstDefFile* astDefFile, Ptr<GlrAstFile> file);
			extern void							CompileLexer(LexerSymbolManager& lexerManager, const WString& input);
			extern Ptr<GlrSyntaxFile>			CompileSyntax(AstSymbolManager& astManager, LexerSymbolManager& lexerManager, SyntaxSymbolManager& syntaxManager, Ptr<CppParserGenOutput> output, collections::List<Ptr<GlrSyntaxFile>>& files);
		}
	}
}

#endif
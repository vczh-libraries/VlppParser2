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
				using ClauseReuseDependencies = collections::Group<GlrClause*, RuleSymbol*>;
				using ClauseTypeMap = collections::Dictionary<GlrClause*, AstClassSymbol*>;

				struct VisitorContext
				{
					ParserSymbolManager&		global;
					AstSymbolManager&			astManager;
					LexerSymbolManager&			lexerManager;
					SyntaxSymbolManager&		syntaxManager;
					Ptr<CppParserGenOutput>		output;

					GlrRuleMap					astRules;
					LiteralTokenMap				literalTokens;
					RuleReuseDependencies		ruleReuseDependencies;
					RuleKnownTypes				ruleKnownTypes;
					ClauseReuseDependencies		clauseReuseDependencies;
					ClauseTypeMap				clauseTypes;
					Ptr<regex::RegexLexer>		cachedLexer;

					VisitorContext(
						AstSymbolManager& _astManager,
						LexerSymbolManager& _lexerManager,
						SyntaxSymbolManager& _syntaxManager,
						Ptr<CppParserGenOutput> _output
					)
						: global(_syntaxManager.Global())
						, astManager(_astManager)
						, lexerManager(_lexerManager)
						, syntaxManager(_syntaxManager)
						, output(_output)
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

			extern void							CompileAst(AstSymbolManager& astManager, AstDefFile* astDefFile, Ptr<GlrAstFile> file);
			extern void							CompileLexer(LexerSymbolManager& lexerManager, const WString& input);
			extern void							CompileSyntax(AstSymbolManager& astManager, LexerSymbolManager& lexerManager, SyntaxSymbolManager& syntaxManager, Ptr<CppParserGenOutput> output, collections::List<Ptr<GlrSyntaxFile>>& files);
		}
	}
}

#endif
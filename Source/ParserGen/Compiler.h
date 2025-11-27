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
#include "../ParserGen_Global/ParserCppGen.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			namespace compile_syntax
			{
				template<typename TClause>
				struct GenericRuleClausePath
				{
					RuleSymbol*							ruleSymbol = nullptr;
					TClause*							clause = nullptr;

					GenericRuleClausePath() = default;
					GenericRuleClausePath(RuleSymbol* _ruleSymbol, TClause* _clause)
						: ruleSymbol(_ruleSymbol), clause(_clause) {}

					bool operator==(const GenericRuleClausePath& p) const
					{
						if (ruleSymbol != p.ruleSymbol) return false;
						if (clause != p.clause) return false;
						return true;
					}
				};
				using RuleClausePath = GenericRuleClausePath<GlrClause>;

				using RuleSymbolPair = collections::Pair<RuleSymbol*, RuleSymbol*>;

				using GlrRuleMap = collections::Dictionary<RuleSymbol*, GlrRule*>;
				using LiteralTokenMap = collections::Dictionary<GlrRefSyntax*, vint32_t>;
				using RuleDependencies = collections::Group<RuleSymbol*, RuleSymbol*>;
				using RuleKnownTypes = collections::Group<RuleSymbol*, AstClassSymbol*>;
				using ClauseReuseDependencies = collections::Group<GlrReuseClause*, RuleSymbol*>;
				using ClauseTypeMap = collections::Dictionary<GlrClause*, AstClassSymbol*>;

				using ClauseList = collections::List<GlrClause*>;

				struct VisitorContext
				{
					const ParserSymbolManager&			global;
					const AstSymbolManager&				astManager;
					const LexerSymbolManager&			lexerManager;
					const SyntaxSymbolManager&			syntaxManager;

					// ResolveName
					GlrRuleMap							astRules;											// RuleSymbol -> GlrRule
					LiteralTokenMap						literalTokens;
					RuleDependencies					ruleReuseDependencies;								// RuleSymbol -> !rule in that rule
					RuleKnownTypes						ruleKnownTypes;										// RuleSymbol -> explicitly declared object type in clauses
					ClauseReuseDependencies				clauseReuseDependencies;							// GlrReuseClause -> !rule in that clause
					ClauseTypeMap						clauseTypes;										// GlrClause -> type
					Ptr<regex::RegexLexer>				cachedLexer;

					VisitorContext(
						const AstSymbolManager& _astManager,
						const LexerSymbolManager& _lexerManager,
						const SyntaxSymbolManager& _syntaxManager
					)
						: global(_syntaxManager.Global())
						, astManager(_astManager)
						, lexerManager(_lexerManager)
						, syntaxManager(_syntaxManager)
					{
					}

					const regex::RegexLexer& GetCachedLexer()
					{
						if (!cachedLexer)
						{
							auto tokens = From(lexerManager.TokenOrder())
								.Select([&](const WString& name) { return lexerManager.Tokens()[name]->regex; });
							cachedLexer = Ptr(new regex::RegexLexer(tokens));
						}
						return *cachedLexer.Obj();
					}
				};

				using SwitchMap = collections::Dictionary<WString, collections::Pair<bool, GlrSwitchItem*>>;
				using RuleSwitchMap = collections::Group<RuleSymbol*, WString>;
				using ClauseSwitchMap = collections::Group<GlrClause*, WString>;

				struct VisitorSwitchContext
				{
					SwitchMap							switches;
					ClauseSwitchMap						clauseAffectedSwitches;								// GlrClause -> all switches that affect how it parses
					RuleSwitchMap						ruleAffectedSwitches;								// RuleSymbol -> all switches that affect how it parses
				};
			}

/***********************************************************************
Compiler
***********************************************************************/

			extern WString						UnescapeLiteral(const WString& literal, wchar_t quot);
			extern void							CompileAst(AstSymbolManager& astManager, collections::List<collections::Pair<AstDefFile*, Ptr<GlrAstFile>>>& files);
			extern void							CompileAst(AstSymbolManager& astManager, AstDefFile* astDefFile, Ptr<GlrAstFile> file);
			extern void							CompileLexer(LexerSymbolManager& lexerManager, const WString& input);
			extern Ptr<GlrSyntaxFile>			CompileSyntax(AstSymbolManager& astManager, LexerSymbolManager& lexerManager, SyntaxSymbolManager& syntaxManager, Ptr<CppParserGenOutput> output, collections::List<Ptr<GlrSyntaxFile>>& files);
			extern Ptr<GlrSyntaxFile>			CompileSyntax(AstSymbolManager& astManager, LexerSymbolManager& lexerManager, SyntaxSymbolManager& syntaxManager, Ptr<CppParserGenOutput> output, Ptr<GlrSyntaxFile> file);
		}
	}
}

#endif
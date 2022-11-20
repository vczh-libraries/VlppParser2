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
				using PushedSwitchList = collections::List<GlrPushConditionSyntax*>;

				inline bool ArePushedSwitchesIdentical(Ptr<PushedSwitchList> p1, Ptr<PushedSwitchList> p2)
				{
					if ((p1 == nullptr) != (p2 == nullptr)) return false;
					if (p1 && collections::CompareEnumerable(*p1.Obj(), *p2.Obj()) != 0) return false;
					return true;
				}

				template<typename TClause>
				struct GenericRuleClausePath
				{
					RuleSymbol*							ruleSymbol = nullptr;
					TClause*							clause = nullptr;
					Ptr<PushedSwitchList>				pushedSwitches;

					GenericRuleClausePath() = default;
					GenericRuleClausePath(RuleSymbol* _ruleSymbol, TClause* _clause, Ptr<PushedSwitchList> _pushedSwitches)
						: ruleSymbol(_ruleSymbol), clause(_clause), pushedSwitches(_pushedSwitches) {}

					bool operator==(const GenericRuleClausePath& p) const
					{
						if (ruleSymbol != p.ruleSymbol) return false;
						if (clause != p.clause) return false;
						return ArePushedSwitchesIdentical(pushedSwitches, p.pushedSwitches);
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

				using SwitchMap = collections::Dictionary<WString, collections::Pair<bool, GlrSwitchItem*>>;
				using RuleSwitchMap = collections::Group<RuleSymbol*, WString>;
				using ClauseSwitchMap = collections::Group<GlrClause*, WString>;

				using LeftRecursiveClauseMap = collections::Group<RuleSymbol*, GlrClause*>;
				using LeftRecursionInjectClauseMap = collections::Group<RuleSymbol*, GlrLeftRecursionInjectClause*>;
				using LeftRecursionPlaceholderClauseMap = collections::Group<RuleSymbol*, GlrLeftRecursionPlaceholderClause*>;
				using PrefixMergeClauseMap = collections::Group<RuleSymbol*, GlrPrefixMergeClause*>;
				using ClauseToRuleMap = collections::Dictionary<GlrClause*, RuleSymbol*>;
				using ClauseToRuleGroup = collections::Group<GlrClause*, RuleSymbol*>;
				using RulePathDependencies = collections::Group<RuleSymbol*, RuleClausePath>;
				using PathToLastRuleMap = collections::Group<RuleSymbolPair, RuleClausePath>;

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
					ClauseToRuleMap						clauseToRules;										// GlrClause -> RuleSymbol that contain this clause
					Ptr<regex::RegexLexer>				cachedLexer;

					// ValidateSwitchesAndConditions (not used after RewriteSyntax_Switch)
					SwitchMap							switches;
					ClauseSwitchMap						clauseAffectedSwitches;								// GlrClause -> all switches that affect how it parses
					RuleSwitchMap						ruleAffectedSwitches;								// RuleSymbol -> all switches that affect how it parses

					LeftRecursiveClauseMap				leftRecursiveClauses;								// RuleSymbol -> all clauses begins with that rule
					LeftRecursionInjectClauseMap		directLriClauses, indirectLriClauses;				// RuleSymbol -> contained left_recursion_injection clauses
																											// RuleSymbol -> reachable left_recursion_injection clauses
					LeftRecursionPlaceholderClauseMap	directLrpClauses, indirectLrpClauses;				// RuleSymbol -> contained left_recursion_placeholder clauses
																											// RuleSymbol -> reachable left_recursion_placeholder clauses
					PrefixMergeClauseMap				directPmClauses, indirectPmClauses;					// RuleSymbol -> contained prefix_merge clauses
																											// RuleSymbol -> reachable prefix_merge clauses

					ClauseToRuleMap						simpleUseClauseToReferencedRules;					// GlrClause -> RuleSymbol when this clause is !RuleSymbol
					ClauseToRuleGroup					clauseToStartRules;									// GlrClause -> RuleSymbol when this clause begins with RuleSymbol

					// PushedSwitchList in following members are organized in the order of execution
					RulePathDependencies				directStartRules, indirectStartRules;				// RuleSymbol -> {rule, clause begins with the rule}
																											// RuleSymbol -> {rule, reachable clause begins with the rule}
					RulePathDependencies				directSimpleUseRules, indirectSimpleUseRules;		// RuleSymbol -> {rule, clause that is !rule}
																											// RuleSymbol -> {rule, reachable clause that is !rule}
					PathToLastRuleMap					indirectStartPathToLastRules;						// {r1, r3} -> {r2, clause}, where r1 --(indirect)--> r2 --(direct)--> {r3, clause begins with r3 contained in r2}
					PathToLastRuleMap					indirectSimpleUsePathToLastRules;					// {r1, r3} -> {r2, clause}, where r1 (indirect)::= !r2 (direct)::= {r3, clause that is !r3 contained in r2}

					// ValidateStructure
					ClauseList							clauseToConvertedToPrefixMerge;						// GlrClause when it should be converted to a prefix_merge clause

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
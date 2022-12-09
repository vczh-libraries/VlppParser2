#include "TestError.h"

namespace TestError_CalculatorAstAndLexer
{
	extern const wchar_t* astCode;
	extern const wchar_t* lexerCode;
}
using namespace TestError_CalculatorAstAndLexer;

TEST_FILE
{
	TypeParser typeParser;
	RuleParser ruleParser;

	//////////////////////////////////////////////////////
	// RuleMixedPrefixMergeWithClauseNotSyntacticallyBeginWithARule
	//////////////////////////////////////////////////////

	TEST_CASE(L"RuleMixedPrefixMergeWithClauseNotSyntacticallyBeginWithARule")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= !prefix_merge(Exp0)
  ::= [Exp0] !Exp0
  ::= Exp1:left "+" Exp0:right as BinaryExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleMixedPrefixMergeWithClauseNotSyntacticallyBeginWithARule,L"Exp1" }
			);
	});

	//////////////////////////////////////////////////////
	// RuleMixedPrefixMergeWithClauseNotBeginWithIndirectPrefixMerge
	//////////////////////////////////////////////////////

	TEST_CASE(L"RuleMixedPrefixMergeWithClauseNotBeginWithIndirectPrefixMerge 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= !prefix_merge(Exp0)
  ::= !Exp0 ADD
  ::= Exp1:left "+" Exp0:right as BinaryExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleMixedPrefixMergeWithClauseNotBeginWithIndirectPrefixMerge,L"Exp1",L"Exp0"}
			);
	});

	TEST_CASE(L"RuleMixedPrefixMergeWithClauseNotBeginWithIndirectPrefixMerge 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Something
  ::= ADD !Exp0
  ;
Exp1
  ::= !prefix_merge(Exp0)
  ::= !Something
  ::= Exp1:left "+" Exp0:right as BinaryExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleMixedPrefixMergeWithClauseNotBeginWithIndirectPrefixMerge,L"Exp1",L"Something"}
			);
	});

	//////////////////////////////////////////////////////
	// RuleIndirectlyBeginsWithPrefixMergeOrLeftRecursionMarkers
	//////////////////////////////////////////////////////

	TEST_CASE(L"RuleIndirectlyBeginsWithPrefixMergeMixedLeftRecursionMarkers 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
LRP
  ::= left_recursion_placeholder(Expr)
  ::= !Exp0
  ;
Exp1
  ::= !PM
  ;
Exp2
  ::= !LRP
  ;
Exp3
  ::= !Exp1
  ::= !Exp2
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleIndirectlyBeginsWithPrefixMergeMixedLeftRecursionMarkers,L"Exp3",L"PM",L"LRP"}
			);
	});

	TEST_CASE(L"RuleIndirectlyBeginsWithPrefixMergeMixedLeftRecursionMarkers 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
LRP
  ::= left_recursion_placeholder(Expr)
  ::= !Exp0
  ::= !LRP ADD
  ;
LRI
  ::= !Exp0 left_recursion_inject(Expr) LRP
  ;
Exp1
  ::= !PM
  ;
Exp2
  ::= !LRI
  ;
Exp3
  ::= !Exp1
  ::= !Exp2
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleIndirectlyBeginsWithPrefixMergeMixedLeftRecursionMarkers,L"Exp3",L"PM",L"LRI"}
			);
	});

	//////////////////////////////////////////////////////
	// PartialRuleIndirectlyBeginsWithPrefixMerge
	//////////////////////////////////////////////////////

	TEST_CASE(L"PartialRuleIndirectlyBeginsWithPrefixMerge")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Exp1
  ::= PM "+" as partial NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::PartialRuleIndirectlyBeginsWithPrefixMerge,L"Exp1",L"PM"}
			);
	});

	//////////////////////////////////////////////////////
	// ClausePartiallyIndirectlyBeginsWithPrefixMergeAndLiteral
	//////////////////////////////////////////////////////

	TEST_CASE(L"ClausePartiallyIndirectlyBeginsWithPrefixMergeAndLiteral 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Exp1
  ::= {"+"} !PM "+"
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ClausePartiallyIndirectlyBeginsWithPrefixMergeAndLiteral,L"Exp1",L"PM",L"\"+\"" }
			);
	});

	TEST_CASE(L"ClausePartiallyIndirectlyBeginsWithPrefixMergeAndLiteral 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Exp1
  ::= ["+"] !PM "+"
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ClausePartiallyIndirectlyBeginsWithPrefixMergeAndLiteral,L"Exp1",L"PM",L"\"+\"" }
			);
	});

	TEST_CASE(L"ClausePartiallyIndirectlyBeginsWithPrefixMergeAndLiteral 3")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Exp1
  ::= (!PM "+" | "+" !PM)
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ClausePartiallyIndirectlyBeginsWithPrefixMergeAndLiteral,L"Exp1",L"PM",L"\"+\"" }
			);
	});

	TEST_CASE(L"ClausePartiallyIndirectlyBeginsWithPrefixMergeAndLiteral 4")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Exp1
  ::= ["+"] !PM "+"
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ClausePartiallyIndirectlyBeginsWithPrefixMergeAndLiteral,L"Exp1",L"PM",L"\"+\"" }
			);
	});

	TEST_CASE(L"ClausePartiallyIndirectlyBeginsWithPrefixMergeAndLiteral 5")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Exp1
  ::= (PM:left "+" | "+" PM:right) as BinaryExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ClausePartiallyIndirectlyBeginsWithPrefixMergeAndLiteral,L"Exp1",L"PM",L"\"+\"" }
			);
	});

	TEST_CASE(L"ClausePartiallyIndirectlyBeginsWithPrefixMergeAndLiteral 6")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Exp1
  ::= {"+"} PM:func "+" as CallExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ClausePartiallyIndirectlyBeginsWithPrefixMergeAndLiteral,L"Exp1",L"PM",L"\"+\"" }
			);
	});

	TEST_CASE(L"ClausePartiallyIndirectlyBeginsWithPrefixMergeAndLiteral 7")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Exp1
  ::= ({"+"} | ["+"]) PM:func "+" as CallExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ClausePartiallyIndirectlyBeginsWithPrefixMergeAndLiteral,L"Exp1",L"PM",L"\"+\"" }
			);
	});

	//////////////////////////////////////////////////////
	// ClausePartiallyIndirectlyBeginsWithPrefixMergeAndRule
	//////////////////////////////////////////////////////

	TEST_CASE(L"ClausePartiallyIndirectlyBeginsWithPrefixMergeAndRule 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Plus
  ::= "+" as NumExpr
  ;
Exp1
  ::= {Plus} !PM Plus
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ClausePartiallyIndirectlyBeginsWithPrefixMergeAndRule,L"Exp1",L"PM",L"Plus" }
			);
	});

	TEST_CASE(L"ClausePartiallyIndirectlyBeginsWithPrefixMergeAndRule 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Plus
  ::= "+" as NumExpr
  ;
Exp1
  ::= [Plus] !PM Plus
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ClausePartiallyIndirectlyBeginsWithPrefixMergeAndRule,L"Exp1",L"PM",L"Plus" }
			);
	});

	TEST_CASE(L"ClausePartiallyIndirectlyBeginsWithPrefixMergeAndRule 3")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Plus
  ::= "+" as NumExpr
  ;
Exp1
  ::= (!PM Plus | Plus !PM)
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ClausePartiallyIndirectlyBeginsWithPrefixMergeAndRule,L"Exp1",L"PM",L"Plus" }
			);
	});

	TEST_CASE(L"ClausePartiallyIndirectlyBeginsWithPrefixMergeAndRule 4")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Plus
  ::= "+" as NumExpr
  ;
Exp1
  ::= [Plus] !PM Plus
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ClausePartiallyIndirectlyBeginsWithPrefixMergeAndRule,L"Exp1",L"PM",L"Plus" }
			);
	});

	TEST_CASE(L"ClausePartiallyIndirectlyBeginsWithPrefixMergeAndRule 5")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Plus
  ::= "+" as NumExpr
  ;
Exp1
  ::= (PM:left Plus | Plus PM:right) as BinaryExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ClausePartiallyIndirectlyBeginsWithPrefixMergeAndRule,L"Exp1",L"PM",L"Plus" }
			);
	});

	TEST_CASE(L"ClausePartiallyIndirectlyBeginsWithPrefixMergeAndRule 6")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Plus
  ::= "+" as NumExpr
  ;
Exp1
  ::= {Plus} PM:func Plus as CallExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ClausePartiallyIndirectlyBeginsWithPrefixMergeAndRule,L"Exp1",L"PM",L"Plus" }
			);
	});

	TEST_CASE(L"ClausePartiallyIndirectlyBeginsWithPrefixMergeAndRule 7")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Plus
  ::= "+" as NumExpr
  ;
Exp1
  ::= ({Plus} PM:left | [Plus] PM:right) Plus as BinaryExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ClausePartiallyIndirectlyBeginsWithPrefixMergeAndRule,L"Exp1",L"PM",L"Plus" }
			);
	});

	//////////////////////////////////////////////////////
	// RuleDeductToPrefixMergeInNonSimpleUseClause
	//////////////////////////////////////////////////////

	TEST_CASE(L"RuleDeductToPrefixMergeInNonSimpleUseClause 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Exp1
  ::= PM:func as CallExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleDeductToPrefixMergeInNonSimpleUseClause,L"Exp1",L"PM",L"PM" }
			);
	});

	TEST_CASE(L"RuleDeductToPrefixMergeInNonSimpleUseClause 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Exp1
  ::= ["+"] PM:func as CallExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleDeductToPrefixMergeInNonSimpleUseClause,L"Exp1",L"PM",L"PM" }
			);
	});

	TEST_CASE(L"RuleDeductToPrefixMergeInNonSimpleUseClause 3")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Exp1
  ::= {"+"} !PM
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleDeductToPrefixMergeInNonSimpleUseClause,L"Exp1",L"PM",L"PM" }
			);
	});

	TEST_CASE(L"RuleDeductToPrefixMergeInNonSimpleUseClause 4")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Exp1
  ::= !PM
  ;
Exp2
  ::= Exp1:func ["+"] as CallExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleDeductToPrefixMergeInNonSimpleUseClause,L"Exp2",L"PM",L"Exp1" }
			);
	});

	TEST_CASE(L"RuleDeductToPrefixMergeInNonSimpleUseClause 5")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Exp1
  ::= !PM
  ;
Exp2
  ::= !Exp1 {"+"}
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleDeductToPrefixMergeInNonSimpleUseClause,L"Exp2",L"PM",L"Exp1" }
			);
	});

	TEST_CASE(L"RuleDeductToPrefixMergeInNonSimpleUseClause 6")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Exp1
  ::= (PM:func | PM:args "+") as CallExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleDeductToPrefixMergeInNonSimpleUseClause,L"Exp1",L"PM",L"PM" }
			);
	});

	TEST_CASE(L"RuleDeductToPrefixMergeInNonSimpleUseClause 7")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Exp1
  ::= (["+"] PM:func | PM:args) as CallExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleDeductToPrefixMergeInNonSimpleUseClause,L"Exp1",L"PM",L"PM" }
			);
	});

	TEST_CASE(L"RuleDeductToPrefixMergeInNonSimpleUseClause 8")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Exp1
  ::= {"+"} !PM | !PM
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleDeductToPrefixMergeInNonSimpleUseClause,L"Exp1",L"PM",L"PM" }
			);
	});

	TEST_CASE(L"RuleDeductToPrefixMergeInNonSimpleUseClause 9")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Exp1
  ::= !PM
  ;
Exp2
  ::= (Exp1:func ["+"] | Exp1:args) as CallExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleDeductToPrefixMergeInNonSimpleUseClause,L"Exp2",L"PM",L"Exp1" }
			);
	});

	TEST_CASE(L"RuleDeductToPrefixMergeInNonSimpleUseClause 10")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Exp1
  ::= !PM
  ;
Exp2
  ::= !Exp1 {"+"} | !Exp1
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleDeductToPrefixMergeInNonSimpleUseClause,L"Exp2",L"PM",L"Exp1" }
			);
	});

	TEST_CASE(L"RuleDeductToPrefixMergeInNonSimpleUseClause 11")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Exp1
  ::= !PM
  ;
Exp2
  ::= !Exp1
  ::= !Exp2 "+"
  ;
Exp3
  ::= (!Exp1 | !Exp0)
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleDeductToPrefixMergeInNonSimpleUseClause,L"Exp3",L"PM",L"Exp1" }
			);
	});

	TEST_CASE(L"RuleDeductToPrefixMergeInNonSimpleUseClause 12")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
PM
  ::= !prefix_merge(Exp0)
  ;
Exp1
  ::= !PM
  ;
Exp2
  ::= !Exp1
  ::= !Exp2 "+"
  ;
Exp3
  ::= (!Exp1 | !Exp0)
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleDeductToPrefixMergeInNonSimpleUseClause,L"Exp3",L"PM",L"Exp1" }
			);
	});
}
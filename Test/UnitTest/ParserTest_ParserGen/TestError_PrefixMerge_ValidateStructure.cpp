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

	TEST_CASE(L"RuleIndirectlyBeginsWithPrefixMergeOrLeftRecursionMarkers 1")
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
			{ ParserErrorType::RuleIndirectlyBeginsWithPrefixMergeOrLeftRecursionMarkers,L"Exp3",L"PM",L"LRP"}
			);
	});

	TEST_CASE(L"RuleIndirectlyBeginsWithPrefixMergeOrLeftRecursionMarkers 2")
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
			{ ParserErrorType::RuleIndirectlyBeginsWithPrefixMergeOrLeftRecursionMarkers,L"Exp3",L"PM",L"LRI"}
			);
	});

	//////////////////////////////////////////////////////
	// RuleIndirectlyBeginsWithPrefixMergeMixedNonSimpleUseClause
	//////////////////////////////////////////////////////

	TEST_CASE(L"RuleIndirectlyBeginsWithPrefixMergeMixedNonSimpleUseClause 1")
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
  ::= !Exp0
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleIndirectlyBeginsWithPrefixMergeMixedNonSimpleUseClause,L"Exp2",L"PM"}
			);
	});

	TEST_CASE(L"RuleIndirectlyBeginsWithPrefixMergeMixedNonSimpleUseClause 2")
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
  ::= !Exp1 ADD
  ;
Exp2
  ::= !Exp1
  ::= (!Exp0 | !Exp0)
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleIndirectlyBeginsWithPrefixMergeMixedNonSimpleUseClause,L"Exp2",L"PM"}
			);
	});
}
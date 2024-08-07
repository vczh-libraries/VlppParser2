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

	TEST_CASE(L"ClauseNotCreateObject")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= "+" | !Exp0
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ClauseNotCreateObject,L"Exp1" }
			);
	});

	TEST_CASE(L"UseRuleUsedInOptionalBody")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= "(" ["+" !Exp0] ")"
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::UseRuleUsedInOptionalBody,L"Exp1",L"Exp0" }
			);
	});

	TEST_CASE(L"UseRuleUsedInLoopBody 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= "(" {"+" !Exp0} ")"
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::UseRuleUsedInLoopBody,L"Exp1",L"Exp0" }
			);
	});

	TEST_CASE(L"UseRuleUsedInLoopBody 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= "(" {"*"; "+" !Exp0} ")"
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::UseRuleUsedInLoopBody,L"Exp1",L"Exp0" }
			);
	});

	TEST_CASE(L"ClauseTooManyUseRule")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= NUM:value as NumExpr
  ;
Exp2
  ::= NUM:value as NumExpr
  ;
Exp3
  ::= (!Exp0 | !Exp1) !Exp2
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ClauseTooManyUseRule,L"Exp3" }
			);
	});

	TEST_CASE(L"NonArrayFieldAssignedInLoop")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= {NUM:value} as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::NonArrayFieldAssignedInLoop,L"Exp0",L"NumExpr",L"value" }
			);
	});

	TEST_CASE(L"NonLoopablePartialRuleUsedInLoop")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as partial NumExpr
  ;
Exp1
  ::= {Exp0} Exp0 as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::NonLoopablePartialRuleUsedInLoop,L"Exp1",L"NumExpr",L"Exp0" }
			);
	});

	TEST_CASE(L"ClauseCouldExpandToEmptySequence")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= [Exp0:left] [Exp1:left] as BinaryExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ClauseCouldExpandToEmptySequence,L"Exp1" }
			);
	});

	TEST_CASE(L"LoopBodyCouldExpandToEmptySequence")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= Exp0:func {[Exp0:args]; [Exp1:args]} as CallExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::LoopBodyCouldExpandToEmptySequence,L"Exp1" }
			);
	});

	TEST_CASE(L"OptionalBodyCouldExpandToEmptySequence")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= Exp0:func [[Exp0:args] [Exp1:args]] as CallExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::OptionalBodyCouldExpandToEmptySequence,L"Exp1" }
			);
	});

	TEST_CASE(L"NegativeOptionalEndsAClause 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value -[NUM:value] as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::NegativeOptionalEndsAClause,L"Exp0" }
			);
	});

	TEST_CASE(L"NegativeOptionalEndsAClause 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value -[NUM:value] [NUM:value] as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::NegativeOptionalEndsAClause,L"Exp0" }
			);
	});

	TEST_CASE(L"NegativeOptionalEndsAClause 3")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= Exp0:func -[Exp0] {Exp0:args} as CallExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::NegativeOptionalEndsAClause,L"Exp1" }
			);
	});

	TEST_CASE(L"FieldAssignedMoreThanOnce 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value NUM:value as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::FieldAssignedMoreThanOnce,L"Exp0",L"NumExpr",L"value" }
			);
	});

	TEST_CASE(L"FieldAssignedMoreThanOnce 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value [NUM:value] as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::FieldAssignedMoreThanOnce,L"Exp0",L"NumExpr",L"value" }
			);
	});

	TEST_CASE(L"FieldAssignedMoreThanOnce 3")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= [NUM:value] "+" [NUM:value] as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::FieldAssignedMoreThanOnce,L"Exp0",L"NumExpr",L"value" }
			);
	});

	TEST_CASE(L"FieldAssignedMoreThanOnce 4")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= (NUM:value | "+") [NUM:value] as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::FieldAssignedMoreThanOnce,L"Exp0",L"NumExpr",L"value" }
			);
	});

	TEST_CASE(L"FieldAssignedMoreThanOnce 5")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= ("+" | NUM:value) [NUM:value] as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::FieldAssignedMoreThanOnce,L"Exp0",L"NumExpr",L"value" }
			);
	});

	TEST_CASE(L"PrioritizedOptionalEndsClause 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= "+" -[NUM:value] as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::NegativeOptionalEndsAClause,L"Exp0" }
			);
	});

	TEST_CASE(L"PrioritizedOptionalEndsClause 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= ("+" | "*" -[NUM:value]) as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::NegativeOptionalEndsAClause,L"Exp0" }
			);
	});

	TEST_CASE(L"PrioritizedOptionalEndsClause 3")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value {"+" -["*"]} as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::NegativeOptionalEndsAClause,L"Exp0" }
			);
	});

	TEST_CASE(L"PrioritizedOptionalEndsClause 4")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value {"(" ")" ; "+" -["*"]} as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::NegativeOptionalEndsAClause,L"Exp0" }
			);
	});

	TEST_CASE(L"PrioritizedOptionalEndsClause 5")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= -[NUM:value] {"+"} as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::NegativeOptionalEndsAClause,L"Exp0" }
			);
	});

	TEST_CASE(L"MultiplePrioritySyntaxInAClause 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= +[NUM:value] -[NUM:value] NUM as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::MultiplePrioritySyntaxInAClause,L"Exp0" }
			);
	});

	TEST_CASE(L"MultiplePrioritySyntaxInAClause 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= +[-[NUM:value] NUM] as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::MultiplePrioritySyntaxInAClause,L"Exp0" }
			);
	});
}
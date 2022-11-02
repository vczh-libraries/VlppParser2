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

	TEST_CASE(L"PushConditionBodyCouldExpandToEmptySequence")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch first;
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= Exp0:func !(first; [Exp0:args] [Exp1:args]) as CallExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::PushConditionBodyCouldExpandToEmptySequence,L"Exp1" }
			);
	});

	TEST_CASE(L"TestConditionBodyCouldExpandToEmptySequence")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch first;
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= Exp0:func ?(first: [Exp0:args] [Exp1:args]) as CallExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::TestConditionBodyCouldExpandToEmptySequence,L"Exp1" }
			);
	});

	TEST_CASE(L"MultipleEmptySyntaxInTestCondition")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch first;
Exp0
  ::= NUM:value ?(first:; | first:;) as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::MultipleEmptySyntaxInTestCondition,L"Exp0" }
		);
	});

	TEST_CASE(L"PushedSwitchIsNotTested 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch first;
Exp0 ::= !(first; NUM:value) as NumExpr;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::PushedSwitchIsNotTested,L"Exp0",L"first"}
		);
	});

	TEST_CASE(L"PushedSwitchIsNotTested 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch first;
Exp0 ::= NUM:value as NumExpr;
Exp1 ::= !(!first; !Exp0);
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::PushedSwitchIsNotTested,L"Exp1",L"first"}
		);
	});

	TEST_CASE(L"PushedSwitchIsNotTested 3")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch first, second;
Exp0 ::= ?(first: value) as NumExpr;
Exp1 ::= !(first, !second; !Exp0);
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::PushedSwitchIsNotTested,L"Exp1",L"second"}
		);
	});

	TEST_CASE(L"PushedSwitchIsNotTested 4")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch first, second;
Exp0 ::= ?(first && second: value) as NumExpr;
Exp1 ::= !(!second; !Exp0);
Exp2 ::= !(first, second; !Exp1);
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::PushedSwitchIsNotTested,L"Exp2",L"second"}
		);
	});
}
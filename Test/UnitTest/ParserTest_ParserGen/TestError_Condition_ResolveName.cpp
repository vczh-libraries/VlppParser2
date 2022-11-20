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

	TEST_CASE(L"SyntaxInvolvesSwitchWithIllegalRuleName")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch first;
Exp0
  ::= ?(first: NUM) as Expr
  ;
SWITCH_Something
  ::= NUM as Expr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::SyntaxInvolvesSwitchWithIllegalRuleName,L"SWITCH_Something" }
			);
	});

	TEST_CASE(L"DuplicatedSwitch")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch first,!first;
Exp0 ::= "+":value as NumExpr;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::DuplicatedSwitch,L"first" }
		);
	});

	TEST_CASE(L"UnusedSwitch")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch first;
Exp0 ::= "+":value as NumExpr;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::UnusedSwitch,L"first" }
		);
	});

	TEST_CASE(L"SwitchNotExists 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= !(first; "+":value) as NumExpr;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::SwitchNotExists,L"Exp0",L"first"}
		);
	});

	TEST_CASE(L"SwitchNotExists 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= ?(first: "+":value) as NumExpr;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::SwitchNotExists,L"Exp0",L"first" }
		);
	});
}
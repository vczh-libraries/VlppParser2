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
	// SwitchUnaffectedRuleExpandedToNoClause
	//////////////////////////////////////////////////////

	TEST_CASE(L"SwitchUnaffectedRuleExpandedToNoClause")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch first;
Exp0
  ::= !(!first; ?(first: NUM)) as Expr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::SwitchUnaffectedRuleExpandedToNoClause,L"Exp0" }
			);
	});

	//////////////////////////////////////////////////////
	// SwitchAffectedRuleExpandedToNoClause
	//////////////////////////////////////////////////////

	TEST_CASE(L"SwitchAffectedRuleExpandedToNoClause")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch first;
Exp0
  ::= ?(first: NUM) as Expr
  ;
Exp1
  ::= !(!first; !Exp0)
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::SwitchAffectedRuleExpandedToNoClause,L"Exp0",L"Exp0_SWITCH_0first" }
			);
	});
}
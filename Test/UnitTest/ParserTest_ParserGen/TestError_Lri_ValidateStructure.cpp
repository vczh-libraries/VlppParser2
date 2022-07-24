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

	TEST_CASE(L"TooManyLeftRecursionPlaceholderClauses")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch first;
Exp0
  ::= left_recursion_placeholder(A, B)
  ::= left_recursion_placeholder(C, D)
  ::= NUM:value ?(first:; | first:;) as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::TooManyLeftRecursionPlaceholderClauses,L"Exp0" }
		);
	});
}
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

	TEST_CASE(L"RuleCannotResolveToDeterministicType 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= NUM:value as NumExpr;
Exp1 ::= "+" as Module ;
Exp2 ::= !Exp0 [left_recursion_inject(Something) Exp1];
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleCannotResolveToDeterministicType,L"Exp2" }
			);
	});

	TEST_CASE(L"RuleCannotResolveToDeterministicType 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= NUM:value as NumExpr;
Exp1 ::= "+" as Module ;
Exp2 ::= !Exp0 left_recursion_inject(Something) (Exp0 [left_recursion_inject(Something) Exp1]);
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleCannotResolveToDeterministicType,L"Exp2" }
			);
	});

	TEST_CASE(L"RuleCannotResolveToDeterministicType 3")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= NUM:value as NumExpr;
Exp1 ::= "+" as Module ;
Exp2 ::= !Exp0 [left_recursion_inject(Something) (Exp0 left_recursion_inject(Something) Exp1)];
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleCannotResolveToDeterministicType,L"Exp2" }
			);
	});
}
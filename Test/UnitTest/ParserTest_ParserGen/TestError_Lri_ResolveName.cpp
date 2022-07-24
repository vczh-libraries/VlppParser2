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

	TEST_CASE(L"TypeNotExistsInRule 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= NUM:value as NumExpr;
Exp1
  ::= !Exp0
  ::= !Unknown left_recursion_inject(Something) Exp0
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::TokenOrRuleNotExistsInRule,L"Exp1",L"Unknown" }
		);
	});

	TEST_CASE(L"TypeNotExistsInRule 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= NUM:value as NumExpr;
Exp1
  ::= !Exp0
  ::= !Unknown [left_recursion_inject(Something) Exp0]
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::TokenOrRuleNotExistsInRule,L"Exp1",L"Unknown" }
		);
	});

	TEST_CASE(L"TypeNotExistsInRule 3")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= NUM:value as NumExpr;
Exp1
  ::= !Exp0
  ::= !Exp0 left_recursion_inject(Something) Unknown
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::TokenOrRuleNotExistsInRule,L"Exp1",L"Unknown" }
		);
	});

	TEST_CASE(L"TypeNotExistsInRule 4")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= NUM:value as NumExpr;
Exp1
  ::= !Exp0
  ::= !Exp0 [left_recursion_inject(Something) Unknown]
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::TokenOrRuleNotExistsInRule,L"Exp1",L"Unknown" }
		);
	});

	TEST_CASE(L"TypeNotExistsInRule 5")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= NUM:value as NumExpr;
Exp1
  ::= !Exp0
  ::= !Exp0 left_recursion_inject(Something)
        (Unknown left_recursion_inject(Something) Exp1)
      | (Exp0 [left_recursion_inject(Something) Exp1])
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::TokenOrRuleNotExistsInRule,L"Exp1",L"Unknown" }
		);
	});

	TEST_CASE(L"TypeNotExistsInRule 6")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= NUM:value as NumExpr;
Exp1
  ::= !Exp0
  ::= !Exp0 left_recursion_inject(Something)
        (Exp0 left_recursion_inject(Something) Exp1)
      | (Exp0 [left_recursion_inject(Something) Unknown])
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::TokenOrRuleNotExistsInRule,L"Exp1",L"Unknown" }
		);
	});
}
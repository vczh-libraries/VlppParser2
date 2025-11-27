#include "TestError.h"

TEST_FILE
{
	TypeParser typeParser;
	RuleParser ruleParser;

	const wchar_t* astCode =
LR"AST(
class Ast {}
)AST";

	const wchar_t* lexerCode =
LR"LEXER(
A:a
)LEXER";

	TEST_CASE(L"DuplicatedRule")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
X ::= A as Ast;
X ::= A as Ast;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::DuplicatedRule,L"X" }
			);
	});

	TEST_CASE(L"RuleIsIndirectlyLeftRecursive")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
X
  ::= A as Ast
  ;
Y
  ::= !X
  ::= Z !Y
  ;
Z
  ::= !X
  ::= Y !Z
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleIsIndirectlyLeftRecursive,L"Z" }
			);
	});
}
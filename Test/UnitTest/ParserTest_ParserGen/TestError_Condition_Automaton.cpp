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

	TEST_CASE(L"LeftRecursiveClauseInsidePushCondition 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch first;
X
  ::= A as Ast
  ;
Y
  ::= !X
  ::= !(first; !Y X)
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::LeftRecursiveClauseInsidePushCondition,L"Y" }
			);
	});

	TEST_CASE(L"LeftRecursiveClauseInsidePushCondition 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch first;
X
  ::= A as Ast
  ;
Y
  ::= !X
  ::= !(first; !Y) X
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::LeftRecursiveClauseInsidePushCondition,L"Y" }
			);
	});

	TEST_CASE(L"LeftRecursiveClauseInsideTestCondition 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch first;
X
  ::= A as Ast
  ;
Y
  ::= !X
  ::= ?(first: !Y X)
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::LeftRecursiveClauseInsideTestCondition,L"Y" }
			);
	});

	TEST_CASE(L"LeftRecursiveClauseInsideTestCondition 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch first;
X
  ::= A as Ast
  ;
Y
  ::= !X
  ::= ?(first: !Y) X
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::LeftRecursiveClauseInsideTestCondition,L"Y" }
			);
	});
}
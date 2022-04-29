#include "TestError.h"

TEST_FILE
{
	TypeParser typeParser;
	RuleParser ruleParser;

	const wchar_t* astCode =
LR"AST(
class Expr
{
}

class NumExpr
{
	var value : token;
}

enum BinaryOp
{
	Add,
	Mul,
}

class BinaryExpr
{
	var op : BinaryOp;
	var left : Expr;
	var right : Expr;
}
)AST";

	const wchar_t* lexerCode =
LR"LEXER(
NUM:/d+(./d+)?
ADD:/+
MUL:/*
OPEN:/(
CLOSE:/)
discard SPACE:/s+
discard MINUS_MINUS:--
)LEXER";

	TEST_CASE(L"TypeNotExistsInRule 1")
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

	TEST_CASE(L"TypeNotExistsInRule 2")
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
}
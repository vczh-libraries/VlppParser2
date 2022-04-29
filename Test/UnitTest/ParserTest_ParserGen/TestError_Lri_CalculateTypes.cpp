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

class RefExpr : Expr
{
	var value : token;
}

class NumExpr : Expr
{
	var value : token;
}

enum BinaryOp
{
	Add,
	Mul,
}

class BinaryExpr : Expr
{
	var op : BinaryOp;
	var left : Expr;
	var right : Expr;
}

class Module
{
	var export : Expr;
}
)AST";

	const wchar_t* lexerCode =
LR"LEXER(
ID:[a-zA-Z_]/w*
NUM:/d+(./d+)?
ADD:/+
MUL:/*
OPEN:/(
CLOSE:/)
discard SPACE:/s+
)LEXER";

	TEST_CASE(L"RuleCannotResolveToDeterministicType")
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
}
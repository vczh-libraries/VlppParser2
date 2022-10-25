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

	TEST_CASE(L"SwitchNotExists 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= NUM:value as NumExpr;
Exp1 ::= !Exp0 left_recursion_inject(Something) !(first; Exp0);
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::SwitchNotExists,L"Exp1",L"first"}
		);
	});

	TEST_CASE(L"SwitchNotExists 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= NUM:value as NumExpr;
Exp1 ::= !Exp0 [left_recursion_inject(Something) (Exp0 left_recursion_inject(Something) !(first; Exp0))];
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::SwitchNotExists,L"Exp1",L"first"}
		);
	});

	TEST_CASE(L"SwitchNotExists 3")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= NUM:value as NumExpr;
Exp1 ::= !Exp0 left_recursion_inject(Something) (Exp0 left_recursion_inject(Something) Exp0) | !(first; Exp0);
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::SwitchNotExists,L"Exp1",L"first"}
		);
	});

	TEST_CASE(L"SwitchNotExists 4")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch first;
Exp0 ::= NUM:value as NumExpr;
Exp1 ::= !Exp0 left_recursion_inject(Something) (Exp0 left_recursion_inject(Something) !(first; Exp0)) | !(first, !second; Exp0);
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::SwitchNotExists,L"Exp1",L"second"}
		);
	});
}
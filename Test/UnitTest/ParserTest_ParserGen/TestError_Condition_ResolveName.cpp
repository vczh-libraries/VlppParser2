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
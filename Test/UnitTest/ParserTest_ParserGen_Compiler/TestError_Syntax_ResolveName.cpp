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
)LEXER";

	TEST_CASE(L"RuleNameConflictedWithToken")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
SPACE ::= NUM:value as NumExpr;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleNameConflictedWithToken,L"SPACE" }
			);
	});

	TEST_CASE(L"TypeNotExistsInRule")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= NUM:value as Unknown;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::TypeNotExistsInRule,L"Exp0",L"Unknown" }
		);
	});

	TEST_CASE(L"TypeNotClassInRule")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= NUM:value as BinaryOp;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::TypeNotClassInRule,L"Exp0",L"BinaryOp" }
		);
	});

	TEST_CASE(L"TokenOrRuleNotExistsInRule 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= NUM2:value as NumExpr;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::TokenOrRuleNotExistsInRule,L"Exp0",L"NUM2" }
		);
	});

	TEST_CASE(L"TokenOrRuleNotExistsInRule 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= "-" NUM:value as NumExpr;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::TokenOrRuleNotExistsInRule,L"Exp0",L"\"-\"" }
		);
	});

	TEST_CASE(L"TokenOrRuleNotExistsInRule 3")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= NUM:value as NumExpr;
Exp1 ::= !Exp;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::TokenOrRuleNotExistsInRule,L"Exp1",L"Exp" }
		);
	});
}
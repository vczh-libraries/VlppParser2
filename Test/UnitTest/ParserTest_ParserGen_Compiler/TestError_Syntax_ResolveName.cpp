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

	TEST_CASE(L"TypeNotExistsInRule 3")
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

	TEST_CASE(L"TypeNotExistsInRule 4")
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

	TEST_CASE(L"LiteralNotValidToken 1")
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
			{ ParserErrorType::LiteralNotValidToken,L"Exp0",L"\"-\"" }
		);
	});

	TEST_CASE(L"LiteralNotValidToken 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= "" NUM:value as NumExpr;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::LiteralNotValidToken,L"Exp0",L"\"\"" }
		);
	});

	TEST_CASE(L"LiteralIsDiscardedToken")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= "--" NUM:value as NumExpr;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::LiteralIsDiscardedToken,L"Exp0",L"\"--\"" }
		);
	});

	TEST_CASE(L"ConditionalLiteralNotValidToken 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= '':value as NumExpr;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ConditionalLiteralNotValidToken,L"Exp0",L"\'\'" }
		);
	});

	TEST_CASE(L"ConditionalLiteralNotValidToken 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= '+123':value as NumExpr;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ConditionalLiteralNotValidToken,L"Exp0",L"\'+123\'" }
		);
	});

	TEST_CASE(L"ConditionalLiteralNotValidToken 3")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= '123.':value as NumExpr;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ConditionalLiteralNotValidToken,L"Exp0",L"\'123.\'" }
		);
	});

	TEST_CASE(L"ConditionalLiteralNotValidToken 4")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= '-':value as NumExpr;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ConditionalLiteralNotValidToken,L"Exp0",L"\'-\'" }
		);
	});

	TEST_CASE(L"ConditionalLiteralIsDiscardedToken")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= ' ':value as NumExpr;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ConditionalLiteralIsDiscardedToken,L"Exp0",L"\' \'" }
		);
	});

	TEST_CASE(L"ConditionalLiteralIsDisplayText")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 ::= '+':value as NumExpr;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ConditionalLiteralIsDisplayText,L"Exp0",L"\'+\'" }
		);
	});

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
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

class CallExpr : Expr
{
	var func : Expr;
	var args : Expr[];
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

	TEST_CASE(L"PushConditionBodyCouldExpandToEmptySequence")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch first;
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= Exp0:func !(first; [Exp0:args] [Exp1:args]) as CallExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::PushConditionBodyCouldExpandToEmptySequence,L"Exp1" }
			);
	});

	TEST_CASE(L"TestConditionBodyCouldExpandToEmptySequence")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch first;
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= Exp0:func ?(first: [Exp0:args] [Exp1:args]) as CallExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::TestConditionBodyCouldExpandToEmptySequence,L"Exp1" }
			);
	});

	TEST_CASE(L"MultipleEmptySyntaxInTestCondition")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
switch first;
Exp0
  ::= NUM:value ?(first:; | first:;) as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::MultipleEmptySyntaxInTestCondition,L"Exp0" }
		);
	});
}
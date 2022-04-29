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

	TEST_CASE(L"LeftRecursionPlaceholderNotFoundInRule")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= !Exp0
  ::= Exp1:left "+" Exp0:right as BinaryExpr
  ;
Exp2
  ::= !Exp0 [left_recursion_inject(Expression) Exp1]
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::LeftRecursionPlaceholderNotFoundInRule,L"Exp2",L"Expression",L"Exp1" }
			);
	});

	TEST_CASE(L"LeftRecursionPlaceholderNotUnique")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= left_recursion_placeholder(Expression)
  ::= !Exp0
  ::= Exp1:left "*" Exp0:right as BinaryExpr
  ;
Exp2
  ::= left_recursion_placeholder(Expression)
  ::= !Exp1
  ::= Exp2:left "+" Exp1:right as BinaryExpr
  ;
Exp3
  ::= !Exp0 [left_recursion_inject(Expression) Exp2]
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::LeftRecursionPlaceholderNotUnique,L"Exp3",L"Expression",L"Exp2" }
			);
	});

	TEST_CASE(L"LeftRecursionPlaceholderTypeMismatched")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= left_recursion_placeholder(Expression)
  ::= !Exp0
  ::= Exp1:left "*" Exp0:right as BinaryExpr
  ;
Exp2
  ::= Exp1 "+" as Module
  ;
Module
  ::= "+" as Module
  ;
Exp3
  ::= !Module [left_recursion_inject(Expression) Exp2]
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::LeftRecursionPlaceholderTypeMismatched,L"Exp3",L"Expression",L"Exp2",L"Exp1"}
			);
	});

	TEST_CASE(L"PartialRuleInLeftRecursionInject")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0Partial
  ::= NUM:value as partial NumExpr
  ;
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= left_recursion_placeholder(Expression)
  ::= !Exp0
  ::= Exp1:left "+" Exp0:right as BinaryExpr
  ;
Exp2
  ::= !Exp0Partial [left_recursion_inject(Expression) Exp1]
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::PartialRuleInLeftRecursionInject,L"Exp2",L"Exp0Partial"}
			);
	});
}
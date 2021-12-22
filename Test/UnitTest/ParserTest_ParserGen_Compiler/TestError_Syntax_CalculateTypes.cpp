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

	TEST_CASE(L"RuleMixedPartialClauseWithOtherClause 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as partial NumExpr
  ::= ID:value as RefExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleMixedPartialClauseWithOtherClause,L"Exp0" }
			);
	});

	TEST_CASE(L"RuleMixedPartialClauseWithOtherClause 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Ref
  ::= ID:value as RefExpr
  ;
Exp0
  ::= NUM:value as partial NumExpr
  ::= !Ref
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleMixedPartialClauseWithOtherClause,L"Exp0" }
			);
	});

	TEST_CASE(L"RuleWithDifferentPartialTypes")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as partial NumExpr
  ::= ID:value as partial RefExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleWithDifferentPartialTypes,L"Exp0" }
			);
	});

	TEST_CASE(L"RuleCannotResolveToDeterministicType 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ::= "+" as Module
  ::= ID:value as RefExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleCannotResolveToDeterministicType,L"Exp0" }
			);
	});

	TEST_CASE(L"RuleCannotResolveToDeterministicType 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ::= ID:value as RefExpr
  ::= "+" as Module
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleCannotResolveToDeterministicType,L"Exp0" }
			);
	});

	TEST_CASE(L"RuleCannotResolveToDeterministicType 3")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ::= ID:value as RefExpr
  ;
Exp1
  ::= !Exp0
  ::= "+" as Module
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleCannotResolveToDeterministicType,L"Exp1" }
			);
	});

	TEST_CASE(L"ReuseClauseCannotResolveToDeterministicType")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ::= ID:value as RefExpr
  ;
Exp1
  ::= Exp0:export as Module
  ;
Exp2
  ::= !Exp0 !Exp1
  ::= "+" as Module
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ReuseClauseCannotResolveToDeterministicType,L"Exp2" }
			);
	});

	TEST_CASE(L"ReuseClauseContainsNoUseRule")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM
  ::= "+" as Module
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ReuseClauseContainsNoUseRule,L"Exp0" }
			);
	});
}
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

	//////////////////////////////////////////////////////
	// PartialRuleInLeftRecursionInject
	//////////////////////////////////////////////////////

	TEST_CASE(L"PartialRuleInPrefixMerge")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Partial
  ::= NUM:value as partial NumExpr
  ;
Exp0
  ::= !prefix_merge(Partial)
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::PartialRuleInPrefixMerge,L"Exp0",L"Partial"}
			);
	});
}
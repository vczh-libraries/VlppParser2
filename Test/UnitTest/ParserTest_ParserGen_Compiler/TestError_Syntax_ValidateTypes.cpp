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

	TEST_CASE(L"FieldNotExistsInClause 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:unknown as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::FieldNotExistsInClause,L"Exp0",L"NumExpr",L"unknown" }
			);
	});

	TEST_CASE(L"FieldNotExistsInClause 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:unknown as partial NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::FieldNotExistsInClause,L"Exp0",L"NumExpr",L"unknown" }
			);
	});

	TEST_CASE(L"FieldNotExistsInClause 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= "+" as NumExpr
  ;
Exp1
  ::= !Exp0 NUM:unknown
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::FieldNotExistsInClause,L"Exp1",L"NumExpr",L"unknown" }
			);
	});

	TEST_CASE(L"FieldNotExistsInClause 4")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= Exp0:left "+" Exp0:unknown as BinaryExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::FieldNotExistsInClause,L"Exp1",L"BinaryExpr",L"unknown" }
			);
	});

	TEST_CASE(L"FieldNotExistsInClause 5")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= Exp0:left "+" Exp0:unknown as partial BinaryExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::FieldNotExistsInClause,L"Exp1",L"BinaryExpr",L"unknown" }
			);
	});

	TEST_CASE(L"FieldNotExistsInClause 6")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= Exp0:left "+" as BinaryExpr
  ;
Exp2
  ::= !Exp1 Exp0:unknown
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::FieldNotExistsInClause,L"Exp2",L"BinaryExpr",L"unknown" }
			);
	});

	TEST_CASE(L"FieldNotExistsInClause 6")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= "+" as NumExpr {unknown = Add}
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::FieldNotExistsInClause,L"Exp0",L"NumExpr",L"unknown" }
			);
	});

	TEST_CASE(L"RuleTypeMismatchedToField 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Module
  ::= "+" as Module
  ;
Exp0
  ::= Module:value as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleTypeMismatchedToField,L"Exp0",L"NumExpr",L"value",L"Module" }
			);
	});

	TEST_CASE(L"RuleTypeMismatchedToField 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Module
  ::= "+" as Module
  ;
Exp0
  ::= Module:left as BinaryExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleTypeMismatchedToField,L"Exp0",L"BinaryExpr",L"left",L"Module" }
			);
	});

	TEST_CASE(L"RuleTypeMismatchedToField 3")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Module
  ::= "+" as Module
  ;
Exp0
  ::= Module:op as BinaryExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleTypeMismatchedToField,L"Exp0",L"BinaryExpr",L"op",L"Module" }
			);
	});

	TEST_CASE(L"RuleTypeMismatchedToField 4")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:left as BinaryExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleTypeMismatchedToField,L"Exp0",L"BinaryExpr",L"left",L"token" }
			);
	});

	TEST_CASE(L"RuleTypeMismatchedToField 5")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:op as BinaryExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleTypeMismatchedToField,L"Exp0",L"BinaryExpr",L"op",L"token" }
			);
	});

	TEST_CASE(L"AssignmentToNonEnumField 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= "+" as NumExpr {value = Add}
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::AssignmentToNonEnumField,L"Exp0",L"NumExpr",L"value" }
			);
	});

	TEST_CASE(L"AssignmentToNonEnumField 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= "+" as BinaryExpr {left = Add}
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::AssignmentToNonEnumField,L"Exp0",L"BinaryExpr",L"left" }
			);
	});

	TEST_CASE(L"EnumItemMismatchedToField")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= "+" as BinaryExpr {op = unknown}
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::EnumItemMismatchedToField,L"Exp0",L"BinaryExpr",L"op",L"unknown" }
			);
	});

	TEST_CASE(L"UseRuleWithPartialRule")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as partial NumExpr
  ;
Exp1
  ::= !Exp0
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::UseRuleWithPartialRule,L"Exp1",L"Exp0" }
			);
	});

	TEST_CASE(L"UseRuleInNonReuseClause 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= !Exp0 as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::UseRuleInNonReuseClause,L"Exp1",L"Exp0" }
			);
	});

	TEST_CASE(L"UseRuleInNonReuseClause 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= !Exp0 as partial NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::UseRuleInNonReuseClause,L"Exp1",L"Exp0" }
			);
	});

	TEST_CASE(L"PartialRuleUsedOnField")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= "+" as partial BinaryExpr
  ;
Exp1
  ::= Exp0:left as BinaryExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::PartialRuleUsedOnField,L"Exp1",L"BinaryExpr",L"Exp0",L"left" }
			);
	});

	TEST_CASE(L"ClauseTypeMismatchedToPartialRule")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as partial NumExpr
  ;
Exp1
  ::= Exp0 as Module
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ClauseTypeMismatchedToPartialRule,L"Exp1",L"Module",L"Exp0",L"NumExpr" }
			);
	});

	TEST_CASE(L"ClauseTypeMismatchedToPartialRule")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= "+" as partial BinaryExpr
  ;
Exp1
  ::= Exp0 as Expr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ClauseTypeMismatchedToPartialRule,L"Exp1",L"Expr",L"Exp0",L"BinaryExpr" }
			);
	});

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
  ::= !Exp0 left_recursion_inject(Expression) Exp1
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
  ::= !Exp0 left_recursion_inject(Expression) Exp2
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
  ::= !Module left_recursion_inject(Expression) Exp2
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
  ::= !Exp0Partial left_recursion_inject(Expression) Exp1
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
#include "../../../Source/ParserGen_Generated/ParserGenTypeParser.h"
#include "../../../Source/ParserGen_Generated/ParserGenRuleParser.h"
#include "../../../Source/ParserGen/Compiler.h"
#include "../../../Source/Ast/AstCppGen.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::glr;
using namespace vl::glr::automaton;
using namespace vl::glr::parsergen;

extern void AssertError(ParserSymbolManager& global, ParserError expectedError);

namespace TestError_Syntax_TestObjects
{
	extern void ExpectError(TypeParser& typeParser, RuleParser& ruleParser, const WString& astCode, const WString& lexerCode, const WString& syntaxCode, ParserError expectedError);
}
using namespace TestError_Syntax_TestObjects;

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
}
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

class RefExpr
{
	var value : token;
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

	TEST_CASE(L"RuleCannotResolveToDeterministicType 2")
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
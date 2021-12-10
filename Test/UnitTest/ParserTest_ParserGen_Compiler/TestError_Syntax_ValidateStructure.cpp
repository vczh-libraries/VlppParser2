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

	TEST_CASE(L"ClauseNotCreateObject")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= "+" | !Exp0
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ClauseNotCreateObject,L"Exp1" }
			);
	});

	TEST_CASE(L"UseRuleUsedInOptionalBody")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= "(" ["+" !Exp0] ")"
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::UseRuleUsedInOptionalBody,L"Exp1",L"Exp0" }
			);
	});

	TEST_CASE(L"UseRuleUsedInLoopBody 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= "(" {"+" !Exp0} ")"
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::UseRuleUsedInLoopBody,L"Exp1",L"Exp0" }
			);
	});

	TEST_CASE(L"UseRuleUsedInLoopBody 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= "(" {"*"; "+" !Exp0} ")"
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::UseRuleUsedInLoopBody,L"Exp1",L"Exp0" }
			);
	});

	TEST_CASE(L"ClauseTooManyUseRule")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= NUM:value as NumExpr
  ;
Exp2
  ::= NUM:value as NumExpr
  ;
Exp3
  ::= (!Exp0 | !Exp1) !Exp2
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ClauseTooManyUseRule,L"Exp3" }
			);
	});

	TEST_CASE(L"NonArrayFieldAssignedInLoop")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= {NUM:value} as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::NonArrayFieldAssignedInLoop,L"Exp0",L"NumExpr",L"value" }
			);
	});

	TEST_CASE(L"NonLoopablePartialRuleUsedInLoop")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as partial NumExpr
  ;
Exp1
  ::= {Exp0} Exp0 as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::NonLoopablePartialRuleUsedInLoop,L"Exp1",L"NumExpr",L"Exp0" }
			);
	});

	TEST_CASE(L"ClauseCouldExpandToEmptySequence")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= [Exp0:left] [Exp1:left] as BinaryExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::ClauseCouldExpandToEmptySequence,L"Exp1" }
			);
	});

	TEST_CASE(L"LoopBodyCouldExpandToEmptySequence")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= Exp0:func {[Exp0:args]; [Exp1:args]} as CallExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::LoopBodyCouldExpandToEmptySequence,L"Exp1" }
			);
	});

	TEST_CASE(L"OptionalBodyCouldExpandToEmptySequence")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= Exp0:func [[Exp0:args] [Exp1:args]] as CallExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::OptionalBodyCouldExpandToEmptySequence,L"Exp1" }
			);
	});

	TEST_CASE(L"UseRuleAppearAfterField 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= NUM:value !Exp0
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::UseRuleAppearAfterField,L"Exp1",L"Exp0",L"value" }
			);
	});

	TEST_CASE(L"UseRuleAppearAfterField 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= (NUM:value | "+") !Exp0
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::UseRuleAppearAfterField,L"Exp1",L"Exp0",L"value" }
			);
	});

	TEST_CASE(L"UseRuleAppearAfterField 3")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= Exp0:func as CallExpr
  ;
Exp2
  ::= {Exp0:args ; "+"} !Exp1
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::UseRuleAppearAfterField,L"Exp2",L"Exp1",L"args" }
			);
	});

	TEST_CASE(L"UseRuleAppearAfterField 4")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= Exp0:func as CallExpr
  ;
Exp2
  ::= {"+" ; Exp0:args} !Exp1
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::UseRuleAppearAfterField,L"Exp2",L"Exp1",L"args" }
			);
	});

	TEST_CASE(L"UseRuleAppearAfterField 5")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= [NUM:value] !Exp0
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::UseRuleAppearAfterField,L"Exp1",L"Exp0",L"value" }
			);
	});

	TEST_CASE(L"UseRuleAppearAfterPartialRule 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= NUM:value as partial NumExpr
  ;
Exp2
  ::= Exp1 !Exp0
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::UseRuleAppearAfterPartialRule,L"Exp2",L"Exp0",L"Exp1" }
			);
	});

	TEST_CASE(L"UseRuleAppearAfterPartialRule 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= NUM:value as partial NumExpr
  ;
Exp2
  ::= (Exp1 | "+") !Exp0
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::UseRuleAppearAfterPartialRule,L"Exp2",L"Exp0",L"Exp1" }
			);
	});

	TEST_CASE(L"UseRuleAppearAfterPartialRule 3")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= Exp0:args as partial CallExpr
  ;
Exp2
  ::= Exp0:func as CallExpr
  ;
Exp3
  ::= {Exp1 ; "+"} !Exp2
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::UseRuleAppearAfterPartialRule,L"Exp3",L"Exp2",L"Exp1" }
			);
	});

	TEST_CASE(L"UseRuleAppearAfterPartialRule 4")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= Exp0:args as partial CallExpr
  ;
Exp2
  ::= Exp0:func as CallExpr
  ;
Exp3
  ::= {"+" ; Exp1} !Exp2
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::UseRuleAppearAfterPartialRule,L"Exp3",L"Exp2",L"Exp1" }
			);
	});

	TEST_CASE(L"UseRuleAppearAfterPartialRule 5")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value as NumExpr
  ;
Exp1
  ::= NUM:value as partial NumExpr
  ;
Exp2
  ::= [Exp1] !Exp0
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::UseRuleAppearAfterPartialRule,L"Exp2",L"Exp0",L"Exp1" }
			);
	});

	TEST_CASE(L"FieldAssignedMoreThanOnce 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value NUM:value as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::FieldAssignedMoreThanOnce,L"Exp0",L"NumExpr",L"value" }
			);
	});

	TEST_CASE(L"FieldAssignedMoreThanOnce 2")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= NUM:value [NUM:value] as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::FieldAssignedMoreThanOnce,L"Exp0",L"NumExpr",L"value" }
			);
	});

	TEST_CASE(L"FieldAssignedMoreThanOnce 3")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= [NUM:value] "+" [NUM:value] as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::FieldAssignedMoreThanOnce,L"Exp0",L"NumExpr",L"value" }
			);
	});

	TEST_CASE(L"FieldAssignedMoreThanOnce 4")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= (NUM:value | "+") [NUM:value] as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::FieldAssignedMoreThanOnce,L"Exp0",L"NumExpr",L"value" }
			);
	});

	TEST_CASE(L"FieldAssignedMoreThanOnce 5")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= ("+" | NUM:value) [NUM:value] as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::FieldAssignedMoreThanOnce,L"Exp0",L"NumExpr",L"value" }
			);
	});

	TEST_CASE(L"PrioritizedOptionalEndsClause 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0
  ::= "+" -[NUM:value] as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::NegativeOptionalEndsAClause,L"Exp0" }
			);
	});

	TEST_CASE(L"PrioritizedOptionalEndsClause 2")
	{
		const wchar_t* syntaxCode =
			LR"SYNTAX(
Exp0
  ::= ("+" | "*" -[NUM:value]) as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::NegativeOptionalEndsAClause,L"Exp0" }
		);
	});

	TEST_CASE(L"PrioritizedOptionalEndsClause 3")
	{
		const wchar_t* syntaxCode =
			LR"SYNTAX(
Exp0
  ::= NUM:value {"+" -["*"]} as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::NegativeOptionalEndsAClause,L"Exp0" }
		);
	});

	TEST_CASE(L"PrioritizedOptionalEndsClause 4")
	{
		const wchar_t* syntaxCode =
			LR"SYNTAX(
Exp0
  ::= NUM:value {"(" ")" ; "+" -["*"]} as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::NegativeOptionalEndsAClause,L"Exp0" }
		);
	});
}
#include "TestError.h"

namespace TestError_CalculatorAstAndLexer
{
	extern const wchar_t* astCode;
	extern const wchar_t* lexerCode;
}
using namespace TestError_CalculatorAstAndLexer;

TEST_FILE
{
	TypeParser typeParser;
	RuleParser ruleParser;

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

	TEST_CASE(L"RuleExplicitTypeIsNotCompatibleWithClauseType")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 : Module
  ::= NUM:value as NumExpr
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleExplicitTypeIsNotCompatibleWithClauseType,L"Exp0" }
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
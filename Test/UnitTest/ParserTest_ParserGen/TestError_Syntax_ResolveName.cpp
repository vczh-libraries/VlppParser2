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

	TEST_CASE(L"TypeNotExistsInRule 1")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
Exp0 : Unknown ::= NUM:value as NumExpr;
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

	TEST_CASE(L"TypeNotExistsInRule 2")
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

	TEST_CASE(L"ReferencedRuleNotPublicInRuleOfDifferentFile 1")
	{
		const wchar_t* syntaxCodes[] = {
LR"SYNTAX(
Exp0 ::= NUM:value as NumExpr;
)SYNTAX",LR"SYNTAX(
Exp1 ::= Exp0:func as CallExpr;
)SYNTAX" };
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCodes,
			{ ParserErrorType::ReferencedRuleNotPublicInRuleOfDifferentFile,L"Exp1",L"Exp0" }
		);
	});

	TEST_CASE(L"ReferencedRuleNotPublicInRuleOfDifferentFile 2")
	{
		const wchar_t* syntaxCodes[] = {
LR"SYNTAX(
Exp0 ::= NUM:value as NumExpr;
)SYNTAX",LR"SYNTAX(
Exp1 ::= !Exp0;
)SYNTAX" };
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCodes,
			{ ParserErrorType::ReferencedRuleNotPublicInRuleOfDifferentFile,L"Exp1",L"Exp0" }
		);
	});

	TEST_CASE(L"ReferencedRuleNotPublicInRuleOfDifferentFile 3")
	{
		const wchar_t* syntaxCodes[] = {
LR"SYNTAX(
Exp0 ::= NUM:value as NumExpr;
Exp1 ::= left_recursion_placeholder(Expression)
     ::= !Exp0;
Exp2 ::= !Exp1 "+";
)SYNTAX",LR"SYNTAX(
Exp3 ::= !Exp0 left_recursion_inject(Expression) Exp2;
)SYNTAX" };
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCodes,
			{ ParserErrorType::ReferencedRuleNotPublicInRuleOfDifferentFile,L"Exp3",L"Exp0" }
		);
	});

	TEST_CASE(L"ReferencedRuleNotPublicInRuleOfDifferentFile 4")
	{
		const wchar_t* syntaxCodes[] = {
LR"SYNTAX(
Exp0 ::= NUM:value as NumExpr;
)SYNTAX",LR"SYNTAX(
Exp1 ::= !prefix_merge(Exp0);
)SYNTAX" };
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCodes,
			{ ParserErrorType::ReferencedRuleNotPublicInRuleOfDifferentFile,L"Exp1",L"Exp0" }
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
}
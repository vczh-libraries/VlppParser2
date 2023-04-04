#include "TestCppHelper.h"

TEST_FILE
{
	TEST_PARSER(L"TypeOrExpr", L"Ambiguous_EE")
	{
		ParseTypeExpr<CppTypeOrExprToResolve>(parser, line);
	});

	TEST_PARSER(L"TypeOrExpr", L"Ambiguous_EQ")
	{
		ParseTypeExpr<CppTypeOrExprToResolve>(parser, line);
	});

	TEST_PARSER(L"TypeOrExpr", L"Ambiguous_ET")
	{
		ParseTypeExpr<CppTypeOrExprToResolve>(parser, line);
	});

	TEST_PARSER(L"TypeOrExpr", L"Ambiguous_TT")
	{
		ParseTypeExpr<CppTypeOrExprToResolve>(parser, line);
	});

	TEST_PARSER(L"TypeOrExpr", L"LambdaAmbiguous")
	{
		ParseTypeExpr<CppLambdaExpr>(parser, line);
	});

	TEST_PARSER(L"Statements", L"Ambiguous")
	{
		ParseStatement<CppStatementToResolve>(parser, line);
	});
}
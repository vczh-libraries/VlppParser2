#include "TestCppHelper.h"

TEST_FILE
{
	TEST_PARSER(L"TypeOrExpr", L"Identifier")
	{
		ParseTypeExpr<CppQualifiedName>(parser, line);
	});

	TEST_PARSER(L"TypeOrExpr", L"PrimitiveExprs")
	{
		ParseTypeExpr<CppExprOnly>(parser, line);
	});

	TEST_PARSER(L"TypeOrExpr", L"PrimitiveTypes")
	{
		ParseTypeExpr<CppTypeOnly>(parser, line);
	});

	TEST_PARSER(L"TypeOrExpr", L"Types")
	{
		ParseTypeExpr<CppConstType, CppVolatileType>(parser, line);
	});

	TEST_PARSER(L"TypeOrExpr", L"Name")
	{
		ParseTypeExpr<CppQualifiedName>(parser, line);
	});

	TEST_PARSER(L"TypeOrExpr", L"Exprs")
	{
		ParseTypeExpr<CppExprOnly>(parser, line);
	});

	TEST_PARSER(L"TypeOrExpr", L"Declarators_Anonymous")
	{
		ParseTypeExpr<CppDeclaratorType>(parser, line);
	});

	TEST_PARSER(L"TypeOrExpr", L"LambdaExprs")
	{
		ParseTypeExpr<CppLambdaExpr>(parser, line);
	});
}
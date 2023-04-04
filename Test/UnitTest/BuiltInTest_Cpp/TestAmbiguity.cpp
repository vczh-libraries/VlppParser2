#include "TestCppHelper.h"

TEST_FILE
{
	TEST_PARSER(L"TypeOrExpr", L"Ambiguous_EE")
	{
		ParseAmbiguousTypeExpr<CppTypeOrExprToResolve, CppExprOnly>(parser, line);
	});

	TEST_PARSER(L"TypeOrExpr", L"Ambiguous_EQ")
	{
		ParseAmbiguousTypeExpr<CppTypeOrExprToResolve, CppExprOnly, CppQualifiedName>(parser, line);
	});

	TEST_PARSER(L"TypeOrExpr", L"Ambiguous_ET")
	{
		ParseAmbiguousTypeExpr<CppTypeOrExprToResolve, CppExprOnly, CppDeclaratorType>(parser, line);
	});

	TEST_PARSER(L"Statements", L"Ambiguous")
	{
		ParseAmbiguousStatement<CppStatementToResolve, CppExprStat, CppDeclStat>(parser, line);
	});
}
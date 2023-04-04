#include "TestCppHelper.h"

TEST_FILE
{
	TEST_PARSER(L"Statements", L"SingleLevel")
	{
		ParseStatement<CppStatement>(parser, line);
	});
}
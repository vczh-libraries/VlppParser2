#include "TestCppHelper.h"

using TExecutor = void(*)(void);
extern void SetRunBeforeTests(TExecutor value);
extern void SetRunAfterTests(TExecutor value);

cpp_parser::Parser* cppParser = nullptr;

cpp_parser::Parser& GetCppParser()
{
	return *cppParser;
}

struct CppParserRegister
{
	CppParserRegister()
	{
		SetRunBeforeTests([]()
		{
			cppParser = new cpp_parser::Parser;
		});
		
		SetRunAfterTests([]()
		{
			delete cppParser;
			cppParser = nullptr;
		});
	}
} cppParserRegister;
#include "../../../Source/Ast/AstSymbol.h"

using namespace vl;
using namespace vl::glr::parsergen;

extern WString GetExePath();

TEST_FILE
{
	TEST_CASE(L"CreateParserGenAst")
	{
		AstSymbolManager manager;
		CreateParserGenAst(manager);
		TEST_ASSERT(manager.Errors().Count() == 0);
	});
}
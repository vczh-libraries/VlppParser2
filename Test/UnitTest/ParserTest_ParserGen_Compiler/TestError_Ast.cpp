#include "../../../Source/ParserGen_Generated/ParserGenTypeParser.h"
#include "../../../Source/ParserGen/Compiler.h"

using namespace vl;
using namespace vl::glr::parsergen;

namespace TestError_Ast_TestObjects
{
	void ExpectError(const WString& astCode, ParserError error)
	{
	}
}
using namespace TestError_Ast_TestObjects;

TEST_FILE
{
	TEST_CASE(L"DuplicatedFile")
	{
	});

	TEST_CASE(L"FileDependencyNotExists")
	{
	});

	TEST_CASE(L"FileCyclicDependency")
	{
	});

	TEST_CASE(L"DuplicatedSymbol")
	{
	});

	TEST_CASE(L"DuplicatedSymbolGlobally")
	{
	});

	TEST_CASE(L"DuplicatedClassProp")
	{
	});

	TEST_CASE(L"DuplicatedEnumItem")
	{
	});

	TEST_CASE(L"BaseClassNotExists")
	{
	});

	TEST_CASE(L"BaseClassNotClass")
	{
	});

	TEST_CASE(L"BaseClassCyclicDependency")
	{
	});

	TEST_CASE(L"FieldTypeNotExists")
	{
	});

	TEST_CASE(L"FieldTypeNotClass")
	{
	});
}
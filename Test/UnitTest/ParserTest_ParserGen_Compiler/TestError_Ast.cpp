#include "../../../Source/ParserGen_Generated/ParserGenTypeParser.h"
#include "../../../Source/ParserGen/Compiler.h"

using namespace vl;
using namespace vl::glr;
using namespace vl::glr::parsergen;

namespace TestError_Ast_TestObjects
{
	void ExpectError(const WString& astCode, ParserError expectedError)
	{
	}
}
using namespace TestError_Ast_TestObjects;

TEST_FILE
{
	TEST_CASE(L"DuplicatedFile")
	{
		ParserSymbolManager global;
		AstSymbolManager astManager(global);
		astManager.CreateFile(L"Ast");
		astManager.CreateFile(L"Ast");

		TEST_ASSERT(global.Errors().Count() == 1);
		auto&& error = global.Errors()[0];
		TEST_ASSERT(error.type == ParserErrorType::DuplicatedFile);
		TEST_ASSERT(error.arg1 == L"Ast");
	});

	TEST_CASE(L"FileDependencyNotExists")
	{
		ParserSymbolManager global;
		AstSymbolManager astManager(global);
		astManager.CreateFile(L"Ast")->AddDependency(L"Random");

		TEST_ASSERT(global.Errors().Count() == 1);
		auto&& error = global.Errors()[0];
		TEST_ASSERT(error.type == ParserErrorType::FileDependencyNotExists);
		TEST_ASSERT(error.arg1 == L"Ast");
		TEST_ASSERT(error.arg2 == L"Random");
	});

	TEST_CASE(L"FileCyclicDependency")
	{
		ParserSymbolManager global;
		AstSymbolManager astManager(global);
		auto f1 = astManager.CreateFile(L"A");
		auto f2 = astManager.CreateFile(L"B");
		f1->AddDependency(L"B");
		f2->AddDependency(L"A");

		TEST_ASSERT(global.Errors().Count() == 1);
		auto&& error = global.Errors()[0];
		TEST_ASSERT(error.type == ParserErrorType::FileCyclicDependency);
		TEST_ASSERT(error.arg1 == L"B");
		TEST_ASSERT(error.arg2 == L"A");
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
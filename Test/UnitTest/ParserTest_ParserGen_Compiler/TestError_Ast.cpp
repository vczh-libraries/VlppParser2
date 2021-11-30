#include "../../../Source/ParserGen_Generated/ParserGenTypeParser.h"
#include "../../../Source/ParserGen/Compiler.h"

using namespace vl;
using namespace vl::glr;
using namespace vl::glr::parsergen;

namespace TestError_Ast_TestObjects
{
	AstDefFile* CompileAstCode(TypeParser& parser, AstSymbolManager& astManager, const WString& astName, const WString& astCode)
	{
		auto astFile = parser.ParseFile(astCode);
		auto astDefFile = astManager.CreateFile(astName);
		CompileAst(astManager, astDefFile, astFile);
		return astDefFile;
	}

	void AssertError(ParserSymbolManager& global, ParserError expectedError)
	{
		TEST_ASSERT(global.Errors().Count() == 1);
		auto&& error = global.Errors()[0];
		TEST_ASSERT(error.type == expectedError.type);
		TEST_ASSERT(error.arg1 == expectedError.arg1);
		TEST_ASSERT(error.arg2 == expectedError.arg2);
		TEST_ASSERT(error.arg3 == expectedError.arg3);
	}

	void ExpectError(TypeParser& parser, const WString& astCode, ParserError expectedError)
	{
		ParserSymbolManager global;
		AstSymbolManager astManager(global);
		CompileAstCode(parser, astManager, L"Ast", astCode);
		AssertError(global, expectedError);
	}
}
using namespace TestError_Ast_TestObjects;

TEST_FILE
{
	TypeParser parser;

	TEST_CASE(L"DuplicatedFile")
	{
		ParserSymbolManager global;
		AstSymbolManager astManager(global);
		astManager.CreateFile(L"Ast");
		astManager.CreateFile(L"Ast");
		AssertError(global, { ParserErrorType::DuplicatedFile,L"Ast" });
	});

	TEST_CASE(L"FileDependencyNotExists")
	{
		ParserSymbolManager global;
		AstSymbolManager astManager(global);
		astManager.CreateFile(L"Ast")->AddDependency(L"Random");
		AssertError(global, { ParserErrorType::FileDependencyNotExists,L"Ast",L"Random"});
	});

	TEST_CASE(L"FileCyclicDependency")
	{
		ParserSymbolManager global;
		AstSymbolManager astManager(global);
		auto f1 = astManager.CreateFile(L"A");
		auto f2 = astManager.CreateFile(L"B");
		f1->AddDependency(L"B");
		f2->AddDependency(L"A");
		AssertError(global, { ParserErrorType::FileCyclicDependency,L"B",L"A" });
	});

	TEST_CASE(L"DuplicatedSymbol")
	{
		ExpectError(parser, LR"AST(
			)AST",
			{ ParserErrorType::DuplicatedSymbol,L"A" }
			);
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
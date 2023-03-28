#include "../../Source/BuiltIn-Cpp/Generated/CppParser.h"
#include "../../Source/BuiltIn-Cpp/Generated/CppAst_Json.h"
#include "../../Source/LogTrace.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::filesystem;
using namespace vl::regex;
using namespace cpp_parser;

extern WString GetTestParserInputPath(const WString& parserName);
extern FilePath GetOutputDir(const WString& parserName);

template<typename ...TArgs>
struct AssertPtrStruct
{
};

template<>
struct AssertPtrStruct<>
{
	static void AssertPtr(glr::ParsingAstBase* ast)
	{
		TEST_ASSERT(false);
	}
};

template<typename T, typename ...TArgs>
struct AssertPtrStruct<T, TArgs...>
{
	static void AssertPtr(glr::ParsingAstBase* ast)
	{
		if (!dynamic_cast<T*>(ast)) AssertPtrStruct<TArgs...>::AssertPtr(ast);
	}
};

template<typename ...TArgs>
void ParseTypeExpr(cpp_parser::Parser& parser, const WString& code)
{
	auto ast = parser.Parse_TypeOrExpr(code);
	TEST_ASSERT(ast);
	AssertPtrStruct<TArgs...>::AssertPtr(ast.Obj());
}

template<typename ...TArgs>
void ParseStatement(cpp_parser::Parser& parser, const WString& code)
{
	auto ast = parser.Parse_Stat(code);
	TEST_ASSERT(ast);
	AssertPtrStruct<TArgs...>::AssertPtr(ast.Obj());
}
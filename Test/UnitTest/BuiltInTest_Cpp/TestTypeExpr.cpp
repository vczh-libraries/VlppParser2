#include "../../Source/BuiltIn-Cpp/Generated/CppParser.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::filesystem;
using namespace vl::regex;
using namespace cpp_parser;

extern WString GetTestParserInputPath(const WString& parserName);

template<typename ...TArgs>
struct AssertPtrStruct
{
};

template<>
struct AssertPtrStruct<>
{
	static void AssertPtr(CppTypeOrExpr* ast)
	{
		TEST_ASSERT(false);
	}
};

template<typename T, typename ...TArgs>
struct AssertPtrStruct<T, TArgs...>
{
	static void AssertPtr(CppTypeOrExpr* ast)
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

TEST_FILE
{
	cpp_parser::Parser parser;

	TEST_CATEGORY(L"Identifier")
	{
		List<WString> lines;
		File(
			FilePath(GetTestParserInputPath(L"BuiltIn-Cpp"))
			/ L"Input"
			/ L"TypeOrExpr_Identifier.txt"
		).ReadAllLinesByBom(lines);
		for (auto&& line : lines)
		{
			if (line != L"")
			{
				TEST_CASE(line)
				{
					ParseTypeExpr<CppQualifiedName>(parser, line);
				});
			}
		}
	});

	TEST_CATEGORY(L"PrimitiveExprs")
	{
		List<WString> lines;
		File(
			FilePath(GetTestParserInputPath(L"BuiltIn-Cpp"))
			/ L"Input"
			/ L"TypeOrExpr_PrimitiveExprs.txt"
		).ReadAllLinesByBom(lines);
		for (auto&& line : lines)
		{
			if (line != L"")
			{
				TEST_CASE(line)
				{
					ParseTypeExpr<CppExprOnly>(parser, line);
				});
			}
		}
	});

	TEST_CATEGORY(L"PrimitiveTypes")
	{
		List<WString> lines;
		File(
			FilePath(GetTestParserInputPath(L"BuiltIn-Cpp"))
			/ L"Input"
			/ L"TypeOrExpr_PrimitiveTypes.txt"
		).ReadAllLinesByBom(lines);
		for (auto&& line : lines)
		{
			if (line != L"")
			{
				TEST_CASE(line)
				{
					ParseTypeExpr<CppTypeOnly>(parser, line);
				});
			}
		}
	});

	TEST_CATEGORY(L"Types")
	{
		List<WString> lines;
		File(
			FilePath(GetTestParserInputPath(L"BuiltIn-Cpp"))
			/ L"Input"
			/ L"TypeOrExpr_Types.txt"
		).ReadAllLinesByBom(lines);
		for (auto&& line : lines)
		{
			if (line != L"")
			{
				TEST_CASE(line)
				{
					ParseTypeExpr<CppConstType, CppVolatileType>(parser, line);
				});
			}
		}
	});

	TEST_CATEGORY(L"Name")
	{
		List<WString> lines;
		File(
			FilePath(GetTestParserInputPath(L"BuiltIn-Cpp"))
			/ L"Input"
			/ L"TypeOrExpr_Name.txt"
		).ReadAllLinesByBom(lines);
		for (auto&& line : lines)
		{
			if (line != L"")
			{
				TEST_CASE(line)
				{
					ParseTypeExpr<CppQualifiedName>(parser, line);
				});
			}
		}
	});

	TEST_CATEGORY(L"Exprs")
	{
		List<WString> lines;
		File(
			FilePath(GetTestParserInputPath(L"BuiltIn-Cpp"))
			/ L"Input"
			/ L"TypeOrExpr_Exprs.txt"
		).ReadAllLinesByBom(lines);
		for (auto&& line : lines)
		{
			if (line != L"")
			{
				TEST_CASE(line)
				{
					ParseTypeExpr<CppExprOnly>(parser, line);
				});
			}
		}
	});

	TEST_CATEGORY(L"Declarators_Anonymous")
	{
		List<WString> lines;
		File(
			FilePath(GetTestParserInputPath(L"BuiltIn-Cpp"))
			/ L"Input"
			/ L"TypeOrExpr_Declarators_Anonymous.txt"
		).ReadAllLinesByBom(lines);
		for (auto&& line : lines)
		{
			if (line != L"")
			{
				TEST_CASE(line)
				{
					ParseTypeExpr<CppDeclaratorType>(parser, line);
				});
			}
		}
	});

	TEST_CATEGORY(L"LambdaExprs")
	{
		List<WString> lines;
		File(
			FilePath(GetTestParserInputPath(L"BuiltIn-Cpp"))
			/ L"Input"
			/ L"TypeOrExpr_LambdaExprs.txt"
		).ReadAllLinesByBom(lines);
		for (auto&& line : lines)
		{
			if (line != L"")
			{
				//TEST_CASE(line)
				//{
				//});
			}
		}
	});

	TEST_CATEGORY(L"Ambiguous")
	{
		List<WString> lines;
		File(
			FilePath(GetTestParserInputPath(L"BuiltIn-Cpp"))
			/ L"Input"
			/ L"TypeOrExpr_Ambiguous.txt"
		).ReadAllLinesByBom(lines);
		for (auto&& line : lines)
		{
			if (line != L"")
			{
				//TEST_CASE(line)
				//{
				//});
			}
		}
	});
}
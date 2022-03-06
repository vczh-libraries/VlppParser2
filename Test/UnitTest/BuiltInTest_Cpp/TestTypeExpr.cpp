#include "../../Source/BuiltIn-Cpp/Generated/CppParser.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::filesystem;
using namespace vl::regex;
using namespace cpp_parser;

extern WString GetTestParserInputPath(const WString& parserName);

template<typename T>
Ptr<T> ParseTypeExpr(cpp_parser::Parser& parser, const WString& code)
{
	auto ast = parser.Parse_TypeOrExpr(code);
	TEST_ASSERT(ast);
	auto typed = ast.Cast<T>();
	TEST_ASSERT(typed);
	return typed;
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
				//TEST_CASE(line)
				//{
				//});
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
				//TEST_CASE(line)
				//{
				//});
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
				//TEST_CASE(line)
				//{
				//});
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
				//TEST_CASE(line)
				//{
				//});
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
				//TEST_CASE(line)
				//{
				//});
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
}
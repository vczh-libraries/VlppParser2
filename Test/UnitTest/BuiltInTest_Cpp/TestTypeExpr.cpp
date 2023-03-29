#include "TestCppHelper.h"

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
				TEST_CASE(line)
				{
					ParseTypeExpr<CppLambdaExpr>(parser, line);
				});
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
				//	ParseTypeExpr<CppTypeOrExpr>(parser, line);
				//});
			}
		}
	});

	TEST_CATEGORY(L"LambdaAmbiguous")
	{
		List<WString> lines;
		File(
			FilePath(GetTestParserInputPath(L"BuiltIn-Cpp"))
			/ L"Input"
			/ L"TypeOrExpr_LambdaAmbiguous.txt"
		).ReadAllLinesByBom(lines);
		for (auto&& line : lines)
		{
			if (line != L"")
			{
				//TEST_CASE(line)
				//{
				//	ParseTypeExpr<CppExprOnly>(parser, line);
				//});
			}
		}
	});
}
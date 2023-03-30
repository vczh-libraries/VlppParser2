#include "TestCppHelper.h"

extern cpp_parser::Parser& GetCppParser();

TEST_FILE
{
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
					ParseTypeExpr<CppQualifiedName>(GetCppParser(), line);
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
					ParseTypeExpr<CppExprOnly>(GetCppParser(), line);
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
					ParseTypeExpr<CppTypeOnly>(GetCppParser(), line);
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
					ParseTypeExpr<CppConstType, CppVolatileType>(GetCppParser(), line);
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
					ParseTypeExpr<CppQualifiedName>(GetCppParser(), line);
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
					ParseTypeExpr<CppExprOnly>(GetCppParser(), line);
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
					ParseTypeExpr<CppDeclaratorType>(GetCppParser(), line);
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
					ParseTypeExpr<CppLambdaExpr>(GetCppParser(), line);
				});
			}
		}
	});
}
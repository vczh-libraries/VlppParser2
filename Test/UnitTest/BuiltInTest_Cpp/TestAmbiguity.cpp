#include "TestCppHelper.h"

extern cpp_parser::Parser& GetCppParser();

TEST_FILE
{
	TEST_CATEGORY(L"Ambiguous (EE)")
	{
		List<WString> lines;
		File(
			FilePath(GetTestParserInputPath(L"BuiltIn-Cpp"))
			/ L"Input"
			/ L"TypeOrExpr_Ambiguous_EE.txt"
		).ReadAllLinesByBom(lines);
		for (auto&& line : lines)
		{
			if (line != L"")
			{
				//TEST_CASE(line)
				//{
				//	ParseTypeExpr<CppTypeOrExpr>(GetCppParser(), line);
				//});
			}
		}
	});

	TEST_CATEGORY(L"Ambiguous (EQ)")
	{
		List<WString> lines;
		File(
			FilePath(GetTestParserInputPath(L"BuiltIn-Cpp"))
			/ L"Input"
			/ L"TypeOrExpr_Ambiguous_EQ.txt"
		).ReadAllLinesByBom(lines);
		for (auto&& line : lines)
		{
			if (line != L"")
			{
				//TEST_CASE(line)
				//{
				//	ParseTypeExpr<CppTypeOrExpr>(GetCppParser(), line);
				//});
			}
		}
	});

	TEST_CATEGORY(L"Ambiguous (ET)")
	{
		List<WString> lines;
		File(
			FilePath(GetTestParserInputPath(L"BuiltIn-Cpp"))
			/ L"Input"
			/ L"TypeOrExpr_Ambiguous_ET.txt"
		).ReadAllLinesByBom(lines);
		for (auto&& line : lines)
		{
			if (line != L"")
			{
				//TEST_CASE(line)
				//{
				//	ParseTypeExpr<CppTypeOrExpr>(GetCppParser(), line);
				//});
			}
		}
	});

	TEST_CATEGORY(L"Ambiguous (TT)")
	{
		List<WString> lines;
		File(
			FilePath(GetTestParserInputPath(L"BuiltIn-Cpp"))
			/ L"Input"
			/ L"TypeOrExpr_Ambiguous_TT.txt"
		).ReadAllLinesByBom(lines);
		for (auto&& line : lines)
		{
			if (line != L"")
			{
				//TEST_CASE(line)
				//{
				//	ParseTypeExpr<CppTypeOrExpr>(GetCppParser(), line);
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
				//	ParseTypeExpr<CppExprOnly>(GetCppParser(), line);
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
			/ L"Statements_Ambiguous.txt"
		).ReadAllLinesByBom(lines);
		for (auto&& line : lines)
		{
			if (line != L"")
			{
				//TEST_CASE(line)
				//{
				//	ParseStatement<CppStatement>(GetCppParser(), line);
				//});
			}
		}
	});
}
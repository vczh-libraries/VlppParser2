#include "TestCppHelper.h"

extern cpp_parser::Parser& GetCppParser();

TEST_FILE
{
	TEST_CATEGORY(L"SingleLevel")
	{
		List<WString> lines;
		File(
			FilePath(GetTestParserInputPath(L"BuiltIn-Cpp"))
			/ L"Input"
			/ L"Statements_SingleLevel.txt"
		).ReadAllLinesByBom(lines);
		for (auto&& line : lines)
		{
			if (line != L"")
			{
				TEST_CASE(line)
				{
					ParseStatement<CppStatement>(GetCppParser(), line);
				});
			}
		}
	});
}
#include "TestCppHelper.h"

TEST_FILE
{
	return;
	cpp_parser::Parser parser;

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
					ParseStatement<CppStatement>(parser, line);
				});
			}
		}
	});
}
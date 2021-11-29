#include "../../Source/Calculator/Generated/CalculatorExprAst_Json.h"
#include "../../Source/Calculator/Generated/CalculatorModuleParser.h"
#include "../../Source/LogAutomaton.h"

using namespace calculator;

extern WString GetTestParserInputPath(const WString& parserName);
extern FilePath GetOutputDir(const WString& parserName);

TEST_FILE
{
	ModuleParser parser;

	TEST_CATEGORY(L"Test Calculator Syntax")
	{
		Folder dirInput = FilePath(GetTestParserInputPath(L"Calculator")) / L"Input";
		FilePath dirBaseline = FilePath(GetTestParserInputPath(L"Calculator")) / L"Output";
		FilePath dirOutput = GetOutputDir(L"Calculator");

		List<File> inputFiles;
		dirInput.GetFiles(inputFiles);
		for (auto&& inputFile : inputFiles)
		{
			auto caseName = inputFile.GetFilePath().GetName();
			caseName = caseName.Left(caseName.Length() - 4);

			TEST_CASE(caseName)
			{
				auto input = inputFile.ReadAllTextByBom();
				auto ast = parser.ParseModule(input);
				auto actualJson = PrintAstJson<json_visitor::ExprAstVisitor>(ast);
				auto expectedJson = File(dirBaseline / (caseName + L".json")).ReadAllTextByBom();
				AssertLines(expectedJson, actualJson);
			});
		}
	});

#undef LEXER
}
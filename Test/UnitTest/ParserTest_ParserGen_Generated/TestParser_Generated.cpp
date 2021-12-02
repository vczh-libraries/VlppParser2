#include "../../Source/Calculator/Generated/CalculatorExprAst_Json.h"
#include "../../Source/Calculator/Generated/CalculatorModuleParser.h"
#include "../../Source/LogAutomaton.h"

extern WString GetTestParserInputPath(const WString& parserName);
extern FilePath GetOutputDir(const WString& parserName);

namespace TestParser_Generated_TestObjects
{
	template<typename TParser, typename TJsonVisitor>
	void TestParser(const WString& parserName)
	{
		TParser parser;

		TEST_CATEGORY(L"Test " + parserName + L" Syntax")
		{
			Folder dirInput = FilePath(GetTestParserInputPath(parserName)) / L"Input";
			FilePath dirBaseline = FilePath(GetTestParserInputPath(parserName)) / L"Output";
			FilePath dirOutput = GetOutputDir(parserName);

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
					auto actualJson = PrintAstJson<TJsonVisitor>(ast);
					auto expectedJson = File(dirBaseline / (caseName + L".json")).ReadAllTextByBom();
					AssertLines(expectedJson, actualJson);
				});
			}
		});
	}
}
using namespace TestParser_Generated_TestObjects;

TEST_FILE
{
	TestParser<calculator::ModuleParser, calculator::json_visitor::ExprAstVisitor>(L"Calculator");
}
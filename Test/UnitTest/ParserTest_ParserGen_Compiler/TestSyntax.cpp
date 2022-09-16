#include "../../Source/Calculator/Parser/CalculatorExprAst_Json.h"
#include "../../Source/Calculator/Parser/CalculatorModuleParser.h"
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
		FilePath dirBaseline = FilePath(GetTestParserInputPath(L"Calculator")) / L"ParserLog";
		FilePath dirOutput = GetOutputDir(L"Calculator");

		List<File> inputFiles;
		dirInput.GetFiles(inputFiles);
		for (auto&& inputFile : inputFiles)
		{
			auto caseName = inputFile.GetFilePath().GetName();
			caseName = caseName.Left(caseName.Length() - 4);

			TEST_CASE(caseName)
			{
				List<ParsingError> errors;
				auto input = inputFile.ReadAllTextByBom();

				auto handler = InstallDefaultErrorMessageGenerator(parser, errors);
				auto ast = parser.ParseModule(input);
				parser.OnError.Remove(handler);

				TEST_ASSERT(ast);
				TEST_ASSERT(errors.Count() == 0);

				auto actualJson = PrintAstJson<json_visitor::ExprAstVisitor>(ast);
				auto expectedJson = File(dirBaseline / (caseName + L".json")).ReadAllTextByBom();
				AssertLines(expectedJson, actualJson);
			});
		}
	});
}
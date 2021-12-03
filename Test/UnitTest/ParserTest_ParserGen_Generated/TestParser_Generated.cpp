#include "../../Source/Calculator/Generated/CalculatorExprAst_Json.h"
#include "../../Source/Calculator/Generated/CalculatorModuleParser.h"
#include "../../Source/LogTrace.h"

extern WString GetTestParserInputPath(const WString& parserName);
extern FilePath GetOutputDir(const WString& parserName);

namespace TestParser_Generated_TestObjects
{
	template<
		typename TParser,
		typename TJsonVisitor,
		typename TStates,
		typename TClasses,
		typename TFields,
		typename TTokens
		>
	void TestParser(
		const WString& parserName,
		TStates startState,
		const wchar_t* (*typeName)(TClasses),
		const wchar_t* (*fieldName)(TFields),
		const wchar_t* (*tokenId)(TTokens)
		)
	{
		TParser parser;

		TEST_CATEGORY(L"Test " + parserName + L" Syntax")
		{
			Folder dirInput = FilePath(GetTestParserInputPath(parserName)) / L"Input";
			FilePath dirBaseline = FilePath(GetTestParserInputPath(parserName)) / L"Output";
			FilePath dirOutput = GetOutputDir(L"Generated-" + parserName);

			List<File> inputFiles;
			dirInput.GetFiles(inputFiles);
			for (auto&& inputFile : inputFiles)
			{
				auto caseName = inputFile.GetFilePath().GetName();
				caseName = caseName.Left(caseName.Length() - 4);

				TEST_CASE(caseName)
				{
					auto input = inputFile.ReadAllTextByBom();
					
					LogTraceExecution(
						L"Generated-" + parserName,
						caseName,
						[=](vint32_t type) { return WString::Unmanaged(typeName((TClasses)type)); },
						[=](vint32_t field) { return WString::Unmanaged(fieldName((TFields)field)); },
						[=](vint32_t token) { return WString::Unmanaged(tokenId((TTokens)token)); },
						[&](IAstInsReceiver& receiver)
						{
							parser.ParseWithReceiver(input, startState, receiver, -1);
						});

					auto ast = parser.ParseModule(input);
					auto actualJson = PrintAstJson<TJsonVisitor>(ast);
					File(dirOutput / (L"Output[" + caseName + L"].json")).WriteAllText(actualJson, true, BomEncoder::Utf8);

					auto expectedJsonFile = File(dirBaseline / (caseName + L".json"));
					if (!expectedJsonFile.Exists()) return;
					auto expectedJson = expectedJsonFile.ReadAllTextByBom();
					AssertLines(expectedJson, actualJson);
				});
			}
		});
	}
}
using namespace TestParser_Generated_TestObjects;

TEST_FILE
{
	TestParser<calculator::ModuleParser, calculator::json_visitor::ExprAstVisitor>(
		L"Calculator",
		calculator::ModuleParserStates::Module,
		&calculator::CalculatorTypeName,
		&calculator::CalculatorFieldName,
		&calculator::CalculatorTokenId
		);
}
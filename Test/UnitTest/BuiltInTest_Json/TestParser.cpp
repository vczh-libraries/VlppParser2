#include "../../../Source/Json/Generated/JsonParser.h"
#include "../../../Source/Json/Generated/JsonAst_Json.h"
#include "../../Source/LogTrace.h"

using namespace vl::glr::json;

extern WString GetTestParserInputPath(const WString& parserName);
extern FilePath GetOutputDir(const WString& parserName);

TEST_FILE
{
	json::Parser parser;
	WString parserName;
	WString caseName;

#if !defined _DEBUG || defined NDEBUG
	parser.OnEndOfInput.Add(
		[&](List<RegexToken>& tokens, Executable& executable, TraceManager& tm, Trace* rootTrace)
		{
			LogTraceManager(
				L"BuiltIn-Json",
				parserName + L"_" + caseName,
				executable,
				tm,
				rootTrace,
				tokens,
				[=](vint32_t type) { return WString::Unmanaged(JsonTypeName((JsonClasses)type)); },
				[=](vint32_t field) { return WString::Unmanaged(JsonFieldName((JsonFields)field)); },
				[=](vint32_t token) { return WString::Unmanaged(JsonTokenId((JsonTokens)token)); },
				[=](vint32_t rule) { return WString::Unmanaged(ParserRuleName(rule)); },
				[=](vint32_t state) { return WString::Unmanaged(ParserStateLabel(state)); }
			);

			if (tm.concurrentCount == 1)
			{
				LogTraceExecution(
					L"BuiltIn-Json",
					parserName + L"_" + caseName,
					[=](vint32_t type) { return WString::Unmanaged(JsonTypeName((JsonClasses)type)); },
					[=](vint32_t field) { return WString::Unmanaged(JsonFieldName((JsonFields)field)); },
					[=](vint32_t token) { return WString::Unmanaged(JsonTokenId((JsonTokens)token)); },
					[&](IAstInsReceiver& receiver)
					{
						tm.ExecuteTrace(rootTrace, receiver, tokens);
					});
			}
		});
#endif

	List<WString> parserNames;
	parserNames.Add(L"Calculator");
	parserNames.Add(L"IfElseAmbiguity");
	parserNames.Add(L"IfElsePriority");
	parserNames.Add(L"GenericAmbiguity");
	parserNames.Add(L"FeatureTest");

	for (vint i = 0; i < parserNames.Count(); i++)
	{
		parserName = parserNames[i];
		TEST_CATEGORY(L"Test JSON on Outputs: " + parserName)
		{
			Folder dirInput = FilePath(GetTestParserInputPath(parserName)) / L"Input";
			FilePath dirBaseline = FilePath(GetTestParserInputPath(parserName)) / L"Output";
			FilePath dirOutput = GetOutputDir(L"BuiltIn-Json");

			List<File> inputFiles;
			dirInput.GetFiles(inputFiles);
			for (auto&& inputFile : inputFiles)
			{
				caseName = inputFile.GetFilePath().GetName();
				if (caseName.Length() < 4 || caseName.Right(4) != L".txt") continue;
				caseName = caseName.Left(caseName.Length() - 4);

				TEST_CASE(caseName)
				{
					auto baselineJsonFile = File(dirBaseline / (caseName + L".json"));
					auto baselineJson = baselineJsonFile.ReadAllTextByBom();
					auto ast = parser.ParseJRoot(baselineJson);
					auto astJson = PrintAstJson<json_visitor::AstVisitor>(ast);
					File(dirOutput / (L"Output[" + parserName + L"_" + caseName + L"].json")).WriteAllText(astJson, true, BomEncoder::Utf8);
				});
			}
		});
	}
}
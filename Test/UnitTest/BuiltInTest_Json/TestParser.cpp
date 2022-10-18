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
	parser.OnTraceProcessing.Add(
		[&](TraceProcessingArgs& args)
		{
			auto& traceManager = *dynamic_cast<TraceManager*>(args.executor);
			LogTraceManager(
				L"BuiltIn-Json",
				parserName + L"_" + caseName,
				args.executable,
				traceManager,
				args.phase,
				args.tokens,
				[=](vint32_t type) { return WString::Unmanaged(JsonTypeName((JsonClasses)type)); },
				[=](vint32_t field) { return WString::Unmanaged(JsonFieldName((JsonFields)field)); },
				[=](vint32_t token) { return WString::Unmanaged(JsonTokenId((JsonTokens)token)); },
				[=](vint32_t rule) { return WString::Unmanaged(ParserRuleName(rule)); },
				[=](vint32_t state) { return WString::Unmanaged(ParserStateLabel(state)); },
				[=](vint32_t switchId) { return WString::Unmanaged(ParserSwitchName(switchId)); }
			);
		});
	parser.OnReadyToExecute.Add(
		[&](ReadyToExecuteArgs& args)
		{
			auto& traceManager = *dynamic_cast<TraceManager*>(args.executor);
			LogTraceExecution(
				L"BuiltIn-Json",
				parserName + L"_" + caseName,
				[=](vint32_t type) { return WString::Unmanaged(JsonTypeName((JsonClasses)type)); },
				[=](vint32_t field) { return WString::Unmanaged(JsonFieldName((JsonFields)field)); },
				[=](vint32_t token) { return WString::Unmanaged(JsonTokenId((JsonTokens)token)); },
				[&](IAstInsReceiver& receiver)
				{
					traceManager.ExecuteTrace(receiver, args.tokens);
				});
		});
#endif

	List<Folder> parserFolders;
	Folder(GetTestParserInputPath(parserName)).GetFolders(parserFolders);

	for (auto parserFolder : parserFolders)
	{
		Folder dirBaseline = parserFolder.GetFilePath() / L"Output";
		if (!dirBaseline.Exists()) continue;

		TEST_CATEGORY(L"Test JSON on Outputs: " + parserName)
		{
			FilePath dirOutput = GetOutputDir(L"BuiltIn-Json");

			List<File> jsonFiles;
			dirBaseline.GetFiles(jsonFiles);
			for (auto&& jsonFile : jsonFiles)
			{
				caseName = jsonFile.GetFilePath().GetName();
				if (caseName.Length() < 5 || caseName.Right(5) != L".json") continue;
				caseName = caseName.Left(caseName.Length() - 5);

				TEST_CASE(caseName)
				{
					auto baselineJson = jsonFile.ReadAllTextByBom();
					auto ast = parser.ParseJRoot(baselineJson);
					auto astJson = PrintAstJson<json_visitor::AstVisitor>(ast);
					File(dirOutput / (L"Output[" + parserName + L"_" + caseName + L"].json")).WriteAllText(astJson, true, BomEncoder::Utf8);
				});
			}
		});
	}
}
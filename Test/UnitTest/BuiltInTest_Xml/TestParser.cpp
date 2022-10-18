#include "../../../Source/Xml/Generated/XmlParser.h"
#include "../../../Source/Xml/Generated/XmlAst_Json.h"
#include "../../Source/LogTrace.h"

using namespace vl::glr::xml;

extern WString GetTestParserInputPath(const WString& parserName);
extern FilePath GetOutputDir(const WString& parserName);

TEST_FILE
{
	xml::Parser parser;
	WString caseName;

#if !defined _DEBUG || defined NDEBUG
	parser.OnTraceProcessing.Add(
		[&](TraceProcessingArgs& args)
		{
			auto& traceManager = *dynamic_cast<TraceManager*>(args.executor);
			LogTraceManager(
				L"BuiltIn-Xml",
				L"DarkSkin_" + caseName,
				args.executable,
				traceManager,
				args.phase,
				args.tokens,
				[=](vint32_t type) { return WString::Unmanaged(XmlTypeName((XmlClasses)type)); },
				[=](vint32_t field) { return WString::Unmanaged(XmlFieldName((XmlFields)field)); },
				[=](vint32_t token) { return WString::Unmanaged(XmlTokenId((XmlTokens)token)); },
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
				L"BuiltIn-Xml",
				L"DarkSkin_" + caseName,
				[=](vint32_t type) { return WString::Unmanaged(XmlTypeName((XmlClasses)type)); },
				[=](vint32_t field) { return WString::Unmanaged(XmlFieldName((XmlFields)field)); },
				[=](vint32_t token) { return WString::Unmanaged(XmlTokenId((XmlTokens)token)); },
				[&](IAstInsReceiver& receiver)
				{
					traceManager.ExecuteTrace(receiver, args.tokens);
				});
		});
#endif

	TEST_CATEGORY(L"Test JSON on Outputs: DarkSkin")
	{
		Folder dirInput = FilePath(GetTestParserInputPath(L"BuiltIn-Xml")) / L"Input";
		FilePath dirOutput = GetOutputDir(L"BuiltIn-Xml");

		List<File> inputFiles;
		dirInput.GetFiles(inputFiles);
		for (auto&& inputFile : inputFiles)
		{
			caseName = inputFile.GetFilePath().GetName();
			if (caseName.Length() < 4 || caseName.Right(4) != L".xml") continue;
			caseName = caseName.Left(caseName.Length() - 4);

			TEST_CASE(caseName)
			{
				auto inputXml = inputFile.ReadAllTextByBom();
				auto ast = parser.ParseXDocument(inputXml);
				auto astJson = PrintAstJson<json_visitor::AstVisitor>(ast);
				File(dirOutput / (L"Output[DarkSkin_" + caseName + L"].json")).WriteAllText(astJson, true, BomEncoder::Utf8);
			});
		}
	});
}
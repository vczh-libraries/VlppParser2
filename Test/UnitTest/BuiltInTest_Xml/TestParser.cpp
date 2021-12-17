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

	parser.OnEndOfInput.Add(
		[&](List<RegexToken>& tokens, Executable& executable, TraceManager& tm, Trace* rootTrace)
		{
			LogTraceManager(
				L"BuiltIn-Xml",
				L"DarkSkin_" + caseName,
				executable,
				tm,
				rootTrace,
				tokens,
				[=](vint32_t type) { return WString::Unmanaged(XmlTypeName((XmlClasses)type)); },
				[=](vint32_t field) { return WString::Unmanaged(XmlFieldName((XmlFields)field)); },
				[=](vint32_t token) { return WString::Unmanaged(XmlTokenId((XmlTokens)token)); },
				[=](vint32_t rule) { return WString::Unmanaged(ParserRuleName(rule)); },
				[=](vint32_t state) { return WString::Unmanaged(ParserStateLabel(state)); }
			);

			if (tm.concurrentCount == 1)
			{
				LogTraceExecution(
					L"BuiltIn-Xml",
					L"DarkSkin_" + caseName,
					[=](vint32_t type) { return WString::Unmanaged(XmlTypeName((XmlClasses)type)); },
					[=](vint32_t field) { return WString::Unmanaged(XmlFieldName((XmlFields)field)); },
					[=](vint32_t token) { return WString::Unmanaged(XmlTokenId((XmlTokens)token)); },
					[&](IAstInsReceiver& receiver)
					{
						tm.ExecuteTrace(rootTrace, receiver, tokens);
					});
			}
		});

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
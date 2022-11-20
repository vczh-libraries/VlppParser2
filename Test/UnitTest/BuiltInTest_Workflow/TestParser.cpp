#include "../../Source/BuiltIn-Workflow/Generated/WorkflowParser.h"
#include "../../Source/BuiltIn-Workflow/Generated/WorkflowAst_Json.h"
#include "../../Source/LogTrace.h"

using namespace vl::regex;
using namespace vl::glr::workflow;

extern WString GetSourcePath();
extern FilePath GetOutputDir(const WString& parserName);

TEST_FILE
{
	FilePath dirWorkflow = FilePath(GetSourcePath()) / L"../../Workflow/Test/Resources";
	FilePath dirOutput = GetOutputDir(L"BuiltIn-Workflow");

	List<File> indexFiles;
	Folder(dirWorkflow).GetFiles(indexFiles);

	workflow::Parser parser;
	Regex regexCaseName(L"^(-)?(<name>/w+)(@/d+)?(/=/.*)?$");
	vint NAME = regexCaseName.CaptureNames().IndexOf(L"name");

	WString indexName;
	WString caseName;

#if !defined _DEBUG || defined NDEBUG
	parser.OnTraceProcessing.Add(
		[&](TraceProcessingArgs& args)
		{
			auto& traceManager = *dynamic_cast<TraceManager*>(args.executor);
			LogTraceManager(
				L"BuiltIn-Workflow",
				indexName + L"_" + caseName,
				args.executable,
				traceManager,
				args.phase,
				args.tokens,
				[=](vint32_t type) { return WString::Unmanaged(WorkflowTypeName((WorkflowClasses)type)); },
				[=](vint32_t field) { return WString::Unmanaged(WorkflowFieldName((WorkflowFields)field)); },
				[=](vint32_t token) { return WString::Unmanaged(WorkflowTokenId((WorkflowTokens)token)); },
				[=](vint32_t rule) { return WString::Unmanaged(ParserRuleName(rule)); },
				[=](vint32_t state) { return WString::Unmanaged(ParserStateLabel(state)); }
			);
		});
	parser.OnReadyToExecute.Add(
		[&](ReadyToExecuteArgs& args)
		{
			auto& traceManager = *dynamic_cast<TraceManager*>(args.executor);
			LogTraceExecution(
				L"BuiltIn-Workflow",
				indexName + L"_" + caseName,
				[=](vint32_t type) { return WString::Unmanaged(WorkflowTypeName((WorkflowClasses)type)); },
				[=](vint32_t field) { return WString::Unmanaged(WorkflowFieldName((WorkflowFields)field)); },
				[=](vint32_t token) { return WString::Unmanaged(WorkflowTokenId((WorkflowTokens)token)); },
				[&](IAstInsReceiver& receiver)
				{
					traceManager.ExecuteTrace(receiver, args.tokens);
				});
		});
#endif

	for (auto indexFile : indexFiles)
	{
		indexName = indexFile.GetFilePath().GetName();
		indexName = indexName.Sub(5, indexName.Length() - 9);

		TEST_CATEGORY(L"Test Workflow on Index: " + indexName)
		{
			List<WString> caseDescs;
			indexFile.ReadAllLinesByBom(caseDescs);
			for (auto caseDesc : caseDescs)
			{
				auto match = regexCaseName.MatchHead(caseDesc);
				caseName = match->Groups()[NAME][0].Value();

				TEST_CASE(caseName)
				{
					WString inputCode = File(dirWorkflow / indexName / (caseName + L".txt")).ReadAllTextByBom();
					if (indexName == L"Declaration")
					{
						auto ast = parser.Parse_Declaration(inputCode);
						auto astJson = PrintAstJson<json_visitor::AstVisitor>(ast);
						File(dirOutput / (L"Output[" + indexName + L"_" + caseName + L"].json")).WriteAllText(astJson, true, BomEncoder::Utf8);
					}
					else if (indexName == L"Expression")
					{
						auto ast = parser.Parse_Expression(inputCode);
						auto astJson = PrintAstJson<json_visitor::AstVisitor>(ast);
						File(dirOutput / (L"Output[" + indexName + L"_" + caseName + L"].json")).WriteAllText(astJson, true, BomEncoder::Utf8);
					}
					else if (indexName == L"Statement")
					{
						auto ast = parser.Parse_Statement(inputCode);
						auto astJson = PrintAstJson<json_visitor::AstVisitor>(ast);
						File(dirOutput / (L"Output[" + indexName + L"_" + caseName + L"].json")).WriteAllText(astJson, true, BomEncoder::Utf8);
					}
					else
					{
						auto ast = parser.Parse_Module(inputCode);
						auto astJson = PrintAstJson<json_visitor::AstVisitor>(ast);
						File(dirOutput / (L"Output[" + indexName + L"_" + caseName + L"].json")).WriteAllText(astJson, true, BomEncoder::Utf8);
					}
				});
			}
		});
	}
}
#include "../../Source/BuiltIn-Workflow/Generated/WorkflowParser.h"
#include "../../Source/BuiltIn-Workflow/Generated/WorkflowAst_Json.h"
#include "../../Source/LogTrace.h"

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

	for (auto indexFile : indexFiles)
	{
		WString indexName = indexFile.GetFilePath().GetName();
		indexName = indexName.Sub(5, indexName.Length() - 9);

		TEST_CATEGORY(L"Test Workflow on Index: " + indexName)
		{
			List<WString> caseNames;
			indexFile.ReadAllLinesByBom(caseNames);
			for (auto caseName : caseNames)
			{
				{
					vint eq = caseName.IndexOf(L'=');
					if (eq != -1)
					{
						caseName = caseName.Left(eq);
					}
				}

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
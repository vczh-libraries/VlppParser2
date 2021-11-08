#include "../../../Source/Ast/AstCppGen.h"

using namespace vl;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::glr::parsergen;

extern WString GetExePath();

TEST_FILE
{
	TEST_CASE(L"CreateParserGenAst")
	{
		AstSymbolManager manager;
		auto expressionAst = CreateParserGenTypeAst(manager);
		TEST_ASSERT(manager.Errors().Count() == 0);

		auto outputDir = FilePath(GetExePath()) / L"../../../Source/AstParserGen/";

		WString fileExpressionH = GenerateToStream([=](StreamWriter& writer)
		{
			WriteAstHeaderFile(expressionAst, writer);
		});
		File(outputDir / (expressionAst->filePrefix + expressionAst->Name() + L".h"))
			.WriteAllText(fileExpressionH, false, BomEncoder::Utf8);

		WString fileExpressionCpp = GenerateToStream([=](StreamWriter& writer)
		{
			WriteAstCppFile(expressionAst, writer);
		});
		File(outputDir / (expressionAst->filePrefix + expressionAst->Name() + L".cpp"))
			.WriteAllText(fileExpressionCpp, false, BomEncoder::Utf8);
	});
}
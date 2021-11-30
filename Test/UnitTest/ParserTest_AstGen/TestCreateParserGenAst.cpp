#include "../../../Source/Ast/AstCppGen.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::glr::parsergen;

extern WString GetExePath();
extern void WriteFilesIfChanged(FilePath outputDir, Dictionary<WString, WString>& files);

TEST_FILE
{
	TEST_CASE(L"CreateParserGenAst")
	{
		ParserSymbolManager global;
		AstSymbolManager astManager(global);

		InitializeParserSymbolManager(global);
		CreateParserGenTypeAst(astManager);
		CreateParserGenRuleAst(astManager);
		TEST_ASSERT(global.Errors().Count() == 0);

		auto output = GenerateParserFileNames(global);
		GenerateAstFileNames(astManager, output);

		Dictionary<WString, WString> files;
		WriteAstFiles(astManager, output, files);

		auto outputDir = FilePath(GetExePath()) / L"../../../Source/ParserGen_Generated/";
		WriteFilesIfChanged(outputDir, files);
	});
}
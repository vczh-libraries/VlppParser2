#include "../../../Source/Ast/AstCppGen.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::glr::parsergen;

extern WString GetTestParserInputPath(const WString& parserName);
extern void WriteFilesIfChanged(FilePath outputDir, Dictionary<WString, WString>& files);
extern void InitializeCalculatorParserSymbolManager(ParserSymbolManager& manager);
extern void GenerateCalculatorAst(AstSymbolManager& manager);

TEST_FILE
{
	TEST_CASE(L"GenerateCalculatorAst")
	{
		ParserSymbolManager global;
		AstSymbolManager astManager(global);

		InitializeCalculatorParserSymbolManager(global);
		GenerateCalculatorAst(astManager);
		TEST_ASSERT(global.Errors().Count() == 0);

		auto output = GenerateParserFileNames(global);
		GenerateAstFileNames(astManager, output);

		Dictionary<WString, WString> files;
		WriteAstFiles(astManager, output, files);

		auto outputDir = FilePath(GetTestParserInputPath(L"Calculator")) / L"Parser";
		WriteFilesIfChanged(outputDir, files);
	});
}
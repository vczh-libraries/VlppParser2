#include "../../../Source/Lexer/LexerCppGen.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::glr::parsergen;

extern WString GetTestParserInputPath(const WString& parserName);
extern void WriteFilesIfChanged(FilePath outputDir, Dictionary<WString, WString>& files);
extern void InitializeCalculatorParserSymbolManager(ParserSymbolManager& manager);
extern void GenerateCalculatorLexer(LexerSymbolManager& manager);

TEST_FILE
{
	TEST_CASE(L"CreateCalculatorLexer")
	{
		ParserSymbolManager global;
		LexerSymbolManager lexerManager(global);

		InitializeCalculatorParserSymbolManager(global);
		GenerateCalculatorLexer(lexerManager);
		TEST_ASSERT(global.Errors().Count() == 0);

		auto output = GenerateParserFileNames(global);

		Dictionary<WString, WString> files;
		WriteLexerFiles(lexerManager, output, files);

		auto outputDir = FilePath(GetTestParserInputPath(L"Calculator")) / L"Parser";
		WriteFilesIfChanged(outputDir, files);
	});
}
#include "../../../Source/Lexer/LexerCppGen.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::glr::parsergen;

extern WString GetExePath();
extern void WriteFilesIfChanged(FilePath outputDir, Dictionary<WString, WString>& files);

TEST_FILE
{
	TEST_CASE(L"CreateParserGenLexer")
	{
		ParserSymbolManager global;
		LexerSymbolManager lexerManager(global);

		InitializeParserSymbolManager(global);
		CreateParserGenLexer(lexerManager);
		TEST_ASSERT(global.Errors().Count() == 0);

		auto output = GenerateParserFileNames(global);

		Dictionary<WString, WString> files;
		WriteLexerFiles(lexerManager, output, files);

		auto outputDir = FilePath(GetExePath()) / L"../../../Source/ParserGen_Generated/";
		WriteFilesIfChanged(outputDir, files);
	});
}
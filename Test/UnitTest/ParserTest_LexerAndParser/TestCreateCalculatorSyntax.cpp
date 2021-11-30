#include "../../../Source/Syntax/SyntaxCppGen.h"
#include "../../Source/Calculator/Parser/Calculator_Lexer.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::glr::parsergen;
using namespace vl::glr::automaton;
using namespace calculator;

extern WString GetTestParserInputPath(const WString& parserName);
extern void WriteFilesIfChanged(FilePath outputDir, Dictionary<WString, WString>& files);
extern void InitializeCalculatorParserSymbolManager(ParserSymbolManager& manager);
extern void GenerateCalculatorSyntax(SyntaxSymbolManager& manager);

TEST_FILE
{
	TEST_CASE(L"CreateCalculatorLexer")
	{
		ParserSymbolManager global;
		SyntaxSymbolManager syntaxManager(global);
		Executable executable;
		Metadata metadata;

		InitializeCalculatorParserSymbolManager(global);
		GenerateCalculatorSyntax(syntaxManager);
		TEST_ASSERT(global.Errors().Count() == 0);

		syntaxManager.BuildCompactNFA();
		TEST_ASSERT(global.Errors().Count() == 0);
		syntaxManager.BuildCrossReferencedNFA();
		TEST_ASSERT(global.Errors().Count() == 0);
		syntaxManager.BuildAutomaton(CalculatorTokenCount, executable, metadata);

		auto output = GenerateParserFileNames(global);
		GenerateSyntaxFileNames(syntaxManager, output);

		Dictionary<WString, WString> files;
		WriteSyntaxFiles(syntaxManager, executable, metadata, output, files);

		auto outputDir = FilePath(GetTestParserInputPath(L"Calculator")) / L"Parser";
		WriteFilesIfChanged(outputDir, files);
	});
}
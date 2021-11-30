#include "../../../Source/Syntax/SyntaxCppGen.h"
#include "../../../Source/ParserGen_Generated/ParserGen_Lexer.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::glr::parsergen;
using namespace vl::glr::automaton;

extern WString GetParserGenGeneratedOutputPath();
extern void WriteFilesIfChanged(FilePath outputDir, Dictionary<WString, WString>& files);

TEST_FILE
{
	TEST_CASE(L"CreateParserGenLexer")
	{
		ParserSymbolManager global;

		SyntaxSymbolManager typeSyntaxManager(global);
		Executable typeExecutable;
		Metadata typeMetadata;

		SyntaxSymbolManager ruleSyntaxManager(global);
		Executable ruleExecutable;
		Metadata ruleMetadata;

		InitializeParserSymbolManager(global);
		CreateParserGenTypeSyntax(typeSyntaxManager);
		CreateParserGenRuleSyntax(ruleSyntaxManager);
		TEST_ASSERT(global.Errors().Count() == 0);
		{
			typeSyntaxManager.BuildCompactNFA();
			TEST_ASSERT(global.Errors().Count() == 0);
			typeSyntaxManager.BuildCrossReferencedNFA();
			TEST_ASSERT(global.Errors().Count() == 0);
			typeSyntaxManager.BuildAutomaton(ParserGenTokenCount, typeExecutable, typeMetadata);
		}
		{
			ruleSyntaxManager.BuildCompactNFA();
			TEST_ASSERT(global.Errors().Count() == 0);
			ruleSyntaxManager.BuildCrossReferencedNFA();
			TEST_ASSERT(global.Errors().Count() == 0);
			ruleSyntaxManager.BuildAutomaton(ParserGenTokenCount, ruleExecutable, ruleMetadata);
		}
		auto output = GenerateParserFileNames(global);
		GenerateSyntaxFileNames(typeSyntaxManager, output);
		GenerateSyntaxFileNames(ruleSyntaxManager, output);

		Dictionary<WString, WString> files;
		WriteSyntaxFiles(typeSyntaxManager, typeExecutable, typeMetadata, output, files);
		WriteSyntaxFiles(ruleSyntaxManager, ruleExecutable, ruleMetadata, output, files);

		auto outputDir = FilePath(GetParserGenGeneratedOutputPath());
		WriteFilesIfChanged(outputDir, files);
	});
}
#include "../../../Source/Syntax/SyntaxCppGen.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::glr::parsergen;

extern WString GetExePath();
extern void GenerateCalculatorSyntax(SyntaxSymbolManager& manager);

TEST_FILE
{
	TEST_CASE(L"CreateCalculatorLexer")
	{
		ParserSymbolManager global;
		SyntaxSymbolManager syntaxManager(global);

		global.name = L"Calculator";
		Fill(global.includes, L"../../../../Source/AstBase.h");
		global.cppNss.Add(L"calculator");
		global.headerGuard = L"VCZH_PARSER2_UNITTEST_CALCULATOR";
		GenerateCalculatorSyntax(syntaxManager);
		TEST_ASSERT(global.Errors().Count() == 0);
		syntaxManager.BuildCompactSyntax();
		TEST_ASSERT(global.Errors().Count() == 0);

		auto output = GenerateParserFileNames(global);

		Dictionary<WString, WString> files;
		WriteSyntaxFiles(syntaxManager, output, files);

		auto outputDir = FilePath(GetExePath()) / L"../../Source/Calculator/Parser/";
		for (auto [key, index] : indexed(files.Keys()))
		{
			//File(outputDir / key).WriteAllText(files.Values()[index], false, BomEncoder::Utf8);
		}
	});
}
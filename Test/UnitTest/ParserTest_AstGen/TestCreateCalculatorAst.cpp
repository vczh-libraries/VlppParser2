#include "../../../Source/Ast/AstCppGen.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::glr::parsergen;

extern WString GetExePath();
extern void GenerateCalculatorAst(AstSymbolManager& manager);

TEST_FILE
{
	TEST_CASE(L"CreateParserGenAst")
	{
		ParserSymbolManager global;
		global.name = L"Calculator";
		global.cppNss.Add(L"calculator");
		global.headerGuard = L"VCZH_PARSER2_UNITTEST_CALCULATOR";

		AstSymbolManager astManager(global);
		GenerateCalculatorAst(astManager);
		TEST_ASSERT(global.Errors().Count() == 0);

		auto output = GenerateParserFileNames(global);
		GenerateAstFileNames(astManager, output);

		Dictionary<WString, WString> files;
		WriteAstFiles(astManager, output, files);

		auto outputDir = FilePath(GetExePath()) / L"../../Source/Calculator/Parser/";
		for (auto [key, index] : indexed(files.Keys()))
		{
			File(outputDir / key).WriteAllText(files.Values()[index], false, BomEncoder::Utf8);
		}
	});
}
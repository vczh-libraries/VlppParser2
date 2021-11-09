#include "../../../Source/Ast/AstCppGen.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::glr::parsergen;

extern WString GetExePath();

TEST_FILE
{
	TEST_CASE(L"CreateParserGenAst")
	{
		AstSymbolManager manager;
		manager.name = L"ParserGen";
		Fill(manager.cppNss, L"vl", L"glr", L"parsergen");
		manager.headerGuard = L"VCZH_PARSER2_PARSERGEN";
		CreateParserGenTypeAst(manager);
		TEST_ASSERT(manager.Errors().Count() == 0);

		Dictionary<WString, WString> files;
		WriteAstFiles(manager, files);

		auto outputDir = FilePath(GetExePath()) / L"../../../Source/AstParserGen/";
		for (auto [key, index] : indexed(files.Keys()))
		{
			File(outputDir / key).WriteAllText(files.Values()[index], false, BomEncoder::Utf8);
		}
	});
}
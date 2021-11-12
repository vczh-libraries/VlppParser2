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
		InitializeParserGenAstSymbolManager(manager);
		CreateParserGenTypeAst(manager);
		TEST_ASSERT(manager.Errors().Count() == 0);

		Dictionary<WString, WString> files;
		auto output = GenerateFileNames(manager);
		WriteAstFiles(manager, output, files);
		auto outputDir = FilePath(GetExePath()) / L"../../../Source/AstParserGen/";
		for (auto [key, index] : indexed(files.Keys()))
		{
			File(outputDir / key).WriteAllText(files.Values()[index], false, BomEncoder::Utf8);
		}
	});
}
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
		auto file = manager.CreateFile(L"CalculatorAst");
		{
			file->includes.Add(L"../../../Source/AstBase.h");
			file->cppNss.Add(L"calculator");
			file->refNss.Add(L"calculator");
			file->filePrefix = L"";
			file->classPrefix = L"";
			file->headerGuard = L"VCZH_PARSER2_UNITTEST_CALCULATOR";
		}
		TEST_ASSERT(manager.Errors().Count() == 0);

		{
			Dictionary<WString, WString> files;
			WriteAstFiles(file, files);

			auto outputDir = FilePath(GetExePath()) / L"../../Source/CalculatorAst/";
			for (auto [key, index] : indexed(files.Keys()))
			{
				File(outputDir / key).WriteAllText(files.Values()[index], false, BomEncoder::Utf8);
			}
		}
	});
}
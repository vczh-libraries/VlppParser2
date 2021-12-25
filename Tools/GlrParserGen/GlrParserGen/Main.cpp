#include <VlppGlrParserCompiler.h>

using namespace vl;
using namespace vl::console;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::glr;
using namespace vl::glr::xml;

int main(int argc, char* argv[])
{
	Console::SetTitle(L"Vczh GLR ParserGen for C++");
	if (argc != 2)
	{
		Console::SetColor(true, false, false, true);
		Console::WriteLine(L"GlrParserGen.exe <config-xml>");
		Console::SetColor(true, true, true, false);
		return 0;
	}

	auto workingDir = FilePath(atow(argv[1])).GetFolder();
	Ptr<XmlDocument> config;
	{
		Parser parser;
		List<ParsingError> errors;
		InstallDefaultErrorMessageGenerator(parser, errors);
		auto text = File(atow(argv[1])).ReadAllTextByBom();
		config = XmlParseDocument(text, parser);

		if (errors.Count() > 0)
		{
			Console::SetColor(true, false, false, true);
			Console::WriteLine(L"Failed to read the input XML file.");
			for (auto error : errors)
			{
				Console::WriteLine(L"[row:" + itow(error.codeRange.start.row + 1) + L"][column:" + itow(error.codeRange.start.column + 1) + L"]: " + error.message);
			}
			Console::SetColor(true, true, true, false);
			return 1;
		}
	}

	return 0;
}
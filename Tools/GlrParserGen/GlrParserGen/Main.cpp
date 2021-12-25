#include <VlppGlrParserCompiler.h>

using namespace vl;
using namespace vl::console;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::filesystem;
using namespace vl::regex;
using namespace vl::glr;
using namespace vl::glr::parsergen;
using namespace vl::glr::xml;

#define EXIT_ERROR(MESSAGE)\
	do\
	{\
		Console::SetColor(true, false, false, true);\
		Console::WriteLine(MESSAGE);\
		Console::SetColor(true, true, true, false);\
		return 1;\
	} while(false)

#define READ_ATTRIBUTE(STORAGE, ELEMENT, NAME, PATH)\
	do\
	{\
		if (auto attributeToRead = XmlGetAttribute(ELEMENT, NAME))\
		{\
			STORAGE = attributeToRead->value.value;\
		}\
		else\
		{\
			EXIT_ERROR(L"Missing " PATH L".");\
		}\
	} while(false)\

#define READ_ELEMENT(STORAGE, ELEMENT, NAME, PATH)\
	do\
	{\
		if (auto elementToRead = XmlGetElement(ELEMENT, NAME))\
		{\
			STORAGE = XmlGetValue(elementToRead);\
		}\
		else\
		{\
			EXIT_ERROR(L"Missing " PATH L".");\
		}\
	} while(false)\

#define READ_ELEMENT_ITEMS(STORAGE, REGEX, ELEMENT, NAME, PATH)\
	do\
	{\
		if (auto elementToRead = XmlGetElement(ELEMENT, NAME))\
		{\
			auto value = XmlGetValue(elementToRead);\
			if (auto match = REGEX.MatchHead(value))\
			{\
				for (auto item : match->Groups()[REGEX.CaptureNames().IndexOf(L"item")])\
				{\
					STORAGE.Add(item.Value());\
				}\
			}\
			else\
			{\
				EXIT_ERROR(L"Incorrect namespace format in: " PATH L".");\
			}\
		}\
		else\
		{\
			EXIT_ERROR(L"Missing " PATH L".");\
		}\
	} while(false)\
	

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

	if (config->rootElement->name.value != L"Parser") EXIT_ERROR(L"Missing /Parser.");

	Ptr<ParserSymbolManager> global;
	Ptr<LexerSymbolManager>	lexerManager;
	Ptr<SyntaxSymbolManager> syntaxManager;
	List<Ptr<AstSymbolManager>> astManagers;
	FilePath generatedDir;

	Regex regexNamespace(L"^(<item>[^:]+)(::(<item>[^:]+))*$");
	Regex regexIncludes(L"^(<item>[^;]+)(;(<item>[^;]+))*$");
	auto indexItem = regexNamespace.CaptureNames().IndexOf(L"item");

	global = MakePtr<ParserSymbolManager>();
	READ_ATTRIBUTE(global->name, config->rootElement, L"name", L"/Parser@name");
	READ_ELEMENT_ITEMS(global->includes, regexIncludes, config->rootElement, L"Includes", L"/Parser/Includes");
	READ_ELEMENT_ITEMS(global->cppNss, regexNamespace, config->rootElement, L"CppNamespace", L"/Parser/CppNamespace");
	READ_ELEMENT(global->headerGuard, config->rootElement, L"HeaderGuard", L"/Parser/HeaderGuard");
	{
		WString outputDir;
		READ_ELEMENT(outputDir, config->rootElement, L"OutputDir", L"/Parser/OutputDir");
		generatedDir = workingDir / outputDir;
	}

	return 0;
}
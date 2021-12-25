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

#define EXIT_IF_PARSER_FAIL(ERRORS, TITLE)\
	do\
	{\
		if (ERRORS.Count() > 0)\
		{\
			Console::SetColor(true, false, false, true);\
			Console::WriteLine(TITLE);\
			for (auto error : ERRORS)\
			{\
				Console::WriteLine(L"[row:" + itow(error.codeRange.start.row + 1) + L"][column:" + itow(error.codeRange.start.column + 1) + L"]: " + error.message);\
			}\
			Console::SetColor(true, true, true, false);\
			return 1;\
		}\
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
		EXIT_IF_PARSER_FAIL(errors, L"Failed to read the input XML file.");
	}

	if (config->rootElement->name.value != L"Parser") EXIT_ERROR(L"Missing /Parser.");

	ParserSymbolManager global;
	AstSymbolManager astManager(global);
	LexerSymbolManager	lexerManager(global);
	SyntaxSymbolManager syntaxManager(global);
	FilePath generatedDir;
	Dictionary<WString, WString> files;

	Regex regexNamespace(L"^(<item>[^:]+)(::(<item>[^:]+))*$");
	Regex regexIncludes(L"^(<item>[^;]+)(;(<item>[^;]+))*$");
	auto indexItem = regexNamespace.CaptureNames().IndexOf(L"item");

	READ_ATTRIBUTE(global.name, config->rootElement, L"name", L"/Parser@name");
	READ_ELEMENT_ITEMS(global.includes, regexIncludes, config->rootElement, L"Includes", L"/Parser/Includes");
	READ_ELEMENT_ITEMS(global.cppNss, regexNamespace, config->rootElement, L"CppNamespace", L"/Parser/CppNamespace");
	READ_ELEMENT(global.headerGuard, config->rootElement, L"HeaderGuard", L"/Parser/HeaderGuard");
	{
		WString outputDir;
		READ_ELEMENT(outputDir, config->rootElement, L"OutputDir", L"/Parser/OutputDir");
		generatedDir = workingDir / outputDir;
	}

	auto output = GenerateParserFileNames(global);

	TypeParser typeParser;
	RuleParser ruleParser;
	List<ParsingError> errors;
	InstallDefaultErrorMessageGenerator(typeParser, errors);
	InstallDefaultErrorMessageGenerator(ruleParser, errors);

	if (auto elementAsts = XmlGetElement(config->rootElement, L"Asts"))
	{
		List<Ptr<XmlElement>> asts;
		CopyFrom(asts, XmlGetElements(elementAsts, L"Ast"));
		if (asts.Count() == 0) EXIT_ERROR(L"Missing /Parser/Asts/Ast");

		for (auto elementAst : asts)
		{
			WString name, file;
			READ_ATTRIBUTE(name, elementAst, L"name", L"/Parser/Asts/Ast@name");
			READ_ATTRIBUTE(file, elementAst, L"file", L"/Parser/Asts/Ast@file[@name=\"" + name + L"\"]");
			Console::WriteLine(L"Processing " + file + L" ...");

			File astFile = workingDir / file;
			if (!astFile.Exists()) EXIT_ERROR(L"Missing ast definition file: " + astFile.GetFilePath().GetFullPath());
			auto astInput = astFile.ReadAllTextByBom();
			auto ast = typeParser.ParseFile(astInput);
			EXIT_IF_PARSER_FAIL(errors, L"Syntax errors found in file: " + astFile.GetFilePath().GetFullPath());

			auto astDefFile = astManager.CreateFile(name);
			READ_ELEMENT_ITEMS(astDefFile->cppNss, regexNamespace, elementAst, L"CppNamespace", L"/Parser/Asts/Ast@file[@name=\"" + name + L"\"]/CppNamespace");
			READ_ELEMENT_ITEMS(astDefFile->refNss, regexNamespace, elementAst, L"ReflectionNamespace", L"/Parser/Asts/Ast@file[@name=\"" + name + L"\"]/ReflectionNamespace");
			READ_ELEMENT(astDefFile->classPrefix, elementAst, L"ClassPrefix", L"/Parser/Asts/Ast@file[@name=\"" + name + L"\"]/ClassPrefix");
			CompileAst(astManager, astDefFile, ast);
		}

		GenerateAstFileNames(astManager, output);
		WriteAstFiles(astManager, output, files);
	}
	else
	{
		EXIT_ERROR(L"Missing /Parser/Asts");
	}

	if (auto elementLexer = XmlGetElement(config->rootElement, L"Lexer"))
	{
		WString file;
		READ_ATTRIBUTE(file, elementLexer, L"file", L"/Parser/Lexer@file");
		Console::WriteLine(L"Processing " + file + L" ...");

		File lexerFile = workingDir / file;
		auto lexerInput = lexerFile.ReadAllTextByBom();
		CompileLexer(lexerManager, lexerInput);
		WriteLexerFiles(lexerManager, output, files);
	}
	else
	{
		EXIT_ERROR(L"Missing /Parser/Asts");
	}

	if (auto elementSyntax = XmlGetElement(config->rootElement, L"Syntax"))
	{
		GenerateSyntaxFileNames(syntaxManager, output);
	}
	else
	{
		EXIT_ERROR(L"Missing /Parser/Asts");
	}

	return 0;
}
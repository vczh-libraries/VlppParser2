#include "../../../Source/ParserGen_Generated/ParserGenTypeAst_Json.h"
#include "../../../Source/ParserGen_Generated/ParserGenRuleAst_Json.h"
#include "../../../Source/ParserGen_Generated/ParserGenTypeParser.h"
#include "../../../Source/ParserGen_Generated/ParserGenRuleParser.h"
#include "../../../Source/ParserGen/Compiler.h"
#include "../../../Source/Ast/AstCppGen.h"
#include "../../../Source/Lexer/LexerCppGen.h"
#include "../../../Source/Syntax/SyntaxCppGen.h"
#include "../../Source/LogAutomaton.h"

using namespace vl::glr::parsergen;

extern WString GetSourcePath();
extern FilePath GetOutputDir(const WString& parserName);
extern void WriteFilesIfChanged(FilePath outputDir, Dictionary<WString, WString>& files);

TEST_FILE
{
	FilePath dirParser = FilePath(GetSourcePath()) / L"Xml";
	FilePath dirOutput = GetOutputDir(L"ParserGen");
	FilePath dirGenerated = dirParser / L"Generated";
	if (!Folder(dirGenerated).Exists())
	{
		Folder(dirGenerated).Create(true);
	}

	Ptr<GlrAstFile> astFile;
	Ptr<GlrSyntaxFile> syntaxFile;

	TEST_CASE(L"Parse Ast.txt")
	{
		TypeParser typeParser;
		auto input = File(dirParser / L"Syntax/Ast.txt").ReadAllTextByBom();
		astFile = typeParser.ParseFile(input);
		auto actualJson = PrintAstJson<json_visitor::TypeAstVisitor>(astFile);
		File(dirOutput / (L"Ast[BuiltIn-Xml].txt")).WriteAllText(actualJson, true, BomEncoder::Utf8);
	});

	TEST_CASE(L"Parse Syntax.txt")
	{
		RuleParser ruleParser;
		auto input = File(dirParser / L"Syntax/Syntax.txt").ReadAllTextByBom();
		syntaxFile = ruleParser.ParseFile(input);
		auto actualJson = PrintAstJson<json_visitor::RuleAstVisitor>(syntaxFile);
		File(dirOutput / (L"Syntax[BuiltIn-Xml].txt")).WriteAllText(actualJson, true, BomEncoder::Utf8);
	});

	ParserSymbolManager global;
	AstSymbolManager astManager(global);
	LexerSymbolManager lexerManager(global);
	SyntaxSymbolManager syntaxManager(global);
	Executable executable;
	Metadata metadata;

	global.name = L"Xml";
	Fill(global.includes, L"../../AstBase.h", L"../../SyntaxBase.h");
	Fill(global.cppNss, L"vl", L"glr", L"xml");
	global.headerGuard = L"VCZH_PARSER2_BUILTIN_XML";
	syntaxManager.name = L"Parser";

	auto astDefFile = astManager.CreateFile(L"Ast");
	auto output = GenerateParserFileNames(global);
	GenerateAstFileNames(astManager, output);
	GenerateSyntaxFileNames(syntaxManager, output);

	Dictionary<WString, WString> files;
	TEST_CASE(L"CompilerAst")
	{
		CompileAst(astManager, astDefFile, astFile);
		TEST_ASSERT(global.Errors().Count() == 0);

		Fill(astDefFile->cppNss, L"vl", L"glr", L"xml");
		Fill(astDefFile->refNss, L"system");
		astDefFile->classPrefix = L"Xml";
		WriteAstFiles(astManager, output, files);
	});

	TEST_CASE(L"CompilerLexer")
	{
		auto lexerInput = File(dirParser / L"Syntax/Lexer.txt").ReadAllTextByBom();
		CompileLexer(lexerManager, lexerInput);
		TEST_ASSERT(global.Errors().Count() == 0);
		WriteLexerFiles(lexerManager, output, files);
	});

	TEST_CASE(L"CompilerSyntax")
	{
		List<Ptr<GlrSyntaxFile>> syntaxFiles;
		syntaxFiles.Add(syntaxFile);
		CompileSyntax(astManager, lexerManager, syntaxManager, output, syntaxFiles);
		TEST_ASSERT(global.Errors().Count() == 0);

		syntaxManager.BuildCompactNFA();
		TEST_ASSERT(global.Errors().Count() == 0);
		syntaxManager.BuildCrossReferencedNFA();
		TEST_ASSERT(global.Errors().Count() == 0);
		syntaxManager.BuildAutomaton(lexerManager.Tokens().Count(), executable, metadata);
		TEST_ASSERT(global.Errors().Count() == 0);

		LogAutomatonWithPath(
			dirOutput / (L"Automaton[BuiltIn-Xml].txt"),
			executable,
			metadata,
			[&](vint32_t index) { auto type = output->classIds.Keys()[output->classIds.Values().IndexOf(index)]; return type->Name(); },
			[&](vint32_t index) { auto prop = output->fieldIds.Keys()[output->fieldIds.Values().IndexOf(index)]; return prop->Parent()->Name() + L"::" + prop->Name(); },
			[&](vint32_t index) { auto token = lexerManager.Tokens()[lexerManager.TokenOrder()[index]]; return token->displayText == L"" ? token->Name() : L"\"" + token->displayText + L"\""; },
			[&](vint32_t index) { return syntaxManager.switches.Keys()[index]; }
			);

		{
			auto rule = syntaxManager.Rules()[L"XElement"];
			syntaxManager.parsableRules.Add(rule);
			syntaxManager.ruleTypes.Add(rule, L"vl::glr::xml::XmlElement");
		}
		{
			auto rule = syntaxManager.Rules()[L"XDocument"];
			syntaxManager.parsableRules.Add(rule);
			syntaxManager.ruleTypes.Add(rule, L"vl::glr::xml::XmlDocument");
		}
		WriteSyntaxFiles(syntaxManager, executable, metadata, output, files);
	});

	if (global.Errors().Count() == 0)
	{
		WriteFilesIfChanged(dirGenerated, files);
	}
}
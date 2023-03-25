#include "../../../Source/ParserGen_Generated/ParserGenTypeAst_Json.h"
#include "../../../Source/ParserGen_Generated/ParserGenRuleAst_Json.h"
#include "../../../Source/ParserGen_Generated/ParserGenTypeParser.h"
#include "../../../Source/ParserGen_Generated/ParserGenRuleParser.h"
#include "../../../Source/ParserGen/Compiler.h"
#include "../../../Source/Ast/AstCppGen.h"
#include "../../../Source/Lexer/LexerCppGen.h"
#include "../../../Source/Syntax/SyntaxCppGen.h"
#include "../../Source/LogAutomaton.h"
#include "../../Source/SyntaxAstToCode.h"

using namespace vl::glr::parsergen;

extern WString GetTestParserInputPath(const WString& parserName);
extern FilePath GetOutputDir(const WString& parserName);
extern void WriteFilesIfChanged(FilePath outputDir, Dictionary<WString, WString>& files);

void PrintError(const ParserError& error)
{
	constexpr auto MKError = unittest::UnitTest::MessageKind::Error;
	constexpr auto MKInfo = unittest::UnitTest::MessageKind::Info;
#define PRINT unittest::UnitTest::PrintMessage

#define CASE_HEADER(LABEL)\
		PRINT(L ## #LABEL L"[" + itow(error.location.codeRange.start.row) + L":" + itow(error.location.codeRange.start.column) + L"]", MKError);\

#define CASE_1(LABEL, P1, ...)\
		CASE_HEADER(LABEL)\
		PRINT(L"    " L ## #P1 L": " + error.arg1, MKInfo);\

#define CASE_2(LABEL, P1, P2, ...)\
		CASE_HEADER(LABEL)\
		PRINT(L"    " L ## #P1 L": " + error.arg1, MKInfo);\
		PRINT(L"    " L ## #P2 L": " + error.arg2, MKInfo);\

#define CASE_3(LABEL, P1, P2, P3, ...)\
		CASE_HEADER(LABEL)\
		PRINT(L"   " L ## #P1 L": " + error.arg1, MKInfo);\
		PRINT(L"   " L ## #P2 L": " + error.arg2, MKInfo);\
		PRINT(L"   " L ## #P3 L": " + error.arg3, MKInfo);\

#define CASE_4(LABEL, P1, P2, P3, P4, ...)\
		CASE_HEADER(LABEL)\
		PRINT(L"   " L ## #P1 L": " + error.arg1, MKInfo);\
		PRINT(L"   " L ## #P2 L": " + error.arg2, MKInfo);\
		PRINT(L"   " L ## #P3 L": " + error.arg3, MKInfo);\
		PRINT(L"   " L ## #P4 L": " + error.arg4, MKInfo);\

#define CASE_CALL2(ARG1, ARG2, ARG3, ARG4, ARG5, FUNC, ...)\
		FUNC(ARG1, ARG2, ARG3, ARG4, ARG5)

#define CASE_CALL(ARGS)\
		CASE_CALL2 ARGS

#define CASE(LABEL, ...)\
		case ParserErrorType::LABEL:\
			CASE_CALL((LABEL, __VA_ARGS__, CASE_4, CASE_3, CASE_2, CASE_1))\
			break;\

	switch (error.type)
	{
	GLR_PARSER_ERROR_LIST(CASE)
	default:
		unittest::UnitTest::PrintMessage(L"<UNKNOWN-ERROR>", unittest::UnitTest::MessageKind::Error);
	}
#undef CASE
#undef CASE_CALL
#undef CASE_CALL2
#undef CASE_4
#undef CASE_3
#undef CASE_2
#undef CASE_1
#undef CASE_HEADER
#undef PRINT
}

TEST_FILE
{
	FilePath dirParser = GetTestParserInputPath(L"BuiltIn-Cpp");
	FilePath dirOutput = GetOutputDir(L"ParserGen");
	FilePath dirGenerated = dirParser / L"Generated";
	if (!Folder(dirGenerated).Exists())
	{
		Folder(dirGenerated).Create(true);
	}

	Ptr<GlrAstFile> astFile;
	List<Ptr<GlrSyntaxFile>> syntaxFiles;

	TEST_CASE(L"Parse Ast.txt")
	{
		TypeParser typeParser;
		auto input = File(dirParser / L"Syntax/Ast.txt").ReadAllTextByBom();
		astFile = typeParser.ParseFile(input);
		auto actualJson = PrintAstJson<json_visitor::TypeAstVisitor>(astFile);
		File(dirOutput / (L"Ast[BuiltIn-Cpp].txt")).WriteAllText(actualJson, true, BomEncoder::Utf8);
	});

	List<WString> syntaxFileNames;
	syntaxFileNames.Add(L"QualifiedName");
	syntaxFileNames.Add(L"Expressions");
	syntaxFileNames.Add(L"Types");
	syntaxFileNames.Add(L"Statements");
	syntaxFileNames.Add(L"API");
	for (auto syntaxFileName : syntaxFileNames)
	{
		TEST_CASE(L"Parse " + syntaxFileName + L".txt")
		{
			RuleParser ruleParser;
			{
				auto input = File(dirParser / L"Syntax" / L"Syntax" / (syntaxFileName + L".txt")).ReadAllTextByBom();
				auto syntaxFile = ruleParser.ParseFile(input);
				syntaxFiles.Add(syntaxFile);
				auto actualJson = PrintAstJson<json_visitor::RuleAstVisitor>(syntaxFile);
				File(dirOutput / (L"Syntax[BuiltIn-Cpp][" + syntaxFileName + L"].txt")).WriteAllText(actualJson, true, BomEncoder::Utf8);
			}
		});
	}

	ParserSymbolManager global;
	AstSymbolManager astManager(global);
	LexerSymbolManager lexerManager(global);
	SyntaxSymbolManager syntaxManager(global);
	Executable executable;
	Metadata metadata;

	global.name = L"Cpp";
	Fill(global.includes, L"../../../../Source/AstBase.h", L"../../../../Source/SyntaxBase.h");
	Fill(global.cppNss, L"cpp_parser");
	global.headerGuard = L"VCZH_PARSER2_BUILTIN_CPP";
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

		Fill(astDefFile->cppNss, L"cpp_parser");
		Fill(astDefFile->refNss, L"cpp_parser");
		astDefFile->classPrefix = L"Cpp";
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
		auto rewritten = CompileSyntax(astManager, lexerManager, syntaxManager, output, syntaxFiles);
		for (auto error : global.Errors())
		{
			PrintError(error);
		}
		TEST_ASSERT(rewritten);
		{
			auto formattedActual = GenerateToStream([&](TextWriter& writer) { SyntaxAstToCode(rewritten, writer); });
			File(dirOutput / (L"SyntaxRewrittenActual[BuiltIn-Cpp].txt")).WriteAllText(formattedActual, true, BomEncoder::Utf8);
		}
		TEST_ASSERT(global.Errors().Count() == 0);
	
		syntaxManager.BuildCompactNFA();
		TEST_ASSERT(global.Errors().Count() == 0);
		syntaxManager.BuildCrossReferencedNFA();
		TEST_ASSERT(global.Errors().Count() == 0);
		syntaxManager.BuildAutomaton(lexerManager.Tokens().Count(), executable, metadata);
		TEST_ASSERT(global.Errors().Count() == 0);
	
		LogAutomatonWithPath(
			dirOutput / (L"Automaton[BuiltIn-Cpp].txt"),
			executable,
			metadata,
			[&](vint32_t index) { auto type = output->classIds.Keys()[output->classIds.Values().IndexOf(index)]; return type->Name(); },
			[&](vint32_t index) { auto prop = output->fieldIds.Keys()[output->fieldIds.Values().IndexOf(index)]; return prop->Parent()->Name() + L"::" + prop->Name(); },
			[&](vint32_t index) { auto token = lexerManager.Tokens()[lexerManager.TokenOrder()[index]]; return token->displayText == L"" ? token->Name() : L"\"" + token->displayText + L"\""; }
			);

		{
			auto rule = syntaxManager.Rules()[L"_TypeOrExpr"];
			syntaxManager.parsableRules.Add(rule);
			syntaxManager.ruleTypes.Add(rule, L"cpp_parser::CppTypeOrExpr");
		}
		{
			auto rule = syntaxManager.Rules()[L"_Type"];
			syntaxManager.parsableRules.Add(rule);
			syntaxManager.ruleTypes.Add(rule, L"cpp_parser::CppTypeOrExpr");
		}
		{
			auto rule = syntaxManager.Rules()[L"_Expr"];
			syntaxManager.parsableRules.Add(rule);
			syntaxManager.ruleTypes.Add(rule, L"cpp_parser::CppTypeOrExpr");
		}
		{
			auto rule = syntaxManager.Rules()[L"_File"];
			syntaxManager.parsableRules.Add(rule);
			syntaxManager.ruleTypes.Add(rule, L"cpp_parser::CppFile");
		}
		WriteSyntaxFiles(syntaxManager, executable, metadata, output, files);
	});

	if (global.Errors().Count() == 0)
	{
		WriteFilesIfChanged(dirGenerated, files);
	}
}
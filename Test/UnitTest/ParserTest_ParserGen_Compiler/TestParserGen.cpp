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

extern WString GetTestParserInputPath(const WString& parserName);
extern FilePath GetOutputDir(const WString& parserName);

struct ParserDef
{
	WString parserName;
	WString astName;
	WString ruleName;
};

TEST_FILE
{
	TypeParser typeParser;
	RuleParser ruleParser;

	List<ParserDef> parsersToLoad;
	parsersToLoad.Add(ParserDef{ L"Calculator", L"ExprAst", L"Module" });

	for (auto&& [parserName, astName, ruleName] : parsersToLoad)
	{
		TEST_CATEGORY(L"Test ParserGen on " + parserName)
		{
			FilePath dirParser = GetTestParserInputPath(parserName);
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
				auto input = File(dirParser / L"Syntax/Ast.txt").ReadAllTextByBom();
				astFile = typeParser.ParseFile(input);
				auto actualJson = PrintAstJson<json_visitor::TypeAstVisitor>(astFile);
				File(dirOutput / (L"Ast[" + parserName + L"].txt")).WriteAllText(actualJson, BomEncoder::Utf8);
			});

			TEST_CASE(L"Parse Syntax.txt")
			{
				auto input = File(dirParser / L"Syntax/Syntax.txt").ReadAllTextByBom();
				syntaxFile = ruleParser.ParseFile(input);
				auto actualJson = PrintAstJson<json_visitor::RuleAstVisitor>(syntaxFile);
				File(dirOutput / (L"Syntax[" + parserName + L"].txt")).WriteAllText(actualJson, BomEncoder::Utf8);
			});

			ParserSymbolManager global;
			AstSymbolManager astManager(global);
			LexerSymbolManager lexerManager(global);
			SyntaxSymbolManager syntaxManager(global);
			Executable executable;
			Metadata metadata;

			global.name = parserName;
			Fill(global.includes, L"../../../../Source/AstBase.h", L"../../../../Source/SyntaxBase.h");
			global.cppNss.Add(wlower(parserName));
			global.headerGuard = L"VCZH_PARSER2_UNITTEST_" + wupper(parserName);
			syntaxManager.name = ruleName + L"Parser";

			auto astDefFile = astManager.CreateFile(astName);
			auto output = GenerateParserFileNames(global);
			GenerateAstFileNames(astManager, output);
			GenerateSyntaxFileNames(syntaxManager, output);

			Dictionary<WString, WString> files;
			TEST_CASE(L"CompilerAst")
			{
				CompileAst(astManager, astDefFile, astFile);
				TEST_ASSERT(global.Errors().Count() == 0);

				astDefFile->cppNss.Add(wlower(parserName));
				astDefFile->refNss.Add(wlower(parserName));
				astDefFile->classPrefix = L"";
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

				syntaxManager.name = parserName;
				syntaxManager.parsableRules.Add(syntaxManager.Rules()[ruleName]);
				syntaxManager.ruleTypes.Add(syntaxManager.Rules()[ruleName], L"*");
				WriteSyntaxFiles(syntaxManager, executable, metadata, output, files);
			});

			if (global.Errors().Count() == 0)
			{
				for (auto&& [name, content] : files)
				{
					File(dirGenerated / name).WriteAllText(content, false, BomEncoder::Utf8);
				}

				if (parserName == L"Calculator")
				{
					TEST_CASE(L"Ensure Calculator Generated Files Identical")
					{
						// Compare Source/Calculator/Generated with Source/Calculator/Parser
						for (auto&& [name, content] : files)
						{
							auto expected = File(FilePath(GetTestParserInputPath(parserName)) / L"Parser" / name).ReadAllTextByBom();
							TEST_ASSERT(content == expected);
						}
					});
				}
			}
		});
	}

#undef LEXER
}
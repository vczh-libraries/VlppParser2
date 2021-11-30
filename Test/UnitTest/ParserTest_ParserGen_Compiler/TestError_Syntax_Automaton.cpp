#include "../../../Source/ParserGen_Generated/ParserGenTypeParser.h"
#include "../../../Source/ParserGen_Generated/ParserGenRuleParser.h"
#include "../../../Source/ParserGen/Compiler.h"
#include "../../../Source/Ast/AstCppGen.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::glr;
using namespace vl::glr::automaton;
using namespace vl::glr::parsergen;

namespace TestError_Syntax_Automaton_TestObjects
{
	void AssertError(ParserSymbolManager& global, ParserError expectedError)
	{
		TEST_ASSERT(global.Errors().Count() == 1);
		auto&& error = global.Errors()[0];
		TEST_ASSERT(error.type == expectedError.type);
		TEST_ASSERT(error.arg1 == expectedError.arg1);
		TEST_ASSERT(error.arg2 == expectedError.arg2);
		TEST_ASSERT(error.arg3 == expectedError.arg3);
	}

	void ExpectError(TypeParser& typeParser, RuleParser& ruleParser, const WString& astCode, const WString& lexerCode, const WString& syntaxCode, ParserError expectedError)
	{
		ParserSymbolManager global;
		AstSymbolManager astManager(global);
		LexerSymbolManager lexerManager(global);
		SyntaxSymbolManager syntaxManager(global);

		auto astFile = typeParser.ParseFile(astCode);
		auto astDefFile = astManager.CreateFile(L"Ast");
		CompileAst(astManager, astDefFile, astFile);
		TEST_ASSERT(global.Errors().Count() == 0);

		Dictionary<WString, WString> files;
		auto output = GenerateParserFileNames(global);
		GenerateAstFileNames(astManager, output);
		WriteAstFiles(astManager, output, files);

		CompileLexer(lexerManager, lexerCode);
		TEST_ASSERT(global.Errors().Count() == 0);

		List<Ptr<GlrSyntaxFile>> syntaxFiles;
		syntaxFiles.Add(ruleParser.ParseFile(syntaxCode));
		CompileSyntax(astManager, lexerManager, syntaxManager, output, syntaxFiles);

		Executable executable;
		Metadata metadata;
		if (global.Errors().Count() == 0) syntaxManager.BuildCompactNFA();
		if (global.Errors().Count() == 0) syntaxManager.BuildCrossReferencedNFA();
		if (global.Errors().Count() == 0) syntaxManager.BuildAutomaton(lexerManager.TokenOrder().Count(), executable, metadata);

		AssertError(global, expectedError);
	}
}
using namespace TestError_Syntax_Automaton_TestObjects;

TEST_FILE
{
	TypeParser typeParser;
	RuleParser ruleParser;

	const wchar_t* astCode =
LR"AST(
)AST";

	const wchar_t* lexerCode =
LR"LEXER(
)LEXER";

	TEST_CASE(L"DuplicatedRule")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::DuplicatedRule,L"EXP" }
			);
	});
}
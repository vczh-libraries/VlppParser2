#include "TestError.h"

namespace TestError_Syntax_TestObjects
{
	void AssertError(ParserSymbolManager& global, ParserErrorWithoutLocation expectedError)
	{
		TEST_ASSERT(global.Errors().Count() > 0);
		auto&& error = global.Errors()[0];
		TEST_ASSERT(error.type == expectedError.type);
		TEST_ASSERT(error.arg1 == (expectedError.arg1 ?  expectedError.arg1 : L""));
		TEST_ASSERT(error.arg2 == (expectedError.arg2 ?  expectedError.arg2 : L""));
		TEST_ASSERT(error.arg3 == (expectedError.arg3 ?  expectedError.arg3 : L""));
		TEST_ASSERT(error.arg4 == (expectedError.arg4 ?  expectedError.arg4 : L""));
	}

	void ExpectError(TypeParser& typeParser, RuleParser& ruleParser, const WString& astCode, const WString& lexerCode, const WString& syntaxCode, ParserErrorWithoutLocation expectedError)
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

TEST_FILE
{
	TypeParser typeParser;
	RuleParser ruleParser;

	const wchar_t* astCode =
LR"AST(
class Ast {}
)AST";

	const wchar_t* lexerCode =
LR"LEXER(
A:a
)LEXER";

	TEST_CASE(L"DuplicatedRule")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
X ::= A as Ast;
X ::= A as Ast;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::DuplicatedRule,L"X" }
			);
	});

	TEST_CASE(L"RuleIsIndirectlyLeftRecursive")
	{
		const wchar_t* syntaxCode =
LR"SYNTAX(
X
  ::= A as Ast
  ;
Y
  ::= !X
  ::= Z !Y
  ;
Z
  ::= !X
  ::= Y !Z
  ;
)SYNTAX";
		ExpectError(
			typeParser,
			ruleParser,
			astCode,
			lexerCode,
			syntaxCode,
			{ ParserErrorType::RuleIsIndirectlyLeftRecursive,L"Z" }
			);
	});
}
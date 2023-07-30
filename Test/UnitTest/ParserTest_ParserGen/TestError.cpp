#include "TestError.h"

namespace TestError_Syntax_TestObjects
{
	void AssertError(ParserSymbolManager& global, ParserErrorWithoutLocation expectedError)
	{
		TEST_ASSERT(global.Errors().Count() > 0);
		auto&& error = global.Errors()[0];
		TEST_ASSERT(error.type == expectedError.type);
		TEST_ASSERT(error.arg1 == (expectedError.arg1 ? expectedError.arg1 : L""));
		TEST_ASSERT(error.arg2 == (expectedError.arg2 ? expectedError.arg2 : L""));
		TEST_ASSERT(error.arg3 == (expectedError.arg3 ? expectedError.arg3 : L""));
		TEST_ASSERT(error.arg4 == (expectedError.arg4 ? expectedError.arg4 : L""));
	}

	void ExpectError(TypeParser& typeParser, RuleParser& ruleParser, const wchar_t* astCode, const wchar_t* lexerCode, List< const wchar_t*>& syntaxCodes, ParserErrorWithoutLocation expectedError)
	{
		ParserSymbolManager global;
		AstSymbolManager astManager(global);
		LexerSymbolManager lexerManager(global);
		SyntaxSymbolManager syntaxManager(global);

		auto astFile = typeParser.ParseFile(WString::Unmanaged(astCode));
		auto astDefFile = astManager.CreateFile(WString::Unmanaged(L"Ast"));
		CompileAst(astManager, astDefFile, astFile);
		TEST_ASSERT(global.Errors().Count() == 0);

		Dictionary<WString, WString> files;
		auto output = GenerateParserFileNames(global);
		GenerateAstFileNames(astManager, output);
		WriteAstFiles(astManager, output, files);

		CompileLexer(lexerManager, WString::Unmanaged(lexerCode));
		TEST_ASSERT(global.Errors().Count() == 0);

		List<Ptr<GlrSyntaxFile>> syntaxFiles;
		for (auto syntaxCode : syntaxCodes)
		{
			auto syntaxFile = ruleParser.ParseFile(WString::Unmanaged(syntaxCode));
			syntaxFiles.Add(syntaxFile);
		}
		CompileSyntax(astManager, lexerManager, syntaxManager, output, syntaxFiles);

		Executable executable;
		Metadata metadata;
		if (global.Errors().Count() == 0) syntaxManager.BuildCompactNFA();
		if (global.Errors().Count() == 0) syntaxManager.BuildCrossReferencedNFA();
		if (global.Errors().Count() == 0) syntaxManager.BuildAutomaton(lexerManager.TokenOrder().Count(), executable, metadata);

		AssertError(global, expectedError);
	}

	void ExpectError(TypeParser& typeParser, RuleParser& ruleParser, const wchar_t* astCode, const wchar_t* lexerCode, const wchar_t* syntaxCode, ParserErrorWithoutLocation expectedError)
	{
		List<const wchar_t*> syntaxCodes;
		syntaxCodes.Add(syntaxCode);
		ExpectError(typeParser, ruleParser, astCode, lexerCode, syntaxCodes, expectedError);
	}
}
#include "../../../Source/ParserGen_Generated/ParserGenTypeParser.h"
#include "../../../Source/ParserGen_Generated/ParserGenRuleParser.h"
#include "../../../Source/ParserGen/Compiler.h"
#include "../../../Source/Ast/AstCppGen.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::glr;
using namespace vl::glr::automaton;
using namespace vl::glr::parsergen;

extern void AssertError(ParserSymbolManager& global, ParserError expectedError);

namespace TestError_Syntax_Automaton_TestObjects
{
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
class Ast {}
)AST";

	const wchar_t* lexerCode =
LR"LEXER(
A:a
B:b
C:c
)LEXER";

	TEST_CASE(L"DuplicatedRule")
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
			{ ParserErrorType::DuplicatedRule,L"Y" }
			);
	});
}
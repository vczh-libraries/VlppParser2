#include "Generate.h"

TEST_FILE
{
	List<WString> astFileNames;
	astFileNames.Add(L"Ast");

	List<WString> syntaxFileNames;
	syntaxFileNames.Add(L"Syntax");

	FilePath dirParser = GetTestParserInputPath(L"BuiltIn-Workflow");

	ParserSymbolManager global;
	AstSymbolManager astManager(global);
	LexerSymbolManager lexerManager(global);
	SyntaxSymbolManager syntaxManager(global);

	global.name = L"Workflow";
	Fill(global.includes, L"../../../../Source/AstBase.h", L"../../../../Source/SyntaxBase.h");
	Fill(global.cppNss, L"vl", L"glr", L"workflow");
	global.headerGuard = L"VCZH_PARSER2_BUILTIN_WORKFLOW";
	syntaxManager.name = L"Parser";

	auto astDefFileGroup = astManager.CreateFileGroup(L"Ast");
	Fill(astDefFileGroup->cppNss, L"vl", L"glr", L"workflow");
	Fill(astDefFileGroup->refNss, L"system", L"workflow");
	astDefFileGroup->classPrefix = L"Wf";

	GenerateParser(
		L"BuiltIn-Workflow",
		astFileNames,
		syntaxFileNames,
		dirParser,
		global,
		astManager,
		lexerManager,
		syntaxManager,
		astDefFileGroup
		);
}
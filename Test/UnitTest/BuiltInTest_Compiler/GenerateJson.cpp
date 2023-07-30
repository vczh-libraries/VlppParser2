#include "Generate.h"

TEST_FILE
{
	List<WString> astFileNames;
	astFileNames.Add(L"Ast");

	List<WString> syntaxFileNames;
	syntaxFileNames.Add(L"Syntax");

	FilePath dirParser = FilePath(GetSourcePath()) / L"Json";

	ParserSymbolManager global;
	AstSymbolManager astManager(global);
	LexerSymbolManager lexerManager(global);
	SyntaxSymbolManager syntaxManager(global);

	global.name = L"Json";
	Fill(global.includes, L"../../AstBase.h", L"../../SyntaxBase.h");
	Fill(global.cppNss, L"vl", L"glr", L"json");
	global.headerGuard = L"VCZH_PARSER2_BUILTIN_JSON";
	syntaxManager.name = L"Parser";

	auto astDefFileGroup = astManager.CreateFileGroup(L"Ast");
	Fill(astDefFileGroup->cppNss, L"vl", L"glr", L"json");
	Fill(astDefFileGroup->refNss, L"system");
	astDefFileGroup->classPrefix = L"Json";

	GenerateParser(
		L"BuiltIn-Json",
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
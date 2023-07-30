#include "Generate.h"

TEST_FILE
{
	List<WString> astFileNames;
	astFileNames.Add(L"Ast");

	List<WString> syntaxFileNames;
	syntaxFileNames.Add(L"Syntax");

	FilePath dirParser = FilePath(GetSourcePath()) / L"Xml";

	ParserSymbolManager global;
	AstSymbolManager astManager(global);
	LexerSymbolManager lexerManager(global);
	SyntaxSymbolManager syntaxManager(global);

	global.name = L"Xml";
	Fill(global.includes, L"../../AstBase.h", L"../../SyntaxBase.h");
	Fill(global.cppNss, L"vl", L"glr", L"xml");
	global.headerGuard = L"VCZH_PARSER2_BUILTIN_XML";
	syntaxManager.name = L"Parser";

	auto astDefFileGroup = astManager.CreateFileGroup(L"Ast");
	Fill(astDefFileGroup->cppNss, L"vl", L"glr", L"xml");
	Fill(astDefFileGroup->refNss, L"system");
	astDefFileGroup->classPrefix = L"Xml";

	GenerateParser(
		L"BuiltIn-Xml",
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
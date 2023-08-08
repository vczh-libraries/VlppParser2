#include "Generate.h"

TEST_FILE
{
	List<WString> astFileNames;
	astFileNames.Add(L"Ast/Ast");
	astFileNames.Add(L"Ast/Types");
	astFileNames.Add(L"Ast/Decls");
	astFileNames.Add(L"Ast/DeclsClass");
	astFileNames.Add(L"Ast/DeclsEnum");
	astFileNames.Add(L"Ast/DeclsStruct");
	astFileNames.Add(L"Ast/DeclsVirtual");
	astFileNames.Add(L"Ast/Statements");
	astFileNames.Add(L"Ast/StatementsVirtual");
	astFileNames.Add(L"Ast/StatementsCoroutine");
	astFileNames.Add(L"Ast/Expressions");
	astFileNames.Add(L"Ast/ExpressionsVirtual");
	astFileNames.Add(L"Ast/Module");

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
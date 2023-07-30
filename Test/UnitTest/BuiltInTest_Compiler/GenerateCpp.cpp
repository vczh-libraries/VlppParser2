#include "Generate.h"

TEST_FILE
{
	List<WString> astFileNames;
	astFileNames.Add(L"Ast");
	astFileNames.Add(L"QualifiedName");
	astFileNames.Add(L"Expressions");
	astFileNames.Add(L"Types");
	astFileNames.Add(L"DeclsFuncVar");
	astFileNames.Add(L"DeclsClass");
	astFileNames.Add(L"DeclsEnum");
	astFileNames.Add(L"Decls");
	astFileNames.Add(L"Statements");

	List<WString> syntaxFileNames;
	syntaxFileNames.Add(L"QualifiedName");
	syntaxFileNames.Add(L"Expressions");
	syntaxFileNames.Add(L"Types");
	syntaxFileNames.Add(L"Statements");
	syntaxFileNames.Add(L"Generic");
	syntaxFileNames.Add(L"DeclaratorComponents");
	syntaxFileNames.Add(L"DeclaratorConfigurations");
	syntaxFileNames.Add(L"DeclarationVariable");
	syntaxFileNames.Add(L"DeclarationClasses");
	syntaxFileNames.Add(L"DeclarationOthers");
	syntaxFileNames.Add(L"Declarations");
	syntaxFileNames.Add(L"API");

	FilePath dirParser = GetTestParserInputPath(L"BuiltIn-Cpp");

	ParserSymbolManager global;
	AstSymbolManager astManager(global);
	LexerSymbolManager lexerManager(global);
	SyntaxSymbolManager syntaxManager(global);

	global.name = L"Cpp";
	Fill(global.includes, L"../../../../Source/AstBase.h", L"../../../../Source/SyntaxBase.h");
	Fill(global.cppNss, L"cpp_parser");
	global.headerGuard = L"VCZH_PARSER2_BUILTIN_CPP";
	syntaxManager.name = L"Parser";

	auto astDefFileGroup = astManager.CreateFileGroup(L"Ast");
	Fill(astDefFileGroup->cppNss, L"cpp_parser");
	Fill(astDefFileGroup->refNss, L"cpp_parser");
	astDefFileGroup->classPrefix = L"Cpp";

	GenerateParser(
		L"BuiltIn-Cpp",
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
#include "Generate.h"

TEST_FILE
{
	List<WString> astFileNames;
	astFileNames.Add(L"Ast/Ast");
	astFileNames.Add(L"Ast/QualifiedName");
	astFileNames.Add(L"Ast/Expressions");
	astFileNames.Add(L"Ast/Types");
	astFileNames.Add(L"Ast/DeclsFuncVar");
	astFileNames.Add(L"Ast/DeclsClass");
	astFileNames.Add(L"Ast/DeclsEnum");
	astFileNames.Add(L"Ast/Decls");
	astFileNames.Add(L"Ast/Statements");

	List<WString> syntaxFileNames;
	syntaxFileNames.Add(L"Syntax/QualifiedName");
	syntaxFileNames.Add(L"Syntax/Expressions");
	syntaxFileNames.Add(L"Syntax/Types");
	syntaxFileNames.Add(L"Syntax/Statements");
	syntaxFileNames.Add(L"Syntax/Generic");
	syntaxFileNames.Add(L"Syntax/DeclaratorComponents");
	syntaxFileNames.Add(L"Syntax/DeclaratorConfigurations");
	syntaxFileNames.Add(L"Syntax/DeclarationVariable");
	syntaxFileNames.Add(L"Syntax/DeclarationClasses");
	syntaxFileNames.Add(L"Syntax/DeclarationOthers");
	syntaxFileNames.Add(L"Syntax/Declarations");
	syntaxFileNames.Add(L"Syntax/API");

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
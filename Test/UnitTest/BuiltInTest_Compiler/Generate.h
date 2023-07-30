#include "../../../Source/ParserGen_Generated/ParserGenTypeAst_Json.h"
#include "../../../Source/ParserGen_Generated/ParserGenRuleAst_Json.h"
#include "../../../Source/ParserGen_Generated/ParserGenTypeParser.h"
#include "../../../Source/ParserGen_Generated/ParserGenRuleParser.h"
#include "../../../Source/ParserGen_Printer/AstToCode.h"
#include "../../../Source/ParserGen/Compiler.h"
#include "../../../Source/Ast/AstCppGen.h"
#include "../../../Source/Lexer/LexerCppGen.h"
#include "../../../Source/Syntax/SyntaxCppGen.h"
#include "../../Source/LogAutomaton.h"

using namespace vl::glr::parsergen;

extern WString		GetSourcePath();
extern WString		GetTestParserInputPath(const WString& parserName);
extern FilePath		GetOutputDir(const WString& parserName);
extern void			WriteFilesIfChanged(FilePath outputDir, Dictionary<WString, WString>& files);

extern void GenerateParser(
	WString parserName,
	List<WString>& astFileNames,
	List<WString>& syntaxFileNames,
	FilePath dirParser,
	ParserSymbolManager& global,
	AstSymbolManager& astManager,
	LexerSymbolManager& lexerManager,
	SyntaxSymbolManager& syntaxManager,
	AstDefFileGroup* astDefFileGroup
	);
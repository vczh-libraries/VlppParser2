/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_PARSERGEN_COMPILER
#define VCZH_PARSER2_PARSERGEN_COMPILER

#include "../ParserGen_Generated/ParserGenTypeAst.h"
#include "../ParserGen_Generated/ParserGenRuleAst.h"
#include "../Ast/AstSymbol.h"
#include "../Lexer/LexerSymbol.h"
#include "../Syntax/SyntaxSymbol.h"
#include "../ParserGen/ParserCppGen.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
/***********************************************************************
Compiler
***********************************************************************/

			extern void							CompileAst(AstSymbolManager& astManager, AstDefFile* astDefFile, Ptr<GlrAstFile> file);
			extern void							CompileLexer(LexerSymbolManager& lexerManager, const WString& input);
			extern void							CompileSyntax(AstSymbolManager& astManager, LexerSymbolManager& lexerManager, SyntaxSymbolManager& syntaxManager, Ptr<CppParserGenOutput> output, collections::List<Ptr<GlrSyntaxFile>>& files);
		}
	}
}

#endif
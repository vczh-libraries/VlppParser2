/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_SYNTAX_SYNTAXCPPGEN
#define VCZH_PARSER2_SYNTAX_SYNTAXCPPGEN

#include <VlppOS.h>
#include "../ParserGen/ParserCppGen.h"
#include "SyntaxSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			extern Ptr<CppSyntaxGenOutput>		GenerateSyntaxFileNames			(SyntaxSymbolManager& manager, Ptr<CppParserGenOutput> output);
			extern void							WriteSyntaxHeaderFile			(SyntaxSymbolManager& manager, automaton::Executable& executable, automaton::Metadata& metadata, Ptr<CppParserGenOutput> output, stream::StreamWriter& writer);
			extern void							WriteSyntaxCppFile				(SyntaxSymbolManager& manager, automaton::Executable& executable, automaton::Metadata& metadata, Ptr<CppParserGenOutput> output, stream::StreamWriter& writer);
			extern void							WriteSyntaxFiles				(SyntaxSymbolManager& manager, automaton::Executable& executable, automaton::Metadata& metadata, Ptr<CppParserGenOutput> output, collections::Dictionary<WString, WString>& files);
		}
	}
}

#endif
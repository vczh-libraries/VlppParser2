#include "ParserCppGen.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;

/***********************************************************************
GenerateParserFileNames
***********************************************************************/

			Ptr<CppParserGenOutput> GenerateParserFileNames(ParserSymbolManager& manager)
			{
				auto parserOutput = MakePtr<CppParserGenOutput>();
				parserOutput->assemblyH = manager.name + L"_Assembler.h";
				parserOutput->assemblyCpp = manager.name + L"_Assembler.cpp";
				parserOutput->lexerH = manager.name + L"_Lexer.h";
				parserOutput->lexerCpp = manager.name + L"_Lexer.cpp";
				return parserOutput;
			}
		}
	}
}
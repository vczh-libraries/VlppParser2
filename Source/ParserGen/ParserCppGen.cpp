#include "ParserCppGen.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace stream;

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

/***********************************************************************
Utility
***********************************************************************/

			void WriteFileComment(const WString& name, stream::StreamWriter& writer)
			{
				writer.WriteLine(L"/***********************************************************************");
				writer.WriteLine(L"This file is generated by: Vczh Parser Generator");
				writer.WriteLine(L"From parser definition:" + name);
				writer.WriteLine(L"Licensed under https://github.com/vczh-libraries/License");
				writer.WriteLine(L"***********************************************************************/");
				writer.WriteLine(L"");
			}

			WString WriteNssBegin(collections::List<WString>& cppNss, stream::StreamWriter& writer)
			{
				WString prefix;
				for (auto ns : cppNss)
				{
					writer.WriteLine(prefix + L"namespace " + ns);
					writer.WriteLine(prefix + L"{");
					prefix += L"\t";
				}
				return prefix;
			}

			void WriteNssEnd(collections::List<WString>& cppNss, stream::StreamWriter& writer)
			{
				vint counter = cppNss.Count();
				for (auto ns : cppNss)
				{
					counter--;
					for (vint i = 0; i < counter; i++) writer.WriteChar(L'\t');
					writer.WriteLine(L"}");
				}
			}
		}
	}
}
#include "AstCppGen.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace stream;

			extern void PrintCppType(AstDefFile* fileContext, AstSymbol* propSymbol, stream::StreamWriter& writer);

/***********************************************************************
WriteAstAssemblerHeaderFile
***********************************************************************/

			void WriteAstAssemblerHeaderFile(AstSymbolManager& manager, stream::StreamWriter& writer)
			{
				WriteParserHeaderFile(manager, L"ASSEMBLER", writer, [&](const WString& prefix)
				{
				});
			}

/***********************************************************************
WriteAstAssemblerCppFile
***********************************************************************/

			void WriteAstAssemblerCppFile(AstSymbolManager& manager, stream::StreamWriter& writer)
			{
				WriteParserCppFile(manager, L"Assembler", writer, [&](const WString& prefix)
				{
				});
			}
		}
	}
}
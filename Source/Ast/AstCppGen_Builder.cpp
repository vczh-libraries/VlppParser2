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
WriteAstBuilderHeaderFile
***********************************************************************/

			void WriteAstBuilderHeaderFile(AstDefFile* file, stream::StreamWriter& writer)
			{
				WriteUtilityHeaderFile(file, L"BUILDER", L"builder", writer, [&](const WString& prefix)
				{
				});
			}

/***********************************************************************
WriteAstBuilderCppFile
***********************************************************************/

			void WriteAstBuilderCppFile(AstDefFile* file, stream::StreamWriter& writer)
			{
				WriteUtilityCppFile(file, L"Builder", L"builder", writer, [&](const WString& prefix)
				{
				});
			}
		}
	}
}
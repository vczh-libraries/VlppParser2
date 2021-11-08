#include "AstCppGen.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace stream;

/***********************************************************************
WriteEmptyVisitorHeaderFile
***********************************************************************/

			void WriteEmptyVisitorHeaderFile(AstDefFile* file, stream::StreamWriter& writer)
			{
				WriteVisitorHeaderFile(file, L"Empty", writer, [&]()
				{
				});
			}

/***********************************************************************
WriteEmptyVisitorCppFile
***********************************************************************/

			void WriteEmptyVisitorCppFile(AstDefFile* file, stream::StreamWriter& writer)
			{
				WriteVisitorCppFile(file, L"Empty", writer, [&]()
				{
				});
			}
		}
	}
}
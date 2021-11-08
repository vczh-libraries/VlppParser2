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
WriteTraverseVisitorHeaderFile
***********************************************************************/

			void WriteTraverseVisitorHeaderFile(AstDefFile* file, stream::StreamWriter& writer)
			{
				WriteVisitorHeaderFile(file, L"Traverse", writer, [&]()
				{
				});
			}

/***********************************************************************
WriteTraverseVisitorCppFile
***********************************************************************/

			void WriteTraverseVisitorCppFile(AstDefFile* file, stream::StreamWriter& writer)
			{
				WriteVisitorCppFile(file, L"Traverse", writer, [&]()
				{
				});
			}
		}
	}
}
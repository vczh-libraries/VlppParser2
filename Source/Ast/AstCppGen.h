/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_AST_ASTCPPGEN
#define VCZH_PARSER2_AST_ASTCPPGEN

#include <VlppOS.h>
#include "AstSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			extern void				WriteFileComment(const WString& name, stream::StreamWriter& writer);
			extern WString			WriteFileBegin(AstDefFile* file, const WString& includeFile, stream::StreamWriter& writer);
			extern void				WriteFileEnd(AstDefFile* file, stream::StreamWriter& writer);

			extern void				WriteAstHeaderFile(AstDefFile* file, stream::StreamWriter& writer);
			extern void				WriteAstCppFile(AstDefFile* file, stream::StreamWriter& writer);

			extern void				WriteEmptyVisitorHeaderFile(AstDefFile* file, stream::StreamWriter& writer);
			extern void				WriteEmptyVisitorCppFile(AstDefFile* file, stream::StreamWriter& writer);
			extern void				WriteCopyVisitorHeaderFile(AstDefFile* file, stream::StreamWriter& writer);
			extern void				WriteCopyVisitorCppFile(AstDefFile* file, stream::StreamWriter& writer);
			extern void				WriteTraverseVisitorHeaderFile(AstDefFile* file, stream::StreamWriter& writer);
			extern void				WriteTraverseVisitorCppFile(AstDefFile* file, stream::StreamWriter& writer);
		}
	}
}

#endif
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
			extern void				WriteAstHeaderFile(AstDefFile* file, stream::StreamWriter& writer);
			extern void				WriteAstCppFile(AstDefFile* file, stream::StreamWriter& writer);
			extern void				WriteVisitorHeaderFile(AstDefFile* file, const WString& visitorName, stream::StreamWriter& writer, Func<void(const WString&)> callback);
			extern void				WriteVisitorCppFile(AstDefFile* file, const WString& visitorName, stream::StreamWriter& writer, Func<void(const WString&)> callback);
			extern void				WriteEmptyVisitorHeaderFile(AstDefFile* file, stream::StreamWriter& writer);
			extern void				WriteEmptyVisitorCppFile(AstDefFile* file, stream::StreamWriter& writer);
			extern void				WriteCopyVisitorHeaderFile(AstDefFile* file, stream::StreamWriter& writer);
			extern void				WriteCopyVisitorCppFile(AstDefFile* file, stream::StreamWriter& writer);
			extern void				WriteTraverseVisitorHeaderFile(AstDefFile* file, stream::StreamWriter& writer);
			extern void				WriteTraverseVisitorCppFile(AstDefFile* file, stream::StreamWriter& writer);
			extern void				WriteJsonVisitorHeaderFile(AstDefFile* file, stream::StreamWriter& writer);
			extern void				WriteJsonVisitorCppFile(AstDefFile* file, stream::StreamWriter& writer);

			struct CppAstGenOutput
			{
				WString														astH;
				WString														astCpp;
				WString														emptyH;
				WString														emptyCpp;
				WString														copyH;
				WString														copyCpp;
				WString														traverseH;
				WString														traverseCpp;
				WString														jsonH;
				WString														jsonCpp;
			};

			extern Ptr<CppAstGenOutput>			WriteAstFiles(AstDefFile* file, collections::Dictionary<WString, WString>& files);
		}
	}
}

#endif
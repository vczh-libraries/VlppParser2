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
WriteCopyVisitorHeaderFile
***********************************************************************/

			void WriteCopyVisitorHeaderFile(AstDefFile* file, stream::StreamWriter& writer)
			{
				WriteFileComment(file->Name(), writer);
				if (file->headerGuard != L"")
				{
					writer.WriteString(L"#ifndef ");
					writer.WriteLine(file->headerGuard + L"_AST_COPYVISITOR");
					writer.WriteString(L"#define ");
					writer.WriteLine(file->headerGuard + L"_AST_COPYVISITOR");
				}
				else
				{
					writer.WriteLine(L"#pragma once");
				}
				writer.WriteLine(L"");
				WString prefix = WriteFileBegin(file, file->Name(), writer);
				writer.WriteLine(prefix + L"namespace copy_visitor");
				writer.WriteLine(prefix + L"{");
				prefix += L"\t";

				// TODO:

				prefix = prefix.Left(prefix.Length() - 1);
				writer.WriteLine(prefix + L"}");
				WriteFileEnd(file, writer);

				if (file->headerGuard != L"")
				{
					writer.WriteString(L"#endif");
				}
			}

/***********************************************************************
WriteCopyVisitorCppFile
***********************************************************************/

			void WriteCopyVisitorCppFile(AstDefFile* file, stream::StreamWriter& writer)
			{
				WriteFileComment(file->Name(), writer);
				WString prefix = WriteFileBegin(file, file->Name() + L"_Copy", writer);
				writer.WriteLine(prefix + L"namespace copy_visitor");
				writer.WriteLine(prefix + L"{");
				prefix += L"\t";

				// TODO:

				prefix = prefix.Left(prefix.Length() - 1);
				writer.WriteLine(prefix + L"}");
				WriteFileEnd(file, writer);
			}
		}
	}
}
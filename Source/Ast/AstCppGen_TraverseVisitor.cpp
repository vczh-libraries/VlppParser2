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
				WriteFileComment(file->Name(), writer);
				if (file->headerGuard != L"")
				{
					writer.WriteString(L"#ifndef ");
					writer.WriteLine(file->headerGuard + L"_AST_TRAVERSEVISITOR");
					writer.WriteString(L"#define ");
					writer.WriteLine(file->headerGuard + L"_AST_TRAVERSEVISITOR");
				}
				else
				{
					writer.WriteLine(L"#pragma once");
				}
				writer.WriteLine(L"");
				WString prefix = WriteFileBegin(file, file->Name(), writer);
				writer.WriteLine(prefix + L"namespace traverse_visitor");
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
WriteTraverseVisitorCppFile
***********************************************************************/

			void WriteTraverseVisitorCppFile(AstDefFile* file, stream::StreamWriter& writer)
			{
				WriteFileComment(file->Name(), writer);
				WString prefix = WriteFileBegin(file, file->Name() + L"_Traverse", writer);
				writer.WriteLine(prefix + L"namespace traverse_visitor");
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
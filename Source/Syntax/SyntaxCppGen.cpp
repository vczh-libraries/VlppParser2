#include "SyntaxCppGen.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace stream;
			using namespace regex;

/***********************************************************************
WriteSyntaxHeaderFile
***********************************************************************/

			void WriteSyntaxHeaderFile(SyntaxSymbolManager& manager, Ptr<CppParserGenOutput> output, stream::StreamWriter& writer)
			{
				WriteFileComment(manager.Global().name, writer);
				if (manager.Global().headerGuard != L"")
				{
					writer.WriteString(L"#ifndef ");
					writer.WriteLine(manager.Global().headerGuard + L"_SYNTAX");
					writer.WriteString(L"#define ");
					writer.WriteLine(manager.Global().headerGuard + L"_SYNTAX");
				}
				else
				{
					writer.WriteLine(L"#pragma once");
				}
				writer.WriteLine(L"");
				for (auto include : manager.Global().includes)
				{
					writer.WriteLine(L"#include \"" + include + L"\"");
				}
				writer.WriteLine(L"");

				WString prefix = WriteNssBegin(manager.Global().cppNss, writer);
				WriteNssEnd(manager.Global().cppNss, writer);

				if (manager.Global().headerGuard != L"")
				{
					writer.WriteString(L"#endif");
				}
			}

/***********************************************************************
WriteSyntaxCppFile
***********************************************************************/

			void WriteSyntaxCppFile(SyntaxSymbolManager& manager, Ptr<CppParserGenOutput> output, stream::StreamWriter& writer)
			{
				WriteFileComment(manager.Global().name, writer);
				writer.WriteLine(L"#include \"" + output->syntaxH + L"\"");
				writer.WriteLine(L"");
				WString prefix = WriteNssBegin(manager.Global().cppNss, writer);
				WriteNssEnd(manager.Global().cppNss, writer);
			}

/***********************************************************************
WriteLexerFiles
***********************************************************************/

			void WriteSyntaxFiles(SyntaxSymbolManager& manager, Ptr<CppParserGenOutput> output, collections::Dictionary<WString, WString>& files)
			{
				WString fileH = GenerateToStream([&](StreamWriter& writer)
				{
						WriteSyntaxHeaderFile(manager, output, writer);
				});

				WString fileCpp = GenerateToStream([&](StreamWriter& writer)
				{
						WriteSyntaxCppFile(manager, output, writer);
				});

				files.Add(output->syntaxH, fileH);
				files.Add(output->syntaxCpp, fileCpp);
			}
		}
	}
}
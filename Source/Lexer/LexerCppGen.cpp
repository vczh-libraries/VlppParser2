#include "LexerCppGen.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace stream;

			extern void					WriteFileComment(const WString& name, stream::StreamWriter& writer);
			extern WString				WriteNssBegin(List<WString>& cppNss, stream::StreamWriter& writer);
			extern void					WriteNssEnd(List<WString>& cppNss, stream::StreamWriter& writer);

/***********************************************************************
WriteLexerHeaderFile
***********************************************************************/

			void WriteLexerHeaderFile(LexerSymbolManager& manager, Ptr<CppParserGenOutput> output, stream::StreamWriter& writer)
			{
				WriteFileComment(manager.Global().name, writer);
				if (manager.Global().headerGuard != L"")
				{
					writer.WriteString(L"#ifndef ");
					writer.WriteLine(manager.Global().headerGuard + L"_LEXER");
					writer.WriteString(L"#define ");
					writer.WriteLine(manager.Global().headerGuard + L"_LEXER");
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

				{
					WString prefix = WriteNssBegin(manager.Global().cppNss, writer);
					WriteNssEnd(manager.Global().cppNss, writer);
				}

				if (manager.Global().headerGuard != L"")
				{
					writer.WriteString(L"#endif");
				}
			}

/***********************************************************************
WriteLexerCppFile
***********************************************************************/

			void WriteLexerCppFile(LexerSymbolManager& manager, Ptr<CppParserGenOutput> output, stream::StreamWriter& writer)
			{
				WriteFileComment(manager.Global().name, writer);
				writer.WriteLine(L"#include \"" + output->lexerH + L"\"");
				writer.WriteLine(L"");
				{
					WString prefix = WriteNssBegin(manager.Global().cppNss, writer);
					WriteNssEnd(manager.Global().cppNss, writer);
				}
			}

/***********************************************************************
WriteLexerFiles
***********************************************************************/

			void WriteLexerFiles(LexerSymbolManager& manager, Ptr<CppParserGenOutput> output, collections::Dictionary<WString, WString>& files)
			{
				{
					WString fileH = GenerateToStream([&](StreamWriter& writer)
					{
						WriteLexerHeaderFile(manager, output, writer);
					});

					WString fileCpp = GenerateToStream([&](StreamWriter& writer)
					{
						WriteLexerCppFile(manager, output, writer);
					});

					files.Add(output->lexerH, fileH);
					files.Add(output->lexerCpp, fileCpp);
				}
			}
		}
	}
}
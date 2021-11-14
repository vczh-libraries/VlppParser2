#include "LexerCppGen.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace stream;

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

				WString prefix = WriteNssBegin(manager.Global().cppNss, writer);
				{
					vint index = 0;
					writer.WriteLine(prefix + L"enum class " + manager.Global().name + L"Tokens : vl::vint32_t");
					writer.WriteLine(prefix + L"{");
					for (auto tokenName : manager.TokenOrder())
					{
						auto tokenSymbol = manager.Tokens()[tokenName];
						output->tokenIds.Add(tokenSymbol, index);
						writer.WriteLine(prefix + L"\t" + tokenSymbol->Name() + L" = " + itow(index) + L",");
						index++;
					}
					writer.WriteLine(prefix + L"};");
				}
				{
					writer.WriteLine(L"");
					writer.WriteLine(prefix + L"extern bool\t\t\t" + manager.Global().name + L"TokenDeleter(vl::vint token);");
					writer.WriteLine(prefix + L"extern void*\t\t" + manager.Global().name + L"LexerData(void);");
					writer.WriteLine(prefix + L"extern vl::vint\t\t" + manager.Global().name + L"LexerDataSize(void);");
				}
				WriteNssEnd(manager.Global().cppNss, writer);

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
				WString prefix = WriteNssBegin(manager.Global().cppNss, writer);
				{
					writer.WriteLine(prefix + L"bool " + manager.Global().name + L"TokenDeleter(vl::vint token)");
					writer.WriteLine(prefix + L"{");

					List<WString> discarded;
					for (auto tokenSymbol : manager.Tokens().Values())
					{
						if (tokenSymbol->discarded)
						{
							discarded.Add(tokenSymbol->Name());
						}
					}

					if (discarded.Count() > 0)
					{
						writer.WriteLine(prefix + L"\tswitch((" + manager.Global().name + L"Tokens)token)");
						writer.WriteLine(prefix + L"\t{");
						for (auto tokenName : discarded)
						{
							writer.WriteLine(prefix + L"\tcase " + manager.Global().name + L"Tokens::" + tokenName + L":");
						}
						writer.WriteLine(prefix + L"\t\treturn true;");
						writer.WriteLine(prefix + L"\tdefault:");
						writer.WriteLine(prefix + L"\t\treturn false;");
						writer.WriteLine(prefix + L"\t}");
					}
					else
					{
						writer.WriteLine(prefix + L"\treturn false;");
					}
					writer.WriteLine(prefix + L"}");
				}
				{
					writer.WriteLine(L"");
					writer.WriteLine(prefix + L"void* " + manager.Global().name + L"LexerData(void)");
					writer.WriteLine(prefix + L"{");
					writer.WriteLine(prefix + L"\treturn nullptr;");
					writer.WriteLine(prefix + L"}");
				}
				{
					writer.WriteLine(L"");
					writer.WriteLine(prefix + L"vl::vint " + manager.Global().name + L"LexerDataSize(void)");
					writer.WriteLine(prefix + L"{");
					writer.WriteLine(prefix + L"\treturn 0;");
					writer.WriteLine(prefix + L"}");
				}
				WriteNssEnd(manager.Global().cppNss, writer);
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
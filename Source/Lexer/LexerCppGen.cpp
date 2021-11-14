#include "LexerCppGen.h"

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
					writer.WriteLine(prefix + L"extern bool\t\t" + manager.Global().name + L"TokenDeleter(vl::vint token);");
					writer.WriteLine(prefix + L"extern void\t\t" + manager.Global().name + L"LexerData(vl::stream::IStream& outputStream);");
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
					MemoryStream lexerData, compressedData;
					{
						RegexLexer lexer(
							From(manager.TokenOrder())
								.Select([&](const WString& name) { return manager.Tokens()[name]->regex; })
							);
						lexer.Serialize(lexerData);
					}
					lexerData.SeekFromBegin(0);
					{
						LzwEncoder encoder;
						EncoderStream encoderStream(compressedData, encoder);
						CopyStream(lexerData, encoderStream);
					}
					compressedData.SeekFromBegin(0);

					writer.WriteLine(L"");
					writer.WriteLine(prefix + L"void " + manager.Global().name + L"LexerData(vl::stream::IStream& outputStream)");
					writer.WriteLine(prefix + L"{");
					writer.WriteLine(prefix + L"\tstatic const char* compressed[] = {");
					{
						vint lengthBeforeCompressing = (vint)lexerData.Size();
						vint length = (vint)compressedData.Size();
						const vint block = 256;
						vint remain = length % block;
						vint solidRows = length / block;
						vint rows = solidRows + (remain ? 1 : 0);

						char buffer[block];
						const wchar_t* hex = L"0123456789ABCDEF";
						for (vint i = 0; i < rows; i++)
						{
							vint size = i == solidRows ? remain : block;
							vint read = compressedData.Read(buffer, size);
							CHECK_ERROR(size == read, L"vl::glr::parsergen::WriteLexerCppFile()#Failed to read compressed data.");
							writer.WriteString(prefix + L"\t\t\"");
							for (vint j = 0; j < size; j++)
							{
								vuint8_t byte = buffer[j];
								writer.WriteString(L"\\x");
								writer.WriteChar(hex[byte / 16]);
								writer.WriteChar(hex[byte % 16]);
							}
							writer.WriteLine(L"\",");
						}
					}
					writer.WriteLine(prefix + L"\t};");
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
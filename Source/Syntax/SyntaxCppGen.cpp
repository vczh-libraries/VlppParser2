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
GenerateSyntaxFileNames
***********************************************************************/

			Ptr<CppSyntaxGenOutput> GenerateSyntaxFileNames(SyntaxSymbolManager& manager, Ptr<CppParserGenOutput> output)
			{
				auto syntaxOutput = MakePtr<CppSyntaxGenOutput>();
				syntaxOutput->syntaxH = manager.Global().name + manager.name + L".h";
				syntaxOutput->syntaxCpp = manager.Global().name + manager.name + L".cpp";
				output->syntaxOutputs.Add(&manager, syntaxOutput);
				return syntaxOutput;
			}

/***********************************************************************
WriteSyntaxHeaderFile
***********************************************************************/

			void WriteSyntaxHeaderFile(SyntaxSymbolManager& manager, automaton::Executable& executable, automaton::Metadata& metadata, Ptr<CppParserGenOutput> output, stream::StreamWriter& writer)
			{
				WriteFileComment(manager.Global().name, writer);
				if (manager.Global().headerGuard != L"")
				{
					writer.WriteString(L"#ifndef ");
					writer.WriteLine(manager.Global().headerGuard + L"_" + wupper(manager.name) + L"_SYNTAX");
					writer.WriteString(L"#define ");
					writer.WriteLine(manager.Global().headerGuard + L"_" + wupper(manager.name) + L"_SYNTAX");
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

			void WriteSyntaxCppFile(SyntaxSymbolManager& manager, automaton::Executable& executable, automaton::Metadata& metadata, Ptr<CppParserGenOutput> output, stream::StreamWriter& writer)
			{
				WriteFileComment(manager.Global().name, writer);
				writer.WriteLine(L"#include \"" + output->syntaxOutputs[&manager]->syntaxH + L"\"");
				writer.WriteLine(L"");
				WString prefix = WriteNssBegin(manager.Global().cppNss, writer);
				WriteNssEnd(manager.Global().cppNss, writer);
			}

/***********************************************************************
WriteLexerFiles
***********************************************************************/

			void WriteSyntaxFiles(SyntaxSymbolManager& manager, automaton::Executable& executable, automaton::Metadata& metadata, Ptr<CppParserGenOutput> output, collections::Dictionary<WString, WString>& files)
			{
				WString fileH = GenerateToStream([&](StreamWriter& writer)
				{
					WriteSyntaxHeaderFile(manager, executable, metadata, output, writer);
				});

				WString fileCpp = GenerateToStream([&](StreamWriter& writer)
				{
					WriteSyntaxCppFile(manager, executable, metadata, output, writer);
				});

				files.Add(output->syntaxOutputs[&manager]->syntaxH, fileH);
				files.Add(output->syntaxOutputs[&manager]->syntaxCpp, fileCpp);
			}
		}
	}
}
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
				writer.WriteLine(L"#include \"" + output->assemblyH + L"\"");
				writer.WriteLine(L"#include \"" + output->lexerH +L"\"");
				writer.WriteLine(L"");

				WString prefix = WriteNssBegin(manager.Global().cppNss, writer);
				{
					writer.WriteLine(prefix + L"enum class " + manager.name + L"States");
					writer.WriteLine(prefix + L"{");
					for (auto&& [ruleName, index] : indexed(metadata.ruleNames))
					{
						writer.WriteLine(prefix + L"\t" + ruleName + L" = " + itow(executable.ruleStartStates[index]) + L",");
					}
					writer.WriteLine(prefix + L"};");
				}
				{
					writer.WriteLine(L"");
					writer.WriteLine(prefix + L"template<" + manager.name + L"States> struct " + manager.name + L"StateTypes;");
					for(auto ruleName : manager.RuleOrder())
					{
						auto ruleSymbol = manager.Rules()[ruleName];
						if (manager.parsableRules.Contains(ruleSymbol))
						{
							auto astType = manager.ruleTypes[ruleSymbol];
							writer.WriteLine(prefix + L"template<> struct " + manager.name + L"StateTypes<" + manager.name + L"States::" + ruleName + L"> { using Type = " + astType + L"; };");
						}
					}
				}
				{
					writer.WriteLine(L"");
					WriteLoadDataFunctionHeader(prefix, manager.Global().name + manager.name + L"Data", writer);
				}
				{
					writer.WriteLine(L"");
					writer.WriteString(prefix + L"class " + manager.name + L": vl::glr::ParserBase<");
					writer.WriteString(manager.Global().name + L"Tokens, ");
					writer.WriteString(manager.name + L"States, ");
					writer.WriteString(manager.Global().name + L"AstInsReceiver, ");
					writer.WriteLine(manager.name + L"StateTypes>");
					writer.WriteLine(prefix + L"{");
					writer.WriteLine(prefix + L"};");
				}
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
				{
					MemoryStream syntaxData;
					executable.Serialize(syntaxData);
					syntaxData.SeekFromBegin(0);
					WriteLoadDataFunctionCpp(prefix, manager.Global().name + manager.name + L"Data", syntaxData, true, writer);
				}
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
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
GenerateAstFileNames
***********************************************************************/

			void GenerateAstFileNames(AstSymbolManager& manager, Ptr<CppParserGenOutput> parserOutput)
			{
				for (auto file : manager.Files().Values())
				{
					auto astOutput = MakePtr<CppAstGenOutput>();
					astOutput->astH			= file->Owner()->Global().name + file->Name() + L".h";
					astOutput->astCpp		= file->Owner()->Global().name + file->Name() + L".cpp";
					astOutput->builderH		= file->Owner()->Global().name + file->Name() + L"_Builder.h";
					astOutput->builderCpp	= file->Owner()->Global().name + file->Name() + L"_Builder.cpp";
					astOutput->emptyH		= file->Owner()->Global().name + file->Name() + L"_Empty.h";
					astOutput->emptyCpp		= file->Owner()->Global().name + file->Name() + L"_Empty.cpp";
					astOutput->copyH		= file->Owner()->Global().name + file->Name() + L"_Copy.h";
					astOutput->copyCpp		= file->Owner()->Global().name + file->Name() + L"_Copy.cpp";
					astOutput->traverseH	= file->Owner()->Global().name + file->Name() + L"_Traverse.h";
					astOutput->traverseCpp	= file->Owner()->Global().name + file->Name() + L"_Traverse.cpp";
					astOutput->jsonH		= file->Owner()->Global().name + file->Name() + L"_Json.h";
					astOutput->jsonCpp		= file->Owner()->Global().name + file->Name() + L"_Json.cpp";
					parserOutput->astOutputs.Add(file, astOutput);
				}
			}

/***********************************************************************
Utility
***********************************************************************/

			void CollectVisitorsAndConcreteClasses(AstDefFile* file, List<AstClassSymbol*>& visitors, List<AstClassSymbol*>& concreteClasses)
			{
				for (auto name : file->SymbolOrder())
				{
					if (auto classSymbol = dynamic_cast<AstClassSymbol*>(file->Symbols()[name]))
					{
						if (classSymbol->derivedClasses.Count() > 0)
						{
							visitors.Add(classSymbol);
						}
						if (!classSymbol->baseClass && classSymbol->derivedClasses.Count() == 0)
						{
							concreteClasses.Add(classSymbol);
						}
					}
				}
			}

/***********************************************************************
Forward Declarations
***********************************************************************/

			extern void		WriteTypeForwardDefinitions(AstDefFile* file, const WString& prefix, stream::StreamWriter& writer);
			extern void		WriteTypeDefinitions(AstDefFile* file, const WString& prefix, stream::StreamWriter& writer);
			extern void		WriteVisitorImpl(AstDefFile* file, const WString& prefix, stream::StreamWriter& writer);
			extern void		WriteTypeReflectionDeclaration(AstDefFile* file, const WString& prefix, stream::StreamWriter& writer);
			extern void		WriteTypeReflectionImplementation(AstDefFile* file, const WString& prefix, stream::StreamWriter& writer);

/***********************************************************************
WriteAstHeaderFile
***********************************************************************/

			void WriteAstHeaderFile(AstDefFile* file, stream::StreamWriter& writer)
			{
				WriteFileComment(file->Name(), writer);
				auto&& headerGuard = file->Owner()->Global().headerGuard;
				if (headerGuard != L"")
				{
					writer.WriteString(L"#ifndef ");
					writer.WriteLine(headerGuard + L"_" + wupper(file->Name()) + L"_AST");
					writer.WriteString(L"#define ");
					writer.WriteLine(headerGuard + L"_" + wupper(file->Name()) + L"_AST");
				}
				else
				{
					writer.WriteLine(L"#pragma once");
				}
				writer.WriteLine(L"");
				for (auto include : file->Owner()->Global().includes)
				{
					if (include.Length() > 0 && include[0] == L'<')
					{
						writer.WriteLine(L"#include " + include);
					}
					else
					{
						writer.WriteLine(L"#include \"" + include + L"\"");
					}
				}
				writer.WriteLine(L"");

				{
					WString prefix = WriteNssBegin(file->cppNss, writer);
					WriteTypeForwardDefinitions(file, prefix, writer);
					WriteTypeDefinitions(file, prefix, writer);
					WriteNssEnd(file->cppNss, writer);
				}
				{
					List<WString> refNss;
					refNss.Add(L"vl");
					refNss.Add(L"reflection");
					refNss.Add(L"description");
					WString prefix = WriteNssBegin(refNss, writer);
					WriteTypeReflectionDeclaration(file, prefix, writer);
					WriteNssEnd(refNss, writer);
				}

				if (headerGuard != L"")
				{
					writer.WriteString(L"#endif");
				}
			}

/***********************************************************************
WriteAstCppFile
***********************************************************************/

			void WriteAstCppFile(AstDefFile* file, const WString& astHeaderName, stream::StreamWriter& writer)
			{
				WriteFileComment(file->Name(), writer);
				writer.WriteLine(L"#include \"" + astHeaderName + L"\"");
				writer.WriteLine(L"");
				{
					WString prefix = WriteNssBegin(file->cppNss, writer);
					writer.WriteLine(L"/***********************************************************************");
					writer.WriteLine(L"Visitor Pattern Implementation");
					writer.WriteLine(L"***********************************************************************/");
					WriteVisitorImpl(file, prefix, writer);
					WriteNssEnd(file->cppNss, writer);
				}
				{
					List<WString> refNss;
					refNss.Add(L"vl");
					refNss.Add(L"reflection");
					refNss.Add(L"description");
					WString prefix = WriteNssBegin(refNss, writer);
					WriteTypeReflectionImplementation(file, prefix, writer);
					WriteNssEnd(refNss, writer);
				}
			}

/***********************************************************************
WriteAstUtilityHeaderFile
***********************************************************************/

			void WriteAstUtilityHeaderFile(
				AstDefFile* file,
				Ptr<CppAstGenOutput> output,
				const WString& extraNss,
				stream::StreamWriter& writer,
				Func<void(const WString&)> callback
			)
			{
				WriteFileComment(file->Name(), writer);
				auto&& headerGuard = file->Owner()->Global().headerGuard;
				if (headerGuard != L"")
				{
					writer.WriteString(L"#ifndef ");
					writer.WriteLine(headerGuard + L"_" + wupper(file->Name()) + L"_AST_" + wupper(extraNss));
					writer.WriteString(L"#define ");
					writer.WriteLine(headerGuard + L"_" + wupper(file->Name()) + L"_AST_" + wupper(extraNss));
				}
				else
				{
					writer.WriteLine(L"#pragma once");
				}
				writer.WriteLine(L"");
				writer.WriteLine(L"#include \"" + output->astH + L"\"");
				writer.WriteLine(L"");
				{
					List<WString> cppNss;
					CopyFrom(cppNss, file->cppNss);
					cppNss.Add(extraNss);
					WString prefix = WriteNssBegin(cppNss, writer);
					callback(prefix);
					WriteNssEnd(cppNss, writer);
				}
				if (headerGuard != L"")
				{
					writer.WriteString(L"#endif");
				}
			}

/***********************************************************************
WriteAstUtilityCppFile
***********************************************************************/

			void WriteAstUtilityCppFile(
				AstDefFile* file,
				const WString& utilityHeaderFile,
				const WString& extraNss,
				stream::StreamWriter& writer,
				Func<void(const WString&)> callback
			)
			{
				WriteFileComment(file->Name(), writer);
				writer.WriteLine(L"#include \"" + utilityHeaderFile + L"\"");
				writer.WriteLine(L"");
				{
					List<WString> cppNss;
					CopyFrom(cppNss, file->cppNss);
					cppNss.Add(extraNss);
					WString prefix = WriteNssBegin(cppNss, writer);
					callback(prefix);
					WriteNssEnd(cppNss, writer);
				}
			}

/***********************************************************************
WriteParserUtilityHeaderFile
***********************************************************************/

			void WriteParserUtilityHeaderFile(
				AstSymbolManager& manager,
				Ptr<CppParserGenOutput> output,
				const WString& guardPostfix,
				stream::StreamWriter& writer,
				Func<void(const WString&)> callback
			)
			{
				WriteFileComment(manager.Global().name, writer);
				if (manager.Global().headerGuard != L"")
				{
					writer.WriteString(L"#ifndef ");
					writer.WriteLine(manager.Global().headerGuard + L"_AST_" + guardPostfix);
					writer.WriteString(L"#define ");
					writer.WriteLine(manager.Global().headerGuard + L"_AST_" + guardPostfix);
				}
				else
				{
					writer.WriteLine(L"#pragma once");
				}

				writer.WriteLine(L"");
				for (auto file : manager.Files().Values())
				{
					writer.WriteLine(L"#include \"" + output->astOutputs[file]->astH + L"\"");
				}

				writer.WriteLine(L"");
				WString prefix = WriteNssBegin(manager.Global().cppNss, writer);
				callback(prefix);
				WriteNssEnd(manager.Global().cppNss, writer);

				if (manager.Global().headerGuard != L"")
				{
					writer.WriteString(L"#endif");
				}
			}

/***********************************************************************
WriteParserUtilityCppFile
***********************************************************************/

			void WriteParserUtilityCppFile(
				AstSymbolManager& manager,
				const WString& utilityHeaderFile,
				stream::StreamWriter& writer,
				Func<void(const WString&)> callback
			)
			{
				WriteFileComment(manager.Global().name, writer);
				writer.WriteLine(L"#include \"" + utilityHeaderFile + L"\"");
				writer.WriteLine(L"");
				WString prefix = WriteNssBegin(manager.Global().cppNss, writer);
				callback(prefix);
				WriteNssEnd(manager.Global().cppNss, writer);
			}

/***********************************************************************
WriteAstFiles
***********************************************************************/

			void WriteAstFiles(AstDefFile* file, Ptr<CppAstGenOutput> output, collections::Dictionary<WString, WString>& files)
			{
				{
					WString fileH = GenerateToStream([&](StreamWriter& writer)
					{
						WriteAstHeaderFile(file, writer);
					});

					WString fileCpp = GenerateToStream([&](StreamWriter& writer)
					{
						WriteAstCppFile(file, output->astH, writer);
					});

					files.Add(output->astH, fileH);
					files.Add(output->astCpp, fileCpp);
				}

				{
					WString fileH = GenerateToStream([&](StreamWriter& writer)
					{
						WriteAstBuilderHeaderFile(file, output, writer);
					});

					WString fileCpp = GenerateToStream([&](StreamWriter& writer)
					{
						WriteAstBuilderCppFile(file, output, writer);
					});

					files.Add(output->builderH, fileH);
					files.Add(output->builderCpp, fileCpp);
				}

				{
					WString fileH = GenerateToStream([&](StreamWriter& writer)
					{
						WriteEmptyVisitorHeaderFile(file, output, writer);
					});

					WString fileCpp = GenerateToStream([&](StreamWriter& writer)
					{
						WriteEmptyVisitorCppFile(file, output, writer);
					});

					files.Add(output->emptyH, fileH);
					files.Add(output->emptyCpp, fileCpp);
				}

				{
					WString fileH = GenerateToStream([&](StreamWriter& writer)
					{
						WriteCopyVisitorHeaderFile(file, output, writer);
					});

					WString fileCpp = GenerateToStream([&](StreamWriter& writer)
					{
						WriteCopyVisitorCppFile(file, output, writer);
					});

					files.Add(output->copyH, fileH);
					files.Add(output->copyCpp, fileCpp);
				}

				{
					WString fileH = GenerateToStream([&](StreamWriter& writer)
					{
						WriteTraverseVisitorHeaderFile(file, output, writer);
					});

					WString fileCpp = GenerateToStream([&](StreamWriter& writer)
					{
						WriteTraverseVisitorCppFile(file, output, writer);
					});

					files.Add(output->traverseH, fileH);
					files.Add(output->traverseCpp, fileCpp);
				}

				{
					WString fileH = GenerateToStream([&](StreamWriter& writer)
					{
						WriteJsonVisitorHeaderFile(file, output, writer);
					});

					WString fileCpp = GenerateToStream([&](StreamWriter& writer)
					{
						WriteJsonVisitorCppFile(file, output, writer);
					});

					files.Add(output->jsonH, fileH);
					files.Add(output->jsonCpp, fileCpp);
				}
			}

			void WriteAstFiles(AstSymbolManager& manager, Ptr<CppParserGenOutput> output, collections::Dictionary<WString, WString>& files)
			{
				for (auto file : manager.Files().Values())
				{
					WriteAstFiles(file, output->astOutputs[file], files);
				}

				{
					WString fileH = GenerateToStream([&](StreamWriter& writer)
					{
						WriteAstAssemblerHeaderFile(manager, output, writer);
					});

					WString fileCpp = GenerateToStream([&](StreamWriter& writer)
					{
						WriteAstAssemblerCppFile(manager, output, writer);
					});

					files.Add(output->assemblyH, fileH);
					files.Add(output->assemblyCpp, fileCpp);
				}
			}
		}
	}
}
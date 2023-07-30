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
				auto globalName = manager.Global().name;
				for (auto group : manager.FileGroups().Values())
				{
					auto astOutput = Ptr(new CppAstGenOutput);
					astOutput->astH			= globalName + group->Name() + L".h";
					astOutput->astCpp		= globalName + group->Name() + L".cpp";
					astOutput->builderH		= globalName + group->Name() + L"_Builder.h";
					astOutput->builderCpp	= globalName + group->Name() + L"_Builder.cpp";
					astOutput->emptyH		= globalName + group->Name() + L"_Empty.h";
					astOutput->emptyCpp		= globalName + group->Name() + L"_Empty.cpp";
					astOutput->copyH		= globalName + group->Name() + L"_Copy.h";
					astOutput->copyCpp		= globalName + group->Name() + L"_Copy.cpp";
					astOutput->traverseH	= globalName + group->Name() + L"_Traverse.h";
					astOutput->traverseCpp	= globalName + group->Name() + L"_Traverse.cpp";
					astOutput->jsonH		= globalName + group->Name() + L"_Json.h";
					astOutput->jsonCpp		= globalName + group->Name() + L"_Json.cpp";
					parserOutput->astOutputs.Add(group, astOutput);
				}
			}

/***********************************************************************
Utility
***********************************************************************/

			void CollectVisitorsAndConcreteClasses(AstDefFileGroup* group, List<AstClassSymbol*>& visitors, List<AstClassSymbol*>& concreteClasses)
			{
				for (auto name : group->SymbolOrder())
				{
					if (auto classSymbol = dynamic_cast<AstClassSymbol*>(group->Symbols()[name]))
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

			extern void		WriteTypeForwardDefinitions(AstDefFileGroup* group, const WString& prefix, stream::StreamWriter& writer);
			extern void		WriteTypeDefinitions(AstDefFileGroup* group, const WString& prefix, stream::StreamWriter& writer);
			extern void		WriteVisitorImpl(AstDefFileGroup* group, const WString& prefix, stream::StreamWriter& writer);
			extern void		WriteTypeReflectionDeclaration(AstDefFileGroup* group, const WString& prefix, stream::StreamWriter& writer);
			extern void		WriteTypeReflectionImplementation(AstDefFileGroup* group, const WString& prefix, stream::StreamWriter& writer);

/***********************************************************************
WriteAstHeaderFile
***********************************************************************/

			void WriteAstHeaderFile(AstDefFileGroup* group, stream::StreamWriter& writer)
			{
				WriteFileComment(group->Name(), writer);
				auto&& headerGuard = group->Owner()->Global().headerGuard;
				if (headerGuard != L"")
				{
					writer.WriteString(L"#ifndef ");
					writer.WriteLine(headerGuard + L"_" + wupper(group->Name()) + L"_AST");
					writer.WriteString(L"#define ");
					writer.WriteLine(headerGuard + L"_" + wupper(group->Name()) + L"_AST");
				}
				else
				{
					writer.WriteLine(L"#pragma once");
				}
				writer.WriteLine(L"");
				for (auto include : group->Owner()->Global().includes)
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
					WString prefix = WriteNssBegin(group->cppNss, writer);
					WriteTypeForwardDefinitions(group, prefix, writer);
					WriteTypeDefinitions(group, prefix, writer);
					WriteNssEnd(group->cppNss, writer);
				}
				{
					List<WString> refNss;
					refNss.Add(L"vl");
					refNss.Add(L"reflection");
					refNss.Add(L"description");
					WString prefix = WriteNssBegin(refNss, writer);
					WriteTypeReflectionDeclaration(group, prefix, writer);
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

			void WriteAstCppFile(AstDefFileGroup* group, const WString& astHeaderName, stream::StreamWriter& writer)
			{
				WriteFileComment(group->Name(), writer);
				writer.WriteLine(L"#include \"" + astHeaderName + L"\"");
				writer.WriteLine(L"");
				{
					WString prefix = WriteNssBegin(group->cppNss, writer);
					writer.WriteLine(L"/***********************************************************************");
					writer.WriteLine(L"Visitor Pattern Implementation");
					writer.WriteLine(L"***********************************************************************/");
					WriteVisitorImpl(group, prefix, writer);
					WriteNssEnd(group->cppNss, writer);
				}
				{
					List<WString> refNss;
					refNss.Add(L"vl");
					refNss.Add(L"reflection");
					refNss.Add(L"description");
					WString prefix = WriteNssBegin(refNss, writer);
					WriteTypeReflectionImplementation(group, prefix, writer);
					WriteNssEnd(refNss, writer);
				}
			}

/***********************************************************************
WriteAstUtilityHeaderFile
***********************************************************************/

			void WriteAstUtilityHeaderFile(
				AstDefFileGroup* group,
				Ptr<CppAstGenOutput> output,
				const WString& extraNss,
				stream::StreamWriter& writer,
				Func<void(const WString&)> callback
			)
			{
				WriteFileComment(group->Name(), writer);
				auto&& headerGuard = group->Owner()->Global().headerGuard;
				if (headerGuard != L"")
				{
					writer.WriteString(L"#ifndef ");
					writer.WriteLine(headerGuard + L"_" + wupper(group->Name()) + L"_AST_" + wupper(extraNss));
					writer.WriteString(L"#define ");
					writer.WriteLine(headerGuard + L"_" + wupper(group->Name()) + L"_AST_" + wupper(extraNss));
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
					CopyFrom(cppNss, group->cppNss);
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
				AstDefFileGroup* group,
				const WString& utilityHeaderFile,
				const WString& extraNss,
				stream::StreamWriter& writer,
				Func<void(const WString&)> callback
			)
			{
				WriteFileComment(group->Name(), writer);
				writer.WriteLine(L"#include \"" + utilityHeaderFile + L"\"");
				writer.WriteLine(L"");
				{
					List<WString> cppNss;
					CopyFrom(cppNss, group->cppNss);
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
				for (auto group : manager.FileGroups().Values())
				{
					writer.WriteLine(L"#include \"" + output->astOutputs[group]->astH + L"\"");
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

			void WriteAstFiles(AstDefFileGroup* group, Ptr<CppAstGenOutput> output, collections::Dictionary<WString, WString>& files)
			{
				{
					WString fileH = GenerateToStream([&](StreamWriter& writer)
					{
						WriteAstHeaderFile(group, writer);
					});

					WString fileCpp = GenerateToStream([&](StreamWriter& writer)
					{
						WriteAstCppFile(group, output->astH, writer);
					});

					files.Add(output->astH, fileH);
					files.Add(output->astCpp, fileCpp);
				}

				{
					WString fileH = GenerateToStream([&](StreamWriter& writer)
					{
						WriteAstBuilderHeaderFile(group, output, writer);
					});

					WString fileCpp = GenerateToStream([&](StreamWriter& writer)
					{
						WriteAstBuilderCppFile(group, output, writer);
					});

					files.Add(output->builderH, fileH);
					files.Add(output->builderCpp, fileCpp);
				}

				{
					WString fileH = GenerateToStream([&](StreamWriter& writer)
					{
						WriteEmptyVisitorHeaderFile(group, output, writer);
					});

					WString fileCpp = GenerateToStream([&](StreamWriter& writer)
					{
						WriteEmptyVisitorCppFile(group, output, writer);
					});

					files.Add(output->emptyH, fileH);
					files.Add(output->emptyCpp, fileCpp);
				}

				{
					WString fileH = GenerateToStream([&](StreamWriter& writer)
					{
						WriteCopyVisitorHeaderFile(group, output, writer);
					});

					WString fileCpp = GenerateToStream([&](StreamWriter& writer)
					{
						WriteCopyVisitorCppFile(group, output, writer);
					});

					files.Add(output->copyH, fileH);
					files.Add(output->copyCpp, fileCpp);
				}

				{
					WString fileH = GenerateToStream([&](StreamWriter& writer)
					{
						WriteTraverseVisitorHeaderFile(group, output, writer);
					});

					WString fileCpp = GenerateToStream([&](StreamWriter& writer)
					{
						WriteTraverseVisitorCppFile(group, output, writer);
					});

					files.Add(output->traverseH, fileH);
					files.Add(output->traverseCpp, fileCpp);
				}

				{
					WString fileH = GenerateToStream([&](StreamWriter& writer)
					{
						WriteJsonVisitorHeaderFile(group, output, writer);
					});

					WString fileCpp = GenerateToStream([&](StreamWriter& writer)
					{
						WriteJsonVisitorCppFile(group, output, writer);
					});

					files.Add(output->jsonH, fileH);
					files.Add(output->jsonCpp, fileCpp);
				}
			}

			void WriteAstFiles(AstSymbolManager& manager, Ptr<CppParserGenOutput> output, collections::Dictionary<WString, WString>& files)
			{
				for (auto group : manager.FileGroups().Values())
				{
					WriteAstFiles(group, output->astOutputs[group], files);
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
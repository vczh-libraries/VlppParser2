#include "AstCppGen.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace stream;

			extern void PrintNss(List<WString>& nss, stream::StreamWriter& writer);
			extern void PrintCppType(AstDefFile* fileContext, AstSymbol* propSymbol, stream::StreamWriter& writer);
			extern void CollectAllVisitors(AstSymbolManager& manager, List<AstClassSymbol*>& visitors);
			extern void CollectConcreteClasses(AstSymbolManager& manager, List<AstClassSymbol*>& classes);

			extern void CollectCopyDependencies(
				AstClassSymbol* classSymbol,
				bool visitBaseClass,
				bool visitDerivedClasses,
				List<AstClassSymbol*>& copyFields,
				List<AstClassSymbol*>& createFields,
				List<AstClassSymbol*>& virtualCreateFields
				);

/***********************************************************************
WriteTraverseVisitorHeaderFile
***********************************************************************/

			void WriteTraverseVisitorHeaderFile(AstDefFile* file, stream::StreamWriter& writer)
			{
				WriteVisitorHeaderFile(file, L"Traverse", writer, [&](const WString& prefix)
				{
					for (auto name : file->SymbolOrder())
					{
						if (auto classSymbol = dynamic_cast<AstClassSymbol*>(file->Symbols()[name]))
						{
							if (classSymbol->derivedClasses.Count() > 0)
							{
								writer.WriteLine(prefix + L"/// <summary>A traverse visitor, overriding all abstract methods with AST visiting code.</summary>");
								writer.WriteString(prefix + L"class " + name + L"Visitor : public virtual vl::Object, public ");
								PrintCppType(file, classSymbol, writer);
								writer.WriteLine(L"::IVisitor");
								writer.WriteLine(prefix + L"{");
								writer.WriteLine(prefix + L"protected:");

								List<AstClassSymbol*> copyFields;
								List<AstClassSymbol*> createFields;
								List<AstClassSymbol*> virtualCreateFields;
								CollectCopyDependencies(classSymbol, true, true, copyFields, createFields, virtualCreateFields);

								writer.WriteLine(prefix + L"\t// Traverse ------------------------------------------");
								for (auto fieldSymbol : copyFields)
								{
									writer.WriteString(prefix + L"\tvirtual void Traverse(");
									PrintCppType(file, fieldSymbol, writer);
									writer.WriteLine(L"* node);");
								}
								writer.WriteLine(L"");

								writer.WriteLine(prefix + L"\t// VisitField ----------------------------------------");
								for (auto fieldSymbol : createFields)
								{
									writer.WriteString(prefix + L"\tvirtual void VisitField(");
									PrintCppType(file, fieldSymbol, writer);
									writer.WriteLine(L"* node);");
								}
								writer.WriteLine(L"");

								writer.WriteLine(prefix + L"\t// VisitField (virtual) ------------------------------");
								for (auto fieldSymbol : virtualCreateFields)
								{
									writer.WriteString(prefix + L"\tvirtual void VisitField(");
									PrintCppType(file, fieldSymbol, writer);
									writer.WriteLine(L"* node) = 0;");
								}
								writer.WriteLine(L"");

								writer.WriteLine(prefix + L"\t// Dispatch (virtual) --------------------------------");
								for (auto childSymbol : classSymbol->derivedClasses)
								{
									if (childSymbol->derivedClasses.Count() > 0)
									{
										writer.WriteString(prefix + L"\tvirtual void Dispatch(");
										PrintCppType(file, childSymbol, writer);
										writer.WriteLine(L"* node) = 0;");
									}
								}
								writer.WriteLine(L"");

								writer.WriteLine(prefix + L"public:");
								writer.WriteLine(prefix + L"\t// Visitor Members -----------------------------------");
								for (auto childSymbol : classSymbol->derivedClasses)
								{
									writer.WriteString(prefix + L"\tvoid Visit(");
									PrintCppType(file, childSymbol, writer);
									writer.WriteLine(L"* node) override;");
								}

								writer.WriteLine(prefix + L"};");
								writer.WriteLine(L"");
							}
						}
					}
				});
			}

/***********************************************************************
WriteTraverseVisitorCppFile
***********************************************************************/

			void WriteTraverseVisitorCppFile(AstDefFile* file, stream::StreamWriter& writer)
			{
				WriteVisitorCppFile(file, L"Traverse", writer, [&](const WString& prefix)
				{
					for (auto name : file->SymbolOrder())
					{
						if (auto classSymbol = dynamic_cast<AstClassSymbol*>(file->Symbols()[name]))
						{
							if (classSymbol->derivedClasses.Count() > 0)
							{
							}
						}
					}
				});
			}

/***********************************************************************
WriteRootTraverseVisitorHeaderFile
***********************************************************************/

			void WriteRootTraverseVisitorHeaderFile(AstSymbolManager& manager, stream::StreamWriter& writer)
			{
				WriteRootVisitorHeaderFile(manager, L"Traverse", writer, [&](const WString& prefix)
				{
				});
			}

/***********************************************************************
WriteRootTraverseVisitorCppFile
***********************************************************************/

			void WriteRootTraverseVisitorCppFile(AstSymbolManager& manager, stream::StreamWriter& writer)
			{
				WriteRootVisitorCppFile(manager, L"Traverse", writer, [&](const WString& prefix)
				{
				});
			}
		}
	}
}
#include "AstCppGen.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace stream;

			extern void PrintCppType(AstDefFile* fileContext, AstSymbol* propSymbol, stream::StreamWriter& writer);

			void CollectCopyDependencies(
				AstClassSymbol* classSymbol,
				bool visitBaseClass,
				bool visitDerivedClasses,
				List<AstClassSymbol*>& copyFields,
				List<AstClassSymbol*>& createFields,
				List<AstClassSymbol*>& virtualCreateFields
				)
			{
				if (!classSymbol) return;

				if (visitBaseClass)
				{
					CollectCopyDependencies(classSymbol->baseClass, true, false, copyFields, createFields, virtualCreateFields);
				}

				if (!copyFields.Contains(classSymbol))
				{
					copyFields.Add(classSymbol);
				}

				for (auto prop : classSymbol->Props().Values())
				{
					if (auto classProp = dynamic_cast<AstClassSymbol*>(prop->propSymbol))
					{
						if (classProp->derivedClasses.Count() == 0)
						{
							if (!createFields.Contains(classProp))
							{
								createFields.Add(classProp);
								CollectCopyDependencies(classProp, true, false, copyFields, createFields, virtualCreateFields);
							}
						}
						else
						{
							if (!virtualCreateFields.Contains(classProp))
							{
								virtualCreateFields.Add(classProp);
							}
						}
					}
				}

				if (visitDerivedClasses)
				{
					for (auto childSymbol : classSymbol->derivedClasses)
					{
						if (childSymbol->derivedClasses.Count() == 0)
						{
							CollectCopyDependencies(childSymbol, false, false, copyFields, createFields, virtualCreateFields);
						}
					}
				}
			}

/***********************************************************************
WriteCopyVisitorHeaderFile
***********************************************************************/

			void WriteCopyVisitorHeaderFile(AstDefFile* file, stream::StreamWriter& writer)
			{
				WriteVisitorHeaderFile(file, L"Copy", writer, [&](const WString& prefix)
				{
					for (auto name : file->SymbolOrder())
					{
						if (auto classSymbol = dynamic_cast<AstClassSymbol*>(file->Symbols()[name]))
						{
							if (classSymbol->derivedClasses.Count() > 0)
							{
								writer.WriteLine(prefix + L"/// <summary>A copy visitor, overriding all abstract methods with AST copying code.</summary>");
								writer.WriteString(prefix + L"class " + name + L"Visitor : public virtual vl::glr::CopyVisitorBase, public ");
								PrintCppType(file, classSymbol, writer);
								writer.WriteLine(L"::IVisitor");
								writer.WriteLine(prefix + L"{");

								List<AstClassSymbol*> copyFields;
								List<AstClassSymbol*> createFields;
								List<AstClassSymbol*> virtualCreateFields;
								CollectCopyDependencies(classSymbol, true, true, copyFields, createFields, virtualCreateFields);

								writer.WriteLine(prefix + L"protected:");

								writer.WriteLine(prefix + L"\t// CopyFields ----------------------------------------");
								for (auto fieldSymbol : copyFields)
								{
									writer.WriteString(prefix + L"\tvoid CopyFields(");
									PrintCppType(file, fieldSymbol, writer);
									writer.WriteString(L"* from, ");
									PrintCppType(file, fieldSymbol, writer);
									writer.WriteLine(L"* to);");
								}
								writer.WriteLine(L"");

								writer.WriteLine(prefix + L"\t// CreateField ---------------------------------------");
								for (auto fieldSymbol : createFields)
								{
									writer.WriteString(prefix + L"\tvl::Ptr<");
									PrintCppType(file, fieldSymbol, writer);
									writer.WriteString(L"> CreateField(vl::Ptr<");
									PrintCppType(file, fieldSymbol, writer);
									writer.WriteLine(L"> from);");
								}
								writer.WriteLine(L"");

								writer.WriteLine(prefix + L"\t// CreateField (virtual) -----------------------------");
								for (auto fieldSymbol : virtualCreateFields)
								{
									writer.WriteString(prefix + L"\tvirtual vl::Ptr<");
									PrintCppType(file, fieldSymbol, writer);
									writer.WriteString(L"> CreateField(vl::Ptr<");
									PrintCppType(file, fieldSymbol, writer);
									writer.WriteLine(L"> from) = 0;");
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
WriteCopyVisitorCppFile
***********************************************************************/

			void WriteCopyVisitorCppFile(AstDefFile* file, stream::StreamWriter& writer)
			{
				WriteVisitorCppFile(file, L"Copy", writer, [&](const WString& prefix)
				{
					for (auto name : file->SymbolOrder())
					{
						if (auto classSymbol = dynamic_cast<AstClassSymbol*>(file->Symbols()[name]))
						{
							if (classSymbol->derivedClasses.Count() > 0)
							{
								writer.WriteLine(L"");
								writer.WriteLine(L"/***********************************************************************");
								writer.WriteLine(classSymbol->Name() + L"Visitor");
								writer.WriteLine(L"***********************************************************************/");

								List<AstClassSymbol*> copyFields;
								List<AstClassSymbol*> createFields;
								List<AstClassSymbol*> virtualCreateFields;
								CollectCopyDependencies(classSymbol, true, true, copyFields, createFields, virtualCreateFields);

								writer.WriteLine(L"");
								writer.WriteLine(prefix + L"// CopyFields ----------------------------------------");
								for (auto fieldSymbol : copyFields)
								{
									writer.WriteLine(L"");
									writer.WriteString(prefix + L"void " + classSymbol->Name() + L"Visitor::CopyFields(");
									PrintCppType(file, fieldSymbol, writer);
									writer.WriteString(L"* from, ");
									PrintCppType(file, fieldSymbol, writer);
									writer.WriteLine(L"* to)");
									writer.WriteLine(prefix + L"{");

									if (fieldSymbol->baseClass)
									{
										writer.WriteString(prefix + L"\tCopyFields(static_cast<");
										PrintCppType(file, fieldSymbol->baseClass, writer);
										writer.WriteString(L"*>(from), static_cast<");
										PrintCppType(file, fieldSymbol->baseClass, writer);
										writer.WriteLine(L"*>(to));");
									}

									for (auto propSymbol : fieldSymbol->Props().Values())
									{
										switch (propSymbol->propType)
										{
										case AstPropType::Token:
											writer.WriteLine(prefix + L"\tto->" + propSymbol->Name() + L" = from->" + propSymbol->Name() + L";");
											break;
										case AstPropType::Array:
											writer.WriteLine(prefix + L"\tfor (auto listItem : from->" + propSymbol->Name() + L")");
											writer.WriteLine(prefix + L"\t{");
											writer.WriteLine(prefix + L"\t\tto->" + propSymbol->Name() + L".Add(CreateField(listItem));");
											writer.WriteLine(prefix + L"\t}");
											break;
										case AstPropType::Type:
											if (dynamic_cast<AstClassSymbol*>(propSymbol->propSymbol))
											{
												writer.WriteLine(prefix + L"\tto->" + propSymbol->Name() + L" = CreateField(from->" + propSymbol->Name() + L");");
											}
											else
											{
												writer.WriteLine(prefix + L"\tto->" + propSymbol->Name() + L" = from->" + propSymbol->Name() + L";");
											}
											break;
										}
									}
									writer.WriteLine(prefix + L"}");
								}

								writer.WriteLine(L"");
								writer.WriteLine(prefix + L"// CreateField ---------------------------------------");
								for (auto fieldSymbol : createFields)
								{
									writer.WriteLine(L"");
									writer.WriteString(prefix + L"vl::Ptr<");
									PrintCppType(file, fieldSymbol, writer);
									writer.WriteString(L"> " + classSymbol->Name() + L"Visitor::CreateField(vl::Ptr<");
									PrintCppType(file, fieldSymbol, writer);
									writer.WriteLine(L"> from)");
									writer.WriteLine(prefix + L"{");
									writer.WriteLine(prefix + L"\tif (!from) return nullptr;");
									writer.WriteString(prefix + L"\tauto to = vl::MakePtr<");
									PrintCppType(file, fieldSymbol, writer);
									writer.WriteLine(L">();");
									writer.WriteLine(prefix + L"\tCopyFields(from.Obj(), to.Obj());");
									writer.WriteLine(prefix + L"\treturn to;");
									writer.WriteLine(prefix + L"}");
								}

								writer.WriteLine(L"");
								writer.WriteLine(prefix + L"// Visitor Members -----------------------------------");
								for (auto childSymbol : classSymbol->derivedClasses)
								{
									writer.WriteLine(L"");
									writer.WriteString(prefix + L"void " + classSymbol->Name() + L"Visitor::Visit(");
									PrintCppType(file, childSymbol, writer);
									writer.WriteLine(L"* node)");
									writer.WriteLine(prefix + L"{");
									if (childSymbol->derivedClasses.Count() > 0)
									{
										writer.WriteLine(prefix + L"\tDispatch(node);");
									}
									else
									{
										writer.WriteString(prefix + L"\tauto newNode = vl::MakePtr<");
										PrintCppType(file, childSymbol, writer);
										writer.WriteLine(L">();");
										writer.WriteLine(prefix + L"\tCopyFields(node, newNode.Obj());");
										writer.WriteLine(prefix + L"\tthis->result = newNode;");
									}
									writer.WriteLine(prefix + L"}");
								}
							}
						}
					}
				});
			}

/***********************************************************************
WriteRootCopyVisitorHeaderFile
***********************************************************************/

			void WriteRootCopyVisitorHeaderFile(AstSymbolManager& manager, stream::StreamWriter& writer)
			{
				WriteRootVisitorHeaderFile(manager, L"Copy", writer, [&](const WString& prefix)
				{
				});
			}

/***********************************************************************
WriteRootCopyVisitorCppFile
***********************************************************************/

			void WriteRootCopyVisitorCppFile(AstSymbolManager& manager, stream::StreamWriter& writer)
			{
				WriteRootVisitorCppFile(manager, L"Copy", writer, [&](const WString& prefix)
				{
				});
			}
		}
	}
}
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

			void WriteVisitFieldFunctionBody(AstDefFile* file, AstClassSymbol* fieldSymbol, const WString& prefix, stream::StreamWriter& writer)
			{
				writer.WriteLine(prefix + L"\tif (!node) return;");
				List<AstClassSymbol*> order;
				{
					auto current = fieldSymbol;
					while (current)
					{
						order.Add(current);
						current = current->baseClass;
					}
				}

				for (auto classSymbol : order)
				{
					writer.WriteString(prefix + L"\tTraverse(static_cast<");
					PrintCppType(file, classSymbol, writer);
					writer.WriteLine(L"*>(node));");
				}
				writer.WriteLine(prefix + L"\tTraverse(static_cast<vl::glr::ParsingAstBase*>(node));");

				for (auto propSymbol : fieldSymbol->Props().Values())
				{
					switch (propSymbol->propType)
					{
					case AstPropType::Token:
						writer.WriteLine(prefix + L"\tTraverse(node->" + propSymbol->Name() + L");");
						break;
					case AstPropType::Array:
						writer.WriteLine(prefix + L"\tfor (auto&& listItem : node->" + propSymbol->Name() + L")");
						writer.WriteLine(prefix + L"\t{");
						writer.WriteLine(prefix + L"\t\tTraverse(listItem.Obj());");
						writer.WriteLine(prefix + L"\t}");
						break;
					case AstPropType::Type:
						if (dynamic_cast<AstClassSymbol*>(propSymbol->propSymbol))
						{
							writer.WriteLine(prefix + L"\tTraverse(node->" + propSymbol->Name() + L".Obj());");
						}
						break;
					}
				}

				writer.WriteLine(prefix + L"\tFinishing(static_cast<vl::glr::ParsingAstBase*>(node));");
				for (auto classSymbol : From(order).Reverse())
				{
					writer.WriteString(prefix + L"\tFinishing(static_cast<");
					PrintCppType(file, classSymbol, writer);
					writer.WriteLine(L"*>(node));");
				}
			}

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
								writer.WriteLine(prefix + L"\tvirtual void Traverse(vl::glr::ParsingToken& token);");
								writer.WriteLine(prefix + L"\tvirtual void Traverse(vl::glr::ParsingAstBase* node);");
								for (auto fieldSymbol : copyFields)
								{
									writer.WriteString(prefix + L"\tvirtual void Traverse(");
									PrintCppType(file, fieldSymbol, writer);
									writer.WriteLine(L"* node);");
								}
								writer.WriteLine(L"");

								writer.WriteLine(prefix + L"\t// Finishing -----------------------------------------");
								writer.WriteLine(prefix + L"\tvirtual void Finishing(vl::glr::ParsingAstBase* node);");
								for (auto fieldSymbol : copyFields)
								{
									writer.WriteString(prefix + L"\tvirtual void Finishing(");
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
								writer.WriteLine(L"");
								writer.WriteLine(L"/***********************************************************************");
								writer.WriteLine(classSymbol->Name() + L"Visitor");
								writer.WriteLine(L"***********************************************************************/");

								List<AstClassSymbol*> copyFields;
								List<AstClassSymbol*> createFields;
								List<AstClassSymbol*> virtualCreateFields;
								CollectCopyDependencies(classSymbol, true, true, copyFields, createFields, virtualCreateFields);

								writer.WriteLine(L"");
								writer.WriteLine(prefix + L"// Traverse ------------------------------------------");
								{
									writer.WriteLine(L"");
									writer.WriteLine(prefix + L"void " + classSymbol->Name() + L"Visitor::Traverse(vl::glr::ParsingToken& token)");
									writer.WriteLine(prefix + L"{");
									writer.WriteLine(prefix + L"}");
								}
								{
									writer.WriteLine(L"");
									writer.WriteLine(prefix + L"void " + classSymbol->Name() + L"Visitor::Traverse(vl::glr::ParsingAstBase* node)");
									writer.WriteLine(prefix + L"{");
									writer.WriteLine(prefix + L"}");
								}
								for (auto fieldSymbol : copyFields)
								{
									writer.WriteLine(L"");
									writer.WriteString(prefix + L"void " + classSymbol->Name() + L"Visitor::Traverse(");
									PrintCppType(file, fieldSymbol, writer);
									writer.WriteLine(L"* node)");
									writer.WriteLine(prefix + L"{");
									writer.WriteLine(prefix + L"}");
								}

								writer.WriteLine(L"");
								writer.WriteLine(prefix + L"// Finishing -----------------------------------------");
								{
									writer.WriteLine(L"");
									writer.WriteLine(prefix + L"void " + classSymbol->Name() + L"Visitor::Finishing(vl::glr::ParsingAstBase* node)");
									writer.WriteLine(prefix + L"{");
									writer.WriteLine(prefix + L"}");
								}
								for (auto fieldSymbol : copyFields)
								{
									writer.WriteLine(L"");
									writer.WriteString(prefix + L"void " + classSymbol->Name() + L"Visitor::Finishing(");
									PrintCppType(file, fieldSymbol, writer);
									writer.WriteLine(L"* node)");
									writer.WriteLine(prefix + L"{");
									writer.WriteLine(prefix + L"}");
								}

								writer.WriteLine(L"");
								writer.WriteLine(prefix + L"// VisitField ----------------------------------------");
								for (auto fieldSymbol : createFields)
								{
									writer.WriteLine(L"");
									writer.WriteString(prefix + L"void " + classSymbol->Name() + L"Visitor::VisitField(");
									PrintCppType(file, fieldSymbol, writer);
									writer.WriteLine(L"* node)");
									writer.WriteLine(prefix + L"{");
									WriteVisitFieldFunctionBody(file, fieldSymbol, prefix, writer);
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
										WriteVisitFieldFunctionBody(file, childSymbol, prefix, writer);
									}
									writer.WriteLine(prefix + L"}");
								}
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
					writer.WriteLine(prefix + L"/// <summary>A traverse visitor, overriding all abstract methods with AST visiting code.</summary>");
					writer.WriteLine(prefix + L"class " + manager.name + L"RootTraverseVisitor");
					writer.WriteLine(prefix + L"\t: public virtual vl::Object");

					List<AstClassSymbol*> concreteClasses, visitors;
					CollectAllVisitors(manager, visitors);
					CollectConcreteClasses(manager, concreteClasses);

					for (auto visitor : visitors)
					{
						writer.WriteString(prefix + L"\t, public virtual ");
						PrintNss(visitor->Owner()->cppNss, writer);
						writer.WriteString(L"traverse_visitor::");
						writer.WriteString(visitor->Name());
						writer.WriteLine(L"Visitor");
					}
					writer.WriteLine(prefix + L"{");
					writer.WriteLine(prefix + L"protected:");

					List<AstClassSymbol*> copyFields, allCopyFields, allCreateFields, allVirtualCreateFields;
					{
						List<AstClassSymbol*> _1;
						List<AstClassSymbol*> _2;
						for (auto concreteSymbol : concreteClasses)
						{
							CollectCopyDependencies(concreteSymbol, true, false, copyFields, _1, _2);
						}
					}
					{
						for (auto visitor : visitors)
						{
							CollectCopyDependencies(visitor, true, true, allCopyFields, allCreateFields, allVirtualCreateFields);
						}
					}

					writer.WriteLine(prefix + L"\t// Traverse ------------------------------------------");
					writer.WriteLine(prefix + L"\tvirtual void Traverse(vl::glr::ParsingToken& token) override;");
					writer.WriteLine(prefix + L"\tvirtual void Traverse(vl::glr::ParsingAstBase* node) override;");
					for (auto fieldSymbol : From(visitors).Concat(copyFields).Distinct())
					{
						bool needOverride = allCopyFields.Contains(fieldSymbol);
						writer.WriteString(prefix + L"\tvirtual void Traverse(");
						PrintCppType(nullptr, fieldSymbol, writer);
						writer.WriteString(L"* node)");
						if (needOverride)
						{
							writer.WriteString(L" override");
						}
						writer.WriteLine(L";");
					}
					writer.WriteLine(L"");

					writer.WriteLine(prefix + L"\t// Finishing -----------------------------------------");
					writer.WriteLine(prefix + L"\tvirtual void Finishing(vl::glr::ParsingAstBase* node) override;");
					for (auto fieldSymbol : From(visitors).Concat(copyFields).Distinct())
					{
						bool needOverride = allCopyFields.Contains(fieldSymbol);
						writer.WriteString(prefix + L"\tvirtual void Finishing(");
						PrintCppType(nullptr, fieldSymbol, writer);
						writer.WriteString(L"* node)");
						if (needOverride)
						{
							writer.WriteString(L" override");
						}
						writer.WriteLine(L";");
					}
					writer.WriteLine(L"");

					writer.WriteLine(prefix + L"\t// Dispatch (virtual) --------------------------------");
					for (auto childSymbol : visitors)
					{
						if (childSymbol->baseClass)
						{
							writer.WriteString(prefix + L"\tvoid Dispatch(");
							PrintCppType(nullptr, childSymbol, writer);
							writer.WriteLine(L"* node) override;");
						}
					}
					writer.WriteLine(L"");

					writer.WriteLine(prefix + L"public:");
					writer.WriteLine(prefix + L"\t// VisitField ----------------------------------------");
					{
						List<AstClassSymbol*> neededSymbols;
						CopyFrom(
							neededSymbols,
							From(concreteClasses)
								.Concat(visitors)
								.Concat(allCreateFields)
								.Concat(allVirtualCreateFields)
								.Distinct()
							);
						for (auto fieldSymbol : neededSymbols)
						{
							bool needOverride = allCreateFields.Contains(fieldSymbol) || allVirtualCreateFields.Contains(fieldSymbol);
							writer.WriteString(prefix + L"\tvoid VisitField(");
							PrintCppType(nullptr, fieldSymbol, writer);
							writer.WriteString(L"* node)");
							if (needOverride)
							{
								writer.WriteString(L" override");
							}
							writer.WriteLine(L";");
						}
					}
					writer.WriteLine(prefix + L"};");
					writer.WriteLine(L"");
				});
			}

/***********************************************************************
WriteRootTraverseVisitorCppFile
***********************************************************************/

			void WriteRootTraverseVisitorCppFile(AstSymbolManager& manager, stream::StreamWriter& writer)
			{
				WriteRootVisitorCppFile(manager, L"Traverse", writer, [&](const WString& prefix)
				{
					List<AstClassSymbol*> concreteClasses, visitors;
					CollectAllVisitors(manager, visitors);
					CollectConcreteClasses(manager, concreteClasses);

					List<AstClassSymbol*> copyFields, allCopyFields, allCreateFields, allVirtualCreateFields;
					{
						List<AstClassSymbol*> _1;
						List<AstClassSymbol*> _2;
						for (auto concreteSymbol : concreteClasses)
						{
							CollectCopyDependencies(concreteSymbol, true, false, copyFields, _1, _2);
						}
					}
					{
						for (auto visitor : visitors)
						{
							CollectCopyDependencies(visitor, true, true, allCopyFields, allCreateFields, allVirtualCreateFields);
						}
					}

					writer.WriteLine(L"");
					writer.WriteLine(prefix + L"// Traverse ------------------------------------------");
					{
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"void " + manager.name + L"RootTraverseVisitor::Traverse(vl::glr::ParsingToken& token)");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"}");
					}
					{
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"void " + manager.name + L"RootTraverseVisitor::Traverse(vl::glr::ParsingAstBase* node)");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"}");
					}
					for (auto fieldSymbol : From(visitors).Concat(copyFields).Distinct())
					{
						writer.WriteLine(L"");
						writer.WriteString(prefix + L"void " + manager.name + L"RootTraverseVisitor::Traverse(");
						PrintCppType(nullptr, fieldSymbol, writer);
						writer.WriteLine(L"* node)");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"}");
					}

					writer.WriteLine(L"");
					writer.WriteLine(prefix + L"// Finishing -----------------------------------------");
					{
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"void " + manager.name + L"RootTraverseVisitor::Finishing(vl::glr::ParsingAstBase* node)");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"}");
					}
					for (auto fieldSymbol : From(visitors).Concat(copyFields).Distinct())
					{
						writer.WriteLine(L"");
						writer.WriteString(prefix + L"void " + manager.name + L"RootTraverseVisitor::Finishing(");
						PrintCppType(nullptr, fieldSymbol, writer);
						writer.WriteLine(L"* node)");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"}");
					}

					writer.WriteLine(L"");
					writer.WriteLine(prefix + L"// Dispatch (virtual) --------------------------------");
					for (auto childSymbol : visitors)
					{
						if (childSymbol->baseClass)
						{
							writer.WriteLine(L"");
							writer.WriteString(prefix + L"void " + manager.name + L"RootTraverseVisitor::Dispatch(");
							PrintCppType(nullptr, childSymbol, writer);
							writer.WriteLine(L"* node)");
							writer.WriteLine(prefix + L"{");
							writer.WriteString(prefix + L"\tnode->Accept(static_cast<");
							PrintCppType(nullptr, childSymbol, writer);
							writer.WriteLine(L"::IVisitor*>(this));");
							writer.WriteLine(prefix + L"}");
						}
					}

					writer.WriteLine(L"");
					writer.WriteLine(prefix + L"// VisitField ----------------------------------------");
					{
						List<AstClassSymbol*> neededSymbols;
						CopyFrom(
							neededSymbols,
							From(concreteClasses)
							.Concat(visitors)
							.Concat(allCreateFields)
							.Concat(allVirtualCreateFields)
							.Distinct()
						);
						for (auto fieldSymbol : neededSymbols)
						{
							writer.WriteLine(L"");
							writer.WriteString(prefix + L"void " + manager.name + L"RootTraverseVisitor::VisitField(");
							PrintCppType(nullptr, fieldSymbol, writer);
							writer.WriteLine(L"* node)");
							writer.WriteLine(prefix + L"{");
							if (fieldSymbol->derivedClasses.Count() > 0)
							{
								if (fieldSymbol->baseClass)
								{
									writer.WriteLine(prefix + L"\tDispatch(node);");
								}
								else
								{
									writer.WriteString(prefix + L"\tnode->Accept(static_cast<");
									PrintCppType(nullptr, fieldSymbol, writer);
									writer.WriteLine(L"::IVisitor*>(this));");
								}
							}
							else
							{
								WriteVisitFieldFunctionBody(nullptr, fieldSymbol, prefix, writer);
							}
							writer.WriteLine(prefix + L"}");
						}
					}
				});
			}
		}
	}
}
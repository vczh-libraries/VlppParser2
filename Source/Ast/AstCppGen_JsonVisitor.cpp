#include "AstCppGen.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace stream;

			extern void PrintCppType(AstDefFileGroup* fileGroupContext, AstSymbol* propSymbol, stream::StreamWriter& writer);
			extern void CollectVisitorsAndConcreteClasses(AstDefFileGroup* group, List<AstClassSymbol*>& visitors, List<AstClassSymbol*>& concreteClasses);

/***********************************************************************
WriteVisitFieldFunctionBody
***********************************************************************/

			void WritePrintFieldsFunctionBody(AstDefFileGroup* group, AstClassSymbol* fieldSymbol, const WString& prefix, stream::StreamWriter& writer)
			{
				for (auto propSymbol : fieldSymbol->Props().Values())
				{
					writer.WriteLine(prefix + L"\tBeginField(vl::WString::Unmanaged(L\"" + propSymbol->Name() + L"\"));");
					switch (propSymbol->propType)
					{
					case AstPropType::Token:
						writer.WriteLine(prefix + L"\tWriteToken(node->" + propSymbol->Name() + L");");
						break;
					case AstPropType::Array:
						writer.WriteLine(prefix + L"\tBeginArray();");
						writer.WriteLine(prefix + L"\tfor (auto&& listItem : node->" + propSymbol->Name() + L")");
						writer.WriteLine(prefix + L"\t{");
						writer.WriteLine(prefix + L"\t\tBeginArrayItem();");
						writer.WriteLine(prefix + L"\t\tPrint(listItem.Obj());");
						writer.WriteLine(prefix + L"\t\tEndArrayItem();");
						writer.WriteLine(prefix + L"\t}");
						writer.WriteLine(prefix + L"\tEndArray();");
						break;
					case AstPropType::Type:
						if (auto enumPropSymbol = dynamic_cast<AstEnumSymbol*>(propSymbol->propSymbol))
						{
							writer.WriteLine(prefix + L"\tswitch (node->" + propSymbol->Name() + L")");
							writer.WriteLine(prefix + L"\t{");
							for (auto enumItemSymbol : enumPropSymbol->Items().Values())
							{
								writer.WriteString(prefix + L"\tcase ");
								PrintCppType(nullptr, enumPropSymbol, writer);
								writer.WriteLine(L"::" + enumItemSymbol->Name() + L":");
								writer.WriteLine(prefix + L"\t\tWriteString(vl::WString::Unmanaged(L\"" + enumItemSymbol->Name() + L"\"));");
								writer.WriteLine(prefix + L"\t\tbreak;");
							}
							writer.WriteLine(prefix + L"\tdefault:");
							writer.WriteLine(prefix + L"\t\tWriteNull();");
							writer.WriteLine(prefix + L"\t}");
						}
						if (dynamic_cast<AstClassSymbol*>(propSymbol->propSymbol))
						{
							writer.WriteLine(prefix + L"\tPrint(node->" + propSymbol->Name() + L".Obj());");
						}
						break;
					}
					writer.WriteLine(prefix + L"\tEndField();");
				}
			}

			void WriteNullAndReturn(const WString& prefix, stream::StreamWriter& writer)
			{
				writer.WriteLine(prefix + L"\tif (!node)");
				writer.WriteLine(prefix + L"\t{");
				writer.WriteLine(prefix + L"\t\tWriteNull();");
				writer.WriteLine(prefix + L"\t\treturn;");
				writer.WriteLine(prefix + L"\t}");
			}

			void WriteVisitFunctionBody(AstDefFileGroup* group, AstClassSymbol* fieldSymbol, const WString& prefix, stream::StreamWriter& writer)
			{
				WriteNullAndReturn(prefix, writer);
				List<AstClassSymbol*> order;
				{
					auto current = fieldSymbol;
					while (current)
					{
						order.Add(current);
						current = current->baseClass;
					}
				}

				writer.WriteLine(prefix + L"\tBeginObject();");
				writer.WriteLine(prefix + L"\tWriteType(vl::WString::Unmanaged(L\"" + fieldSymbol->Name() + L"\"), node);");
				for (auto classSymbol : From(order).Reverse())
				{
					writer.WriteString(prefix + L"\tPrintFields(static_cast<");
					PrintCppType(group, classSymbol, writer);
					writer.WriteLine(L"*>(node));");
				}
				writer.WriteLine(prefix + L"\tEndObject();");
			}

/***********************************************************************
WriteJsonVisitorHeaderFile
***********************************************************************/

			void WriteJsonVisitorHeaderFile(AstDefFileGroup* group, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer)
			{
				WriteAstUtilityHeaderFile(group, output, L"json_visitor", writer, [&](const WString& prefix)
				{
					List<AstClassSymbol*> visitors, concreteClasses;
					CollectVisitorsAndConcreteClasses(group, visitors, concreteClasses);

					writer.WriteLine(prefix + L"/// <summary>A JSON visitor, overriding all abstract methods with AST to JSON serialization code.</summary>");
					writer.WriteLine(prefix + L"class " + group->Name() + L"Visitor");
					writer.WriteLine(prefix + L"\t: public vl::glr::JsonVisitorBase");
					for (auto visitorSymbol : visitors)
					{
						writer.WriteString(prefix + L"\t, protected virtual ");
						PrintCppType(group, visitorSymbol, writer);
						writer.WriteLine(L"::IVisitor");
					}
					writer.WriteLine(prefix + L"{");

					writer.WriteLine(prefix + L"protected:");
					for (auto typeSymbol : group->Symbols().Values())
					{
						if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
						{
							writer.WriteString(prefix + L"\tvirtual void PrintFields(");
							PrintCppType(group, classSymbol, writer);
							writer.WriteLine(L"* node);");
						}
					}
					writer.WriteLine(L"");

					writer.WriteLine(prefix + L"protected:");
					for (auto visitorSymbol : visitors)
					{
						for (auto classSymbol : visitorSymbol->derivedClasses)
						{
							writer.WriteString(prefix + L"\tvoid Visit(");
							PrintCppType(group, classSymbol, writer);
							writer.WriteLine(L"* node) override;");
						}
						writer.WriteLine(L"");
					}

					writer.WriteLine(prefix + L"public:");
					writer.WriteLine(prefix + L"\t" + group->Name() + L"Visitor(vl::stream::StreamWriter& _writer);");
					writer.WriteLine(L"");
					for (auto classSymbol :
						From(visitors)
							.Where([](AstClassSymbol* visitor) { return !visitor->baseClass; })
							.Concat(concreteClasses)
						)
					{
						writer.WriteString(prefix + L"\tvoid Print(");
						PrintCppType(group, classSymbol, writer);
						writer.WriteLine(L"* node);");
					}
					writer.WriteLine(prefix + L"};");
				});
			}

/***********************************************************************
WriteJsonVisitorCppFile
***********************************************************************/

			void WriteJsonVisitorCppFile(AstDefFileGroup* group, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer)
			{
				WriteAstUtilityCppFile(group, output->jsonH, L"json_visitor", writer, [&](const WString& prefix)
				{
					List<AstClassSymbol*> visitors, concreteClasses;
					CollectVisitorsAndConcreteClasses(group, visitors, concreteClasses);

					for (auto typeSymbol : group->Symbols().Values())
					{
						if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
						{
							writer.WriteString(prefix + L"void " + group->Name() + L"Visitor::PrintFields(");
							PrintCppType(group, classSymbol, writer);
							writer.WriteLine(L"* node)");
							writer.WriteLine(prefix + L"{");
							WritePrintFieldsFunctionBody(group, classSymbol, prefix, writer);
							writer.WriteLine(prefix + L"}");
						}
					}
					writer.WriteLine(L"");

					for (auto visitorSymbol : visitors)
					{
						for (auto classSymbol : visitorSymbol->derivedClasses)
						{
							writer.WriteString(prefix + L"void " + group->Name() + L"Visitor::Visit(");
							PrintCppType(group, classSymbol, writer);
							writer.WriteLine(L"* node)");
							writer.WriteLine(prefix + L"{");
							if (classSymbol->derivedClasses.Count() == 0)
							{
								WriteVisitFunctionBody(group, classSymbol, prefix, writer);
							}
							else
							{
								writer.WriteString(prefix + L"\tnode->Accept(static_cast<");
								PrintCppType(group, classSymbol, writer);
								writer.WriteLine(L"::IVisitor*>(this));");
							}
							writer.WriteLine(prefix + L"}");
							writer.WriteLine(L"");
						}
					}

					writer.WriteLine(prefix + group->Name() + L"Visitor::" + group->Name() + L"Visitor(vl::stream::StreamWriter& _writer)");
					writer.WriteLine(prefix + L"\t: vl::glr::JsonVisitorBase(_writer)");
					writer.WriteLine(prefix + L"{");
					writer.WriteLine(prefix + L"}");
					writer.WriteLine(L"");

					for (auto classSymbol : visitors)
					{
						if (!classSymbol->baseClass)
						{
							writer.WriteString(prefix + L"void " + group->Name() + L"Visitor::Print(");
							PrintCppType(group, classSymbol, writer);
							writer.WriteLine(L"* node)");
							writer.WriteLine(prefix + L"{");
							WriteNullAndReturn(prefix, writer);
							writer.WriteString(prefix + L"\tnode->Accept(static_cast<");
							PrintCppType(group, classSymbol, writer);
							writer.WriteLine(L"::IVisitor*>(this));");
							writer.WriteLine(prefix + L"}");
							writer.WriteLine(L"");
						}
					}

					for (auto classSymbol : concreteClasses)
					{
						writer.WriteString(prefix + L"void " + group->Name() + L"Visitor::Print(");
						PrintCppType(group, classSymbol, writer);
						writer.WriteLine(L"* node)");
						writer.WriteLine(prefix + L"{");
						WriteVisitFunctionBody(group, classSymbol, prefix, writer);
						writer.WriteLine(prefix + L"}");
						writer.WriteLine(L"");
					}
				});
			}

/***********************************************************************
WriteJsonVisitorDtsFile
***********************************************************************/

			void CollectAllLeafClasses(AstClassSymbol* classSymbol, List<AstClassSymbol*>& leafClasses)
			{
				if (classSymbol->derivedClasses.Count() == 0)
				{
					leafClasses.Add(classSymbol);
				}
				else
				{
					for (auto derived : classSymbol->derivedClasses)
					{
						CollectAllLeafClasses(derived, leafClasses);
					}
				}
			}

			AstClassSymbol* FindNearestAncestorWithProps(AstClassSymbol* classSymbol)
			{
				auto current = classSymbol->baseClass;
				while (current)
				{
					if (current->Props().Count() > 0)
					{
						return current;
					}
					current = current->baseClass;
				}
				return nullptr;
			}

			void WriteDtsPropType(AstDefFileGroup* group, AstClassPropSymbol* propSymbol, stream::StreamWriter& writer)
			{
				switch (propSymbol->propType)
				{
				case AstPropType::Token:
					writer.WriteString(L"string");
					break;
				case AstPropType::Type:
					if (auto enumPropSymbol = dynamic_cast<AstEnumSymbol*>(propSymbol->propSymbol))
					{
						writer.WriteString(enumPropSymbol->Name());
					}
					else if (auto classPropSymbol = dynamic_cast<AstClassSymbol*>(propSymbol->propSymbol))
					{
						writer.WriteString(classPropSymbol->Name() + L" | null");
					}
					break;
				case AstPropType::Array:
					if (auto classPropSymbol = dynamic_cast<AstClassSymbol*>(propSymbol->propSymbol))
					{
						writer.WriteString(L"(" + classPropSymbol->Name() + L" | null)[]");
					}
					else
					{
						writer.WriteString(L"unknown[]");
					}
					break;
				}
			}

			void WriteJsonVisitorDtsFile(AstDefFileGroup* group, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer)
			{
				List<AstClassSymbol*> visitors, concreteClasses;
				CollectVisitorsAndConcreteClasses(group, visitors, concreteClasses);

				// write enums
				for (auto name : group->SymbolOrder())
				{
					if (auto enumSymbol = dynamic_cast<AstEnumSymbol*>(group->Symbols()[name]))
					{
						writer.WriteString(L"export type " + enumSymbol->Name() + L" =");
						bool first = true;
						for (auto itemName : enumSymbol->ItemOrder())
						{
							if (!first) writer.WriteString(L" |");
							writer.WriteString(L" \"" + itemName + L"\"");
							first = false;
						}
						writer.WriteLine(L";");
						writer.WriteLine(L"");
					}
				}

				// write union types for abstract classes (classes with derived classes)
				for (auto visitorSymbol : visitors)
				{
					List<AstClassSymbol*> leafClasses;
					CollectAllLeafClasses(visitorSymbol, leafClasses);

					writer.WriteString(L"export type " + visitorSymbol->Name() + L" =");
					for (auto [leafClass, index] : indexed(leafClasses))
					{
						if (index > 0) writer.WriteString(L" |");
						writer.WriteString(L" " + leafClass->Name());
					}
					writer.WriteLine(L";");
					writer.WriteLine(L"");
				}

				// write _Common interfaces for abstract classes that have own properties
				for (auto name : group->SymbolOrder())
				{
					if (auto classSymbol = dynamic_cast<AstClassSymbol*>(group->Symbols()[name]))
					{
						if (classSymbol->derivedClasses.Count() > 0 && classSymbol->Props().Count() > 0)
						{
							auto ancestor = FindNearestAncestorWithProps(classSymbol);
							writer.WriteString(L"export interface " + classSymbol->Name() + L"_Common");
							if (ancestor)
							{
								writer.WriteString(L" extends " + ancestor->Name() + L"_Common");
							}
							writer.WriteLine(L" {");
							for (auto propName : classSymbol->PropOrder())
							{
								auto propSymbol = classSymbol->Props()[propName];
								writer.WriteString(L"    " + propName + L": ");
								WriteDtsPropType(group, propSymbol, writer);
								writer.WriteLine(L";");
							}
							writer.WriteLine(L"}");
							writer.WriteLine(L"");
						}
					}
				}

				// write interfaces for leaf classes (classes with no derived classes)
				for (auto name : group->SymbolOrder())
				{
					if (auto classSymbol = dynamic_cast<AstClassSymbol*>(group->Symbols()[name]))
					{
						if (classSymbol->derivedClasses.Count() == 0)
						{
							// find the nearest ancestor with properties for extends
							auto ancestor = FindNearestAncestorWithProps(classSymbol);
							writer.WriteString(L"export interface " + classSymbol->Name());
							if (ancestor)
							{
								if (ancestor->derivedClasses.Count() > 0)
								{
									writer.WriteString(L" extends " + ancestor->Name() + L"_Common");
								}
								else
								{
									writer.WriteString(L" extends " + ancestor->Name());
								}
							}
							writer.WriteLine(L" {");
							writer.WriteLine(L"    $ast: \"" + classSymbol->Name() + L"\";");
							for (auto propName : classSymbol->PropOrder())
							{
								auto propSymbol = classSymbol->Props()[propName];
								writer.WriteString(L"    " + propName + L": ");
								WriteDtsPropType(group, propSymbol, writer);
								writer.WriteLine(L";");
							}
							writer.WriteLine(L"}");
							writer.WriteLine(L"");
						}
					}
				}

				// write interfaces for concrete classes (standalone, no base and no derived)
				for (auto classSymbol : concreteClasses)
				{
					// concreteClasses are !baseClass && derivedClasses.Count()==0
					// these are already handled in the leaf class section above
					// (they would appear as leaf classes with no ancestor)
				}
			}
		}
	}
}
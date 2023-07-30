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

			void WriteVisitFieldFunctionBody(AstDefFileGroup* group, AstClassSymbol* fieldSymbol, const WString& prefix, stream::StreamWriter& writer)
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

				writer.WriteLine(prefix + L"\tTraverse(static_cast<vl::glr::ParsingAstBase*>(node));");
				for (auto classSymbol : From(order).Reverse())
				{
					writer.WriteString(prefix + L"\tTraverse(static_cast<");
					PrintCppType(group, classSymbol, writer);
					writer.WriteLine(L"*>(node));");
				}

				{
					auto current = fieldSymbol;
					while (current)
					{
						for (auto propSymbol : current->Props().Values())
						{
							switch (propSymbol->propType)
							{
							case AstPropType::Token:
								writer.WriteLine(prefix + L"\tTraverse(node->" + propSymbol->Name() + L");");
								break;
							case AstPropType::Array:
								writer.WriteLine(prefix + L"\tfor (auto&& listItem : node->" + propSymbol->Name() + L")");
								writer.WriteLine(prefix + L"\t{");
								writer.WriteLine(prefix + L"\t\tInspectInto(listItem.Obj());");
								writer.WriteLine(prefix + L"\t}");
								break;
							case AstPropType::Type:
								if (dynamic_cast<AstClassSymbol*>(propSymbol->propSymbol))
								{
									writer.WriteLine(prefix + L"\tInspectInto(node->" + propSymbol->Name() + L".Obj());");
								}
								break;
							}
						}
						current = current->baseClass;
					}
				}

				for (auto classSymbol : order)
				{
					writer.WriteString(prefix + L"\tFinishing(static_cast<");
					PrintCppType(group, classSymbol, writer);
					writer.WriteLine(L"*>(node));");
				}
				writer.WriteLine(prefix + L"\tFinishing(static_cast<vl::glr::ParsingAstBase*>(node));");
			}

/***********************************************************************
WriteTraverseVisitorHeaderFile
***********************************************************************/

			void WriteTraverseVisitorHeaderFile(AstDefFileGroup* group, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer)
			{
				WriteAstUtilityHeaderFile(group, output, L"traverse_visitor", writer, [&](const WString& prefix)
				{
					List<AstClassSymbol*> visitors, concreteClasses;
					CollectVisitorsAndConcreteClasses(group, visitors, concreteClasses);

					writer.WriteLine(prefix + L"/// <summary>A traverse visitor, overriding all abstract methods with AST visiting code.</summary>");
					writer.WriteLine(prefix + L"class " + group->Name() + L"Visitor");
					writer.WriteLine(prefix + L"\t: public vl::Object");
					for (auto visitorSymbol : visitors)
					{
						writer.WriteString(prefix + L"\t, protected virtual ");
						PrintCppType(group, visitorSymbol, writer);
						writer.WriteLine(L"::IVisitor");
					}
					writer.WriteLine(prefix + L"{");

					writer.WriteLine(prefix + L"protected:");
					writer.WriteLine(prefix + L"\tvirtual void Traverse(vl::glr::ParsingToken& token);");
					writer.WriteLine(prefix + L"\tvirtual void Traverse(vl::glr::ParsingAstBase* node);");
					for (auto typeSymbol : group->Symbols().Values())
					{
						if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
						{
							writer.WriteString(prefix + L"\tvirtual void Traverse(");
							PrintCppType(group, classSymbol, writer);
							writer.WriteLine(L"* node);");
						}
					}
					writer.WriteLine(L"");

					writer.WriteLine(prefix + L"protected:");
					writer.WriteLine(prefix + L"\tvirtual void Finishing(vl::glr::ParsingAstBase* node);");
					for (auto typeSymbol : group->Symbols().Values())
					{
						if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
						{
							writer.WriteString(prefix + L"\tvirtual void Finishing(");
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
					for (auto classSymbol :
						From(visitors)
							.Where([](AstClassSymbol* visitor) { return !visitor->baseClass; })
							.Concat(concreteClasses)
						)
					{
						writer.WriteString(prefix + L"\tvoid InspectInto(");
						PrintCppType(group, classSymbol, writer);
						writer.WriteLine(L"* node);");
					}
					writer.WriteLine(prefix + L"};");
				});
			}

/***********************************************************************
WriteTraverseVisitorCppFile
***********************************************************************/

			void WriteTraverseVisitorCppFile(AstDefFileGroup* group, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer)
			{
				WriteAstUtilityCppFile(group, output->traverseH, L"traverse_visitor", writer, [&](const WString& prefix)
				{
					List<AstClassSymbol*> visitors, concreteClasses;
					CollectVisitorsAndConcreteClasses(group, visitors, concreteClasses);

					writer.WriteLine(prefix + L"void " + group->Name() + L"Visitor::Traverse(vl::glr::ParsingToken& token) {}");
					writer.WriteLine(prefix + L"void " + group->Name() + L"Visitor::Traverse(vl::glr::ParsingAstBase* node) {}");
					for (auto typeSymbol : group->Symbols().Values())
					{
						if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
						{
							writer.WriteString(prefix + L"void " + group->Name() + L"Visitor::Traverse(");
							PrintCppType(group, classSymbol, writer);
							writer.WriteLine(L"* node) {}");
						}
					}
					writer.WriteLine(L"");

					writer.WriteLine(prefix + L"void " + group->Name() + L"Visitor::Finishing(vl::glr::ParsingAstBase* node) {}");
					for (auto typeSymbol : group->Symbols().Values())
					{
						if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
						{
							writer.WriteString(prefix + L"void " + group->Name() + L"Visitor::Finishing(");
							PrintCppType(group, classSymbol, writer);
							writer.WriteLine(L"* node) {}");
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
								WriteVisitFieldFunctionBody(group, classSymbol, prefix, writer);
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

					for (auto classSymbol : visitors)
					{
						if (!classSymbol->baseClass)
						{
							writer.WriteString(prefix + L"void " + group->Name() + L"Visitor::InspectInto(");
							PrintCppType(group, classSymbol, writer);
							writer.WriteLine(L"* node)");
							writer.WriteLine(prefix + L"{");
							writer.WriteLine(prefix + L"\tif (!node) return;");
							writer.WriteString(prefix + L"\tnode->Accept(static_cast<");
							PrintCppType(group, classSymbol, writer);
							writer.WriteLine(L"::IVisitor*>(this));");
							writer.WriteLine(prefix + L"}");
							writer.WriteLine(L"");
						}
					}

					for (auto classSymbol : concreteClasses)
					{
						writer.WriteString(prefix + L"void " + group->Name() + L"Visitor::InspectInto(");
						PrintCppType(group, classSymbol, writer);
						writer.WriteLine(L"* node)");
						writer.WriteLine(prefix + L"{");
						WriteVisitFieldFunctionBody(group, classSymbol, prefix, writer);
						writer.WriteLine(prefix + L"}");
						writer.WriteLine(L"");
					}
				});
			}
		}
	}
}
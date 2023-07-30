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
					writer.WriteLine(prefix + L"\tBeginField(L\"" + propSymbol->Name() + L"\");");
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
								writer.WriteLine(prefix + L"\t\tWriteString(L\"" + enumItemSymbol->Name() + L"\");");
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
				writer.WriteLine(prefix + L"\tWriteType(L\"" + fieldSymbol->Name() + L"\", node);");
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
		}
	}
}
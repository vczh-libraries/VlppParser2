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
			extern void CollectVisitorsAndConcreteClasses(AstDefFile* file, List<AstClassSymbol*>& visitors, List<AstClassSymbol*>& concreteClasses);

/***********************************************************************
WriteVisitFieldFunctionBody
***********************************************************************/

			void WritePrintFieldsFunctionBody(AstDefFile* file, AstClassSymbol* fieldSymbol, const WString& prefix, stream::StreamWriter& writer)
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
						if (dynamic_cast<AstClassSymbol*>(propSymbol->propSymbol))
						{
							writer.WriteLine(prefix + L"\tPrint(node->" + propSymbol->Name() + L".Obj());");
						}
						break;
					}
					writer.WriteLine(prefix + L"\tEndField();");
				}
			}

			void WriteVisitFunctionBody(AstDefFile* file, AstClassSymbol* fieldSymbol, const WString& prefix, stream::StreamWriter& writer)
			{
				writer.WriteLine(prefix + L"\tif (!node)");
				writer.WriteLine(prefix + L"\t{");
				writer.WriteLine(prefix + L"\t\tWriteNull();");
				writer.WriteLine(prefix + L"\t\treturn;");
				writer.WriteLine(prefix + L"\t}");
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
					PrintCppType(file, classSymbol, writer);
					writer.WriteLine(L"*>(node));");
				}
				writer.WriteLine(prefix + L"\tEndObject();");
			}

/***********************************************************************
WriteJsonVisitorHeaderFile
***********************************************************************/

			void WriteJsonVisitorHeaderFile(AstDefFile* file, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer)
			{
				WriteAstUtilityHeaderFile(file, output, L"json_visitor", writer, [&](const WString& prefix)
				{
					List<AstClassSymbol*> visitors, concreteClasses;
					CollectVisitorsAndConcreteClasses(file, visitors, concreteClasses);

					writer.WriteLine(prefix + L"/// <summary>A JSON visitor, overriding all abstract methods with AST to JSON serialization code.</summary>");
					writer.WriteLine(prefix + L"class " + file->Name() + L"Visitor");
					writer.WriteLine(prefix + L"\t: public vl::glr::JsonVisitorBase");
					for (auto visitorSymbol : visitors)
					{
						writer.WriteString(prefix + L"\t, protected virtual ");
						PrintCppType(file, visitorSymbol, writer);
						writer.WriteLine(L"::IVisitor");
					}
					writer.WriteLine(prefix + L"{");

					writer.WriteLine(prefix + L"protected:");
					for (auto typeSymbol : file->Symbols().Values())
					{
						if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
						{
							writer.WriteString(prefix + L"\tvirtual void PrintFields(");
							PrintCppType(file, classSymbol, writer);
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
							PrintCppType(file, classSymbol, writer);
							writer.WriteLine(L"* node) override;");
						}
						writer.WriteLine(L"");
					}

					writer.WriteLine(prefix + L"public:");
					writer.WriteLine(prefix + L"\t" + file->Name() + L"Visitor(vl::stream::StreamWriter& _writer);");
					writer.WriteLine(L"");
					for (auto classSymbol :
						From(visitors)
							.Where([](AstClassSymbol* visitor) { return !visitor->baseClass; })
							.Concat(concreteClasses)
						)
					{
						writer.WriteString(prefix + L"\tvoid Print(");
						PrintCppType(file, classSymbol, writer);
						writer.WriteLine(L"* node);");
					}
					writer.WriteLine(prefix + L"};");
				});
			}

/***********************************************************************
WriteJsonVisitorCppFile
***********************************************************************/

			void WriteJsonVisitorCppFile(AstDefFile* file, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer)
			{
				WriteAstUtilityCppFile(file, output->jsonH, L"json_visitor", writer, [&](const WString& prefix)
				{
					List<AstClassSymbol*> visitors, concreteClasses;
					CollectVisitorsAndConcreteClasses(file, visitors, concreteClasses);

					for (auto typeSymbol : file->Symbols().Values())
					{
						if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
						{
							writer.WriteString(prefix + L"void " + file->Name() + L"Visitor::PrintFields(");
							PrintCppType(file, classSymbol, writer);
							writer.WriteLine(L"* node)");
							writer.WriteLine(prefix + L"{");
							WritePrintFieldsFunctionBody(file, classSymbol, prefix, writer);
							writer.WriteLine(prefix + L"}");
						}
					}
					writer.WriteLine(L"");

					for (auto visitorSymbol : visitors)
					{
						for (auto classSymbol : visitorSymbol->derivedClasses)
						{
							writer.WriteString(prefix + L"void " + file->Name() + L"Visitor::Visit(");
							PrintCppType(file, classSymbol, writer);
							writer.WriteLine(L"* node)");
							writer.WriteLine(prefix + L"{");
							if (classSymbol->derivedClasses.Count() == 0)
							{
								WriteVisitFunctionBody(file, classSymbol, prefix, writer);
							}
							else
							{
								writer.WriteString(prefix + L"\tnode->Accept(static_cast<");
								PrintCppType(file, classSymbol, writer);
								writer.WriteLine(L"::IVisitor*>(this));");
							}
							writer.WriteLine(prefix + L"}");
							writer.WriteLine(L"");
						}
					}

					writer.WriteLine(prefix + file->Name() + L"Visitor::" + file->Name() + L"Visitor(vl::stream::StreamWriter& _writer)");
					writer.WriteLine(prefix + L"\t: vl::glr::JsonVisitorBase(_writer)");
					writer.WriteLine(prefix + L"{");
					writer.WriteLine(prefix + L"}");
					writer.WriteLine(L"");

					for (auto classSymbol : visitors)
					{
						if (!classSymbol->baseClass)
						{
							writer.WriteString(prefix + L"void " + file->Name() + L"Visitor::Print(");
							PrintCppType(file, classSymbol, writer);
							writer.WriteLine(L"* node)");
							writer.WriteLine(prefix + L"{");
							writer.WriteLine(prefix + L"\tif (!node) return;");
							writer.WriteString(prefix + L"\tnode->Accept(static_cast<");
							PrintCppType(file, classSymbol, writer);
							writer.WriteLine(L"::IVisitor*>(this));");
							writer.WriteLine(prefix + L"}");
							writer.WriteLine(L"");
						}
					}

					for (auto classSymbol : concreteClasses)
					{
						writer.WriteString(prefix + L"void " + file->Name() + L"Visitor::Print(");
						PrintCppType(file, classSymbol, writer);
						writer.WriteLine(L"* node)");
						writer.WriteLine(prefix + L"{");
						WriteVisitFunctionBody(file, classSymbol, prefix, writer);
						writer.WriteLine(prefix + L"}");
						writer.WriteLine(L"");
					}
				});
			}
		}
	}
}
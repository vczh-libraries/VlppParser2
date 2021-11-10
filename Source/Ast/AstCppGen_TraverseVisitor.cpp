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
					List<AstClassSymbol*> visitors, concreteClasses;
					CollectVisitorsAndConcreteClasses(file, visitors, concreteClasses);

					writer.WriteLine(prefix + L"/// <summary>A traverse visitor, overriding all abstract methods with AST visiting code.</summary>");
					writer.WriteLine(prefix + L"class " + file->Name() + L"Visitor");
					writer.WriteLine(prefix + L"\t: public vl::Object");
					for (auto visitorSymbol : visitors)
					{
						writer.WriteString(prefix + L"\t, protected virtual ");
						PrintCppType(file, visitorSymbol, writer);
						writer.WriteLine(L"::IVisitor");
					}
					writer.WriteLine(prefix + L"{");

					writer.WriteLine(prefix + L"protected:");
					writer.WriteLine(prefix + L"\tvirtual void Traverse(vl::glr::ParsingToken& token);");
					writer.WriteLine(prefix + L"\tvirtual void Traverse(vl::glr::ParsingAstBase* node);");
					for (auto typeSymbol : file->Symbols().Values())
					{
						if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
						{
							writer.WriteString(prefix + L"\tvirtual void Traverse(");
							PrintCppType(file, classSymbol, writer);
							writer.WriteString(L"* from, ");
							PrintCppType(file, classSymbol, writer);
							writer.WriteLine(L"* to);");
						}
					}
					writer.WriteLine(L"");

					writer.WriteLine(prefix + L"protected:");
					writer.WriteLine(prefix + L"\tvirtual void Finishing(vl::glr::ParsingAstBase* node);");
					for (auto typeSymbol : file->Symbols().Values())
					{
						if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
						{
							writer.WriteString(prefix + L"\tvirtual void Finishing(");
							PrintCppType(file, classSymbol, writer);
							writer.WriteString(L"* from, ");
							PrintCppType(file, classSymbol, writer);
							writer.WriteLine(L"* to);");
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
					for (auto classSymbol :
						From(visitors)
							.Where([](AstClassSymbol* visitor) { return !visitor->baseClass; })
							.Concat(concreteClasses)
						)
					{
						writer.WriteString(prefix + L"\tvoid InspectInto(");
						PrintCppType(file, classSymbol, writer);
						writer.WriteLine(L"* node);");
					}
					writer.WriteLine(prefix + L"};");
				});
			}

/***********************************************************************
WriteTraverseVisitorCppFile
***********************************************************************/

			void WriteTraverseVisitorCppFile(AstDefFile* file, stream::StreamWriter& writer)
			{
				WriteVisitorCppFile(file, L"Traverse", writer, [&](const WString& prefix)
				{
				});
			}
		}
	}
}
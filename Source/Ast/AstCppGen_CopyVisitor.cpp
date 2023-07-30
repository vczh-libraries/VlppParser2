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
WriteCopyFieldFunctionBody
***********************************************************************/

			void WriteCopyFieldFunctionBody(AstDefFileGroup* group, AstClassSymbol* fieldSymbol, const WString& prefix, stream::StreamWriter& writer)
			{
				if (fieldSymbol->baseClass)
				{
					writer.WriteString(prefix + L"\tCopyFields(static_cast<");
					PrintCppType(group, fieldSymbol->baseClass, writer);
					writer.WriteString(L"*>(from), static_cast<");
					PrintCppType(group, fieldSymbol->baseClass, writer);
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
						writer.WriteLine(prefix + L"\tfor (auto&& listItem : from->" + propSymbol->Name() + L")");
						writer.WriteLine(prefix + L"\t{");
						writer.WriteLine(prefix + L"\t\tto->" + propSymbol->Name() + L".Add(CopyNode(listItem.Obj()));");
						writer.WriteLine(prefix + L"\t}");
						break;
					case AstPropType::Type:
						if (dynamic_cast<AstClassSymbol*>(propSymbol->propSymbol))
						{
							writer.WriteLine(prefix + L"\tto->" + propSymbol->Name() + L" = CopyNode(from->" + propSymbol->Name() + L".Obj());");
						}
						else
						{
							writer.WriteLine(prefix + L"\tto->" + propSymbol->Name() + L" = from->" + propSymbol->Name() + L";");
						}
						break;
					}
				}
			}

/***********************************************************************
WriteCopyVisitorHeaderFile
***********************************************************************/

			void WriteCopyVisitorHeaderFile(AstDefFileGroup* group, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer)
			{
				WriteAstUtilityHeaderFile(group, output, L"copy_visitor", writer, [&](const WString& prefix)
				{
					List<AstClassSymbol*> visitors, concreteClasses;
					CollectVisitorsAndConcreteClasses(group, visitors, concreteClasses);

					writer.WriteLine(prefix + L"/// <summary>A copy visitor, overriding all abstract methods with AST copying code.</summary>");
					writer.WriteLine(prefix + L"class " + group->Name() + L"Visitor");
					writer.WriteLine(prefix + L"\t: public virtual vl::glr::CopyVisitorBase");
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
							writer.WriteString(prefix + L"\tvoid CopyFields(");
							PrintCppType(group, classSymbol, writer);
							writer.WriteString(L"* from, ");
							PrintCppType(group, classSymbol, writer);
							writer.WriteLine(L"* to);");
						}
					}
					writer.WriteLine(L"");

					writer.WriteLine(prefix + L"protected:");
					for (auto classSymbol : concreteClasses)
					{
						writer.WriteString(prefix + L"\tvirtual void Visit(");
						PrintCppType(group, classSymbol, writer);
						writer.WriteLine(L"* node);");
					}
					writer.WriteLine(L"");
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
							.Where([](AstClassSymbol* visitor) { return visitor->baseClass == nullptr; })
							.Concat(concreteClasses)
						)
					{
						writer.WriteString(prefix + L"\tvirtual vl::Ptr<");
						PrintCppType(group, classSymbol, writer);
						writer.WriteString(L"> CopyNode(");
						PrintCppType(group, classSymbol, writer);
						writer.WriteLine(L"* node);");
					}

					writer.WriteLine(L"");
					for (auto classSymbol :
						From(group->Symbols().Values())
							.Select([](AstSymbol* derivedClass) { return dynamic_cast<AstClassSymbol*>(derivedClass); })
							.Where([](AstClassSymbol* derivedClass) { return derivedClass && derivedClass->baseClass != nullptr; })
						)
					{
						writer.WriteString(prefix + L"\tvl::Ptr<");
						PrintCppType(group, classSymbol, writer);
						writer.WriteString(L"> CopyNode(");
						PrintCppType(group, classSymbol, writer);
						writer.WriteLine(L"* node);");
					}
					writer.WriteLine(prefix + L"};");
				});
			}

/***********************************************************************
WriteCopyVisitorCppFile
***********************************************************************/

			void WriteCopyVisitorCppFile(AstDefFileGroup* group, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer)
			{
				WriteAstUtilityCppFile(group, output->copyH, L"copy_visitor", writer, [&](const WString& prefix)
				{
					List<AstClassSymbol*> visitors, concreteClasses;
					CollectVisitorsAndConcreteClasses(group, visitors, concreteClasses);

					for (auto typeSymbol : group->Symbols().Values())
					{
						if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
						{
							writer.WriteString(prefix + L"void " + group->Name() + L"Visitor::CopyFields(");
							PrintCppType(group, classSymbol, writer);
							writer.WriteString(L"* from, ");
							PrintCppType(group, classSymbol, writer);
							writer.WriteLine(L"* to)");
							writer.WriteLine(prefix + L"{");
							WriteCopyFieldFunctionBody(group, classSymbol, prefix, writer);
							writer.WriteLine(prefix + L"}");
							writer.WriteLine(L"");
						}
					}

					for (auto classSymbol : concreteClasses)
					{
						writer.WriteString(prefix + L"void " + group->Name() + L"Visitor::Visit(");
						PrintCppType(group, classSymbol, writer);
						writer.WriteLine(L"* node)");
						writer.WriteLine(prefix + L"{");
						writer.WriteString(prefix + L"\tauto newNode = vl::Ptr(new ");
						PrintCppType(group, classSymbol, writer);
						writer.WriteLine(L");");
						writer.WriteLine(prefix + L"\tCopyFields(node, newNode.Obj());");
						writer.WriteLine(prefix + L"\tthis->result = newNode;");
						writer.WriteLine(prefix + L"}");
						writer.WriteLine(L"");
					}

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
								writer.WriteString(prefix + L"\tauto newNode = vl::Ptr(new ");
								PrintCppType(group, classSymbol, writer);
								writer.WriteLine(L");");
								writer.WriteLine(prefix + L"\tCopyFields(node, newNode.Obj());");
								writer.WriteLine(prefix + L"\tthis->result = newNode;");
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
							writer.WriteString(prefix + L"vl::Ptr<");
							PrintCppType(group, classSymbol, writer);
							writer.WriteString(L"> " + group->Name() + L"Visitor::CopyNode(");
							PrintCppType(group, classSymbol, writer);
							writer.WriteLine(L"* node)");
							writer.WriteLine(prefix + L"{");
							writer.WriteLine(prefix + L"\tif (!node) return nullptr;");
							writer.WriteString(prefix + L"\tnode->Accept(static_cast<");
							PrintCppType(group, classSymbol, writer);
							writer.WriteLine(L"::IVisitor*>(this));");
							writer.WriteLine(prefix + L"\tthis->result->codeRange = node->codeRange;");
							writer.WriteString(prefix + L"\treturn this->result.Cast<");
							PrintCppType(group, classSymbol, writer);
							writer.WriteLine(L">();");
							writer.WriteLine(prefix + L"}");
							writer.WriteLine(L"");
						}
					}

					for (auto classSymbol : concreteClasses)
					{
						writer.WriteString(prefix + L"vl::Ptr<");
						PrintCppType(group, classSymbol, writer);
						writer.WriteString(L"> " + group->Name() + L"Visitor::CopyNode(");
						PrintCppType(group, classSymbol, writer);
						writer.WriteLine(L"* node)");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"\tif (!node) return nullptr;");
						writer.WriteLine(prefix + L"\tVisit(node);");
						writer.WriteLine(prefix + L"\tthis->result->codeRange = node->codeRange;");
						writer.WriteString(prefix + L"\treturn this->result.Cast<");
						PrintCppType(group, classSymbol, writer);
						writer.WriteLine(L">();");
						writer.WriteLine(prefix + L"}");
						writer.WriteLine(L"");
					}

					for (auto classSymbol :
						From(group->Symbols().Values())
							.Select([](AstSymbol* derivedClass) { return dynamic_cast<AstClassSymbol*>(derivedClass); })
							.Where([](AstClassSymbol* derivedClass) { return derivedClass && derivedClass->baseClass != nullptr; })
						)
					{
						auto rootBaseClass = classSymbol;
						while (rootBaseClass->baseClass)
						{
							rootBaseClass = rootBaseClass->baseClass;
						}

						writer.WriteString(prefix + L"vl::Ptr<");
						PrintCppType(group, classSymbol, writer);
						writer.WriteString(L"> " + group->Name() + L"Visitor::CopyNode(");
						PrintCppType(group, classSymbol, writer);
						writer.WriteLine(L"* node)");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"\tif (!node) return nullptr;");
						writer.WriteString(prefix + L"\treturn CopyNode(static_cast<");
						PrintCppType(group, rootBaseClass, writer);
						writer.WriteString(L"*>(node)).Cast<");
						PrintCppType(group, classSymbol, writer);
						writer.WriteLine(L">();");
						writer.WriteLine(prefix + L"}");
						writer.WriteLine(L"");
					}
				});
			}
		}
	}
}
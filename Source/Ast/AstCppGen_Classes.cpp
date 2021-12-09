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
WriteTypeForwardDefinitions
***********************************************************************/

			void WriteTypeForwardDefinitions(AstDefFile* file, const WString& prefix, stream::StreamWriter& writer)
			{
				for (auto [name, index] : indexed(file->Symbols().Keys()))
				{
					if (dynamic_cast<AstClassSymbol*>(file->Symbols().Values()[index]))
					{
						writer.WriteString(prefix);
						writer.WriteString(L"class ");
						writer.WriteString(file->classPrefix);
						writer.WriteString(name);
						writer.WriteLine(L";");
					}
				}
			}

/***********************************************************************
PrintCppType
***********************************************************************/

			void PrintNss(List<WString>& nss, stream::StreamWriter& writer)
			{
				for (auto&& ns : nss)
				{
					writer.WriteString(ns);
					writer.WriteString(L"::");
				}
			}

			enum class PrintTypePurpose
			{
				TypeName,
				ReflectionName,
				Value,
			};

			void PrintAstType(AstDefFile* fileContext, AstPropType propType, AstSymbol* propSymbol, PrintTypePurpose purpose, stream::StreamWriter& writer)
			{
				if (propType == AstPropType::Token)
				{
					writer.WriteString(L"vl::glr::ParsingToken");
					return;
				}

				if (propType == AstPropType::Array)
				{
					writer.WriteString(L"vl::collections::List<vl::Ptr<");
				}
				else if (purpose == PrintTypePurpose::Value && dynamic_cast<AstClassSymbol*>(propSymbol))
				{
					writer.WriteString(L"vl::Ptr<");
				}

				auto file = propSymbol->Owner();
				if (purpose == PrintTypePurpose::ReflectionName)
				{
					PrintNss(file->refNss, writer);
				}
				else
				{
					if (fileContext != file)
					{
						PrintNss(file->cppNss, writer);
					}
				}
				writer.WriteString(file->classPrefix);
				writer.WriteString(propSymbol->Name());

				if (propType == AstPropType::Array)
				{
					writer.WriteString(L">>");
				}
				else if (purpose == PrintTypePurpose::Value && dynamic_cast<AstClassSymbol*>(propSymbol))
				{
					writer.WriteString(L">");
				}
			}

			void PrintFieldType(AstDefFile* fileContext, AstPropType propType, AstSymbol* propSymbol, stream::StreamWriter& writer)
			{
				PrintAstType(fileContext, propType, propSymbol, PrintTypePurpose::Value, writer);
			}

			void PrintCppType(AstDefFile* fileContext, AstSymbol* propSymbol, stream::StreamWriter& writer)
			{
				PrintAstType(fileContext, AstPropType::Type, propSymbol, PrintTypePurpose::TypeName, writer);
			}

/***********************************************************************
WriteTypeDefinitions
***********************************************************************/

			void WriteTypeDefinitions(AstDefFile* file, const WString& prefix, stream::StreamWriter& writer)
			{
				for (auto name : file->SymbolOrder())
				{
					auto typeSymbol = file->Symbols()[name];
					if (auto enumSymbol = dynamic_cast<AstEnumSymbol*>(typeSymbol))
					{
						writer.WriteLine(L"");
						writer.WriteString(prefix);
						writer.WriteString(L"enum class ");
						writer.WriteString(file->classPrefix);
						writer.WriteLine(name);
						writer.WriteString(prefix);
						writer.WriteLine(L"{");

						{
							writer.WriteString(prefix);
							writer.WriteLine(L"\tUNDEFINED_ENUM_ITEM_VALUE = -1,");
						}

						for (auto itemName : enumSymbol->ItemOrder())
						{
							auto itemSymbol = enumSymbol->Items()[itemName];
							writer.WriteString(prefix);
							writer.WriteString(L"\t");
							writer.WriteString(itemName);
							writer.WriteString(L" = ");
							writer.WriteString(itow(itemSymbol->value));
							writer.WriteLine(L",");
						}

						writer.WriteString(prefix);
						writer.WriteLine(L"};");
					}
				}

				for (auto name : file->SymbolOrder())
				{
					auto typeSymbol = file->Symbols()[name];
					if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
					{
						writer.WriteLine(L"");
						writer.WriteString(prefix);
						writer.WriteString(L"class ");
						writer.WriteString(file->classPrefix);
						writer.WriteString(name);
						if (classSymbol->derivedClasses.Count() > 0)
						{
							writer.WriteString(L" abstract");
						}
						writer.WriteString(L" : public ");
						if (classSymbol->baseClass)
						{
							PrintCppType(file, classSymbol->baseClass, writer);
						}
						else
						{
							writer.WriteString(L"vl::glr::ParsingAstBase");
						}
						writer.WriteString(L", vl::reflection::Description<");
						writer.WriteString(file->classPrefix);
						writer.WriteString(name);
						writer.WriteLine(L">");

						writer.WriteString(prefix);
						writer.WriteLine(L"{");
						writer.WriteString(prefix);
						writer.WriteLine(L"public:");

						if (classSymbol->derivedClasses.Count() > 0)
						{
							writer.WriteString(prefix);
							writer.WriteLine(L"\tclass IVisitor : public virtual vl::reflection::IDescriptable, vl::reflection::Description<IVisitor>");
							writer.WriteString(prefix);
							writer.WriteLine(L"\t{");
							writer.WriteString(prefix);
							writer.WriteLine(L"\tpublic:");

							for (auto childSymbol : classSymbol->derivedClasses)
							{
								writer.WriteString(prefix);
								writer.WriteString(L"\t\tvirtual void Visit(");
								PrintCppType(file, childSymbol, writer);
								writer.WriteLine(L"* node) = 0;");
							}

							writer.WriteString(prefix);
							writer.WriteLine(L"\t};");
							writer.WriteLine(L"");
							writer.WriteString(prefix);
							writer.WriteString(L"\tvirtual void Accept(");
							PrintCppType(file, classSymbol, writer);
							writer.WriteLine(L"::IVisitor* visitor) = 0;");
							writer.WriteLine(L"");
						}

						for (auto propName : classSymbol->PropOrder())
						{
							auto propSymbol = classSymbol->Props()[propName];
							writer.WriteString(prefix);
							writer.WriteString(L"\t");
							PrintFieldType(file, propSymbol->propType, propSymbol->propSymbol, writer);
							writer.WriteString(L" ");
							writer.WriteString(propName);
							if (dynamic_cast<AstEnumSymbol*>(propSymbol->propSymbol))
							{
								writer.WriteString(L" = ");
								PrintCppType(file, propSymbol->propSymbol, writer);
								writer.WriteString(L"::UNDEFINED_ENUM_ITEM_VALUE");
							}
							writer.WriteLine(L";");
						}

						if (classSymbol->baseClass)
						{
							writer.WriteLine(L"");
							writer.WriteString(prefix);
							writer.WriteString(L"\tvoid Accept(");
							PrintCppType(file, classSymbol->baseClass, writer);
							writer.WriteLine(L"::IVisitor* visitor) override;");
						}

						writer.WriteString(prefix);
						writer.WriteLine(L"};");
					}
				}
			}

/***********************************************************************
WriteVisitorImpl
***********************************************************************/

			void WriteVisitorImpl(AstDefFile* file, const WString& prefix, stream::StreamWriter& writer)
			{
				for (auto name : file->SymbolOrder())
				{
					if (auto classSymbol = dynamic_cast<AstClassSymbol*>(file->Symbols()[name]))
					{
						if (classSymbol->baseClass)
						{
							writer.WriteLine(L"");
							writer.WriteString(prefix);
							writer.WriteString(L"void ");
							PrintCppType(file, classSymbol, writer);
							writer.WriteString(L"::Accept(");
							PrintCppType(file, classSymbol->baseClass, writer);
							writer.WriteLine(L"::IVisitor* visitor)");
							writer.WriteString(prefix);
							writer.WriteLine(L"{");
							writer.WriteString(prefix);
							writer.WriteLine(L"\tvisitor->Visit(this);");
							writer.WriteString(prefix);
							writer.WriteLine(L"}");
						}
					}
				}
			}

/***********************************************************************
WriteTypeReflectionDeclaration
***********************************************************************/

			void WriteTypeReflectionDeclaration(AstDefFile* file, const WString& prefix, stream::StreamWriter& writer)
			{
				writer.WriteLine(L"#ifndef VCZH_DEBUG_NO_REFLECTION");

				for (auto&& name : file->SymbolOrder())
				{
					auto typeSymbol = file->Symbols()[name];
					writer.WriteString(prefix);
					writer.WriteString(L"DECL_TYPE_INFO(");
					PrintCppType(nullptr, typeSymbol, writer);
					writer.WriteLine(L")");

					if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
					{
						if (classSymbol->derivedClasses.Count() > 0)
						{
							writer.WriteString(prefix);
							writer.WriteString(L"DECL_TYPE_INFO(");
							PrintCppType(nullptr, typeSymbol, writer);
							writer.WriteLine(L"::IVisitor)");
						}
					}
				}

				writer.WriteLine(L"");
				writer.WriteLine(L"#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA");
				writer.WriteLine(L"");
				for (auto&& name : file->SymbolOrder())
				{
					if (auto classSymbol = dynamic_cast<AstClassSymbol*>(file->Symbols()[name]))
					{
						if (classSymbol->derivedClasses.Count() > 0)
						{
							writer.WriteString(prefix);
							writer.WriteString(L"BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(");
							PrintCppType(nullptr, classSymbol, writer);
							writer.WriteLine(L"::IVisitor)");

							for (auto childSymbol : classSymbol->derivedClasses)
							{
								writer.WriteString(prefix);
								writer.WriteString(L"\tvoid Visit(");
								PrintCppType(nullptr, childSymbol, writer);
								writer.WriteLine(L"* node) override");
								writer.WriteString(prefix);
								writer.WriteLine(L"\t{");
								writer.WriteString(prefix);
								writer.WriteLine(L"\t\tINVOKE_INTERFACE_PROXY(Visit, node);");
								writer.WriteString(prefix);
								writer.WriteLine(L"\t}");
								writer.WriteLine(L"");
							}

							writer.WriteString(prefix);
							writer.WriteString(L"END_INTERFACE_PROXY(");
							PrintCppType(nullptr, classSymbol, writer);
							writer.WriteLine(L"::IVisitor)");
							writer.WriteLine(L"");
						}
					}
				}

				writer.WriteLine(L"#endif");
				writer.WriteLine(L"#endif");

				writer.WriteString(prefix);
				writer.WriteLine(L"/// <summary>Load all reflectable AST types, only available when <b>VCZH_DEBUG_NO_REFLECTION</b> is off.</summary>");
				writer.WriteString(prefix);
				writer.WriteLine(L"/// <returns>Returns true if this operation succeeded.</returns>");

				writer.WriteString(prefix);
				writer.WriteString(L"extern bool ");
				writer.WriteString(file->Owner()->Global().name);
				writer.WriteString(file->Name());
				writer.WriteLine(L"LoadTypes();");
			}

/***********************************************************************
WriteTypeReflectionImplementation
***********************************************************************/

			void WriteTypeReflectionImplementation(AstDefFile* file, const WString& prefix, stream::StreamWriter& writer)
			{
				writer.WriteLine(L"#ifndef VCZH_DEBUG_NO_REFLECTION");

				writer.WriteLine(L"");

				for (auto&& name : file->SymbolOrder())
				{
					auto typeSymbol = file->Symbols()[name];
					writer.WriteString(prefix);
					writer.WriteString(L"IMPL_TYPE_INFO_RENAME(");
					PrintCppType(nullptr, typeSymbol, writer);
					writer.WriteString(L", ");
					PrintAstType(nullptr, AstPropType::Type, typeSymbol, PrintTypePurpose::ReflectionName, writer);
					writer.WriteLine(L")");

					if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
					{
						if (classSymbol->derivedClasses.Count() > 0)
						{
							writer.WriteString(prefix);
							writer.WriteString(L"IMPL_TYPE_INFO_RENAME(");
							PrintCppType(nullptr, typeSymbol, writer);
							writer.WriteString(L"::IVisitor");
							writer.WriteString(L", ");
							PrintAstType(nullptr, AstPropType::Type, typeSymbol, PrintTypePurpose::ReflectionName, writer);
							writer.WriteLine(L"::IVisitor)");
						}
					}
				}

				writer.WriteLine(L"");
				writer.WriteLine(L"#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA");

				for (auto&& name : file->SymbolOrder())
				{
					auto typeSymbol = file->Symbols()[name];
					writer.WriteLine(L"");

					if (auto enumSymbol = dynamic_cast<AstEnumSymbol*>(typeSymbol))
					{
						writer.WriteString(prefix);
						writer.WriteString(L"BEGIN_ENUM_ITEM(");
						PrintCppType(nullptr, enumSymbol, writer);
						writer.WriteLine(L")");

						writer.WriteString(prefix);
						writer.WriteString(L"\tENUM_ITEM_NAMESPACE(");
						PrintCppType(nullptr, enumSymbol, writer);
						writer.WriteLine(L")");

						for (auto itemName : enumSymbol->ItemOrder())
						{
							writer.WriteString(prefix);
							writer.WriteString(L"\tENUM_NAMESPACE_ITEM(");
							writer.WriteString(itemName);
							writer.WriteLine(L")");
						}

						writer.WriteString(prefix);
						writer.WriteString(L"END_ENUM_ITEM(");
						PrintCppType(nullptr, enumSymbol, writer);
						writer.WriteLine(L")");
					}

					if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
					{
						writer.WriteString(prefix);
						writer.WriteString(L"BEGIN_CLASS_MEMBER(");
						PrintCppType(nullptr, classSymbol, writer);
						writer.WriteLine(L")");

						if (classSymbol->baseClass)
						{
							writer.WriteString(prefix);
							writer.WriteString(L"\tCLASS_MEMBER_BASE(");
							PrintCppType(nullptr, classSymbol->baseClass, writer);
							writer.WriteLine(L")");
						}
						else
						{
							writer.WriteLine(prefix + L"\tCLASS_MEMBER_BASE(vl::glr::ParsingAstBase)");
						}
						writer.WriteLine(L"");

						if (classSymbol->derivedClasses.Count() == 0)
						{
							writer.WriteString(prefix);
							writer.WriteString(L"\tCLASS_MEMBER_CONSTRUCTOR(vl::Ptr<");
							PrintCppType(nullptr, classSymbol, writer);
							writer.WriteLine(L">(), NO_PARAMETER)");
							writer.WriteLine(L"");
						}

						for (auto propName : classSymbol->PropOrder())
						{
							auto propSymbol = classSymbol->Props()[propName];
							writer.WriteString(prefix);
							writer.WriteString(L"\tCLASS_MEMBER_FIELD(");
							writer.WriteString(propSymbol->Name());
							writer.WriteLine(L")");
						}

						writer.WriteString(prefix);
						writer.WriteString(L"END_CLASS_MEMBER(");
						PrintCppType(nullptr, classSymbol, writer);
						writer.WriteLine(L")");
					}
				}

				for (auto&& name : file->SymbolOrder())
				{
					if (auto classSymbol = dynamic_cast<AstClassSymbol*>(file->Symbols()[name]))
					{
						if (classSymbol->derivedClasses.Count() > 0)
						{
							writer.WriteLine(L"");
							writer.WriteString(prefix);
							writer.WriteString(L"BEGIN_INTERFACE_MEMBER(");
							PrintCppType(nullptr, classSymbol, writer);
							writer.WriteLine(L"::IVisitor)");

							for (auto childSymbol : classSymbol->derivedClasses)
							{
								writer.WriteString(prefix);
								writer.WriteString(L"\tCLASS_MEMBER_METHOD_OVERLOAD(Visit, {L\"node\"}, void(");
								PrintCppType(nullptr, classSymbol, writer);
								writer.WriteString(L"::IVisitor::*)(");
								PrintCppType(nullptr, childSymbol, writer);
								writer.WriteLine(L"* node))");
							}

							writer.WriteString(prefix);
							writer.WriteString(L"END_INTERFACE_MEMBER(");
							PrintCppType(nullptr, classSymbol, writer);
							writer.WriteLine(L")");
						}
					}
				}

				writer.WriteLine(L"");
				writer.WriteLine(L"#endif");

				writer.WriteLine(L"");
				writer.WriteLine(L"#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA");
				writer.WriteString(prefix);
				writer.WriteString(L"class ");
				writer.WriteString(file->Owner()->Global().name);
				writer.WriteString(file->Name());
				writer.WriteLine(L"TypeLoader : public vl::Object, public ITypeLoader");
				writer.WriteString(prefix);
				writer.WriteLine(L"{");
				writer.WriteString(prefix);
				writer.WriteLine(L"public:");

				writer.WriteString(prefix);
				writer.WriteLine(L"\tvoid Load(ITypeManager* manager)");
				writer.WriteString(prefix);
				writer.WriteLine(L"\t{");

				for (auto&& name : file->SymbolOrder())
				{
					auto typeSymbol = file->Symbols()[name];
					writer.WriteString(prefix);
					writer.WriteString(L"\t\tADD_TYPE_INFO(");
					PrintCppType(nullptr, typeSymbol, writer);
					writer.WriteLine(L")");

					if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
					{
						if (classSymbol->derivedClasses.Count() > 0)
						{
							writer.WriteString(prefix);
							writer.WriteString(L"\t\tADD_TYPE_INFO(");
							PrintCppType(nullptr, classSymbol, writer);
							writer.WriteLine(L"::IVisitor)");
						}
					}
				}

				writer.WriteString(prefix);
				writer.WriteLine(L"\t}");

				writer.WriteLine(L"");
				writer.WriteString(prefix);
				writer.WriteLine(L"\tvoid Unload(ITypeManager* manager)");
				writer.WriteString(prefix);
				writer.WriteLine(L"\t{");
				writer.WriteString(prefix);
				writer.WriteLine(L"\t}");

				writer.WriteString(prefix);
				writer.WriteLine(L"};");
				writer.WriteLine(L"#endif");

				writer.WriteLine(L"#endif");

				writer.WriteLine(L"");
				writer.WriteString(prefix);
				writer.WriteString(L"bool ");
				writer.WriteString(file->Owner()->Global().name);
				writer.WriteString(file->Name());
				writer.WriteLine(L"LoadTypes()");
				writer.WriteString(prefix);
				writer.WriteLine(L"{");
				writer.WriteLine(L"#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA");

				writer.WriteString(prefix);
				writer.WriteLine(L"\tif (auto manager = GetGlobalTypeManager())");
				writer.WriteString(prefix);
				writer.WriteLine(L"\t{");
				writer.WriteString(prefix);
				writer.WriteString(L"\t\tPtr<ITypeLoader> loader = new ");
				writer.WriteString(file->Owner()->Global().name);
				writer.WriteString(file->Name());
				writer.WriteLine(L"TypeLoader;");
				writer.WriteString(prefix);
				writer.WriteLine(L"\t\treturn manager->AddTypeLoader(loader);");
				writer.WriteString(prefix);
				writer.WriteLine(L"\t}");

				writer.WriteLine(L"#endif");
				writer.WriteString(prefix);
				writer.WriteLine(L"\treturn false;");
				writer.WriteString(prefix);
				writer.WriteLine(L"}");
			}
		}
	}
}
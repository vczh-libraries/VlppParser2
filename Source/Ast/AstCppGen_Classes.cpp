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

			void PrintCppType(AstPropType propType, AstSymbol* propSymbol, stream::StreamWriter& writer)
			{
				if (propType == AstPropType::Token)
				{
					writer.WriteString(L"vl::glr::ParsingToken");
					return;
				}

				if (propType == AstPropType::Array)
				{
					writer.WriteString(L"vl::collections::List<");
				}

				auto file = propSymbol->Owner();
				for (auto&& ns : file->cppNss)
				{
					writer.WriteString(ns);
					writer.WriteString(L"::");
				}
				writer.WriteString(file->classPrefix);
				writer.WriteString(propSymbol->Name());

				if (propType == AstPropType::Array)
				{
					writer.WriteString(L">");
				}
			}

/***********************************************************************
WriteTypeDefinitions
***********************************************************************/

			void WriteTypeDefinitions(AstDefFile* file, const WString& prefix, stream::StreamWriter& writer)
			{
				for (auto name : file->SymbolOrder())
				{
					auto typeSymbol = file->Symbols()[name];
					writer.WriteLine(L"");

					if (auto enumSymbol = dynamic_cast<AstEnumSymbol*>(typeSymbol))
					{
						writer.WriteString(prefix);
						writer.WriteString(L"enum class ");
						writer.WriteString(file->classPrefix);
						writer.WriteLine(name);
						writer.WriteString(prefix);
						writer.WriteLine(L"{");

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

					if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
					{
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
							PrintCppType(AstPropType::Type, classSymbol->baseClass, writer);
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
							writer.WriteLine(L"\tclass IVisitor : public vl::reflection::IDescriptable, vl::reflection::Description<IVisitor>");
							writer.WriteString(prefix);
							writer.WriteLine(L"\t{");
							writer.WriteString(prefix);
							writer.WriteLine(L"\tpublic:");

							for (auto childSymbol : classSymbol->derivedClasses)
							{
								writer.WriteString(prefix);
								writer.WriteString(L"\t\tvirtual void Visit(");
								PrintCppType(AstPropType::Type, childSymbol, writer);
								writer.WriteLine(L"* node) = 0;");
							}

							writer.WriteString(prefix);
							writer.WriteLine(L"\t};");
							writer.WriteLine(L"");
							writer.WriteString(prefix);
							writer.WriteString(L"\tvirtual void Accept(");
							PrintCppType(AstPropType::Type, classSymbol, writer);
							writer.WriteLine(L"::IVisitor* visitor)=0;");
							writer.WriteLine(L"");
						}

						for (auto propName : classSymbol->PropOrder())
						{
							auto propSymbol = classSymbol->Props()[propName];
							writer.WriteString(prefix);
							writer.WriteString(L"\t");
							PrintCppType(propSymbol->propType, propSymbol->propSymbol, writer);
							writer.WriteString(L" ");
							writer.WriteString(propName);
							writer.WriteLine(L";");
						}

						if (classSymbol->baseClass)
						{
							writer.WriteLine(L"");
							writer.WriteString(prefix);
							writer.WriteString(L"\tvoid Accept(");
							PrintCppType(AstPropType::Type, classSymbol->baseClass, writer);
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
							writer.WriteString(prefix);
							writer.WriteString(L"void ");
							PrintCppType(AstPropType::Type, classSymbol, writer);
							writer.WriteString(L"::Accept(");
							PrintCppType(AstPropType::Type, classSymbol->baseClass, writer);
							writer.WriteLine(L"::IVisitor* visitor)");
							writer.WriteString(prefix);
							writer.WriteLine(L"{");
							writer.WriteString(prefix);
							writer.WriteLine(L"\tvisitor->Visit(this);");
							writer.WriteString(prefix);
							writer.WriteLine(L"}");
							writer.WriteLine(L"");
						}
					}
				}
			}
		}
	}
}
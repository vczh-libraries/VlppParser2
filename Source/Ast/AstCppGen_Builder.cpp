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

/***********************************************************************
WriteAstBuilderHeaderFile
***********************************************************************/

			void WriteAstBuilderHeaderFile(AstDefFile* file, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer)
			{
				WriteAstUtilityHeaderFile(file, output, L"builder", writer, [&](const WString& prefix)
				{
					for(auto typeSymbol : file->Symbols().Values())
					{
						if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
						{
							if (classSymbol->Props().Count() > 0)
							{
								WString className = L"Make" + classSymbol->Name();
								writer.WriteString(prefix + L"class " + className);
								writer.WriteString(L" : public vl::glr::ParsingAstBuilder<");
								PrintCppType(file, classSymbol, writer);
								writer.WriteLine(L">");
								writer.WriteLine(prefix + L"{");
								writer.WriteLine(prefix + L"public:");

								auto currentClass = classSymbol;
								while (currentClass)
								{
									for (auto propSymbol : currentClass->Props().Values())
									{
										switch (propSymbol->propType)
										{
										case AstPropType::Token:
											writer.WriteLine(prefix + L"\t" + className + L"& " + propSymbol->Name() + L"(const vl::WString& value);");
											break;
										case AstPropType::Type:
											if (dynamic_cast<AstEnumSymbol*>(propSymbol->propSymbol))
											{
												writer.WriteString(prefix + L"\t" + className + L"& " + propSymbol->Name() + L"(");
												PrintCppType(file, propSymbol->propSymbol, writer);
												writer.WriteLine(L" value);");
												break;
											}
										case AstPropType::Array:
											writer.WriteString(prefix + L"\t" + className + L"& " + propSymbol->Name() + L"(const vl::Ptr<");
											PrintCppType(file, propSymbol->propSymbol, writer);
											writer.WriteLine(L">& value);");
											break;
										}
									}
									currentClass = currentClass->baseClass;
								}
								writer.WriteLine(prefix + L"};");
								writer.WriteLine(L"");
							}
						}
					}
				});
			}

/***********************************************************************
WriteAstBuilderCppFile
***********************************************************************/

			void WriteAstBuilderCppFile(AstDefFile* file, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer)
			{
				WriteAstUtilityCppFile(file, output->builderH, L"builder", writer, [&](const WString& prefix)
				{
					for (auto typeSymbol : file->Symbols().Values())
					{
						if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
						{
							if (classSymbol->Props().Count() > 0)
							{
								WString className = L"Make" + classSymbol->Name();
								writer.WriteLine(L"");
								writer.WriteLine(L"/***********************************************************************");
								writer.WriteLine(className);
								writer.WriteLine(L"***********************************************************************/");

								auto currentClass = classSymbol;
								while (currentClass)
								{
									for (auto propSymbol : currentClass->Props().Values())
									{
										writer.WriteLine(L"");
										switch (propSymbol->propType)
										{
										case AstPropType::Token:
											writer.WriteLine(prefix + className + L"& " + className + L"::" + propSymbol->Name() + L"(const vl::WString& value)");
											writer.WriteLine(prefix + L"{");
											writer.WriteLine(prefix + L"\tnode->" + propSymbol->Name() + L".value = value;");
											writer.WriteLine(prefix + L"\treturn *this;");
											writer.WriteLine(prefix + L"}");
											break;
										case AstPropType::Type:
											if (dynamic_cast<AstEnumSymbol*>(propSymbol->propSymbol))
											{
												writer.WriteString(prefix + className + L"& " + className + L"::" + propSymbol->Name() + L"(");
												PrintCppType(file, propSymbol->propSymbol, writer);
												writer.WriteLine(L" value)");
											}
											if (dynamic_cast<AstClassSymbol*>(propSymbol->propSymbol))
											{
												writer.WriteString(prefix + className + L"& " + className + L"::" + propSymbol->Name() + L"(const vl::Ptr<");
												PrintCppType(file, propSymbol->propSymbol, writer);
												writer.WriteLine(L">& value)");
											}
											writer.WriteLine(prefix + L"{");
											writer.WriteLine(prefix + L"\tnode->" + propSymbol->Name() + L" = value;");
											writer.WriteLine(prefix + L"\treturn *this;");
											writer.WriteLine(prefix + L"}");
											break;
										case AstPropType::Array:
											writer.WriteString(prefix + className + L"& " + className + L"::" + propSymbol->Name() + L"(const vl::Ptr<");
											PrintCppType(file, propSymbol->propSymbol, writer);
											writer.WriteLine(L">& value)");
											writer.WriteLine(prefix + L"{");
											writer.WriteLine(prefix + L"\tnode->" + propSymbol->Name() + L".Add(value);");
											writer.WriteLine(prefix + L"\treturn *this;");
											writer.WriteLine(prefix + L"}");
											break;
										}
									}
									currentClass = currentClass->baseClass;
								}
							}
						}
					}
				});
			}
		}
	}
}
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
WriteAstAssemblerHeaderFile
***********************************************************************/

			void WriteAstAssemblerHeaderFile(AstSymbolManager& manager, Ptr<CppParserGenOutput> output, stream::StreamWriter& writer)
			{
				WriteParserUtilityHeaderFile(manager, output, L"ASSEMBLER", writer, [&](const WString& prefix)
				{
					{
						vint index = 0;
						writer.WriteLine(prefix + L"enum class " + manager.Global().name + L"Classes : vl::vint32_t");
						writer.WriteLine(prefix + L"{");
						for (auto typeSymbol : manager.Symbols().Values())
						{
							if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
							{
								output->classIds.Add(classSymbol, (vint32_t)index);
								writer.WriteLine(prefix + L"\t" + classSymbol->Name() + L" = " + itow(index) + L",");
								index++;
							}
						}
						writer.WriteLine(prefix + L"};");
					}
					{
						vint index = 0;
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"enum class " + manager.Global().name + L"Fields : vl::vint32_t");
						writer.WriteLine(prefix + L"{");
						for (auto typeSymbol : manager.Symbols().Values())
						{
							if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
							{
								for (auto propSymbol : classSymbol->Props().Values())
								{
									output->fieldIds.Add(propSymbol, (vint32_t)index);
									writer.WriteLine(prefix + L"\t" + classSymbol->Name() + L"_" + propSymbol->Name() + L" = " + itow(index) + L",");
									index++;
								}
							}
						}
						writer.WriteLine(prefix + L"};");
					}
					{
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"extern const wchar_t* " + manager.Global().name + L"TypeName(" + manager.Global().name + L"Classes type);");
						writer.WriteLine(prefix + L"extern const wchar_t* " + manager.Global().name + L"CppTypeName(" + manager.Global().name + L"Classes type);");
						writer.WriteLine(prefix + L"extern const wchar_t* " + manager.Global().name + L"FieldName(" + manager.Global().name + L"Fields field);");
						writer.WriteLine(prefix + L"extern const wchar_t* " + manager.Global().name + L"CppFieldName(" + manager.Global().name + L"Fields field);");
					}
					{
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"class " + manager.Global().name + L"AstInsReceiver : public vl::glr::AstInsReceiverBase");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"protected:");
						writer.WriteLine(prefix + L"\tvl::Ptr<vl::glr::ParsingAstBase> CreateAstNode(vl::vint32_t type) override;");
						writer.WriteLine(prefix + L"\tvoid SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, vl::Ptr<vl::glr::ParsingAstBase> value) override;");
						writer.WriteLine(prefix + L"\tvoid SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, const vl::regex::RegexToken& token, vl::vint32_t tokenIndex) override;");
						writer.WriteLine(prefix + L"\tvoid SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, vl::vint32_t enumItem, bool weakAssignment) override;");
						writer.WriteLine(prefix + L"\tvl::Ptr<vl::glr::ParsingAstBase> ResolveAmbiguity(vl::vint32_t type, vl::collections::Array<vl::Ptr<vl::glr::ParsingAstBase>>& candidates) override;");
						writer.WriteLine(prefix + L"};");
					}
				});
			}

/***********************************************************************
WriteAstAssemblerCppFile
***********************************************************************/

			void WriteAstAssemblerCppFile(AstSymbolManager& manager, Ptr<CppParserGenOutput> output, stream::StreamWriter& writer)
			{
				WriteParserUtilityCppFile(manager, output->assemblyH, writer, [&](const WString& prefix)
				{
					writer.WriteLine(L"");
					writer.WriteLine(L"/***********************************************************************");
					writer.WriteLine(manager.Global().name + L"AstInsReceiver : public vl::glr::AstInsReceiverBase");
					writer.WriteLine(L"***********************************************************************/");

					/***********************************************************************
					CreateAstNode
					***********************************************************************/

					{
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"vl::Ptr<vl::glr::ParsingAstBase> " + manager.Global().name + L"AstInsReceiver::CreateAstNode(vl::vint32_t type)");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"\tauto cppTypeName = " + manager.Global().name + L"CppTypeName((" + manager.Global().name + L"Classes)type);");
						writer.WriteLine(prefix + L"\tswitch((" + manager.Global().name + L"Classes)type)");
						writer.WriteLine(prefix + L"\t{");
						for (auto typeSymbol : manager.Symbols().Values())
						{
							if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
							{
								if (classSymbol->derivedClasses.Count() == 0)
								{
									writer.WriteLine(prefix + L"\tcase " + manager.Global().name + L"Classes::" + classSymbol->Name() + L":");
									writer.WriteString(prefix + L"\t\treturn new ");
									PrintCppType(nullptr, classSymbol, writer);
									writer.WriteLine(L"();");
								}
							}
						}
						writer.WriteLine(prefix + L"\tdefault:");
						writer.WriteLine(prefix + L"\t\treturn vl::glr::AssemblyThrowCannotCreateAbstractType(type, cppTypeName);");
						writer.WriteLine(prefix + L"\t}");
						writer.WriteLine(prefix + L"}");
					}

					/***********************************************************************
					SetField(Object)
					***********************************************************************/

					{
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"void " + manager.Global().name + L"AstInsReceiver::SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, vl::Ptr<vl::glr::ParsingAstBase> value)");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"\tauto cppFieldName = " + manager.Global().name + L"CppFieldName((" + manager.Global().name + L"Fields)field);");

						List<AstClassPropSymbol*> props;
						for (auto typeSymbol : manager.Symbols().Values())
						{
							if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
							{
								for (auto [propSymbol, index] : indexed(classSymbol->Props().Values()))
								{
									if (propSymbol->propType != AstPropType::Token)
									{
										if (dynamic_cast<AstClassSymbol*>(propSymbol->propSymbol))
										{
											props.Add(propSymbol);
										}
									}
								}
							}
						}

						if (props.Count() > 0)
						{
							writer.WriteLine(prefix + L"\tswitch((" + manager.Global().name + L"Fields)field)");
							writer.WriteLine(prefix + L"\t{");
							for (auto propSymbol : props)
							{
								auto classSymbol = propSymbol->Parent();
								writer.WriteLine(prefix + L"\tcase " + manager.Global().name + L"Fields::" + classSymbol->Name() + L"_" + propSymbol->Name() + L":");
								writer.WriteString(prefix + L"\t\treturn vl::glr::AssemblerSetObjectField(&");
								PrintCppType(nullptr, classSymbol, writer);
								writer.WriteString(L"::");
								writer.WriteString(propSymbol->Name());
								writer.WriteLine(L", object, field, value, cppFieldName);");
							}
							writer.WriteLine(prefix + L"\tdefault:");
							writer.WriteLine(prefix + L"\t\treturn vl::glr::AssemblyThrowFieldNotObject(field, cppFieldName);");
							writer.WriteLine(prefix + L"\t}");
						}
						else
						{
							writer.WriteLine(prefix + L"\treturn vl::glr::AssemblyThrowFieldNotObject(field, cppFieldName);");
						}
						writer.WriteLine(prefix + L"}");
					}

					/***********************************************************************
					SetField(Token)
					***********************************************************************/

					{
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"void " + manager.Global().name + L"AstInsReceiver::SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, const vl::regex::RegexToken& token, vl::vint32_t tokenIndex)");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"\tauto cppFieldName = " + manager.Global().name + L"CppFieldName((" + manager.Global().name + L"Fields)field);");

						List<AstClassPropSymbol*> props;
						for (auto typeSymbol : manager.Symbols().Values())
						{
							if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
							{
								for (auto [propSymbol, index] : indexed(classSymbol->Props().Values()))
								{
									if (propSymbol->propType == AstPropType::Token)
									{
										props.Add(propSymbol);
									}
								}
							}
						}

						if (props.Count() > 0)
						{
							writer.WriteLine(prefix + L"\tswitch((" + manager.Global().name + L"Fields)field)");
							writer.WriteLine(prefix + L"\t{");
							for (auto propSymbol : props)
							{
								auto classSymbol = propSymbol->Parent();
								writer.WriteLine(prefix + L"\tcase " + manager.Global().name + L"Fields::" + classSymbol->Name() + L"_" + propSymbol->Name() + L":");
								writer.WriteString(prefix + L"\t\treturn vl::glr::AssemblerSetTokenField(&");
								PrintCppType(nullptr, classSymbol, writer);
								writer.WriteString(L"::");
								writer.WriteString(propSymbol->Name());
								writer.WriteLine(L", object, field, token, tokenIndex, cppFieldName);");
							}
							writer.WriteLine(prefix + L"\tdefault:");
							writer.WriteLine(prefix + L"\t\treturn vl::glr::AssemblyThrowFieldNotToken(field, cppFieldName);");
							writer.WriteLine(prefix + L"\t}");
						}
						else
						{
							writer.WriteLine(prefix + L"\treturn vl::glr::AssemblyThrowFieldNotToken(field, cppFieldName);");
						}
						writer.WriteLine(prefix + L"}");
					}

					/***********************************************************************
					SetField(Enum)
					***********************************************************************/

					{
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"void " + manager.Global().name + L"AstInsReceiver::SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, vl::vint32_t enumItem, bool weakAssignment)");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"\tauto cppFieldName = " + manager.Global().name + L"CppFieldName((" + manager.Global().name + L"Fields)field);");

						List<AstClassPropSymbol*> props;
						for (auto typeSymbol : manager.Symbols().Values())
						{
							if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
							{
								for (auto [propSymbol, index] : indexed(classSymbol->Props().Values()))
								{
									if (propSymbol->propType == AstPropType::Type)
									{
										if (dynamic_cast<AstEnumSymbol*>(propSymbol->propSymbol))
										{
											props.Add(propSymbol);
										}
									}
								}
							}
						}

						if (props.Count() > 0)
						{
							writer.WriteLine(prefix + L"\tswitch((" + manager.Global().name + L"Fields)field)");
							writer.WriteLine(prefix + L"\t{");
							for (auto propSymbol : props)
							{
								auto classSymbol = propSymbol->Parent();
								writer.WriteLine(prefix + L"\tcase " + manager.Global().name + L"Fields::" + classSymbol->Name() + L"_" + propSymbol->Name() + L":");
								writer.WriteString(prefix + L"\t\treturn vl::glr::AssemblerSetEnumField(&");
								PrintCppType(nullptr, classSymbol, writer);
								writer.WriteString(L"::");
								writer.WriteString(propSymbol->Name());
								writer.WriteLine(L", object, field, enumItem, weakAssignment, cppFieldName);");
							}
							writer.WriteLine(prefix + L"\tdefault:");
							writer.WriteLine(prefix + L"\t\treturn vl::glr::AssemblyThrowFieldNotEnum(field, cppFieldName);");
							writer.WriteLine(prefix + L"\t}");
						}
						else
						{
							writer.WriteLine(prefix + L"\treturn vl::glr::AssemblyThrowFieldNotEnum(field, cppFieldName);");
						}
						writer.WriteLine(prefix + L"}");
					}

					/***********************************************************************
					TypeName
					***********************************************************************/
					{
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"const wchar_t* " + manager.Global().name + L"TypeName(" + manager.Global().name + L"Classes type)");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"\tconst wchar_t* results[] = {");

						Array<AstClassSymbol*> idToClasses(output->classIds.Count());
						for (auto [k, v] : output->classIds)
						{
							idToClasses[v] = k;
						}

						for (auto classSymbol : idToClasses)
						{
							writer.WriteLine(prefix + L"\t\tL\"" + classSymbol->Name() + L"\",");
						}

						writer.WriteLine(prefix + L"\t};");
						writer.WriteLine(prefix + L"\tvl::vint index = (vl::vint)type;");
						writer.WriteLine(prefix + L"\treturn 0 <= index && index < " + itow(idToClasses.Count()) + L" ? results[index] : nullptr;");
						writer.WriteLine(prefix + L"}");
					}

					/***********************************************************************
					CppTypeName
					***********************************************************************/
					{
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"const wchar_t* " + manager.Global().name + L"CppTypeName(" + manager.Global().name + L"Classes type)");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"\tconst wchar_t* results[] = {");

						Array<AstClassSymbol*> idToClasses(output->classIds.Count());
						for (auto [k, v] : output->classIds)
						{
							idToClasses[v] = k;
						}

						for (auto classSymbol : idToClasses)
						{
							writer.WriteString(prefix + L"\t\tL\"");
							PrintCppType(nullptr, classSymbol, writer);
							writer.WriteLine(L"\",");
						}

						writer.WriteLine(prefix + L"\t};");
						writer.WriteLine(prefix + L"\tvl::vint index = (vl::vint)type;");
						writer.WriteLine(prefix + L"\treturn 0 <= index && index < " + itow(idToClasses.Count()) + L" ? results[index] : nullptr;");
						writer.WriteLine(prefix + L"}");
					}

					/***********************************************************************
					FieldName
					***********************************************************************/
					{
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"const wchar_t* " + manager.Global().name + L"FieldName(" + manager.Global().name + L"Fields field)");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"\tconst wchar_t* results[] = {");

						Array<AstClassPropSymbol*> idToFields(output->fieldIds.Count());
						for (auto [k, v] : output->fieldIds)
						{
							idToFields[v] = k;
						}

						for (auto propSymbol : idToFields)
						{
							auto classSymbol = propSymbol->Parent();
							writer.WriteLine(prefix + L"\t\tL\"" + classSymbol->Name() + L"::" + propSymbol->Name() + L"\",");
						}

						writer.WriteLine(prefix + L"\t};");
						writer.WriteLine(prefix + L"\tvl::vint index = (vl::vint)field;");
						writer.WriteLine(prefix + L"\treturn 0 <= index && index < " + itow(idToFields.Count()) + L" ? results[index] : nullptr;");
						writer.WriteLine(prefix + L"}");
					}

					/***********************************************************************
					CppFieldName
					***********************************************************************/
					{
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"const wchar_t* " + manager.Global().name + L"CppFieldName(" + manager.Global().name + L"Fields field)");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"\tconst wchar_t* results[] = {");

						Array<AstClassPropSymbol*> idToFields(output->fieldIds.Count());
						for (auto [k, v] : output->fieldIds)
						{
							idToFields[v] = k;
						}

						for (auto propSymbol : idToFields)
						{
							auto classSymbol = propSymbol->Parent();
							writer.WriteString(prefix + L"\t\tL\"");
							PrintCppType(nullptr, classSymbol, writer);
							writer.WriteLine(L"::" + propSymbol->Name() + L"\",");
						}

						writer.WriteLine(prefix + L"\t};");
						writer.WriteLine(prefix + L"\tvl::vint index = (vl::vint)field;");
						writer.WriteLine(prefix + L"\treturn 0 <= index && index < " + itow(idToFields.Count()) + L" ? results[index] : nullptr;");
						writer.WriteLine(prefix + L"}");
					}

					/***********************************************************************
					ResolveAmbiguity
					***********************************************************************/
					{
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"vl::Ptr<vl::glr::ParsingAstBase> " + manager.Global().name + L"AstInsReceiver::ResolveAmbiguity(vl::vint32_t type, vl::collections::Array<vl::Ptr<vl::glr::ParsingAstBase>>& candidates)");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"\tauto cppTypeName = " + manager.Global().name + L"CppTypeName((" + manager.Global().name + L"Classes)type);");

						Dictionary<AstClassSymbol*, AstClassSymbol*> resolvables;
						for (auto typeSymbol : manager.Symbols().Values())
						{
							if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
							{
								auto current = classSymbol;
								while (current)
								{
									if (current->ambiguousDerivedClass)
									{
										resolvables.Add(classSymbol, current->ambiguousDerivedClass);
										break;
									}
									current = current->baseClass;
								}
							}
						}

						if (resolvables.Count() > 0)
						{
							writer.WriteLine(prefix + L"\tswitch((" + manager.Global().name + L"Classes)type)");
							writer.WriteLine(prefix + L"\t{");

							for (auto typeSymbol : manager.Symbols().Values())
							{
								if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
								{
									if (resolvables.Keys().Contains(classSymbol))
									{
										auto ambiguousClassSymbol = resolvables[classSymbol];
										writer.WriteLine(prefix + L"\tcase " + manager.Global().name + L"Classes::" + classSymbol->Name() + L":");
										writer.WriteString(prefix + L"\t\treturn vl::glr::AssemblerResolveAmbiguity<");
										PrintCppType(nullptr, classSymbol, writer);
										writer.WriteString(L", ");
										PrintCppType(nullptr, ambiguousClassSymbol, writer);
										writer.WriteLine(L">(type, candidates, cppTypeName);");
									}
								}
							}

							writer.WriteLine(prefix + L"\tdefault:");
							writer.WriteLine(prefix + L"\t\treturn vl::glr::AssemblyThrowTypeNotAllowAmbiguity(type, cppTypeName);");
							writer.WriteLine(prefix + L"\t}");
						}
						else
						{
							writer.WriteLine(prefix + L"\treturn vl::glr::AssemblyThrowTypeNotAllowAmbiguity(type, cppTypeName);");
						}
						writer.WriteLine(prefix + L"}");
					}
				});
			}
		}
	}
}
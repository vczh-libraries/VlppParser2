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
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"enum class " + manager.Global().name + L"Fields : vl::vint32_t");
						writer.WriteLine(prefix + L"{");
						for (auto typeSymbol : manager.Symbols().Values())
						{
							if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
							{
								for (auto [propSymbol, index] : indexed(classSymbol->Props().Values()))
								{
									output->fieldIds.Add(propSymbol, (vint32_t)((output->classIds[classSymbol] << 8) + index));
									writer.WriteString(prefix + L"\t" + classSymbol->Name() + L"_" + propSymbol->Name());
									writer.WriteString(L" = ");
									writer.WriteString(L"(static_cast<vl::vint32_t>(" + manager.Global().name + L"Classes::" + classSymbol->Name() + L") << 8) + " + itow(index));
									writer.WriteLine(L",");
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
						writer.WriteLine(prefix + L"\tvoid SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, const vl::regex::RegexToken& token) override;");
						writer.WriteLine(prefix + L"\tvoid SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, vl::vint32_t enumItem) override;");
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
						writer.WriteLine(prefix + L"\tswitch((" + manager.Global().name + L"Classes)type)");
						writer.WriteLine(prefix + L"\t{");
						for (auto typeSymbol : manager.Symbols().Values())
						{
							if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
							{
								writer.WriteLine(prefix + L"\tcase " + manager.Global().name + L"Classes::" + classSymbol->Name() + L":");
								if (classSymbol->derivedClasses.Count() > 0)
								{
									writer.WriteString(prefix + L"\t\tthrow vl::glr::AstInsException(L\"Unable to create abstract class \\\"");
									PrintCppType(nullptr, classSymbol, writer);
									writer.WriteLine(L"\\\".\", vl::glr::AstInsErrorType::UnknownType, type);");
								}
								else
								{
									writer.WriteString(prefix + L"\t\treturn new ");
									PrintCppType(nullptr, classSymbol, writer);
									writer.WriteLine(L"();");
								}
							}
						}
						writer.WriteLine(prefix + L"\tdefault:");
						writer.WriteLine(prefix + L"\t\tthrow vl::glr::AstInsException(L\"The type id does not exist.\", vl::glr::AstInsErrorType::UnknownType, type);");
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
						writer.WriteLine(prefix + L"\tswitch((" + manager.Global().name + L"Fields)field)");
						writer.WriteLine(prefix + L"\t{");
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
											writer.WriteLine(prefix + L"\tcase " + manager.Global().name + L"Fields::" + classSymbol->Name() + L"_" + propSymbol->Name() + L":");
											writer.WriteString(prefix + L"\t\treturn vl::glr::AssemblerSetObjectField(&");
											PrintCppType(nullptr, classSymbol, writer);
											writer.WriteString(L"::");
											writer.WriteString(propSymbol->Name());
											writer.WriteLine(L", object, field, value, cppFieldName);");
										}
									}
								}
							}
						}
						writer.WriteLine(prefix + L"\tdefault:");
						writer.WriteLine(prefix + L"\t\tif (cppFieldName)");
						writer.WriteLine(prefix + L"\t\t\tthrow vl::glr::AstInsException(vl::WString::Unmanaged(L\"Field \\\"\") + vl::WString::Unmanaged(cppFieldName) + vl::WString::Unmanaged(L\"\\\" is not an object.\"), vl::glr::AstInsErrorType::ObjectTypeMismatchedToField, field);");
						writer.WriteLine(prefix + L"\t\telse");
						writer.WriteLine(prefix + L"\t\t\tthrow vl::glr::AstInsException(L\"The field id does not exist.\", vl::glr::AstInsErrorType::UnknownField, field);");
						writer.WriteLine(prefix + L"\t}");
						writer.WriteLine(prefix + L"}");
					}

					/***********************************************************************
					SetField(Token)
					***********************************************************************/

					{
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"void " + manager.Global().name + L"AstInsReceiver::SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, const vl::regex::RegexToken& token)");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"\tauto cppFieldName = " + manager.Global().name + L"CppFieldName((" + manager.Global().name + L"Fields)field);");
						writer.WriteLine(prefix + L"\tswitch((" + manager.Global().name + L"Fields)field)");
						writer.WriteLine(prefix + L"\t{");
						for (auto typeSymbol : manager.Symbols().Values())
						{
							if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
							{
								for (auto [propSymbol, index] : indexed(classSymbol->Props().Values()))
								{
									if (propSymbol->propType == AstPropType::Token)
									{
										writer.WriteLine(prefix + L"\tcase " + manager.Global().name + L"Fields::" + classSymbol->Name() + L"_" + propSymbol->Name() + L":");
										writer.WriteString(prefix + L"\t\treturn vl::glr::AssemblerSetTokenField(&");
										PrintCppType(nullptr, classSymbol, writer);
										writer.WriteString(L"::");
										writer.WriteString(propSymbol->Name());
										writer.WriteLine(L", object, field, token,cppFieldName);");
									}
								}
							}
						}
						writer.WriteLine(prefix + L"\tdefault:");
						writer.WriteLine(prefix + L"\t\tif (cppFieldName)");
						writer.WriteLine(prefix + L"\t\t\tthrow vl::glr::AstInsException(vl::WString::Unmanaged(L\"Field \\\"\") + vl::WString::Unmanaged(cppFieldName) + vl::WString::Unmanaged(L\"\\\" is not a token.\"), vl::glr::AstInsErrorType::ObjectTypeMismatchedToField, field);");
						writer.WriteLine(prefix + L"\t\telse");
						writer.WriteLine(prefix + L"\t\t\tthrow vl::glr::AstInsException(L\"The field id does not exist.\", vl::glr::AstInsErrorType::UnknownField, field);");
						writer.WriteLine(prefix + L"\t}");
						writer.WriteLine(prefix + L"}");
					}

					/***********************************************************************
					SetField(Enum)
					***********************************************************************/

					{
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"void " + manager.Global().name + L"AstInsReceiver::SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, vl::vint32_t enumItem)");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"\tauto cppFieldName = " + manager.Global().name + L"CppFieldName((" + manager.Global().name + L"Fields)field);");
						writer.WriteLine(prefix + L"\tswitch((" + manager.Global().name + L"Fields)field)");
						writer.WriteLine(prefix + L"\t{");
						for (auto typeSymbol : manager.Symbols().Values())
						{
							if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
							{
								for (auto [propSymbol, index] : indexed(classSymbol->Props().Values()))
								{
									if (propSymbol->propType == AstPropType::Type)
									{
										if (auto propEnumSymbol = dynamic_cast<AstEnumSymbol*>(propSymbol->propSymbol))
										{
											writer.WriteLine(prefix + L"\tcase " + manager.Global().name + L"Fields::" + classSymbol->Name() + L"_" + propSymbol->Name() + L":");
											writer.WriteString(prefix + L"\t\treturn vl::glr::AssemblerSetEnumField(&");
											PrintCppType(nullptr, classSymbol, writer);
											writer.WriteString(L"::");
											writer.WriteString(propSymbol->Name());
											writer.WriteLine(L", object, field, enumItem, cppFieldName);");
										}
									}
								}
							}
						}
						writer.WriteLine(prefix + L"\tdefault:");
						writer.WriteLine(prefix + L"\t\tif (cppFieldName)");
						writer.WriteLine(prefix + L"\t\t\tthrow vl::glr::AstInsException(vl::WString::Unmanaged(L\"Field \\\"\") + vl::WString::Unmanaged(cppFieldName) + vl::WString::Unmanaged(L\"\\\" is not an enum item.\"), vl::glr::AstInsErrorType::ObjectTypeMismatchedToField, field);");
						writer.WriteLine(prefix + L"\t\telse");
						writer.WriteLine(prefix + L"\t\t\tthrow vl::glr::AstInsException(L\"The field id does not exist.\", vl::glr::AstInsErrorType::UnknownField, field);");
						writer.WriteLine(prefix + L"\t}");
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
						vint count = 0;
						for (auto typeSymbol : manager.Symbols().Values())
						{
							if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
							{
								writer.WriteLine(prefix + L"\t\tL\"" + classSymbol->Name() + L"\",");
								count++;
							}
						}
						writer.WriteLine(prefix + L"\t};");
						writer.WriteLine(prefix + L"\tvl::vint index = (vl::vint)type;");
						writer.WriteLine(prefix + L"\treturn 0 <= index && index < " + itow(count) + L" ? results[index] : nullptr;");
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
						vint count = 0;
						for (auto typeSymbol : manager.Symbols().Values())
						{
							if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
							{
								writer.WriteString(prefix + L"\t\tL\"");
								PrintCppType(nullptr, classSymbol, writer);
								writer.WriteLine(L"\",");
								count++;
							}
						}
						writer.WriteLine(prefix + L"\t};");
						writer.WriteLine(prefix + L"\tvl::vint index = (vl::vint)type;");
						writer.WriteLine(prefix + L"\treturn 0 <= index && index < " + itow(count) + L" ? results[index] : nullptr;");
						writer.WriteLine(prefix + L"}");
					}

					/***********************************************************************
					FieldName
					***********************************************************************/
					{
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"const wchar_t* " + manager.Global().name + L"FieldName(" + manager.Global().name + L"Fields field)");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"\tswitch(field)");
						writer.WriteLine(prefix + L"\t{");
						for (auto typeSymbol : manager.Symbols().Values())
						{
							if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
							{
								for (auto [propSymbol, index] : indexed(classSymbol->Props().Values()))
								{
									writer.WriteLine(prefix + L"\tcase " + manager.Global().name + L"Fields::" + classSymbol->Name() + L"_" + propSymbol->Name() + L":");
									writer.WriteLine(prefix + L"\t\treturn L\"" + classSymbol->Name() + L"::" + propSymbol->Name() + L"\";");
								}
							}
						}
						writer.WriteLine(prefix + L"\tdefault:");
						writer.WriteLine(prefix + L"\t\treturn nullptr;");
						writer.WriteLine(prefix + L"\t}");
						writer.WriteLine(prefix + L"}");
					}

					/***********************************************************************
					CppFieldName
					***********************************************************************/
					{
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"const wchar_t* " + manager.Global().name + L"CppFieldName(" + manager.Global().name + L"Fields field)");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"\tswitch(field)");
						writer.WriteLine(prefix + L"\t{");
						for (auto typeSymbol : manager.Symbols().Values())
						{
							if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
							{
								for (auto [propSymbol, index] : indexed(classSymbol->Props().Values()))
								{
									writer.WriteLine(prefix + L"\tcase " + manager.Global().name + L"Fields::" + classSymbol->Name() + L"_" + propSymbol->Name() + L":");
									writer.WriteString(prefix + L"\t\treturn L\"");
									PrintCppType(nullptr, classSymbol, writer);
									writer.WriteLine(L"::" + propSymbol->Name() + L"\";");
								}
							}
						}
						writer.WriteLine(prefix + L"\tdefault:");
						writer.WriteLine(prefix + L"\t\treturn nullptr;");
						writer.WriteLine(prefix + L"\t}");
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
						writer.WriteLine(prefix + L"\tswitch((" + manager.Global().name + L"Classes)type)");
						writer.WriteLine(prefix + L"\t{");

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
						writer.WriteLine(prefix + L"\t\tif (cppTypeName)");
						writer.WriteLine(prefix + L"\t\t\tthrow vl::glr::AstInsException(vl::WString::Unmanaged(L\"Type \\\"\") + vl::WString::Unmanaged(cppTypeName) + vl::WString::Unmanaged(L\"\\\" is not configured to allow ambiguity.\"), vl::glr::AstInsErrorType::UnsupportedAmbiguityType, type);");
						writer.WriteLine(prefix + L"\t\telse");
						writer.WriteLine(prefix + L"\t\t\tthrow vl::glr::AstInsException(L\"The type id does not exist.\", vl::glr::AstInsErrorType::UnknownType, type);");
						writer.WriteLine(prefix + L"\t}");
						writer.WriteLine(prefix + L"}");
					}
				});
			}
		}
	}
}
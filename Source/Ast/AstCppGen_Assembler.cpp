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
						writer.WriteLine(prefix + L"extern const wchar_t* " + manager.Global().name + L"FieldName(" + manager.Global().name + L"Fields field);");
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
										if (auto propClassSymbol = dynamic_cast<AstClassSymbol*>(propSymbol->propSymbol))
										{
											writer.WriteLine(prefix + L"\tcase " + manager.Global().name + L"Fields::" + classSymbol->Name() + L"_" + propSymbol->Name() + L":");
											writer.WriteLine(prefix + L"\t\t{");
											writer.WriteString(prefix + L"\t\t\tauto typedObject = dynamic_cast<");
											PrintCppType(nullptr, classSymbol, writer);
											writer.WriteLine(L"*>(object);");
											writer.WriteString(prefix+L"\t\t\tif (!typedObject) throw vl::glr::AstInsException(L\"Field \\\"");
											PrintCppType(nullptr, classSymbol, writer);
											writer.WriteLine(L"::" + propSymbol->Name() + L"\\\" does not exist in the current object.\", vl::glr::AstInsErrorType::FieldNotExistsInType, field);");
											if (propSymbol->propType != AstPropType::Array)
											{
												writer.WriteString(prefix + L"\t\t\tif (typedObject->" + propSymbol->Name() + L") throw vl::glr::AstInsException(L\"Field \\\"");
												PrintCppType(nullptr, classSymbol, writer);
												writer.WriteLine(L"::" + propSymbol->Name() + L"\\\" has already been assigned.\", vl::glr::AstInsErrorType::FieldReassigned, field);");
											}
											writer.WriteString(prefix + L"\t\t\tauto typedValue = value.Cast<");
											PrintCppType(nullptr, propClassSymbol, writer);
											writer.WriteLine(L">();");
											writer.WriteString(prefix + L"\t\t\tif (!typedValue) throw vl::glr::AstInsException(L\"Field \\\"");
											PrintCppType(nullptr, classSymbol, writer);
											writer.WriteLine(L"::" + propSymbol->Name() + L"\\\" cannot be assigned with an uncompatible value.\", vl::glr::AstInsErrorType::ObjectTypeMismatchedToField, field);");
											if (propSymbol->propType == AstPropType::Array)
											{
												writer.WriteLine(prefix + L"\t\t\ttypedObject->" + propSymbol->Name() + L".Add(typedValue);");
											}
											else
											{
												writer.WriteLine(prefix + L"\t\t\ttypedObject->" + propSymbol->Name() + L" = typedValue;");
											}
											writer.WriteLine(prefix + L"\t\t}");
											writer.WriteLine(prefix + L"\t\tbreak;");
										}
									}
								}
							}
						}
						for (auto typeSymbol : manager.Symbols().Values())
						{
							if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
							{
								for (auto [propSymbol, index] : indexed(classSymbol->Props().Values()))
								{
									if (propSymbol->propType == AstPropType::Token || !dynamic_cast<AstClassSymbol*>(propSymbol->propSymbol))
									{
										writer.WriteLine(prefix + L"\tcase " + manager.Global().name + L"Fields::" + classSymbol->Name() + L"_" + propSymbol->Name() + L":");
										writer.WriteString(prefix + L"\t\tthrow vl::glr::AstInsException(L\"Field \\\"");
										PrintCppType(nullptr, classSymbol, writer);
										writer.WriteLine(L"::" + propSymbol->Name() + L"\\\" is not an object.\", vl::glr::AstInsErrorType::ObjectTypeMismatchedToField, field);");
									}
								}
							}
						}
						writer.WriteLine(prefix + L"\tdefault:");
						writer.WriteLine(prefix + L"\t\tthrow vl::glr::AstInsException(L\"The field id does not exist.\", vl::glr::AstInsErrorType::UnknownField, field);");
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
										writer.WriteLine(prefix + L"\t\t{");
										writer.WriteString(prefix + L"\t\t\tauto typedObject = dynamic_cast<");
										PrintCppType(nullptr, classSymbol, writer);
										writer.WriteLine(L"*>(object);");
										writer.WriteString(prefix + L"\t\t\tif (!typedObject) throw vl::glr::AstInsException(L\"Field \\\"");
										PrintCppType(nullptr, classSymbol, writer);
										writer.WriteLine(L"::" + propSymbol->Name() + L"\\\" does not exist in the current object.\", vl::glr::AstInsErrorType::FieldNotExistsInType, field);");
										writer.WriteString(prefix + L"\t\t\tif (typedObject->" + propSymbol->Name() + L".value.Length() != 0) throw vl::glr::AstInsException(L\"Field \\\"");
										PrintCppType(nullptr, classSymbol, writer);
										writer.WriteLine(L"::" + propSymbol->Name() + L"\\\" has already been assigned.\", vl::glr::AstInsErrorType::FieldReassigned, field);");
										writer.WriteLine(prefix + L"\t\t\tAssignToken(typedObject->" + propSymbol->Name() + L", token);");
										writer.WriteLine(prefix + L"\t\t}");
										writer.WriteLine(prefix + L"\t\tbreak;");
									}
								}
							}
						}
						for (auto typeSymbol : manager.Symbols().Values())
						{
							if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
							{
								for (auto [propSymbol, index] : indexed(classSymbol->Props().Values()))
								{
									if (propSymbol->propType != AstPropType::Token)
									{
										writer.WriteLine(prefix + L"\tcase " + manager.Global().name + L"Fields::" + classSymbol->Name() + L"_" + propSymbol->Name() + L":");
										writer.WriteString(prefix + L"\t\tthrow vl::glr::AstInsException(L\"Field \\\"");
										PrintCppType(nullptr, classSymbol, writer);
										writer.WriteLine(L"::" + propSymbol->Name() + L"\\\" is not a token.\", vl::glr::AstInsErrorType::ObjectTypeMismatchedToField, field);");
									}
								}
							}
						}
						writer.WriteLine(prefix + L"\tdefault:");
						writer.WriteLine(prefix + L"\t\tthrow vl::glr::AstInsException(L\"The field id does not exist.\", vl::glr::AstInsErrorType::UnknownField, field);");
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
											writer.WriteLine(prefix + L"\t\t{");
											writer.WriteString(prefix + L"\t\t\tauto typedObject = dynamic_cast<");
											PrintCppType(nullptr, classSymbol, writer);
											writer.WriteLine(L"*>(object);");
											writer.WriteString(prefix + L"\t\t\tif (!typedObject) throw vl::glr::AstInsException(L\"Field \\\"");
											PrintCppType(nullptr, classSymbol, writer);
											writer.WriteLine(L"::" + propSymbol->Name() + L"\\\" does not exist in the current object.\", vl::glr::AstInsErrorType::FieldNotExistsInType, field);");
											writer.WriteString(prefix + L"\t\t\tif (typedObject->" + propSymbol->Name() + L" == ");
											PrintCppType(nullptr, propEnumSymbol, writer);
											writer.WriteString(L"::UNDEFINED_ENUM_ITEM_VALUE) throw vl::glr::AstInsException(L\"Field \\\"");
											PrintCppType(nullptr, classSymbol, writer);
											writer.WriteLine(L"::" + propSymbol->Name() + L"\\\" has already been assigned.\", vl::glr::AstInsErrorType::FieldReassigned, field);");
											writer.WriteString(prefix + L"\t\t\ttypedObject->" + propSymbol->Name() + L" = (");
											PrintCppType(nullptr, propEnumSymbol, writer);
											writer.WriteLine(L")enumItem;");
											writer.WriteLine(prefix + L"\t\t}");
											writer.WriteLine(prefix + L"\t\tbreak;");
										}
									}
								}
							}
						}
						for (auto typeSymbol : manager.Symbols().Values())
						{
							if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
							{
								for (auto [propSymbol, index] : indexed(classSymbol->Props().Values()))
								{
									if (propSymbol->propType != AstPropType::Type || !dynamic_cast<AstEnumSymbol*>(propSymbol->propSymbol))
									{
										writer.WriteLine(prefix + L"\tcase " + manager.Global().name + L"Fields::" + classSymbol->Name() + L"_" + propSymbol->Name() + L":");
										writer.WriteString(prefix + L"\t\tthrow vl::glr::AstInsException(L\"Field \\\"");
										PrintCppType(nullptr, classSymbol, writer);
										writer.WriteLine(L"::" + propSymbol->Name() + L"\\\" is not an enum item.\", vl::glr::AstInsErrorType::ObjectTypeMismatchedToField, field);");
									}
								}
							}
						}
						writer.WriteLine(prefix + L"\tdefault:");
						writer.WriteLine(prefix + L"\t\tthrow vl::glr::AstInsException(L\"The field id does not exist.\", vl::glr::AstInsErrorType::UnknownField, field);");
						writer.WriteLine(prefix + L"\t}");
						writer.WriteLine(prefix + L"}");
					}

					/***********************************************************************
					ResolveAmbiguity
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
					ResolveAmbiguity
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
					ResolveAmbiguity
					***********************************************************************/
					{
						writer.WriteLine(L"");
						writer.WriteLine(prefix + L"vl::Ptr<vl::glr::ParsingAstBase> " + manager.Global().name + L"AstInsReceiver::ResolveAmbiguity(vl::vint32_t type, vl::collections::Array<vl::Ptr<vl::glr::ParsingAstBase>>& candidates)");
						writer.WriteLine(prefix + L"{");
						writer.WriteLine(prefix + L"\tswitch((" + manager.Global().name + L"Classes)type)");
						writer.WriteLine(prefix + L"\t{");
						for (auto typeSymbol : manager.Symbols().Values())
						{
							if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
							{
								writer.WriteLine(prefix + L"\tcase " + manager.Global().name + L"Classes::" + classSymbol->Name() + L":");
							}
						}
						writer.WriteLine(prefix + L"\t\tthrow vl::glr::AstInsException(L\"The type is not configured to allow ambiguity.\", vl::glr::AstInsErrorType::UnsupportedAmbiguityType, type);");
						writer.WriteLine(prefix + L"\tdefault:");
						writer.WriteLine(prefix + L"\t\tthrow vl::glr::AstInsException(L\"The type id does not exist.\", vl::glr::AstInsErrorType::UnknownType, type);");
						writer.WriteLine(prefix + L"\t}");
						writer.WriteLine(prefix + L"}");
					}
				});
			}
		}
	}
}
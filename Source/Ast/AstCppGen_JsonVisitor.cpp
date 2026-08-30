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
					writer.WriteLine(prefix + L"\tBeginField(vl::WString::Unmanaged(L\"" + propSymbol->Name() + L"\"));");
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
								writer.WriteLine(prefix + L"\t\tWriteString(vl::WString::Unmanaged(L\"" + enumItemSymbol->Name() + L"\"));");
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
				writer.WriteLine(prefix + L"\tWriteType(vl::WString::Unmanaged(L\"" + fieldSymbol->Name() + L"\"), node);");
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

			void WriteJsonWriterHeaderBody(AstDefFileGroup* group, const WString& prefix, stream::StreamWriter& writer)
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
			}

			void WriteJsonReaderHeaderBody(AstDefFileGroup* group, const WString& prefix, stream::StreamWriter& writer)
			{
				List<AstClassSymbol*> visitors, concreteClasses;
				CollectVisitorsAndConcreteClasses(group, visitors, concreteClasses);

				writer.WriteLine(prefix + L"/// <summary>A JSON reader, overriding all abstract methods with JSON to AST deserialization code.</summary>");
				writer.WriteLine(prefix + L"class " + group->Name() + L"Visitor");
				bool firstBase = true;
				for (auto visitorSymbol : visitors)
				{
					writer.WriteString(prefix + (firstBase ? L"\t: protected virtual " : L"\t, protected virtual "));
					PrintCppType(group, visitorSymbol, writer);
					writer.WriteLine(L"::IVisitor");
					firstBase = false;
				}
				writer.WriteLine(prefix + L"{");
				writer.WriteLine(prefix + L"protected:");
				writer.WriteLine(prefix + L"\tclass JsonObjectScope");
				writer.WriteLine(prefix + L"\t{");
				writer.WriteLine(prefix + L"\tprotected:");
				writer.WriteLine(prefix + L"\t\tvl::collections::List<vl::glr::json::JsonObject*>& jsonObjects;");
				writer.WriteLine(L"");
				writer.WriteLine(prefix + L"\tpublic:");
				writer.WriteLine(prefix + L"\t\tJsonObjectScope(vl::collections::List<vl::glr::json::JsonObject*>& _jsonObjects, vl::glr::json::JsonObject* json);");
				writer.WriteLine(prefix + L"\t\t~JsonObjectScope();");
				writer.WriteLine(prefix + L"\t};");
				writer.WriteLine(L"");
				writer.WriteLine(prefix + L"\tvl::collections::List<vl::glr::json::JsonObject*> jsonObjects;");
				writer.WriteLine(prefix + L"\tvl::glr::json::JsonObject* CurrentObject();");
				writer.WriteLine(prefix + L"\tvl::glr::json::JsonNode* FindField(const vl::WString& name);");
				writer.WriteLine(prefix + L"\tbool IsNull(vl::glr::json::JsonNode* value);");
				writer.WriteLine(prefix + L"\tvl::WString ReadType(vl::glr::json::JsonObject* json);");
				writer.WriteLine(prefix + L"\tvoid ValidateFields(vl::glr::json::JsonObject* json, const vl::WString& typeName);");
				writer.WriteLine(L"");

				for (auto typeSymbol : group->Symbols().Values())
				{
					if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
					{
						writer.WriteString(prefix + L"\tvirtual void FillFields(");
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
				writer.WriteLine(prefix + L"\tvl::Ptr<vl::glr::ParsingAstBase> ReadJson(vl::glr::json::JsonObject* json);");
				writer.WriteLine(prefix + L"};");
			}

			void WriteJsonVisitorHeaderFile(AstDefFileGroup* group, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer)
			{
				List<WString> extraNss;
				WriteAstUtilityHeaderFile(group, output, L"json", group->Owner()->Global().jsonIncludes, extraNss, writer, [&](const WString& prefix)
				{
					writer.WriteLine(prefix + L"namespace json_visitor");
					writer.WriteLine(prefix + L"{");
					WriteJsonWriterHeaderBody(group, prefix + L"\t", writer);
					writer.WriteLine(prefix + L"}");
					writer.WriteLine(L"");
					writer.WriteLine(prefix + L"namespace json_reader");
					writer.WriteLine(prefix + L"{");
					WriteJsonReaderHeaderBody(group, prefix + L"\t", writer);
					writer.WriteLine(prefix + L"}");
				});
			}

/***********************************************************************
WriteJsonVisitorCppFile
***********************************************************************/

			void WriteJsonWriterCppBody(AstDefFileGroup* group, const WString& prefix, stream::StreamWriter& writer)
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
			}

			void CollectClassProps(AstClassSymbol* classSymbol, List<AstClassPropSymbol*>& props)
			{
				if (classSymbol->baseClass)
				{
					CollectClassProps(classSymbol->baseClass, props);
				}
				for (auto propName : classSymbol->PropOrder())
				{
					props.Add(classSymbol->Props()[propName]);
				}
			}

			void WriteJsonReaderFillFieldsBody(AstDefFileGroup* group, AstClassSymbol* classSymbol, const WString& prefix, stream::StreamWriter& writer)
			{
				if (classSymbol->baseClass)
				{
					writer.WriteString(prefix + L"\tFillFields(static_cast<");
					PrintCppType(group, classSymbol->baseClass, writer);
					writer.WriteLine(L"*>(node));");
				}

				for (auto propName : classSymbol->PropOrder())
				{
					auto propSymbol = classSymbol->Props()[propName];
					if (auto enumPropSymbol = dynamic_cast<AstEnumSymbol*>(propSymbol->propSymbol))
					{
						if (enumPropSymbol->ItemOrder().Count() > 0)
						{
							writer.WriteString(prefix + L"\tnode->" + propName + L" = ");
							PrintCppType(group, enumPropSymbol, writer);
							writer.WriteLine(L"::" + enumPropSymbol->ItemOrder()[0] + L";");
						}
					}

					writer.WriteLine(prefix + L"\tif (auto value = FindField(vl::WString::Unmanaged(L\"" + propName + L"\")))");
					writer.WriteLine(prefix + L"\t{");
					switch (propSymbol->propType)
					{
					case AstPropType::Token:
						writer.WriteLine(prefix + L"\t\tauto jsonString = dynamic_cast<vl::glr::json::JsonString*>(value);");
						writer.WriteLine(prefix + L"\t\tif (!jsonString) throw vl::Exception(L\"AST JSON field \\\"" + propName + L"\\\" must be a string.\");");
						writer.WriteLine(prefix + L"\t\tnode->" + propName + L".value = jsonString->content.value;");
						break;
					case AstPropType::Type:
						if (auto enumPropSymbol = dynamic_cast<AstEnumSymbol*>(propSymbol->propSymbol))
						{
							writer.WriteLine(prefix + L"\t\tauto jsonString = dynamic_cast<vl::glr::json::JsonString*>(value);");
							writer.WriteLine(prefix + L"\t\tif (!jsonString) throw vl::Exception(L\"AST JSON field \\\"" + propName + L"\\\" must be a string.\");");
							bool first = true;
							for (auto itemName : enumPropSymbol->ItemOrder())
							{
								writer.WriteString(prefix + (first ? L"\t\tif" : L"\t\telse if") + L" (jsonString->content.value == L\"" + itemName + L"\") node->" + propName + L" = ");
								PrintCppType(group, enumPropSymbol, writer);
								writer.WriteLine(L"::" + itemName + L";");
								first = false;
							}
							writer.WriteLine(prefix + L"\t\t" + (first ? WString() : L"else ") + L"throw vl::Exception(L\"AST JSON field \\\"" + propName + L"\\\" contains an unknown enum item.\");");
						}
						else if (auto classPropSymbol = dynamic_cast<AstClassSymbol*>(propSymbol->propSymbol))
						{
							writer.WriteLine(prefix + L"\t\tif (IsNull(value))");
							writer.WriteLine(prefix + L"\t\t{");
							writer.WriteLine(prefix + L"\t\t\tnode->" + propName + L" = nullptr;");
							writer.WriteLine(prefix + L"\t\t}");
							writer.WriteLine(prefix + L"\t\telse if (auto jsonObject = dynamic_cast<vl::glr::json::JsonObject*>(value))");
							writer.WriteLine(prefix + L"\t\t{");
							writer.WriteString(prefix + L"\t\t\tauto ast = ReadJson(jsonObject).Cast<");
							PrintCppType(group, classPropSymbol, writer);
							writer.WriteLine(L">();");
							writer.WriteLine(prefix + L"\t\t\tif (!ast) throw vl::Exception(L\"AST JSON field \\\"" + propName + L"\\\" contains an incompatible AST type.\");");
							writer.WriteLine(prefix + L"\t\t\tnode->" + propName + L" = ast;");
							writer.WriteLine(prefix + L"\t\t}");
							writer.WriteLine(prefix + L"\t\telse throw vl::Exception(L\"AST JSON field \\\"" + propName + L"\\\" must be an object or null.\");");
						}
						break;
					case AstPropType::Array:
						if (auto classPropSymbol = dynamic_cast<AstClassSymbol*>(propSymbol->propSymbol))
						{
							writer.WriteLine(prefix + L"\t\tauto jsonArray = dynamic_cast<vl::glr::json::JsonArray*>(value);");
							writer.WriteLine(prefix + L"\t\tif (!jsonArray) throw vl::Exception(L\"AST JSON field \\\"" + propName + L"\\\" must be an array.\");");
							writer.WriteLine(prefix + L"\t\tfor (auto item : jsonArray->items)");
							writer.WriteLine(prefix + L"\t\t{");
							writer.WriteLine(prefix + L"\t\t\tif (IsNull(item.Obj()))");
							writer.WriteLine(prefix + L"\t\t\t{");
							writer.WriteString(prefix + L"\t\t\t\tnode->" + propName + L".Add(vl::Ptr<");
							PrintCppType(group, classPropSymbol, writer);
							writer.WriteLine(L">());");
							writer.WriteLine(prefix + L"\t\t\t}");
							writer.WriteLine(prefix + L"\t\t\telse if (auto jsonObject = item.Cast<vl::glr::json::JsonObject>())");
							writer.WriteLine(prefix + L"\t\t\t{");
							writer.WriteString(prefix + L"\t\t\t\tauto ast = ReadJson(jsonObject.Obj()).Cast<");
							PrintCppType(group, classPropSymbol, writer);
							writer.WriteLine(L">();");
							writer.WriteLine(prefix + L"\t\t\t\tif (!ast) throw vl::Exception(L\"AST JSON field \\\"" + propName + L"\\\" contains an incompatible AST type.\");");
							writer.WriteLine(prefix + L"\t\t\t\tnode->" + propName + L".Add(ast);");
							writer.WriteLine(prefix + L"\t\t\t}");
							writer.WriteLine(prefix + L"\t\t\telse throw vl::Exception(L\"AST JSON field \\\"" + propName + L"\\\" contains a non-object, non-null item.\");");
							writer.WriteLine(prefix + L"\t\t}");
						}
						break;
					}
					writer.WriteLine(prefix + L"\t}");
				}
			}

			void WriteJsonReaderCppBody(AstDefFileGroup* group, const WString& prefix, stream::StreamWriter& writer)
			{
				List<AstClassSymbol*> visitors, concreteClasses;
				CollectVisitorsAndConcreteClasses(group, visitors, concreteClasses);

				writer.WriteLine(prefix + group->Name() + L"Visitor::JsonObjectScope::JsonObjectScope(vl::collections::List<vl::glr::json::JsonObject*>& _jsonObjects, vl::glr::json::JsonObject* json)");
				writer.WriteLine(prefix + L"\t: jsonObjects(_jsonObjects)");
				writer.WriteLine(prefix + L"{");
				writer.WriteLine(prefix + L"\tjsonObjects.Add(json);");
				writer.WriteLine(prefix + L"}");
				writer.WriteLine(L"");
				writer.WriteLine(prefix + group->Name() + L"Visitor::JsonObjectScope::~JsonObjectScope()");
				writer.WriteLine(prefix + L"{");
				writer.WriteLine(prefix + L"\tjsonObjects.RemoveAt(jsonObjects.Count() - 1);");
				writer.WriteLine(prefix + L"}");
				writer.WriteLine(L"");

				writer.WriteLine(prefix + L"vl::glr::json::JsonObject* " + group->Name() + L"Visitor::CurrentObject()");
				writer.WriteLine(prefix + L"{");
				writer.WriteLine(prefix + L"\treturn jsonObjects[jsonObjects.Count() - 1];");
				writer.WriteLine(prefix + L"}");
				writer.WriteLine(L"");

				writer.WriteLine(prefix + L"vl::glr::json::JsonNode* " + group->Name() + L"Visitor::FindField(const vl::WString& name)");
				writer.WriteLine(prefix + L"{");
				writer.WriteLine(prefix + L"\tfor (auto field : CurrentObject()->fields)");
				writer.WriteLine(prefix + L"\t{");
				writer.WriteLine(prefix + L"\t\tif (field && field->name.value == name) return field->value.Obj();");
				writer.WriteLine(prefix + L"\t}");
				writer.WriteLine(prefix + L"\treturn nullptr;");
				writer.WriteLine(prefix + L"}");
				writer.WriteLine(L"");

				writer.WriteLine(prefix + L"bool " + group->Name() + L"Visitor::IsNull(vl::glr::json::JsonNode* value)");
				writer.WriteLine(prefix + L"{");
				writer.WriteLine(prefix + L"\tauto literal = dynamic_cast<vl::glr::json::JsonLiteral*>(value);");
				writer.WriteLine(prefix + L"\treturn literal && literal->value == vl::glr::json::JsonLiteralValue::Null;");
				writer.WriteLine(prefix + L"}");
				writer.WriteLine(L"");

				writer.WriteLine(prefix + L"vl::WString " + group->Name() + L"Visitor::ReadType(vl::glr::json::JsonObject* json)");
				writer.WriteLine(prefix + L"{");
				writer.WriteLine(prefix + L"\tif (!json) throw vl::Exception(L\"AST JSON object cannot be null.\");");
				writer.WriteLine(prefix + L"\tbool typeFound = false;");
				writer.WriteLine(prefix + L"\tvl::WString typeName;");
				writer.WriteLine(prefix + L"\tfor (auto field : json->fields)");
				writer.WriteLine(prefix + L"\t{");
				writer.WriteLine(prefix + L"\t\tif (field && field->name.value == L\"$ast\")");
				writer.WriteLine(prefix + L"\t\t{");
				writer.WriteLine(prefix + L"\t\t\tif (typeFound) throw vl::Exception(L\"AST JSON object contains duplicate \\\"$ast\\\" fields.\");");
				writer.WriteLine(prefix + L"\t\t\ttypeFound = true;");
				writer.WriteLine(prefix + L"\t\t\tauto jsonString = field->value.Cast<vl::glr::json::JsonString>();");
				writer.WriteLine(prefix + L"\t\t\tif (!jsonString) throw vl::Exception(L\"AST JSON field \\\"$ast\\\" must be a string.\");");
				writer.WriteLine(prefix + L"\t\t\ttypeName = jsonString->content.value;");
				writer.WriteLine(prefix + L"\t\t}");
				writer.WriteLine(prefix + L"\t}");
				writer.WriteLine(prefix + L"\tif (!typeFound) throw vl::Exception(L\"AST JSON object is missing field \\\"$ast\\\".\");");
				writer.WriteLine(prefix + L"\treturn typeName;");
				writer.WriteLine(prefix + L"}");
				writer.WriteLine(L"");

				writer.WriteLine(prefix + L"void " + group->Name() + L"Visitor::ValidateFields(vl::glr::json::JsonObject* json, const vl::WString& typeName)");
				writer.WriteLine(prefix + L"{");
				writer.WriteLine(prefix + L"\tvl::collections::List<vl::WString> fieldNames;");
				writer.WriteLine(prefix + L"\tfor (auto field : json->fields)");
				writer.WriteLine(prefix + L"\t{");
				writer.WriteLine(prefix + L"\t\tif (!field || !field->value) throw vl::Exception(L\"AST JSON object contains an invalid field.\");");
				writer.WriteLine(prefix + L"\t\tauto name = field->name.value;");
				writer.WriteLine(prefix + L"\t\tif (fieldNames.Contains(name)) throw vl::Exception(L\"AST JSON object contains duplicate field \\\"\" + name + L\"\\\".\");");
				writer.WriteLine(prefix + L"\t\tfieldNames.Add(name);");
				writer.WriteLine(prefix + L"\t\tbool fieldFound = name == L\"$ast\";");
				bool firstClass = true;
				for (auto typeName : group->SymbolOrder())
				{
					auto classSymbol = dynamic_cast<AstClassSymbol*>(group->Symbols()[typeName]);
					if (!classSymbol || classSymbol->derivedClasses.Count() > 0) continue;
					writer.WriteLine(prefix + (firstClass ? L"\t\tif" : L"\t\telse if") + L" (typeName == L\"" + classSymbol->Name() + L"\")");
					writer.WriteLine(prefix + L"\t\t{");
					List<AstClassPropSymbol*> props;
					CollectClassProps(classSymbol, props);
					for (auto prop : props)
					{
						writer.WriteLine(prefix + L"\t\t\tfieldFound = fieldFound || name == L\"" + prop->Name() + L"\";");
					}
					writer.WriteLine(prefix + L"\t\t}");
					firstClass = false;
				}
				writer.WriteLine(prefix + L"\t\tif (!fieldFound) throw vl::Exception(L\"AST JSON object contains unknown field \\\"\" + name + L\"\\\" for type \\\"\" + typeName + L\"\\\".\");");
				writer.WriteLine(prefix + L"\t}");
				writer.WriteLine(prefix + L"}");
				writer.WriteLine(L"");

				for (auto typeSymbol : group->Symbols().Values())
				{
					if (auto classSymbol = dynamic_cast<AstClassSymbol*>(typeSymbol))
					{
						writer.WriteString(prefix + L"void " + group->Name() + L"Visitor::FillFields(");
						PrintCppType(group, classSymbol, writer);
						writer.WriteLine(L"* node)");
						writer.WriteLine(prefix + L"{");
						WriteJsonReaderFillFieldsBody(group, classSymbol, prefix, writer);
						writer.WriteLine(prefix + L"}");
						writer.WriteLine(L"");
					}
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
							writer.WriteLine(prefix + L"\tFillFields(node);");
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

				writer.WriteLine(prefix + L"vl::Ptr<vl::glr::ParsingAstBase> " + group->Name() + L"Visitor::ReadJson(vl::glr::json::JsonObject* json)");
				writer.WriteLine(prefix + L"{");
				writer.WriteLine(prefix + L"\tauto typeName = ReadType(json);");
				for (auto symbolName : group->SymbolOrder())
				{
					auto classSymbol = dynamic_cast<AstClassSymbol*>(group->Symbols()[symbolName]);
					if (!classSymbol || classSymbol->derivedClasses.Count() > 0) continue;
					writer.WriteLine(prefix + L"\tif (typeName == L\"" + classSymbol->Name() + L"\")");
					writer.WriteLine(prefix + L"\t{");
					writer.WriteString(prefix + L"\t\tauto node = vl::Ptr(new ");
					PrintCppType(group, classSymbol, writer);
					writer.WriteLine(L");");
					writer.WriteLine(prefix + L"\t\tJsonObjectScope scope(jsonObjects, json);");
					if (!classSymbol->baseClass)
					{
						writer.WriteLine(prefix + L"\t\tFillFields(node.Obj());");
					}
					else
					{
						auto rootClass = classSymbol;
						while (rootClass->baseClass) rootClass = rootClass->baseClass;
						writer.WriteString(prefix + L"\t\tstatic_cast<");
						PrintCppType(group, rootClass, writer);
						writer.WriteString(L"*>(node.Obj())->Accept(static_cast<");
						PrintCppType(group, rootClass, writer);
						writer.WriteLine(L"::IVisitor*>(this));");
					}
					writer.WriteLine(prefix + L"\t\tValidateFields(json, typeName);");
					writer.WriteLine(prefix + L"\t\treturn node;");
					writer.WriteLine(prefix + L"\t}");
				}
				writer.WriteLine(prefix + L"\tthrow vl::Exception(L\"AST JSON field \\\"$ast\\\" contains an unknown or abstract type \\\"\" + typeName + L\"\\\".\");");
				writer.WriteLine(prefix + L"}");
			}

			void WriteJsonVisitorCppFile(AstDefFileGroup* group, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer)
			{
				List<WString> extraNss;
				WriteAstUtilityCppFile(group, output->jsonH, extraNss, writer, [&](const WString& prefix)
				{
					writer.WriteLine(prefix + L"namespace json_visitor");
					writer.WriteLine(prefix + L"{");
					WriteJsonWriterCppBody(group, prefix + L"\t", writer);
					writer.WriteLine(prefix + L"}");
					writer.WriteLine(L"");
					writer.WriteLine(prefix + L"namespace json_reader");
					writer.WriteLine(prefix + L"{");
					WriteJsonReaderCppBody(group, prefix + L"\t", writer);
					writer.WriteLine(prefix + L"}");
				});
			}

/***********************************************************************
WriteJsonVisitorDtsFile
***********************************************************************/

			void CollectAllLeafClasses(AstClassSymbol* classSymbol, List<AstClassSymbol*>& leafClasses)
			{
				if (classSymbol->derivedClasses.Count() == 0)
				{
					leafClasses.Add(classSymbol);
				}
				else
				{
					for (auto derived : classSymbol->derivedClasses)
					{
						CollectAllLeafClasses(derived, leafClasses);
					}
				}
			}

			AstClassSymbol* FindNearestAncestorWithProps(AstClassSymbol* classSymbol)
			{
				auto current = classSymbol->baseClass;
				while (current)
				{
					if (current->Props().Count() > 0)
					{
						return current;
					}
					current = current->baseClass;
				}
				return nullptr;
			}

			void WriteDtsPropType(AstDefFileGroup* group, AstClassPropSymbol* propSymbol, stream::StreamWriter& writer)
			{
				switch (propSymbol->propType)
				{
				case AstPropType::Token:
					writer.WriteString(L"string");
					break;
				case AstPropType::Type:
					if (auto enumPropSymbol = dynamic_cast<AstEnumSymbol*>(propSymbol->propSymbol))
					{
						writer.WriteString(enumPropSymbol->Name());
					}
					else if (auto classPropSymbol = dynamic_cast<AstClassSymbol*>(propSymbol->propSymbol))
					{
						writer.WriteString(classPropSymbol->Name() + L" | null");
					}
					break;
				case AstPropType::Array:
					if (auto classPropSymbol = dynamic_cast<AstClassSymbol*>(propSymbol->propSymbol))
					{
						writer.WriteString(L"(" + classPropSymbol->Name() + L" | null)[]");
					}
					else
					{
						writer.WriteString(L"unknown[]");
					}
					break;
				}
			}

			void WriteJsonVisitorDtsFile(AstDefFileGroup* group, Ptr<CppAstGenOutput> output, stream::StreamWriter& writer)
			{
				List<AstClassSymbol*> visitors, concreteClasses;
				CollectVisitorsAndConcreteClasses(group, visitors, concreteClasses);

				// write enums
				for (auto name : group->SymbolOrder())
				{
					if (auto enumSymbol = dynamic_cast<AstEnumSymbol*>(group->Symbols()[name]))
					{
						writer.WriteString(L"export type " + enumSymbol->Name() + L" =");
						bool first = true;
						for (auto itemName : enumSymbol->ItemOrder())
						{
							if (!first) writer.WriteString(L" |");
							writer.WriteString(L" \"" + itemName + L"\"");
							first = false;
						}
						writer.WriteLine(L";");
						writer.WriteLine(L"");
					}
				}

				// write union types for abstract classes (classes with derived classes)
				for (auto visitorSymbol : visitors)
				{
					List<AstClassSymbol*> leafClasses;
					CollectAllLeafClasses(visitorSymbol, leafClasses);

					writer.WriteString(L"export type " + visitorSymbol->Name() + L" =");
					for (auto [leafClass, index] : indexed(leafClasses))
					{
						if (index > 0) writer.WriteString(L" |");
						writer.WriteString(L" " + leafClass->Name());
					}
					writer.WriteLine(L";");
					writer.WriteLine(L"");
				}

				// write _Common interfaces for abstract classes that have own properties
				for (auto name : group->SymbolOrder())
				{
					if (auto classSymbol = dynamic_cast<AstClassSymbol*>(group->Symbols()[name]))
					{
						if (classSymbol->derivedClasses.Count() > 0 && classSymbol->Props().Count() > 0)
						{
							auto ancestor = FindNearestAncestorWithProps(classSymbol);
							writer.WriteString(L"export interface " + classSymbol->Name() + L"_Common");
							if (ancestor)
							{
								writer.WriteString(L" extends " + ancestor->Name() + L"_Common");
							}
							writer.WriteLine(L" {");
							for (auto propName : classSymbol->PropOrder())
							{
								auto propSymbol = classSymbol->Props()[propName];
								writer.WriteString(L"    " + propName + L": ");
								WriteDtsPropType(group, propSymbol, writer);
								writer.WriteLine(L";");
							}
							writer.WriteLine(L"}");
							writer.WriteLine(L"");
						}
					}
				}

				// write interfaces for leaf classes (classes with no derived classes)
				for (auto name : group->SymbolOrder())
				{
					if (auto classSymbol = dynamic_cast<AstClassSymbol*>(group->Symbols()[name]))
					{
						if (classSymbol->derivedClasses.Count() == 0)
						{
							// find the nearest ancestor with properties for extends
							auto ancestor = FindNearestAncestorWithProps(classSymbol);
							writer.WriteString(L"export interface " + classSymbol->Name());
							if (ancestor)
							{
								if (ancestor->derivedClasses.Count() > 0)
								{
									writer.WriteString(L" extends " + ancestor->Name() + L"_Common");
								}
								else
								{
									writer.WriteString(L" extends " + ancestor->Name());
								}
							}
							writer.WriteLine(L" {");
							writer.WriteLine(L"    $ast: \"" + classSymbol->Name() + L"\";");
							for (auto propName : classSymbol->PropOrder())
							{
								auto propSymbol = classSymbol->Props()[propName];
								writer.WriteString(L"    " + propName + L": ");
								WriteDtsPropType(group, propSymbol, writer);
								writer.WriteLine(L";");
							}
							writer.WriteLine(L"}");
							writer.WriteLine(L"");
						}
					}
				}

				// write interfaces for concrete classes (standalone, no base and no derived)
				for (auto classSymbol : concreteClasses)
				{
					// concreteClasses are !baseClass && derivedClasses.Count()==0
					// these are already handled in the leaf class section above
					// (they would appear as leaf classes with no ancestor)
				}
			}
		}
	}
}

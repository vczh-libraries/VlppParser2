#include "GlrJson.h"
#include "Generated/JsonAst_Traverse.h"

namespace vl
{
	namespace glr
	{
		namespace json
		{
			using namespace stream;
			using namespace collections;

/***********************************************************************
JsonUnescapeVisitor
***********************************************************************/

			class JsonUnescapeVisitor : public traverse_visitor::AstVisitor
			{
			protected:
				void Traverse(JsonObjectField* node) override
				{
					node->name.value = GenerateToStream(
						[node](TextWriter& writer)
						{
							JsonUnescapeString(node->name.value.Sub(1, node->name.value.Length() - 2), writer);
						});
				}

				void Traverse(JsonString* node) override
				{
					node->content.value = GenerateToStream(
						[node](TextWriter& writer)
						{
							JsonUnescapeString(node->content.value.Sub(1, node->content.value.Length() - 2), writer);
						});
				}
			};

/***********************************************************************
JsonFormatting
***********************************************************************/

			JsonFormatting::JsonFormatting()
				: indentation(L"  ")
			{
			}

			JsonFormatting::JsonFormatting(bool _crlf, bool _compact, const wchar_t* _indentation)
				: crlf(_crlf)
				, compact(_compact)
				, indentation(_indentation)
			{
			}

/***********************************************************************
JsonIsCompactVisitor
***********************************************************************/

			class JsonIsCompactVisitorBase : public Object, public JsonNode::IVisitor
			{
			public:
				bool						result = true;

				void Visit(JsonLiteral* node) override
				{
				}

				void Visit(JsonString* node) override
				{
				}

				void Visit(JsonNumber* node) override
				{
				}
			};

			class JsonIsCompactFieldVisitor : public JsonIsCompactVisitorBase
			{
			public:
				void Visit(JsonArray* node) override
				{
					result = node->items.Count() == 0;
				}

				void Visit(JsonObject* node) override
				{
					result = node->fields.Count() == 0;
				}
			};

			class JsonIsCompactVisitor : public JsonIsCompactVisitorBase
			{
			public:
				void Visit(JsonArray* node) override
				{
					for (auto item : node->items)
					{
						JsonIsCompactFieldVisitor visitor;
						item->Accept(&visitor);
						if (!visitor.result)
						{
							result = false;
							break;
						}
					}
				}

				void Visit(JsonObject* node) override
				{
					for (auto field : node->fields)
					{
						JsonIsCompactFieldVisitor visitor;
						field->value->Accept(&visitor);
						if (!visitor.result)
						{
							result = false;
							break;
						}
					}
				}
			};

/***********************************************************************
JsonPrintVisitor
***********************************************************************/

			class JsonPrintVisitor : public Object, public JsonNode::IVisitor
			{
			public:
				JsonFormatting				formatting;
				TextWriter&					writer;
				vint						indent = 0;

				JsonPrintVisitor(JsonFormatting _formatting, TextWriter& _writer)
					: formatting(_formatting)
					, writer(_writer)
				{
				}

				void WriteIndentation()
				{
					for (vint i = 0; i < indent; i++)
					{
						writer.WriteString(formatting.indentation);
					}
				}

				bool IsCompact(JsonNode* node)
				{
					JsonIsCompactVisitor visitor;
					node->Accept(&visitor);
					return visitor.result;
				}

				void Visit(JsonLiteral* node) override
				{
					switch(node->value)
					{
					case JsonLiteralValue::True:
						writer.WriteString(L"true");
						break;
					case JsonLiteralValue::False:
						writer.WriteString(L"false");
						break;
					case JsonLiteralValue::Null:
						writer.WriteString(L"null");
						break;
					default:;
					}
				}

				void Visit(JsonString* node) override
				{
					writer.WriteChar(L'\"');
					JsonEscapeString(node->content.value, writer);
					writer.WriteChar(L'\"');
				}

				void Visit(JsonNumber* node) override
				{
					writer.WriteString(node->content.value);
				}

				void AfterOpeningObject(bool insertCrlf)
				{
					if (insertCrlf)
					{
						writer.WriteString(L"\r\n");
						indent++;
					}
				}

				void BeforeClosingObject(bool insertCrlf)
				{
					if (insertCrlf)
					{
						indent--;
					}
				}

				void BeforeChildNode(bool insertCrlf)
				{
					if (insertCrlf) WriteIndentation();
				}

				void AfterChildNode(bool insertCrlf, bool lastNode)
				{
					if (!lastNode)
					{
						if (!insertCrlf && formatting.crlf)
						{
							writer.WriteString(L", ");
						}
						else
						{
							writer.WriteChar(L',');
						}
					}
					if (insertCrlf) writer.WriteString(L"\r\n");
				}

				void Visit(JsonArray* node) override
				{
					writer.WriteChar(L'[');
					if (node->items.Count() > 0)
					{
						bool insertCrlf = formatting.crlf && !(formatting.compact && IsCompact(node));
						AfterOpeningObject(insertCrlf);
						for (auto [item, i] : indexed(node->items))
						{
							BeforeChildNode(insertCrlf);
							item->Accept(this);
							AfterChildNode(insertCrlf, i == node->items.Count() - 1);
						}
						BeforeClosingObject(insertCrlf);
					}
					writer.WriteChar(L']');
				}

				void Visit(JsonObject* node) override
				{
					writer.WriteChar(L'{');
					if (node->fields.Count() > 0)
					{
						bool insertCrlf = formatting.crlf && !(formatting.compact && IsCompact(node));
						AfterOpeningObject(insertCrlf);
						for (auto [field, i] : indexed(node->fields))
						{
							BeforeChildNode(insertCrlf);
							writer.WriteChar(L'\"');
							JsonEscapeString(field->name.value, writer);
							writer.WriteString(L"\":");
							field->value->Accept(this);
							AfterChildNode(insertCrlf, i == node->fields.Count() - 1);
						}
						BeforeClosingObject(insertCrlf);
					}
					writer.WriteChar(L'}');
				}
			};

/***********************************************************************
API
***********************************************************************/

			Ptr<JsonNode> JsonParse(const WString& input, const Parser& parser)
			{
				auto ast = parser.ParseJRoot(input);
				JsonUnescapeVisitor().InspectInto(ast.Obj());
				return ast;
			}

			void JsonPrint(Ptr<JsonNode> node, stream::TextWriter& writer, JsonFormatting formatting)
			{
				JsonPrintVisitor visitor(formatting, writer);
				node->Accept(&visitor);
			}

			WString JsonToString(Ptr<JsonNode> node, JsonFormatting formatting)
			{
				return GenerateToStream([&](StreamWriter& writer)
				{
					JsonPrint(node, writer, formatting);
				});
			}
		}
	}
}

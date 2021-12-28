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
JsonPrintVisitor
***********************************************************************/

			class JsonPrintVisitor : public Object, public JsonNode::IVisitor
			{
			public:
				TextWriter&					writer;

				JsonPrintVisitor(TextWriter& _writer)
					:writer(_writer)
				{
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

				void Visit(JsonArray* node) override
				{
					writer.WriteChar(L'[');
					for (auto [item, i] : indexed(node->items))
					{
						if(i>0) writer.WriteChar(L',');
						item->Accept(this);
					}
					writer.WriteChar(L']');
				}

				void Visit(JsonObject* node) override
				{
					writer.WriteChar(L'{');
					for (auto [field, i] : indexed(node->fields))
					{
						if(i>0) writer.WriteChar(L',');
						writer.WriteChar(L'\"');
						JsonEscapeString(field->name.value, writer);
						writer.WriteString(L"\":");
						field->value->Accept(this);
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

			void JsonPrint(Ptr<JsonNode> node, stream::TextWriter& writer)
			{
				JsonPrintVisitor visitor(writer);
				node->Accept(&visitor);
			}

			WString JsonToString(Ptr<JsonNode> node)
			{
				return GenerateToStream([&](StreamWriter& writer)
				{
					JsonPrint(node, writer);
				});
			}
		}
	}
}

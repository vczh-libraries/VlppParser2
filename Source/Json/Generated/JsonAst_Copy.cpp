/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:Ast
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "JsonAst_Copy.h"

namespace vl
{
	namespace glr
	{
		namespace json
		{
			namespace copy_visitor
			{
				void AstVisitor::CopyFields(JsonArray* from, JsonArray* to)
				{
					CopyFields(static_cast<JsonNode*>(from), static_cast<JsonNode*>(to));
					for (auto&& listItem : from->items)
					{
						to->items.Add(CopyNode(listItem.Obj()));
					}
				}

				void AstVisitor::CopyFields(JsonLiteral* from, JsonLiteral* to)
				{
					CopyFields(static_cast<JsonNode*>(from), static_cast<JsonNode*>(to));
					to->value = from->value;
				}

				void AstVisitor::CopyFields(JsonNode* from, JsonNode* to)
				{
				}

				void AstVisitor::CopyFields(JsonNumber* from, JsonNumber* to)
				{
					CopyFields(static_cast<JsonNode*>(from), static_cast<JsonNode*>(to));
					to->content = from->content;
				}

				void AstVisitor::CopyFields(JsonObject* from, JsonObject* to)
				{
					CopyFields(static_cast<JsonNode*>(from), static_cast<JsonNode*>(to));
					for (auto&& listItem : from->fields)
					{
						to->fields.Add(CopyNode(listItem.Obj()));
					}
				}

				void AstVisitor::CopyFields(JsonObjectField* from, JsonObjectField* to)
				{
					to->name = from->name;
					to->value = CopyNode(from->value.Obj());
				}

				void AstVisitor::CopyFields(JsonString* from, JsonString* to)
				{
					CopyFields(static_cast<JsonNode*>(from), static_cast<JsonNode*>(to));
					to->content = from->content;
				}

				void AstVisitor::Visit(JsonObjectField* node)
				{
					auto newNode = vl::MakePtr<JsonObjectField>();
					CopyFields(node, newNode.Obj());
					this->result = newNode;
				}

				void AstVisitor::Visit(JsonLiteral* node)
				{
					auto newNode = vl::MakePtr<JsonLiteral>();
					CopyFields(node, newNode.Obj());
					this->result = newNode;
				}

				void AstVisitor::Visit(JsonString* node)
				{
					auto newNode = vl::MakePtr<JsonString>();
					CopyFields(node, newNode.Obj());
					this->result = newNode;
				}

				void AstVisitor::Visit(JsonNumber* node)
				{
					auto newNode = vl::MakePtr<JsonNumber>();
					CopyFields(node, newNode.Obj());
					this->result = newNode;
				}

				void AstVisitor::Visit(JsonArray* node)
				{
					auto newNode = vl::MakePtr<JsonArray>();
					CopyFields(node, newNode.Obj());
					this->result = newNode;
				}

				void AstVisitor::Visit(JsonObject* node)
				{
					auto newNode = vl::MakePtr<JsonObject>();
					CopyFields(node, newNode.Obj());
					this->result = newNode;
				}

				vl::Ptr<JsonNode> AstVisitor::CopyNode(JsonNode* node)
				{
					if (!node) return nullptr;
					node->Accept(static_cast<JsonNode::IVisitor*>(this));
					return this->result.Cast<JsonNode>();
				}

				vl::Ptr<JsonObjectField> AstVisitor::CopyNode(JsonObjectField* node)
				{
					if (!node) return nullptr;
					Visit(node);
					return this->result.Cast<JsonObjectField>();
				}

				vl::Ptr<JsonArray> AstVisitor::CopyNode(JsonArray* node)
				{
					if (!node) return nullptr;
					return CopyNode(static_cast<JsonNode*>(node)).Cast<JsonArray>();
				}

				vl::Ptr<JsonLiteral> AstVisitor::CopyNode(JsonLiteral* node)
				{
					if (!node) return nullptr;
					return CopyNode(static_cast<JsonNode*>(node)).Cast<JsonLiteral>();
				}

				vl::Ptr<JsonNumber> AstVisitor::CopyNode(JsonNumber* node)
				{
					if (!node) return nullptr;
					return CopyNode(static_cast<JsonNode*>(node)).Cast<JsonNumber>();
				}

				vl::Ptr<JsonObject> AstVisitor::CopyNode(JsonObject* node)
				{
					if (!node) return nullptr;
					return CopyNode(static_cast<JsonNode*>(node)).Cast<JsonObject>();
				}

				vl::Ptr<JsonString> AstVisitor::CopyNode(JsonString* node)
				{
					if (!node) return nullptr;
					return CopyNode(static_cast<JsonNode*>(node)).Cast<JsonString>();
				}

			}
		}
	}
}
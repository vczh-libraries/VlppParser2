/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:RuleAst
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "ParserGenRuleAst_Json.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			namespace json_visitor
			{
				void RuleAstVisitor::PrintFields(GlrAlternativeSyntax* node)
				{
					BeginField(L"first");
					Print(node->first.Obj());
					EndField();
					BeginField(L"second");
					Print(node->second.Obj());
					EndField();
				}
				void RuleAstVisitor::PrintFields(GlrAssignment* node)
				{
					BeginField(L"field");
					WriteToken(node->field);
					EndField();
					BeginField(L"value");
					WriteToken(node->value);
					EndField();
				}
				void RuleAstVisitor::PrintFields(GlrClause* node)
				{
				}
				void RuleAstVisitor::PrintFields(GlrCreateClause* node)
				{
					BeginField(L"assignments");
					BeginArray();
					for (auto&& listItem : node->assignments)
					{
						BeginArrayItem();
						Print(listItem.Obj());
						EndArrayItem();
					}
					EndArray();
					EndField();
					BeginField(L"syntax");
					Print(node->syntax.Obj());
					EndField();
					BeginField(L"type");
					WriteToken(node->type);
					EndField();
				}
				void RuleAstVisitor::PrintFields(GlrLiteralSyntax* node)
				{
					BeginField(L"value");
					WriteToken(node->value);
					EndField();
				}
				void RuleAstVisitor::PrintFields(GlrLoopSyntax* node)
				{
					BeginField(L"delimiter");
					Print(node->delimiter.Obj());
					EndField();
					BeginField(L"syntax");
					Print(node->syntax.Obj());
					EndField();
				}
				void RuleAstVisitor::PrintFields(GlrOptionalSyntax* node)
				{
					BeginField(L"syntax");
					Print(node->syntax.Obj());
					EndField();
				}
				void RuleAstVisitor::PrintFields(GlrPartialClause* node)
				{
					BeginField(L"assignments");
					BeginArray();
					for (auto&& listItem : node->assignments)
					{
						BeginArrayItem();
						Print(listItem.Obj());
						EndArrayItem();
					}
					EndArray();
					EndField();
					BeginField(L"syntax");
					Print(node->syntax.Obj());
					EndField();
					BeginField(L"type");
					WriteToken(node->type);
					EndField();
				}
				void RuleAstVisitor::PrintFields(GlrRefSyntax* node)
				{
					BeginField(L"field");
					WriteToken(node->field);
					EndField();
					BeginField(L"name");
					WriteToken(node->name);
					EndField();
				}
				void RuleAstVisitor::PrintFields(GlrRule* node)
				{
					BeginField(L"clauses");
					BeginArray();
					for (auto&& listItem : node->clauses)
					{
						BeginArrayItem();
						Print(listItem.Obj());
						EndArrayItem();
					}
					EndArray();
					EndField();
					BeginField(L"name");
					WriteToken(node->name);
					EndField();
				}
				void RuleAstVisitor::PrintFields(GlrSequenceSyntax* node)
				{
					BeginField(L"first");
					Print(node->first.Obj());
					EndField();
					BeginField(L"second");
					Print(node->second.Obj());
					EndField();
				}
				void RuleAstVisitor::PrintFields(GlrSyntax* node)
				{
				}
				void RuleAstVisitor::PrintFields(GlrSyntaxFile* node)
				{
					BeginField(L"rules");
					BeginArray();
					for (auto&& listItem : node->rules)
					{
						BeginArrayItem();
						Print(listItem.Obj());
						EndArrayItem();
					}
					EndArray();
					EndField();
				}
				void RuleAstVisitor::PrintFields(GlrUseSyntax* node)
				{
					BeginField(L"name");
					WriteToken(node->name);
					EndField();
				}
				void RuleAstVisitor::PrintFields(Glr_ReuseClause* node)
				{
					BeginField(L"assignments");
					BeginArray();
					for (auto&& listItem : node->assignments)
					{
						BeginArrayItem();
						Print(listItem.Obj());
						EndArrayItem();
					}
					EndArray();
					EndField();
					BeginField(L"syntax");
					Print(node->syntax.Obj());
					EndField();
				}

				void RuleAstVisitor::Visit(GlrRefSyntax* node)
				{
					if (!node)
					{
						WriteNull();
						return;
					}
					BeginObject();
					WriteType(L"RefSyntax", node);
					PrintFields(static_cast<GlrSyntax*>(node));
					PrintFields(static_cast<GlrRefSyntax*>(node));
					EndObject();
				}

				void RuleAstVisitor::Visit(GlrLiteralSyntax* node)
				{
					if (!node)
					{
						WriteNull();
						return;
					}
					BeginObject();
					WriteType(L"LiteralSyntax", node);
					PrintFields(static_cast<GlrSyntax*>(node));
					PrintFields(static_cast<GlrLiteralSyntax*>(node));
					EndObject();
				}

				void RuleAstVisitor::Visit(GlrLoopSyntax* node)
				{
					if (!node)
					{
						WriteNull();
						return;
					}
					BeginObject();
					WriteType(L"LoopSyntax", node);
					PrintFields(static_cast<GlrSyntax*>(node));
					PrintFields(static_cast<GlrLoopSyntax*>(node));
					EndObject();
				}

				void RuleAstVisitor::Visit(GlrOptionalSyntax* node)
				{
					if (!node)
					{
						WriteNull();
						return;
					}
					BeginObject();
					WriteType(L"OptionalSyntax", node);
					PrintFields(static_cast<GlrSyntax*>(node));
					PrintFields(static_cast<GlrOptionalSyntax*>(node));
					EndObject();
				}

				void RuleAstVisitor::Visit(GlrSequenceSyntax* node)
				{
					if (!node)
					{
						WriteNull();
						return;
					}
					BeginObject();
					WriteType(L"SequenceSyntax", node);
					PrintFields(static_cast<GlrSyntax*>(node));
					PrintFields(static_cast<GlrSequenceSyntax*>(node));
					EndObject();
				}

				void RuleAstVisitor::Visit(GlrAlternativeSyntax* node)
				{
					if (!node)
					{
						WriteNull();
						return;
					}
					BeginObject();
					WriteType(L"AlternativeSyntax", node);
					PrintFields(static_cast<GlrSyntax*>(node));
					PrintFields(static_cast<GlrAlternativeSyntax*>(node));
					EndObject();
				}

				void RuleAstVisitor::Visit(GlrCreateClause* node)
				{
					if (!node)
					{
						WriteNull();
						return;
					}
					BeginObject();
					WriteType(L"CreateClause", node);
					PrintFields(static_cast<GlrClause*>(node));
					PrintFields(static_cast<GlrCreateClause*>(node));
					EndObject();
				}

				void RuleAstVisitor::Visit(GlrPartialClause* node)
				{
					if (!node)
					{
						WriteNull();
						return;
					}
					BeginObject();
					WriteType(L"PartialClause", node);
					PrintFields(static_cast<GlrClause*>(node));
					PrintFields(static_cast<GlrPartialClause*>(node));
					EndObject();
				}

				void RuleAstVisitor::Visit(Glr_ReuseClause* node)
				{
					if (!node)
					{
						WriteNull();
						return;
					}
					BeginObject();
					WriteType(L"_ReuseClause", node);
					PrintFields(static_cast<GlrClause*>(node));
					PrintFields(static_cast<Glr_ReuseClause*>(node));
					EndObject();
				}

				RuleAstVisitor::RuleAstVisitor(vl::stream::StreamWriter& _writer)
					: vl::glr::JsonVisitorBase(_writer)
				{
				}

				void RuleAstVisitor::Print(GlrSyntax* node)
				{
					if (!node)
					{
						WriteNull();
						return;
					}
					node->Accept(static_cast<GlrSyntax::IVisitor*>(this));
				}

				void RuleAstVisitor::Print(GlrClause* node)
				{
					if (!node)
					{
						WriteNull();
						return;
					}
					node->Accept(static_cast<GlrClause::IVisitor*>(this));
				}

				void RuleAstVisitor::Print(GlrUseSyntax* node)
				{
					if (!node)
					{
						WriteNull();
						return;
					}
					BeginObject();
					WriteType(L"UseSyntax", node);
					PrintFields(static_cast<GlrUseSyntax*>(node));
					EndObject();
				}

				void RuleAstVisitor::Print(GlrAssignment* node)
				{
					if (!node)
					{
						WriteNull();
						return;
					}
					BeginObject();
					WriteType(L"Assignment", node);
					PrintFields(static_cast<GlrAssignment*>(node));
					EndObject();
				}

				void RuleAstVisitor::Print(GlrRule* node)
				{
					if (!node)
					{
						WriteNull();
						return;
					}
					BeginObject();
					WriteType(L"Rule", node);
					PrintFields(static_cast<GlrRule*>(node));
					EndObject();
				}

				void RuleAstVisitor::Print(GlrSyntaxFile* node)
				{
					if (!node)
					{
						WriteNull();
						return;
					}
					BeginObject();
					WriteType(L"SyntaxFile", node);
					PrintFields(static_cast<GlrSyntaxFile*>(node));
					EndObject();
				}

			}
		}
	}
}
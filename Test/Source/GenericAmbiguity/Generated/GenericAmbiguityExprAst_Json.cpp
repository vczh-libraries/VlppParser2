/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:ExprAst
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "GenericAmbiguityExprAst_Json.h"

namespace genericambiguity
{
	namespace json_visitor
	{
		void ExprAstVisitor::PrintFields(BinaryExpr* node)
		{
			BeginField(L"left");
			Print(node->left.Obj());
			EndField();
			BeginField(L"op");
			switch (node->op)
			{
			case genericambiguity::BinaryOp::GT:
				WriteString(L"GT");
				break;
			case genericambiguity::BinaryOp::LT:
				WriteString(L"LT");
				break;
			default:
				WriteNull();
			}
			EndField();
			BeginField(L"right");
			Print(node->right.Obj());
			EndField();
		}
		void ExprAstVisitor::PrintFields(CallExpr* node)
		{
			BeginField(L"args");
			BeginArray();
			for (auto&& listItem : node->args)
			{
				BeginArrayItem();
				Print(listItem.Obj());
				EndArrayItem();
			}
			EndArray();
			EndField();
			BeginField(L"func");
			Print(node->func.Obj());
			EndField();
		}
		void ExprAstVisitor::PrintFields(DecrementExpr* node)
		{
			BeginField(L"expr");
			Print(node->expr.Obj());
			EndField();
		}
		void ExprAstVisitor::PrintFields(Expr* node)
		{
		}
		void ExprAstVisitor::PrintFields(ExprToResolve* node)
		{
			BeginField(L"candidates");
			BeginArray();
			for (auto&& listItem : node->candidates)
			{
				BeginArrayItem();
				Print(listItem.Obj());
				EndArrayItem();
			}
			EndArray();
			EndField();
		}
		void ExprAstVisitor::PrintFields(GenericExpr* node)
		{
			BeginField(L"args");
			BeginArray();
			for (auto&& listItem : node->args)
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
		void ExprAstVisitor::PrintFields(Module* node)
		{
			BeginField(L"expr");
			Print(node->expr.Obj());
			EndField();
		}
		void ExprAstVisitor::PrintFields(PostfixExpr* node)
		{
			BeginField(L"expr");
			Print(node->expr.Obj());
			EndField();
			BeginField(L"op");
			switch (node->op)
			{
			case genericambiguity::PostfixOp::Add:
				WriteString(L"Add");
				break;
			case genericambiguity::PostfixOp::Increment:
				WriteString(L"Increment");
				break;
			case genericambiguity::PostfixOp::Sub:
				WriteString(L"Sub");
				break;
			default:
				WriteNull();
			}
			EndField();
		}
		void ExprAstVisitor::PrintFields(RefExpr* node)
		{
			BeginField(L"name");
			WriteToken(node->name);
			EndField();
		}

		void ExprAstVisitor::Visit(ExprToResolve* node)
		{
			if (!node)
			{
				WriteNull();
				return;
			}
			BeginObject();
			WriteType(L"ExprToResolve", node);
			PrintFields(static_cast<Expr*>(node));
			PrintFields(static_cast<ExprToResolve*>(node));
			EndObject();
		}

		void ExprAstVisitor::Visit(RefExpr* node)
		{
			if (!node)
			{
				WriteNull();
				return;
			}
			BeginObject();
			WriteType(L"RefExpr", node);
			PrintFields(static_cast<Expr*>(node));
			PrintFields(static_cast<RefExpr*>(node));
			EndObject();
		}

		void ExprAstVisitor::Visit(GenericExpr* node)
		{
			if (!node)
			{
				WriteNull();
				return;
			}
			BeginObject();
			WriteType(L"GenericExpr", node);
			PrintFields(static_cast<Expr*>(node));
			PrintFields(static_cast<GenericExpr*>(node));
			EndObject();
		}

		void ExprAstVisitor::Visit(CallExpr* node)
		{
			if (!node)
			{
				WriteNull();
				return;
			}
			BeginObject();
			WriteType(L"CallExpr", node);
			PrintFields(static_cast<Expr*>(node));
			PrintFields(static_cast<CallExpr*>(node));
			EndObject();
		}

		void ExprAstVisitor::Visit(PostfixExpr* node)
		{
			if (!node)
			{
				WriteNull();
				return;
			}
			BeginObject();
			WriteType(L"PostfixExpr", node);
			PrintFields(static_cast<Expr*>(node));
			PrintFields(static_cast<PostfixExpr*>(node));
			EndObject();
		}

		void ExprAstVisitor::Visit(DecrementExpr* node)
		{
			if (!node)
			{
				WriteNull();
				return;
			}
			BeginObject();
			WriteType(L"DecrementExpr", node);
			PrintFields(static_cast<Expr*>(node));
			PrintFields(static_cast<DecrementExpr*>(node));
			EndObject();
		}

		void ExprAstVisitor::Visit(BinaryExpr* node)
		{
			if (!node)
			{
				WriteNull();
				return;
			}
			BeginObject();
			WriteType(L"BinaryExpr", node);
			PrintFields(static_cast<Expr*>(node));
			PrintFields(static_cast<BinaryExpr*>(node));
			EndObject();
		}

		ExprAstVisitor::ExprAstVisitor(vl::stream::StreamWriter& _writer)
			: vl::glr::JsonVisitorBase(_writer)
		{
		}

		void ExprAstVisitor::Print(Expr* node)
		{
			if (!node)
			{
				WriteNull();
				return;
			}
			node->Accept(static_cast<Expr::IVisitor*>(this));
		}

		void ExprAstVisitor::Print(Module* node)
		{
			if (!node)
			{
				WriteNull();
				return;
			}
			BeginObject();
			WriteType(L"Module", node);
			PrintFields(static_cast<Module*>(node));
			EndObject();
		}

	}
}
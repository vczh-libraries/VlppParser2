/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:Ast
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "CalculatorAst_Traverse.h"

namespace calculator
{
	namespace traverse_visitor
	{
		void AstVisitor::Traverse(vl::glr::ParsingToken& token) {}
		void AstVisitor::Traverse(vl::glr::ParsingAstBase* node) {}
		void AstVisitor::Traverse(Arg* node) {}
		void AstVisitor::Traverse(Binary* node) {}
		void AstVisitor::Traverse(Call* node) {}
		void AstVisitor::Traverse(Expandable* node) {}
		void AstVisitor::Traverse(Expr* node) {}
		void AstVisitor::Traverse(False* node) {}
		void AstVisitor::Traverse(Func* node) {}
		void AstVisitor::Traverse(Import* node) {}
		void AstVisitor::Traverse(LetExpr* node) {}
		void AstVisitor::Traverse(Module* node) {}
		void AstVisitor::Traverse(NumExpr* node) {}
		void AstVisitor::Traverse(Ref* node) {}
		void AstVisitor::Traverse(True* node) {}
		void AstVisitor::Traverse(Unary* node) {}

		void AstVisitor::Finishing(vl::glr::ParsingAstBase* node) {}
		void AstVisitor::Finishing(Arg* node) {}
		void AstVisitor::Finishing(Binary* node) {}
		void AstVisitor::Finishing(Call* node) {}
		void AstVisitor::Finishing(Expandable* node) {}
		void AstVisitor::Finishing(Expr* node) {}
		void AstVisitor::Finishing(False* node) {}
		void AstVisitor::Finishing(Func* node) {}
		void AstVisitor::Finishing(Import* node) {}
		void AstVisitor::Finishing(LetExpr* node) {}
		void AstVisitor::Finishing(Module* node) {}
		void AstVisitor::Finishing(NumExpr* node) {}
		void AstVisitor::Finishing(Ref* node) {}
		void AstVisitor::Finishing(True* node) {}
		void AstVisitor::Finishing(Unary* node) {}

		void AstVisitor::Visit(NumExpr* node)
		{
			if (!node) return;
			Traverse(static_cast<vl::glr::ParsingAstBase*>(node));
			Traverse(static_cast<Expr*>(node));
			Traverse(static_cast<NumExpr*>(node));
			Traverse(node->value);
			Finishing(static_cast<NumExpr*>(node));
			Finishing(static_cast<Expr*>(node));
			Finishing(static_cast<vl::glr::ParsingAstBase*>(node));
		}

		void AstVisitor::Visit(Ref* node)
		{
			if (!node) return;
			Traverse(static_cast<vl::glr::ParsingAstBase*>(node));
			Traverse(static_cast<Expr*>(node));
			Traverse(static_cast<Ref*>(node));
			Traverse(node->name);
			Finishing(static_cast<Ref*>(node));
			Finishing(static_cast<Expr*>(node));
			Finishing(static_cast<vl::glr::ParsingAstBase*>(node));
		}

		void AstVisitor::Visit(True* node)
		{
			if (!node) return;
			Traverse(static_cast<vl::glr::ParsingAstBase*>(node));
			Traverse(static_cast<Expr*>(node));
			Traverse(static_cast<True*>(node));
			Finishing(static_cast<True*>(node));
			Finishing(static_cast<Expr*>(node));
			Finishing(static_cast<vl::glr::ParsingAstBase*>(node));
		}

		void AstVisitor::Visit(False* node)
		{
			if (!node) return;
			Traverse(static_cast<vl::glr::ParsingAstBase*>(node));
			Traverse(static_cast<Expr*>(node));
			Traverse(static_cast<False*>(node));
			Finishing(static_cast<False*>(node));
			Finishing(static_cast<Expr*>(node));
			Finishing(static_cast<vl::glr::ParsingAstBase*>(node));
		}

		void AstVisitor::Visit(Func* node)
		{
			if (!node) return;
			Traverse(static_cast<vl::glr::ParsingAstBase*>(node));
			Traverse(static_cast<Expr*>(node));
			Traverse(static_cast<Func*>(node));
			for (auto&& listItem : node->args)
			{
				InspectInto(listItem.Obj());
			}
			InspectInto(node->value.Obj());
			Finishing(static_cast<Func*>(node));
			Finishing(static_cast<Expr*>(node));
			Finishing(static_cast<vl::glr::ParsingAstBase*>(node));
		}

		void AstVisitor::Visit(Call* node)
		{
			if (!node) return;
			Traverse(static_cast<vl::glr::ParsingAstBase*>(node));
			Traverse(static_cast<Expr*>(node));
			Traverse(static_cast<Call*>(node));
			InspectInto(node->arg.Obj());
			InspectInto(node->func.Obj());
			Finishing(static_cast<Call*>(node));
			Finishing(static_cast<Expr*>(node));
			Finishing(static_cast<vl::glr::ParsingAstBase*>(node));
		}

		void AstVisitor::Visit(Expandable* node)
		{
			node->Accept(static_cast<Expandable::IVisitor*>(this));
		}

		void AstVisitor::Visit(LetExpr* node)
		{
			if (!node) return;
			Traverse(static_cast<vl::glr::ParsingAstBase*>(node));
			Traverse(static_cast<Expr*>(node));
			Traverse(static_cast<Expandable*>(node));
			Traverse(static_cast<LetExpr*>(node));
			Traverse(node->name);
			InspectInto(node->value.Obj());
			InspectInto(node->expanded.Obj());
			Finishing(static_cast<LetExpr*>(node));
			Finishing(static_cast<Expandable*>(node));
			Finishing(static_cast<Expr*>(node));
			Finishing(static_cast<vl::glr::ParsingAstBase*>(node));
		}

		void AstVisitor::Visit(Unary* node)
		{
			if (!node) return;
			Traverse(static_cast<vl::glr::ParsingAstBase*>(node));
			Traverse(static_cast<Expr*>(node));
			Traverse(static_cast<Expandable*>(node));
			Traverse(static_cast<Unary*>(node));
			InspectInto(node->operand.Obj());
			InspectInto(node->expanded.Obj());
			Finishing(static_cast<Unary*>(node));
			Finishing(static_cast<Expandable*>(node));
			Finishing(static_cast<Expr*>(node));
			Finishing(static_cast<vl::glr::ParsingAstBase*>(node));
		}

		void AstVisitor::Visit(Binary* node)
		{
			if (!node) return;
			Traverse(static_cast<vl::glr::ParsingAstBase*>(node));
			Traverse(static_cast<Expr*>(node));
			Traverse(static_cast<Expandable*>(node));
			Traverse(static_cast<Binary*>(node));
			InspectInto(node->left.Obj());
			InspectInto(node->right.Obj());
			InspectInto(node->expanded.Obj());
			Finishing(static_cast<Binary*>(node));
			Finishing(static_cast<Expandable*>(node));
			Finishing(static_cast<Expr*>(node));
			Finishing(static_cast<vl::glr::ParsingAstBase*>(node));
		}

		void AstVisitor::InspectInto(Expr* node)
		{
			if (!node) return;
			node->Accept(static_cast<Expr::IVisitor*>(this));
		}

		void AstVisitor::InspectInto(Arg* node)
		{
			if (!node) return;
			Traverse(static_cast<vl::glr::ParsingAstBase*>(node));
			Traverse(static_cast<Arg*>(node));
			Traverse(node->name);
			Finishing(static_cast<Arg*>(node));
			Finishing(static_cast<vl::glr::ParsingAstBase*>(node));
		}

		void AstVisitor::InspectInto(Import* node)
		{
			if (!node) return;
			Traverse(static_cast<vl::glr::ParsingAstBase*>(node));
			Traverse(static_cast<Import*>(node));
			Traverse(node->name);
			Finishing(static_cast<Import*>(node));
			Finishing(static_cast<vl::glr::ParsingAstBase*>(node));
		}

		void AstVisitor::InspectInto(Module* node)
		{
			if (!node) return;
			Traverse(static_cast<vl::glr::ParsingAstBase*>(node));
			Traverse(static_cast<Module*>(node));
			InspectInto(node->exported.Obj());
			for (auto&& listItem : node->imports)
			{
				InspectInto(listItem.Obj());
			}
			Finishing(static_cast<Module*>(node));
			Finishing(static_cast<vl::glr::ParsingAstBase*>(node));
		}

	}
}
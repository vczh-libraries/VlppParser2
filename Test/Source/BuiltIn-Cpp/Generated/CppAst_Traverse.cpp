/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:Ast
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "CppAst_Traverse.h"

namespace cpp_parser
{
	namespace traverse_visitor
	{
		void AstVisitor::Traverse(vl::glr::ParsingToken& token) {}
		void AstVisitor::Traverse(vl::glr::ParsingAstBase* node) {}
		void AstVisitor::Traverse(CppConstType* node) {}
		void AstVisitor::Traverse(CppExprOnly* node) {}
		void AstVisitor::Traverse(CppFile* node) {}
		void AstVisitor::Traverse(CppName* node) {}
		void AstVisitor::Traverse(CppNumericExprLiteral* node) {}
		void AstVisitor::Traverse(CppOperatorName* node) {}
		void AstVisitor::Traverse(CppPrimitiveExprLiteral* node) {}
		void AstVisitor::Traverse(CppPrimitiveType* node) {}
		void AstVisitor::Traverse(CppQualifiedName* node) {}
		void AstVisitor::Traverse(CppStringLiteral* node) {}
		void AstVisitor::Traverse(CppStringLiteralFragment* node) {}
		void AstVisitor::Traverse(CppTypeOnly* node) {}
		void AstVisitor::Traverse(CppTypeOrExpr* node) {}
		void AstVisitor::Traverse(CppVolatileType* node) {}

		void AstVisitor::Finishing(vl::glr::ParsingAstBase* node) {}
		void AstVisitor::Finishing(CppConstType* node) {}
		void AstVisitor::Finishing(CppExprOnly* node) {}
		void AstVisitor::Finishing(CppFile* node) {}
		void AstVisitor::Finishing(CppName* node) {}
		void AstVisitor::Finishing(CppNumericExprLiteral* node) {}
		void AstVisitor::Finishing(CppOperatorName* node) {}
		void AstVisitor::Finishing(CppPrimitiveExprLiteral* node) {}
		void AstVisitor::Finishing(CppPrimitiveType* node) {}
		void AstVisitor::Finishing(CppQualifiedName* node) {}
		void AstVisitor::Finishing(CppStringLiteral* node) {}
		void AstVisitor::Finishing(CppStringLiteralFragment* node) {}
		void AstVisitor::Finishing(CppTypeOnly* node) {}
		void AstVisitor::Finishing(CppTypeOrExpr* node) {}
		void AstVisitor::Finishing(CppVolatileType* node) {}

		void AstVisitor::Visit(CppQualifiedName* node)
		{
			node->Accept(static_cast<CppQualifiedName::IVisitor*>(this));
		}

		void AstVisitor::Visit(CppExprOnly* node)
		{
			node->Accept(static_cast<CppExprOnly::IVisitor*>(this));
		}

		void AstVisitor::Visit(CppTypeOnly* node)
		{
			node->Accept(static_cast<CppTypeOnly::IVisitor*>(this));
		}

		void AstVisitor::Visit(CppName* node)
		{
			if (!node) return;
			Traverse(static_cast<vl::glr::ParsingAstBase*>(node));
			Traverse(static_cast<CppTypeOrExpr*>(node));
			Traverse(static_cast<CppQualifiedName*>(node));
			Traverse(static_cast<CppName*>(node));
			Traverse(node->name);
			Finishing(static_cast<CppName*>(node));
			Finishing(static_cast<CppQualifiedName*>(node));
			Finishing(static_cast<CppTypeOrExpr*>(node));
			Finishing(static_cast<vl::glr::ParsingAstBase*>(node));
		}

		void AstVisitor::Visit(CppOperatorName* node)
		{
			if (!node) return;
			Traverse(static_cast<vl::glr::ParsingAstBase*>(node));
			Traverse(static_cast<CppTypeOrExpr*>(node));
			Traverse(static_cast<CppQualifiedName*>(node));
			Traverse(static_cast<CppOperatorName*>(node));
			Finishing(static_cast<CppOperatorName*>(node));
			Finishing(static_cast<CppQualifiedName*>(node));
			Finishing(static_cast<CppTypeOrExpr*>(node));
			Finishing(static_cast<vl::glr::ParsingAstBase*>(node));
		}

		void AstVisitor::Visit(CppPrimitiveExprLiteral* node)
		{
			if (!node) return;
			Traverse(static_cast<vl::glr::ParsingAstBase*>(node));
			Traverse(static_cast<CppTypeOrExpr*>(node));
			Traverse(static_cast<CppExprOnly*>(node));
			Traverse(static_cast<CppPrimitiveExprLiteral*>(node));
			Finishing(static_cast<CppPrimitiveExprLiteral*>(node));
			Finishing(static_cast<CppExprOnly*>(node));
			Finishing(static_cast<CppTypeOrExpr*>(node));
			Finishing(static_cast<vl::glr::ParsingAstBase*>(node));
		}

		void AstVisitor::Visit(CppNumericExprLiteral* node)
		{
			if (!node) return;
			Traverse(static_cast<vl::glr::ParsingAstBase*>(node));
			Traverse(static_cast<CppTypeOrExpr*>(node));
			Traverse(static_cast<CppExprOnly*>(node));
			Traverse(static_cast<CppNumericExprLiteral*>(node));
			Traverse(node->literal);
			Finishing(static_cast<CppNumericExprLiteral*>(node));
			Finishing(static_cast<CppExprOnly*>(node));
			Finishing(static_cast<CppTypeOrExpr*>(node));
			Finishing(static_cast<vl::glr::ParsingAstBase*>(node));
		}

		void AstVisitor::Visit(CppStringLiteral* node)
		{
			if (!node) return;
			Traverse(static_cast<vl::glr::ParsingAstBase*>(node));
			Traverse(static_cast<CppTypeOrExpr*>(node));
			Traverse(static_cast<CppExprOnly*>(node));
			Traverse(static_cast<CppStringLiteral*>(node));
			for (auto&& listItem : node->fragments)
			{
				InspectInto(listItem.Obj());
			}
			Finishing(static_cast<CppStringLiteral*>(node));
			Finishing(static_cast<CppExprOnly*>(node));
			Finishing(static_cast<CppTypeOrExpr*>(node));
			Finishing(static_cast<vl::glr::ParsingAstBase*>(node));
		}

		void AstVisitor::Visit(CppPrimitiveType* node)
		{
			if (!node) return;
			Traverse(static_cast<vl::glr::ParsingAstBase*>(node));
			Traverse(static_cast<CppTypeOrExpr*>(node));
			Traverse(static_cast<CppTypeOnly*>(node));
			Traverse(static_cast<CppPrimitiveType*>(node));
			Traverse(node->literal1);
			Traverse(node->literal2);
			Finishing(static_cast<CppPrimitiveType*>(node));
			Finishing(static_cast<CppTypeOnly*>(node));
			Finishing(static_cast<CppTypeOrExpr*>(node));
			Finishing(static_cast<vl::glr::ParsingAstBase*>(node));
		}

		void AstVisitor::Visit(CppConstType* node)
		{
			if (!node) return;
			Traverse(static_cast<vl::glr::ParsingAstBase*>(node));
			Traverse(static_cast<CppTypeOrExpr*>(node));
			Traverse(static_cast<CppTypeOnly*>(node));
			Traverse(static_cast<CppConstType*>(node));
			InspectInto(node->type.Obj());
			Finishing(static_cast<CppConstType*>(node));
			Finishing(static_cast<CppTypeOnly*>(node));
			Finishing(static_cast<CppTypeOrExpr*>(node));
			Finishing(static_cast<vl::glr::ParsingAstBase*>(node));
		}

		void AstVisitor::Visit(CppVolatileType* node)
		{
			if (!node) return;
			Traverse(static_cast<vl::glr::ParsingAstBase*>(node));
			Traverse(static_cast<CppTypeOrExpr*>(node));
			Traverse(static_cast<CppTypeOnly*>(node));
			Traverse(static_cast<CppVolatileType*>(node));
			InspectInto(node->type.Obj());
			Finishing(static_cast<CppVolatileType*>(node));
			Finishing(static_cast<CppTypeOnly*>(node));
			Finishing(static_cast<CppTypeOrExpr*>(node));
			Finishing(static_cast<vl::glr::ParsingAstBase*>(node));
		}

		void AstVisitor::InspectInto(CppTypeOrExpr* node)
		{
			if (!node) return;
			node->Accept(static_cast<CppTypeOrExpr::IVisitor*>(this));
		}

		void AstVisitor::InspectInto(CppStringLiteralFragment* node)
		{
			if (!node) return;
			Traverse(static_cast<vl::glr::ParsingAstBase*>(node));
			Traverse(static_cast<CppStringLiteralFragment*>(node));
			Traverse(node->literal);
			Finishing(static_cast<CppStringLiteralFragment*>(node));
			Finishing(static_cast<vl::glr::ParsingAstBase*>(node));
		}

		void AstVisitor::InspectInto(CppFile* node)
		{
			if (!node) return;
			Traverse(static_cast<vl::glr::ParsingAstBase*>(node));
			Traverse(static_cast<CppFile*>(node));
			Finishing(static_cast<CppFile*>(node));
			Finishing(static_cast<vl::glr::ParsingAstBase*>(node));
		}

	}
}
/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:Ast
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "CppAst_Empty.h"

namespace cpp_parser
{
	namespace empty_visitor
	{

/***********************************************************************
TypeOrExprVisitor
***********************************************************************/

		// Visitor Members -----------------------------------

		void TypeOrExprVisitor::Visit(CppQualifiedName* node)
		{
			Dispatch(node);
		}

		void TypeOrExprVisitor::Visit(CppExprOnly* node)
		{
			Dispatch(node);
		}

		void TypeOrExprVisitor::Visit(CppTypeOnly* node)
		{
			Dispatch(node);
		}

/***********************************************************************
QualifiedNameVisitor
***********************************************************************/

		// Visitor Members -----------------------------------

		void QualifiedNameVisitor::Visit(CppName* node)
		{
		}

		void QualifiedNameVisitor::Visit(CppOperatorName* node)
		{
		}

/***********************************************************************
ExprOnlyVisitor
***********************************************************************/

		// Visitor Members -----------------------------------

		void ExprOnlyVisitor::Visit(CppPrimitiveExprLiteral* node)
		{
		}

		void ExprOnlyVisitor::Visit(CppNumericExprLiteral* node)
		{
		}

		void ExprOnlyVisitor::Visit(CppStringLiteral* node)
		{
		}

/***********************************************************************
TypeOnlyVisitor
***********************************************************************/

		// Visitor Members -----------------------------------

		void TypeOnlyVisitor::Visit(CppPrimitiveType* node)
		{
		}

		void TypeOnlyVisitor::Visit(CppConstType* node)
		{
		}

		void TypeOnlyVisitor::Visit(CppVolatileType* node)
		{
		}
	}
}
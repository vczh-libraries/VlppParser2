/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:ExprAst
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "BinaryOpExprAst_Builder.h"

namespace binaryop
{
	namespace builder
	{

/***********************************************************************
BinaryExprBuilder
***********************************************************************/

		BinaryExprBuilder& BinaryExprBuilder::left(const vl::Ptr<Expr>& value)
		{
			node->left = value;
			return *this;
		}

		BinaryExprBuilder& BinaryExprBuilder::op(BinaryOp value)
		{
			node->op = value;
			return *this;
		}

		BinaryExprBuilder& BinaryExprBuilder::right(const vl::Ptr<Expr>& value)
		{
			node->right = value;
			return *this;
		}

/***********************************************************************
RefExprBuilder
***********************************************************************/

		RefExprBuilder& RefExprBuilder::name(const vl::WString& value)
		{
			node->name.value = value;
			return *this;
		}
	}
}
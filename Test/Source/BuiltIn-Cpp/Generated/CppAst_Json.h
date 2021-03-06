/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:Ast
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_BUILTIN_CPP_AST_AST_JSON_VISITOR
#define VCZH_PARSER2_BUILTIN_CPP_AST_AST_JSON_VISITOR

#include "CppAst.h"

namespace cpp_parser
{
	namespace json_visitor
	{
		/// <summary>A JSON visitor, overriding all abstract methods with AST to JSON serialization code.</summary>
		class AstVisitor
			: public vl::glr::JsonVisitorBase
			, protected virtual CppTypeOrExpr::IVisitor
			, protected virtual CppExprOnly::IVisitor
			, protected virtual CppTypeOnly::IVisitor
			, protected virtual CppIdentifier::IVisitor
		{
		protected:
			virtual void PrintFields(CppAdvancedType* node);
			virtual void PrintFields(CppBinaryExpr* node);
			virtual void PrintFields(CppBraceExpr* node);
			virtual void PrintFields(CppCallExpr* node);
			virtual void PrintFields(CppCastExpr* node);
			virtual void PrintFields(CppConstType* node);
			virtual void PrintFields(CppDeclarator* node);
			virtual void PrintFields(CppDeclaratorArrayPart* node);
			virtual void PrintFields(CppDeclaratorFunctionPart* node);
			virtual void PrintFields(CppDeclaratorKeyword* node);
			virtual void PrintFields(CppDeclaratorType* node);
			virtual void PrintFields(CppDeleteExpr* node);
			virtual void PrintFields(CppExprOnly* node);
			virtual void PrintFields(CppFile* node);
			virtual void PrintFields(CppFunctionKeyword* node);
			virtual void PrintFields(CppFunctionParameter* node);
			virtual void PrintFields(CppGenericArgument* node);
			virtual void PrintFields(CppGenericArguments* node);
			virtual void PrintFields(CppIdentifier* node);
			virtual void PrintFields(CppIfExpr* node);
			virtual void PrintFields(CppIndexExpr* node);
			virtual void PrintFields(CppNameIdentifier* node);
			virtual void PrintFields(CppNewExpr* node);
			virtual void PrintFields(CppNumericExprLiteral* node);
			virtual void PrintFields(CppOperatorIdentifier* node);
			virtual void PrintFields(CppParenthesisExpr* node);
			virtual void PrintFields(CppPostfixUnaryExpr* node);
			virtual void PrintFields(CppPrefixUnaryExpr* node);
			virtual void PrintFields(CppPrimitiveExprLiteral* node);
			virtual void PrintFields(CppPrimitiveType* node);
			virtual void PrintFields(CppQualifiedName* node);
			virtual void PrintFields(CppSizeofExpr* node);
			virtual void PrintFields(CppStringLiteral* node);
			virtual void PrintFields(CppStringLiteralFragment* node);
			virtual void PrintFields(CppSysFuncExpr* node);
			virtual void PrintFields(CppThrowExpr* node);
			virtual void PrintFields(CppTypeOnly* node);
			virtual void PrintFields(CppTypeOrExpr* node);
			virtual void PrintFields(CppVolatileType* node);

		protected:
			void Visit(CppExprOnly* node) override;
			void Visit(CppTypeOnly* node) override;
			void Visit(CppQualifiedName* node) override;
			void Visit(CppDeclaratorType* node) override;

			void Visit(CppPrimitiveExprLiteral* node) override;
			void Visit(CppNumericExprLiteral* node) override;
			void Visit(CppStringLiteral* node) override;
			void Visit(CppParenthesisExpr* node) override;
			void Visit(CppBraceExpr* node) override;
			void Visit(CppCastExpr* node) override;
			void Visit(CppSysFuncExpr* node) override;
			void Visit(CppSizeofExpr* node) override;
			void Visit(CppDeleteExpr* node) override;
			void Visit(CppNewExpr* node) override;
			void Visit(CppPrefixUnaryExpr* node) override;
			void Visit(CppPostfixUnaryExpr* node) override;
			void Visit(CppIndexExpr* node) override;
			void Visit(CppCallExpr* node) override;
			void Visit(CppBinaryExpr* node) override;
			void Visit(CppIfExpr* node) override;
			void Visit(CppThrowExpr* node) override;

			void Visit(CppPrimitiveType* node) override;
			void Visit(CppConstType* node) override;
			void Visit(CppVolatileType* node) override;

			void Visit(CppNameIdentifier* node) override;
			void Visit(CppOperatorIdentifier* node) override;

		public:
			AstVisitor(vl::stream::StreamWriter& _writer);

			void Print(CppTypeOrExpr* node);
			void Print(CppIdentifier* node);
			void Print(CppGenericArgument* node);
			void Print(CppGenericArguments* node);
			void Print(CppStringLiteralFragment* node);
			void Print(CppAdvancedType* node);
			void Print(CppDeclaratorKeyword* node);
			void Print(CppFunctionKeyword* node);
			void Print(CppFunctionParameter* node);
			void Print(CppDeclaratorFunctionPart* node);
			void Print(CppDeclaratorArrayPart* node);
			void Print(CppDeclarator* node);
			void Print(CppFile* node);
		};
	}
}
#endif
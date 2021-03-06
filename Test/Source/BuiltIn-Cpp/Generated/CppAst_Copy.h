/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:Ast
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_BUILTIN_CPP_AST_AST_COPY_VISITOR
#define VCZH_PARSER2_BUILTIN_CPP_AST_AST_COPY_VISITOR

#include "CppAst.h"

namespace cpp_parser
{
	namespace copy_visitor
	{
		/// <summary>A copy visitor, overriding all abstract methods with AST copying code.</summary>
		class AstVisitor
			: public virtual vl::glr::CopyVisitorBase
			, protected virtual CppTypeOrExpr::IVisitor
			, protected virtual CppExprOnly::IVisitor
			, protected virtual CppTypeOnly::IVisitor
			, protected virtual CppIdentifier::IVisitor
		{
		protected:
			void CopyFields(CppAdvancedType* from, CppAdvancedType* to);
			void CopyFields(CppBinaryExpr* from, CppBinaryExpr* to);
			void CopyFields(CppBraceExpr* from, CppBraceExpr* to);
			void CopyFields(CppCallExpr* from, CppCallExpr* to);
			void CopyFields(CppCastExpr* from, CppCastExpr* to);
			void CopyFields(CppConstType* from, CppConstType* to);
			void CopyFields(CppDeclarator* from, CppDeclarator* to);
			void CopyFields(CppDeclaratorArrayPart* from, CppDeclaratorArrayPart* to);
			void CopyFields(CppDeclaratorFunctionPart* from, CppDeclaratorFunctionPart* to);
			void CopyFields(CppDeclaratorKeyword* from, CppDeclaratorKeyword* to);
			void CopyFields(CppDeclaratorType* from, CppDeclaratorType* to);
			void CopyFields(CppDeleteExpr* from, CppDeleteExpr* to);
			void CopyFields(CppExprOnly* from, CppExprOnly* to);
			void CopyFields(CppFile* from, CppFile* to);
			void CopyFields(CppFunctionKeyword* from, CppFunctionKeyword* to);
			void CopyFields(CppFunctionParameter* from, CppFunctionParameter* to);
			void CopyFields(CppGenericArgument* from, CppGenericArgument* to);
			void CopyFields(CppGenericArguments* from, CppGenericArguments* to);
			void CopyFields(CppIdentifier* from, CppIdentifier* to);
			void CopyFields(CppIfExpr* from, CppIfExpr* to);
			void CopyFields(CppIndexExpr* from, CppIndexExpr* to);
			void CopyFields(CppNameIdentifier* from, CppNameIdentifier* to);
			void CopyFields(CppNewExpr* from, CppNewExpr* to);
			void CopyFields(CppNumericExprLiteral* from, CppNumericExprLiteral* to);
			void CopyFields(CppOperatorIdentifier* from, CppOperatorIdentifier* to);
			void CopyFields(CppParenthesisExpr* from, CppParenthesisExpr* to);
			void CopyFields(CppPostfixUnaryExpr* from, CppPostfixUnaryExpr* to);
			void CopyFields(CppPrefixUnaryExpr* from, CppPrefixUnaryExpr* to);
			void CopyFields(CppPrimitiveExprLiteral* from, CppPrimitiveExprLiteral* to);
			void CopyFields(CppPrimitiveType* from, CppPrimitiveType* to);
			void CopyFields(CppQualifiedName* from, CppQualifiedName* to);
			void CopyFields(CppSizeofExpr* from, CppSizeofExpr* to);
			void CopyFields(CppStringLiteral* from, CppStringLiteral* to);
			void CopyFields(CppStringLiteralFragment* from, CppStringLiteralFragment* to);
			void CopyFields(CppSysFuncExpr* from, CppSysFuncExpr* to);
			void CopyFields(CppThrowExpr* from, CppThrowExpr* to);
			void CopyFields(CppTypeOnly* from, CppTypeOnly* to);
			void CopyFields(CppTypeOrExpr* from, CppTypeOrExpr* to);
			void CopyFields(CppVolatileType* from, CppVolatileType* to);

		protected:
			virtual void Visit(CppGenericArgument* node);
			virtual void Visit(CppGenericArguments* node);
			virtual void Visit(CppStringLiteralFragment* node);
			virtual void Visit(CppAdvancedType* node);
			virtual void Visit(CppDeclaratorKeyword* node);
			virtual void Visit(CppFunctionKeyword* node);
			virtual void Visit(CppFunctionParameter* node);
			virtual void Visit(CppDeclaratorFunctionPart* node);
			virtual void Visit(CppDeclaratorArrayPart* node);
			virtual void Visit(CppDeclarator* node);
			virtual void Visit(CppFile* node);

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
			virtual vl::Ptr<CppTypeOrExpr> CopyNode(CppTypeOrExpr* node);
			virtual vl::Ptr<CppIdentifier> CopyNode(CppIdentifier* node);
			virtual vl::Ptr<CppGenericArgument> CopyNode(CppGenericArgument* node);
			virtual vl::Ptr<CppGenericArguments> CopyNode(CppGenericArguments* node);
			virtual vl::Ptr<CppStringLiteralFragment> CopyNode(CppStringLiteralFragment* node);
			virtual vl::Ptr<CppAdvancedType> CopyNode(CppAdvancedType* node);
			virtual vl::Ptr<CppDeclaratorKeyword> CopyNode(CppDeclaratorKeyword* node);
			virtual vl::Ptr<CppFunctionKeyword> CopyNode(CppFunctionKeyword* node);
			virtual vl::Ptr<CppFunctionParameter> CopyNode(CppFunctionParameter* node);
			virtual vl::Ptr<CppDeclaratorFunctionPart> CopyNode(CppDeclaratorFunctionPart* node);
			virtual vl::Ptr<CppDeclaratorArrayPart> CopyNode(CppDeclaratorArrayPart* node);
			virtual vl::Ptr<CppDeclarator> CopyNode(CppDeclarator* node);
			virtual vl::Ptr<CppFile> CopyNode(CppFile* node);

			vl::Ptr<CppBinaryExpr> CopyNode(CppBinaryExpr* node);
			vl::Ptr<CppBraceExpr> CopyNode(CppBraceExpr* node);
			vl::Ptr<CppCallExpr> CopyNode(CppCallExpr* node);
			vl::Ptr<CppCastExpr> CopyNode(CppCastExpr* node);
			vl::Ptr<CppConstType> CopyNode(CppConstType* node);
			vl::Ptr<CppDeclaratorType> CopyNode(CppDeclaratorType* node);
			vl::Ptr<CppDeleteExpr> CopyNode(CppDeleteExpr* node);
			vl::Ptr<CppExprOnly> CopyNode(CppExprOnly* node);
			vl::Ptr<CppIfExpr> CopyNode(CppIfExpr* node);
			vl::Ptr<CppIndexExpr> CopyNode(CppIndexExpr* node);
			vl::Ptr<CppNameIdentifier> CopyNode(CppNameIdentifier* node);
			vl::Ptr<CppNewExpr> CopyNode(CppNewExpr* node);
			vl::Ptr<CppNumericExprLiteral> CopyNode(CppNumericExprLiteral* node);
			vl::Ptr<CppOperatorIdentifier> CopyNode(CppOperatorIdentifier* node);
			vl::Ptr<CppParenthesisExpr> CopyNode(CppParenthesisExpr* node);
			vl::Ptr<CppPostfixUnaryExpr> CopyNode(CppPostfixUnaryExpr* node);
			vl::Ptr<CppPrefixUnaryExpr> CopyNode(CppPrefixUnaryExpr* node);
			vl::Ptr<CppPrimitiveExprLiteral> CopyNode(CppPrimitiveExprLiteral* node);
			vl::Ptr<CppPrimitiveType> CopyNode(CppPrimitiveType* node);
			vl::Ptr<CppQualifiedName> CopyNode(CppQualifiedName* node);
			vl::Ptr<CppSizeofExpr> CopyNode(CppSizeofExpr* node);
			vl::Ptr<CppStringLiteral> CopyNode(CppStringLiteral* node);
			vl::Ptr<CppSysFuncExpr> CopyNode(CppSysFuncExpr* node);
			vl::Ptr<CppThrowExpr> CopyNode(CppThrowExpr* node);
			vl::Ptr<CppTypeOnly> CopyNode(CppTypeOnly* node);
			vl::Ptr<CppVolatileType> CopyNode(CppVolatileType* node);
		};
	}
}
#endif
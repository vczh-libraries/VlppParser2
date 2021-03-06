/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:Calculator
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_UNITTEST_CALCULATOR_AST_ASSEMBLER
#define VCZH_PARSER2_UNITTEST_CALCULATOR_AST_ASSEMBLER

#include "CalculatorExprAst.h"

namespace calculator
{
	enum class CalculatorClasses : vl::vint32_t
	{
		Arg = 0,
		Binary = 1,
		Call = 2,
		Expandable = 3,
		Expr = 4,
		False = 5,
		Func = 6,
		Import = 7,
		LetExpr = 8,
		Module = 9,
		NumExpr = 10,
		Ref = 11,
		True = 12,
		Unary = 13,
	};

	enum class CalculatorFields : vl::vint32_t
	{
		Arg_name = 0,
		Binary_left = 1,
		Binary_op = 2,
		Binary_right = 3,
		Call_args = 4,
		Call_func = 5,
		Expandable_expanded = 6,
		Func_args = 7,
		Func_value = 8,
		Import_name = 9,
		LetExpr_name = 10,
		LetExpr_result = 11,
		LetExpr_value = 12,
		Module_exported = 13,
		Module_imports = 14,
		NumExpr_value = 15,
		Ref_name = 16,
		Unary_op = 17,
		Unary_operand = 18,
	};

	extern const wchar_t* CalculatorTypeName(CalculatorClasses type);
	extern const wchar_t* CalculatorCppTypeName(CalculatorClasses type);
	extern const wchar_t* CalculatorFieldName(CalculatorFields field);
	extern const wchar_t* CalculatorCppFieldName(CalculatorFields field);

	class CalculatorAstInsReceiver : public vl::glr::AstInsReceiverBase
	{
	protected:
		vl::Ptr<vl::glr::ParsingAstBase> CreateAstNode(vl::vint32_t type) override;
		void SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, vl::Ptr<vl::glr::ParsingAstBase> value) override;
		void SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, const vl::regex::RegexToken& token, vl::vint32_t tokenIndex) override;
		void SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, vl::vint32_t enumItem, bool weakAssignment) override;
		vl::Ptr<vl::glr::ParsingAstBase> ResolveAmbiguity(vl::vint32_t type, vl::collections::Array<vl::Ptr<vl::glr::ParsingAstBase>>& candidates) override;
	};
}
#endif
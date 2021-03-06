/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:PrefixSubset2
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_UNITTEST_PREFIXSUBSET2_AST_ASSEMBLER
#define VCZH_PARSER2_UNITTEST_PREFIXSUBSET2_AST_ASSEMBLER

#include "PrefixSubset2TypeOrExpr.h"

namespace prefixsubset2
{
	enum class PrefixSubset2Classes : vl::vint32_t
	{
		CallExpr = 0,
		ConstType = 1,
		FunctionType = 2,
		MemberName = 3,
		MulExpr = 4,
		Name = 5,
		PointerType = 6,
		QualifiedName = 7,
		TypeOrExpr = 8,
		TypeOrExprToResolve = 9,
	};

	enum class PrefixSubset2Fields : vl::vint32_t
	{
		CallExpr_args = 0,
		CallExpr_func = 1,
		ConstType_type = 2,
		FunctionType_args = 3,
		FunctionType_returnType = 4,
		MemberName_member = 5,
		MemberName_parent = 6,
		MulExpr_first = 7,
		MulExpr_second = 8,
		Name_name = 9,
		PointerType_type = 10,
		TypeOrExprToResolve_candidates = 11,
	};

	extern const wchar_t* PrefixSubset2TypeName(PrefixSubset2Classes type);
	extern const wchar_t* PrefixSubset2CppTypeName(PrefixSubset2Classes type);
	extern const wchar_t* PrefixSubset2FieldName(PrefixSubset2Fields field);
	extern const wchar_t* PrefixSubset2CppFieldName(PrefixSubset2Fields field);

	class PrefixSubset2AstInsReceiver : public vl::glr::AstInsReceiverBase
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
/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:Cpp
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "Cpp_Assembler.h"

namespace cpp_parser
{

/***********************************************************************
CppAstInsReceiver : public vl::glr::AstInsReceiverBase
***********************************************************************/

	vl::Ptr<vl::glr::ParsingAstBase> CppAstInsReceiver::CreateAstNode(vl::vint32_t type)
	{
		auto cppTypeName = CppCppTypeName((CppClasses)type);
		switch((CppClasses)type)
		{
		case CppClasses::AdvancedType:
			return new cpp_parser::CppAdvancedType();
		case CppClasses::BinaryExpr:
			return new cpp_parser::CppBinaryExpr();
		case CppClasses::BraceExpr:
			return new cpp_parser::CppBraceExpr();
		case CppClasses::CallExpr:
			return new cpp_parser::CppCallExpr();
		case CppClasses::CastExpr:
			return new cpp_parser::CppCastExpr();
		case CppClasses::ConstType:
			return new cpp_parser::CppConstType();
		case CppClasses::Declarator:
			return new cpp_parser::CppDeclarator();
		case CppClasses::DeclaratorArrayPart:
			return new cpp_parser::CppDeclaratorArrayPart();
		case CppClasses::DeclaratorFunctionPart:
			return new cpp_parser::CppDeclaratorFunctionPart();
		case CppClasses::DeclaratorKeyword:
			return new cpp_parser::CppDeclaratorKeyword();
		case CppClasses::DeclaratorType:
			return new cpp_parser::CppDeclaratorType();
		case CppClasses::DeleteExpr:
			return new cpp_parser::CppDeleteExpr();
		case CppClasses::File:
			return new cpp_parser::CppFile();
		case CppClasses::FunctionKeyword:
			return new cpp_parser::CppFunctionKeyword();
		case CppClasses::FunctionParameter:
			return new cpp_parser::CppFunctionParameter();
		case CppClasses::GenericArgument:
			return new cpp_parser::CppGenericArgument();
		case CppClasses::GenericArguments:
			return new cpp_parser::CppGenericArguments();
		case CppClasses::IfExpr:
			return new cpp_parser::CppIfExpr();
		case CppClasses::IndexExpr:
			return new cpp_parser::CppIndexExpr();
		case CppClasses::NameIdentifier:
			return new cpp_parser::CppNameIdentifier();
		case CppClasses::NewExpr:
			return new cpp_parser::CppNewExpr();
		case CppClasses::NumericExprLiteral:
			return new cpp_parser::CppNumericExprLiteral();
		case CppClasses::OperatorIdentifier:
			return new cpp_parser::CppOperatorIdentifier();
		case CppClasses::ParenthesisExpr:
			return new cpp_parser::CppParenthesisExpr();
		case CppClasses::PostfixUnaryExpr:
			return new cpp_parser::CppPostfixUnaryExpr();
		case CppClasses::PrefixUnaryExpr:
			return new cpp_parser::CppPrefixUnaryExpr();
		case CppClasses::PrimitiveExprLiteral:
			return new cpp_parser::CppPrimitiveExprLiteral();
		case CppClasses::PrimitiveType:
			return new cpp_parser::CppPrimitiveType();
		case CppClasses::QualifiedName:
			return new cpp_parser::CppQualifiedName();
		case CppClasses::SizeofExpr:
			return new cpp_parser::CppSizeofExpr();
		case CppClasses::StringLiteral:
			return new cpp_parser::CppStringLiteral();
		case CppClasses::StringLiteralFragment:
			return new cpp_parser::CppStringLiteralFragment();
		case CppClasses::SysFuncExpr:
			return new cpp_parser::CppSysFuncExpr();
		case CppClasses::ThrowExpr:
			return new cpp_parser::CppThrowExpr();
		case CppClasses::VolatileType:
			return new cpp_parser::CppVolatileType();
		default:
			return vl::glr::AssemblyThrowCannotCreateAbstractType(type, cppTypeName);
		}
	}

	void CppAstInsReceiver::SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, vl::Ptr<vl::glr::ParsingAstBase> value)
	{
		auto cppFieldName = CppCppFieldName((CppFields)field);
		switch((CppFields)field)
		{
		case CppFields::AdvancedType_argument:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppAdvancedType::argument, object, field, value, cppFieldName);
		case CppFields::BinaryExpr_left:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppBinaryExpr::left, object, field, value, cppFieldName);
		case CppFields::BinaryExpr_right:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppBinaryExpr::right, object, field, value, cppFieldName);
		case CppFields::BraceExpr_arguments:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppBraceExpr::arguments, object, field, value, cppFieldName);
		case CppFields::CallExpr_arguments:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppCallExpr::arguments, object, field, value, cppFieldName);
		case CppFields::CallExpr_operand:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppCallExpr::operand, object, field, value, cppFieldName);
		case CppFields::CastExpr_expr:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppCastExpr::expr, object, field, value, cppFieldName);
		case CppFields::CastExpr_type:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppCastExpr::type, object, field, value, cppFieldName);
		case CppFields::ConstType_type:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppConstType::type, object, field, value, cppFieldName);
		case CppFields::Declarator_advancedTypes:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppDeclarator::advancedTypes, object, field, value, cppFieldName);
		case CppFields::Declarator_arrayParts:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppDeclarator::arrayParts, object, field, value, cppFieldName);
		case CppFields::Declarator_funcPart:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppDeclarator::funcPart, object, field, value, cppFieldName);
		case CppFields::Declarator_id:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppDeclarator::id, object, field, value, cppFieldName);
		case CppFields::Declarator_innerDeclarator:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppDeclarator::innerDeclarator, object, field, value, cppFieldName);
		case CppFields::Declarator_keywords:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppDeclarator::keywords, object, field, value, cppFieldName);
		case CppFields::DeclaratorArrayPart_argument:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppDeclaratorArrayPart::argument, object, field, value, cppFieldName);
		case CppFields::DeclaratorFunctionPart_deferredType:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppDeclaratorFunctionPart::deferredType, object, field, value, cppFieldName);
		case CppFields::DeclaratorFunctionPart_keywords:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppDeclaratorFunctionPart::keywords, object, field, value, cppFieldName);
		case CppFields::DeclaratorFunctionPart_parameters:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppDeclaratorFunctionPart::parameters, object, field, value, cppFieldName);
		case CppFields::DeclaratorType_declarator:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppDeclaratorType::declarator, object, field, value, cppFieldName);
		case CppFields::DeclaratorType_keywords:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppDeclaratorType::keywords, object, field, value, cppFieldName);
		case CppFields::DeclaratorType_type:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppDeclaratorType::type, object, field, value, cppFieldName);
		case CppFields::DeleteExpr_argument:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppDeleteExpr::argument, object, field, value, cppFieldName);
		case CppFields::FunctionKeyword_arguments:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppFunctionKeyword::arguments, object, field, value, cppFieldName);
		case CppFields::FunctionParameter_declarator:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppFunctionParameter::declarator, object, field, value, cppFieldName);
		case CppFields::FunctionParameter_defaultValue:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppFunctionParameter::defaultValue, object, field, value, cppFieldName);
		case CppFields::FunctionParameter_keywords:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppFunctionParameter::keywords, object, field, value, cppFieldName);
		case CppFields::FunctionParameter_type:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppFunctionParameter::type, object, field, value, cppFieldName);
		case CppFields::GenericArgument_argument:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppGenericArgument::argument, object, field, value, cppFieldName);
		case CppFields::GenericArguments_arguments:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppGenericArguments::arguments, object, field, value, cppFieldName);
		case CppFields::IfExpr_condition:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppIfExpr::condition, object, field, value, cppFieldName);
		case CppFields::IfExpr_falseBranch:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppIfExpr::falseBranch, object, field, value, cppFieldName);
		case CppFields::IfExpr_trueBranch:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppIfExpr::trueBranch, object, field, value, cppFieldName);
		case CppFields::IndexExpr_index:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppIndexExpr::index, object, field, value, cppFieldName);
		case CppFields::IndexExpr_operand:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppIndexExpr::operand, object, field, value, cppFieldName);
		case CppFields::NewExpr_initArguments:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppNewExpr::initArguments, object, field, value, cppFieldName);
		case CppFields::NewExpr_placementArguments:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppNewExpr::placementArguments, object, field, value, cppFieldName);
		case CppFields::NewExpr_type:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppNewExpr::type, object, field, value, cppFieldName);
		case CppFields::ParenthesisExpr_expr:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppParenthesisExpr::expr, object, field, value, cppFieldName);
		case CppFields::PostfixUnaryExpr_operand:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppPostfixUnaryExpr::operand, object, field, value, cppFieldName);
		case CppFields::PrefixUnaryExpr_operand:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppPrefixUnaryExpr::operand, object, field, value, cppFieldName);
		case CppFields::QualifiedName_arguments:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppQualifiedName::arguments, object, field, value, cppFieldName);
		case CppFields::QualifiedName_expr:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppQualifiedName::expr, object, field, value, cppFieldName);
		case CppFields::QualifiedName_id:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppQualifiedName::id, object, field, value, cppFieldName);
		case CppFields::QualifiedName_parent:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppQualifiedName::parent, object, field, value, cppFieldName);
		case CppFields::SizeofExpr_argument:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppSizeofExpr::argument, object, field, value, cppFieldName);
		case CppFields::StringLiteral_fragments:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppStringLiteral::fragments, object, field, value, cppFieldName);
		case CppFields::SysFuncExpr_argument:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppSysFuncExpr::argument, object, field, value, cppFieldName);
		case CppFields::ThrowExpr_argument:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppThrowExpr::argument, object, field, value, cppFieldName);
		case CppFields::VolatileType_type:
			return vl::glr::AssemblerSetObjectField(&cpp_parser::CppVolatileType::type, object, field, value, cppFieldName);
		default:
			return vl::glr::AssemblyThrowFieldNotObject(field, cppFieldName);
		}
	}

	void CppAstInsReceiver::SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, const vl::regex::RegexToken& token, vl::vint32_t tokenIndex)
	{
		auto cppFieldName = CppCppFieldName((CppFields)field);
		switch((CppFields)field)
		{
		case CppFields::CastExpr_keyword:
			return vl::glr::AssemblerSetTokenField(&cpp_parser::CppCastExpr::keyword, object, field, token, tokenIndex, cppFieldName);
		case CppFields::Declarator_variadic:
			return vl::glr::AssemblerSetTokenField(&cpp_parser::CppDeclarator::variadic, object, field, token, tokenIndex, cppFieldName);
		case CppFields::DeclaratorFunctionPart_variadic:
			return vl::glr::AssemblerSetTokenField(&cpp_parser::CppDeclaratorFunctionPart::variadic, object, field, token, tokenIndex, cppFieldName);
		case CppFields::DeclaratorKeyword_keyword:
			return vl::glr::AssemblerSetTokenField(&cpp_parser::CppDeclaratorKeyword::keyword, object, field, token, tokenIndex, cppFieldName);
		case CppFields::FunctionKeyword_keyword:
			return vl::glr::AssemblerSetTokenField(&cpp_parser::CppFunctionKeyword::keyword, object, field, token, tokenIndex, cppFieldName);
		case CppFields::GenericArgument_variadic:
			return vl::glr::AssemblerSetTokenField(&cpp_parser::CppGenericArgument::variadic, object, field, token, tokenIndex, cppFieldName);
		case CppFields::NameIdentifier_name:
			return vl::glr::AssemblerSetTokenField(&cpp_parser::CppNameIdentifier::name, object, field, token, tokenIndex, cppFieldName);
		case CppFields::NumericExprLiteral_literal:
			return vl::glr::AssemblerSetTokenField(&cpp_parser::CppNumericExprLiteral::literal, object, field, token, tokenIndex, cppFieldName);
		case CppFields::PrimitiveType_literal1:
			return vl::glr::AssemblerSetTokenField(&cpp_parser::CppPrimitiveType::literal1, object, field, token, tokenIndex, cppFieldName);
		case CppFields::PrimitiveType_literal2:
			return vl::glr::AssemblerSetTokenField(&cpp_parser::CppPrimitiveType::literal2, object, field, token, tokenIndex, cppFieldName);
		case CppFields::SizeofExpr_variadic:
			return vl::glr::AssemblerSetTokenField(&cpp_parser::CppSizeofExpr::variadic, object, field, token, tokenIndex, cppFieldName);
		case CppFields::StringLiteralFragment_literal:
			return vl::glr::AssemblerSetTokenField(&cpp_parser::CppStringLiteralFragment::literal, object, field, token, tokenIndex, cppFieldName);
		case CppFields::SysFuncExpr_keyword:
			return vl::glr::AssemblerSetTokenField(&cpp_parser::CppSysFuncExpr::keyword, object, field, token, tokenIndex, cppFieldName);
		case CppFields::SysFuncExpr_variadic:
			return vl::glr::AssemblerSetTokenField(&cpp_parser::CppSysFuncExpr::variadic, object, field, token, tokenIndex, cppFieldName);
		default:
			return vl::glr::AssemblyThrowFieldNotToken(field, cppFieldName);
		}
	}

	void CppAstInsReceiver::SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, vl::vint32_t enumItem, bool weakAssignment)
	{
		auto cppFieldName = CppCppFieldName((CppFields)field);
		switch((CppFields)field)
		{
		case CppFields::AdvancedType_kind:
			return vl::glr::AssemblerSetEnumField(&cpp_parser::CppAdvancedType::kind, object, field, enumItem, weakAssignment, cppFieldName);
		case CppFields::BinaryExpr_op:
			return vl::glr::AssemblerSetEnumField(&cpp_parser::CppBinaryExpr::op, object, field, enumItem, weakAssignment, cppFieldName);
		case CppFields::CallExpr_kind:
			return vl::glr::AssemblerSetEnumField(&cpp_parser::CppCallExpr::kind, object, field, enumItem, weakAssignment, cppFieldName);
		case CppFields::DeleteExpr_array:
			return vl::glr::AssemblerSetEnumField(&cpp_parser::CppDeleteExpr::array, object, field, enumItem, weakAssignment, cppFieldName);
		case CppFields::DeleteExpr_scope:
			return vl::glr::AssemblerSetEnumField(&cpp_parser::CppDeleteExpr::scope, object, field, enumItem, weakAssignment, cppFieldName);
		case CppFields::NameIdentifier_kind:
			return vl::glr::AssemblerSetEnumField(&cpp_parser::CppNameIdentifier::kind, object, field, enumItem, weakAssignment, cppFieldName);
		case CppFields::NewExpr_init:
			return vl::glr::AssemblerSetEnumField(&cpp_parser::CppNewExpr::init, object, field, enumItem, weakAssignment, cppFieldName);
		case CppFields::NewExpr_scope:
			return vl::glr::AssemblerSetEnumField(&cpp_parser::CppNewExpr::scope, object, field, enumItem, weakAssignment, cppFieldName);
		case CppFields::NumericExprLiteral_kind:
			return vl::glr::AssemblerSetEnumField(&cpp_parser::CppNumericExprLiteral::kind, object, field, enumItem, weakAssignment, cppFieldName);
		case CppFields::OperatorIdentifier_op:
			return vl::glr::AssemblerSetEnumField(&cpp_parser::CppOperatorIdentifier::op, object, field, enumItem, weakAssignment, cppFieldName);
		case CppFields::PostfixUnaryExpr_op:
			return vl::glr::AssemblerSetEnumField(&cpp_parser::CppPostfixUnaryExpr::op, object, field, enumItem, weakAssignment, cppFieldName);
		case CppFields::PrefixUnaryExpr_op:
			return vl::glr::AssemblerSetEnumField(&cpp_parser::CppPrefixUnaryExpr::op, object, field, enumItem, weakAssignment, cppFieldName);
		case CppFields::PrimitiveExprLiteral_kind:
			return vl::glr::AssemblerSetEnumField(&cpp_parser::CppPrimitiveExprLiteral::kind, object, field, enumItem, weakAssignment, cppFieldName);
		case CppFields::PrimitiveType_kind:
			return vl::glr::AssemblerSetEnumField(&cpp_parser::CppPrimitiveType::kind, object, field, enumItem, weakAssignment, cppFieldName);
		case CppFields::QualifiedName_kind:
			return vl::glr::AssemblerSetEnumField(&cpp_parser::CppQualifiedName::kind, object, field, enumItem, weakAssignment, cppFieldName);
		case CppFields::StringLiteralFragment_kind:
			return vl::glr::AssemblerSetEnumField(&cpp_parser::CppStringLiteralFragment::kind, object, field, enumItem, weakAssignment, cppFieldName);
		default:
			return vl::glr::AssemblyThrowFieldNotEnum(field, cppFieldName);
		}
	}

	const wchar_t* CppTypeName(CppClasses type)
	{
		const wchar_t* results[] = {
			L"AdvancedType",
			L"BinaryExpr",
			L"BraceExpr",
			L"CallExpr",
			L"CastExpr",
			L"ConstType",
			L"Declarator",
			L"DeclaratorArrayPart",
			L"DeclaratorFunctionPart",
			L"DeclaratorKeyword",
			L"DeclaratorType",
			L"DeleteExpr",
			L"ExprOnly",
			L"File",
			L"FunctionKeyword",
			L"FunctionParameter",
			L"GenericArgument",
			L"GenericArguments",
			L"Identifier",
			L"IfExpr",
			L"IndexExpr",
			L"NameIdentifier",
			L"NewExpr",
			L"NumericExprLiteral",
			L"OperatorIdentifier",
			L"ParenthesisExpr",
			L"PostfixUnaryExpr",
			L"PrefixUnaryExpr",
			L"PrimitiveExprLiteral",
			L"PrimitiveType",
			L"QualifiedName",
			L"SizeofExpr",
			L"StringLiteral",
			L"StringLiteralFragment",
			L"SysFuncExpr",
			L"ThrowExpr",
			L"TypeOnly",
			L"TypeOrExpr",
			L"VolatileType",
		};
		vl::vint index = (vl::vint)type;
		return 0 <= index && index < 39 ? results[index] : nullptr;
	}

	const wchar_t* CppCppTypeName(CppClasses type)
	{
		const wchar_t* results[] = {
			L"cpp_parser::CppAdvancedType",
			L"cpp_parser::CppBinaryExpr",
			L"cpp_parser::CppBraceExpr",
			L"cpp_parser::CppCallExpr",
			L"cpp_parser::CppCastExpr",
			L"cpp_parser::CppConstType",
			L"cpp_parser::CppDeclarator",
			L"cpp_parser::CppDeclaratorArrayPart",
			L"cpp_parser::CppDeclaratorFunctionPart",
			L"cpp_parser::CppDeclaratorKeyword",
			L"cpp_parser::CppDeclaratorType",
			L"cpp_parser::CppDeleteExpr",
			L"cpp_parser::CppExprOnly",
			L"cpp_parser::CppFile",
			L"cpp_parser::CppFunctionKeyword",
			L"cpp_parser::CppFunctionParameter",
			L"cpp_parser::CppGenericArgument",
			L"cpp_parser::CppGenericArguments",
			L"cpp_parser::CppIdentifier",
			L"cpp_parser::CppIfExpr",
			L"cpp_parser::CppIndexExpr",
			L"cpp_parser::CppNameIdentifier",
			L"cpp_parser::CppNewExpr",
			L"cpp_parser::CppNumericExprLiteral",
			L"cpp_parser::CppOperatorIdentifier",
			L"cpp_parser::CppParenthesisExpr",
			L"cpp_parser::CppPostfixUnaryExpr",
			L"cpp_parser::CppPrefixUnaryExpr",
			L"cpp_parser::CppPrimitiveExprLiteral",
			L"cpp_parser::CppPrimitiveType",
			L"cpp_parser::CppQualifiedName",
			L"cpp_parser::CppSizeofExpr",
			L"cpp_parser::CppStringLiteral",
			L"cpp_parser::CppStringLiteralFragment",
			L"cpp_parser::CppSysFuncExpr",
			L"cpp_parser::CppThrowExpr",
			L"cpp_parser::CppTypeOnly",
			L"cpp_parser::CppTypeOrExpr",
			L"cpp_parser::CppVolatileType",
		};
		vl::vint index = (vl::vint)type;
		return 0 <= index && index < 39 ? results[index] : nullptr;
	}

	const wchar_t* CppFieldName(CppFields field)
	{
		const wchar_t* results[] = {
			L"AdvancedType::argument",
			L"AdvancedType::kind",
			L"BinaryExpr::left",
			L"BinaryExpr::op",
			L"BinaryExpr::right",
			L"BraceExpr::arguments",
			L"CallExpr::arguments",
			L"CallExpr::kind",
			L"CallExpr::operand",
			L"CastExpr::expr",
			L"CastExpr::keyword",
			L"CastExpr::type",
			L"ConstType::type",
			L"Declarator::advancedTypes",
			L"Declarator::arrayParts",
			L"Declarator::funcPart",
			L"Declarator::id",
			L"Declarator::innerDeclarator",
			L"Declarator::keywords",
			L"Declarator::variadic",
			L"DeclaratorArrayPart::argument",
			L"DeclaratorFunctionPart::deferredType",
			L"DeclaratorFunctionPart::keywords",
			L"DeclaratorFunctionPart::parameters",
			L"DeclaratorFunctionPart::variadic",
			L"DeclaratorKeyword::keyword",
			L"DeclaratorType::declarator",
			L"DeclaratorType::keywords",
			L"DeclaratorType::type",
			L"DeleteExpr::argument",
			L"DeleteExpr::array",
			L"DeleteExpr::scope",
			L"FunctionKeyword::arguments",
			L"FunctionKeyword::keyword",
			L"FunctionParameter::declarator",
			L"FunctionParameter::defaultValue",
			L"FunctionParameter::keywords",
			L"FunctionParameter::type",
			L"GenericArgument::argument",
			L"GenericArgument::variadic",
			L"GenericArguments::arguments",
			L"IfExpr::condition",
			L"IfExpr::falseBranch",
			L"IfExpr::trueBranch",
			L"IndexExpr::index",
			L"IndexExpr::operand",
			L"NameIdentifier::kind",
			L"NameIdentifier::name",
			L"NewExpr::init",
			L"NewExpr::initArguments",
			L"NewExpr::placementArguments",
			L"NewExpr::scope",
			L"NewExpr::type",
			L"NumericExprLiteral::kind",
			L"NumericExprLiteral::literal",
			L"OperatorIdentifier::op",
			L"ParenthesisExpr::expr",
			L"PostfixUnaryExpr::op",
			L"PostfixUnaryExpr::operand",
			L"PrefixUnaryExpr::op",
			L"PrefixUnaryExpr::operand",
			L"PrimitiveExprLiteral::kind",
			L"PrimitiveType::kind",
			L"PrimitiveType::literal1",
			L"PrimitiveType::literal2",
			L"QualifiedName::arguments",
			L"QualifiedName::expr",
			L"QualifiedName::id",
			L"QualifiedName::kind",
			L"QualifiedName::parent",
			L"SizeofExpr::argument",
			L"SizeofExpr::variadic",
			L"StringLiteral::fragments",
			L"StringLiteralFragment::kind",
			L"StringLiteralFragment::literal",
			L"SysFuncExpr::argument",
			L"SysFuncExpr::keyword",
			L"SysFuncExpr::variadic",
			L"ThrowExpr::argument",
			L"VolatileType::type",
		};
		vl::vint index = (vl::vint)field;
		return 0 <= index && index < 80 ? results[index] : nullptr;
	}

	const wchar_t* CppCppFieldName(CppFields field)
	{
		const wchar_t* results[] = {
			L"cpp_parser::CppAdvancedType::argument",
			L"cpp_parser::CppAdvancedType::kind",
			L"cpp_parser::CppBinaryExpr::left",
			L"cpp_parser::CppBinaryExpr::op",
			L"cpp_parser::CppBinaryExpr::right",
			L"cpp_parser::CppBraceExpr::arguments",
			L"cpp_parser::CppCallExpr::arguments",
			L"cpp_parser::CppCallExpr::kind",
			L"cpp_parser::CppCallExpr::operand",
			L"cpp_parser::CppCastExpr::expr",
			L"cpp_parser::CppCastExpr::keyword",
			L"cpp_parser::CppCastExpr::type",
			L"cpp_parser::CppConstType::type",
			L"cpp_parser::CppDeclarator::advancedTypes",
			L"cpp_parser::CppDeclarator::arrayParts",
			L"cpp_parser::CppDeclarator::funcPart",
			L"cpp_parser::CppDeclarator::id",
			L"cpp_parser::CppDeclarator::innerDeclarator",
			L"cpp_parser::CppDeclarator::keywords",
			L"cpp_parser::CppDeclarator::variadic",
			L"cpp_parser::CppDeclaratorArrayPart::argument",
			L"cpp_parser::CppDeclaratorFunctionPart::deferredType",
			L"cpp_parser::CppDeclaratorFunctionPart::keywords",
			L"cpp_parser::CppDeclaratorFunctionPart::parameters",
			L"cpp_parser::CppDeclaratorFunctionPart::variadic",
			L"cpp_parser::CppDeclaratorKeyword::keyword",
			L"cpp_parser::CppDeclaratorType::declarator",
			L"cpp_parser::CppDeclaratorType::keywords",
			L"cpp_parser::CppDeclaratorType::type",
			L"cpp_parser::CppDeleteExpr::argument",
			L"cpp_parser::CppDeleteExpr::array",
			L"cpp_parser::CppDeleteExpr::scope",
			L"cpp_parser::CppFunctionKeyword::arguments",
			L"cpp_parser::CppFunctionKeyword::keyword",
			L"cpp_parser::CppFunctionParameter::declarator",
			L"cpp_parser::CppFunctionParameter::defaultValue",
			L"cpp_parser::CppFunctionParameter::keywords",
			L"cpp_parser::CppFunctionParameter::type",
			L"cpp_parser::CppGenericArgument::argument",
			L"cpp_parser::CppGenericArgument::variadic",
			L"cpp_parser::CppGenericArguments::arguments",
			L"cpp_parser::CppIfExpr::condition",
			L"cpp_parser::CppIfExpr::falseBranch",
			L"cpp_parser::CppIfExpr::trueBranch",
			L"cpp_parser::CppIndexExpr::index",
			L"cpp_parser::CppIndexExpr::operand",
			L"cpp_parser::CppNameIdentifier::kind",
			L"cpp_parser::CppNameIdentifier::name",
			L"cpp_parser::CppNewExpr::init",
			L"cpp_parser::CppNewExpr::initArguments",
			L"cpp_parser::CppNewExpr::placementArguments",
			L"cpp_parser::CppNewExpr::scope",
			L"cpp_parser::CppNewExpr::type",
			L"cpp_parser::CppNumericExprLiteral::kind",
			L"cpp_parser::CppNumericExprLiteral::literal",
			L"cpp_parser::CppOperatorIdentifier::op",
			L"cpp_parser::CppParenthesisExpr::expr",
			L"cpp_parser::CppPostfixUnaryExpr::op",
			L"cpp_parser::CppPostfixUnaryExpr::operand",
			L"cpp_parser::CppPrefixUnaryExpr::op",
			L"cpp_parser::CppPrefixUnaryExpr::operand",
			L"cpp_parser::CppPrimitiveExprLiteral::kind",
			L"cpp_parser::CppPrimitiveType::kind",
			L"cpp_parser::CppPrimitiveType::literal1",
			L"cpp_parser::CppPrimitiveType::literal2",
			L"cpp_parser::CppQualifiedName::arguments",
			L"cpp_parser::CppQualifiedName::expr",
			L"cpp_parser::CppQualifiedName::id",
			L"cpp_parser::CppQualifiedName::kind",
			L"cpp_parser::CppQualifiedName::parent",
			L"cpp_parser::CppSizeofExpr::argument",
			L"cpp_parser::CppSizeofExpr::variadic",
			L"cpp_parser::CppStringLiteral::fragments",
			L"cpp_parser::CppStringLiteralFragment::kind",
			L"cpp_parser::CppStringLiteralFragment::literal",
			L"cpp_parser::CppSysFuncExpr::argument",
			L"cpp_parser::CppSysFuncExpr::keyword",
			L"cpp_parser::CppSysFuncExpr::variadic",
			L"cpp_parser::CppThrowExpr::argument",
			L"cpp_parser::CppVolatileType::type",
		};
		vl::vint index = (vl::vint)field;
		return 0 <= index && index < 80 ? results[index] : nullptr;
	}

	vl::Ptr<vl::glr::ParsingAstBase> CppAstInsReceiver::ResolveAmbiguity(vl::vint32_t type, vl::collections::Array<vl::Ptr<vl::glr::ParsingAstBase>>& candidates)
	{
		auto cppTypeName = CppCppTypeName((CppClasses)type);
		return vl::glr::AssemblyThrowTypeNotAllowAmbiguity(type, cppTypeName);
	}
}

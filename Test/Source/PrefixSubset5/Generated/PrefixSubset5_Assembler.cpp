/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:PrefixSubset5
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "PrefixSubset5_Assembler.h"

namespace prefixsubset5
{

/***********************************************************************
PrefixSubset5AstInsReceiver : public vl::glr::AstInsReceiverBase
***********************************************************************/

	vl::Ptr<vl::glr::ParsingAstBase> PrefixSubset5AstInsReceiver::CreateAstNode(vl::vint32_t type)
	{
		auto cppTypeName = PrefixSubset5CppTypeName((PrefixSubset5Classes)type);
		switch((PrefixSubset5Classes)type)
		{
		case PrefixSubset5Classes::CallExpr:
			return new prefixsubset5::CallExpr();
		case PrefixSubset5Classes::ConstType:
			return new prefixsubset5::ConstType();
		case PrefixSubset5Classes::CtorExpr:
			return new prefixsubset5::CtorExpr();
		case PrefixSubset5Classes::FunctionType:
			return new prefixsubset5::FunctionType();
		case PrefixSubset5Classes::MemberName:
			return new prefixsubset5::MemberName();
		case PrefixSubset5Classes::MulExpr:
			return new prefixsubset5::MulExpr();
		case PrefixSubset5Classes::Name:
			return new prefixsubset5::Name();
		case PrefixSubset5Classes::PointerType:
			return new prefixsubset5::PointerType();
		case PrefixSubset5Classes::TypeOrExprToResolve:
			return new prefixsubset5::TypeOrExprToResolve();
		default:
			return vl::glr::AssemblyThrowCannotCreateAbstractType(type, cppTypeName);
		}
	}

	void PrefixSubset5AstInsReceiver::SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, vl::Ptr<vl::glr::ParsingAstBase> value)
	{
		auto cppFieldName = PrefixSubset5CppFieldName((PrefixSubset5Fields)field);
		switch((PrefixSubset5Fields)field)
		{
		case PrefixSubset5Fields::CallExpr_args:
			return vl::glr::AssemblerSetObjectField(&prefixsubset5::CallExpr::args, object, field, value, cppFieldName);
		case PrefixSubset5Fields::CallExpr_func:
			return vl::glr::AssemblerSetObjectField(&prefixsubset5::CallExpr::func, object, field, value, cppFieldName);
		case PrefixSubset5Fields::ConstType_type:
			return vl::glr::AssemblerSetObjectField(&prefixsubset5::ConstType::type, object, field, value, cppFieldName);
		case PrefixSubset5Fields::CtorExpr_args:
			return vl::glr::AssemblerSetObjectField(&prefixsubset5::CtorExpr::args, object, field, value, cppFieldName);
		case PrefixSubset5Fields::CtorExpr_type:
			return vl::glr::AssemblerSetObjectField(&prefixsubset5::CtorExpr::type, object, field, value, cppFieldName);
		case PrefixSubset5Fields::FunctionType_args:
			return vl::glr::AssemblerSetObjectField(&prefixsubset5::FunctionType::args, object, field, value, cppFieldName);
		case PrefixSubset5Fields::FunctionType_returnType:
			return vl::glr::AssemblerSetObjectField(&prefixsubset5::FunctionType::returnType, object, field, value, cppFieldName);
		case PrefixSubset5Fields::MemberName_parent:
			return vl::glr::AssemblerSetObjectField(&prefixsubset5::MemberName::parent, object, field, value, cppFieldName);
		case PrefixSubset5Fields::MulExpr_first:
			return vl::glr::AssemblerSetObjectField(&prefixsubset5::MulExpr::first, object, field, value, cppFieldName);
		case PrefixSubset5Fields::MulExpr_second:
			return vl::glr::AssemblerSetObjectField(&prefixsubset5::MulExpr::second, object, field, value, cppFieldName);
		case PrefixSubset5Fields::PointerType_type:
			return vl::glr::AssemblerSetObjectField(&prefixsubset5::PointerType::type, object, field, value, cppFieldName);
		case PrefixSubset5Fields::TypeOrExprToResolve_candidates:
			return vl::glr::AssemblerSetObjectField(&prefixsubset5::TypeOrExprToResolve::candidates, object, field, value, cppFieldName);
		default:
			return vl::glr::AssemblyThrowFieldNotObject(field, cppFieldName);
		}
	}

	void PrefixSubset5AstInsReceiver::SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, const vl::regex::RegexToken& token, vl::vint32_t tokenIndex)
	{
		auto cppFieldName = PrefixSubset5CppFieldName((PrefixSubset5Fields)field);
		switch((PrefixSubset5Fields)field)
		{
		case PrefixSubset5Fields::MemberName_member:
			return vl::glr::AssemblerSetTokenField(&prefixsubset5::MemberName::member, object, field, token, tokenIndex, cppFieldName);
		case PrefixSubset5Fields::Name_name:
			return vl::glr::AssemblerSetTokenField(&prefixsubset5::Name::name, object, field, token, tokenIndex, cppFieldName);
		default:
			return vl::glr::AssemblyThrowFieldNotToken(field, cppFieldName);
		}
	}

	void PrefixSubset5AstInsReceiver::SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, vl::vint32_t enumItem, bool weakAssignment)
	{
		auto cppFieldName = PrefixSubset5CppFieldName((PrefixSubset5Fields)field);
		return vl::glr::AssemblyThrowFieldNotEnum(field, cppFieldName);
	}

	const wchar_t* PrefixSubset5TypeName(PrefixSubset5Classes type)
	{
		const wchar_t* results[] = {
			L"CallExpr",
			L"ConstType",
			L"CtorExpr",
			L"FunctionType",
			L"MemberName",
			L"MulExpr",
			L"Name",
			L"PointerType",
			L"QualifiedName",
			L"TypeOrExpr",
			L"TypeOrExprToResolve",
		};
		vl::vint index = (vl::vint)type;
		return 0 <= index && index < 11 ? results[index] : nullptr;
	}

	const wchar_t* PrefixSubset5CppTypeName(PrefixSubset5Classes type)
	{
		const wchar_t* results[] = {
			L"prefixsubset5::CallExpr",
			L"prefixsubset5::ConstType",
			L"prefixsubset5::CtorExpr",
			L"prefixsubset5::FunctionType",
			L"prefixsubset5::MemberName",
			L"prefixsubset5::MulExpr",
			L"prefixsubset5::Name",
			L"prefixsubset5::PointerType",
			L"prefixsubset5::QualifiedName",
			L"prefixsubset5::TypeOrExpr",
			L"prefixsubset5::TypeOrExprToResolve",
		};
		vl::vint index = (vl::vint)type;
		return 0 <= index && index < 11 ? results[index] : nullptr;
	}

	const wchar_t* PrefixSubset5FieldName(PrefixSubset5Fields field)
	{
		const wchar_t* results[] = {
			L"CallExpr::args",
			L"CallExpr::func",
			L"ConstType::type",
			L"CtorExpr::args",
			L"CtorExpr::type",
			L"FunctionType::args",
			L"FunctionType::returnType",
			L"MemberName::member",
			L"MemberName::parent",
			L"MulExpr::first",
			L"MulExpr::second",
			L"Name::name",
			L"PointerType::type",
			L"TypeOrExprToResolve::candidates",
		};
		vl::vint index = (vl::vint)field;
		return 0 <= index && index < 14 ? results[index] : nullptr;
	}

	const wchar_t* PrefixSubset5CppFieldName(PrefixSubset5Fields field)
	{
		const wchar_t* results[] = {
			L"prefixsubset5::CallExpr::args",
			L"prefixsubset5::CallExpr::func",
			L"prefixsubset5::ConstType::type",
			L"prefixsubset5::CtorExpr::args",
			L"prefixsubset5::CtorExpr::type",
			L"prefixsubset5::FunctionType::args",
			L"prefixsubset5::FunctionType::returnType",
			L"prefixsubset5::MemberName::member",
			L"prefixsubset5::MemberName::parent",
			L"prefixsubset5::MulExpr::first",
			L"prefixsubset5::MulExpr::second",
			L"prefixsubset5::Name::name",
			L"prefixsubset5::PointerType::type",
			L"prefixsubset5::TypeOrExprToResolve::candidates",
		};
		vl::vint index = (vl::vint)field;
		return 0 <= index && index < 14 ? results[index] : nullptr;
	}

	vl::Ptr<vl::glr::ParsingAstBase> PrefixSubset5AstInsReceiver::ResolveAmbiguity(vl::vint32_t type, vl::collections::Array<vl::Ptr<vl::glr::ParsingAstBase>>& candidates)
	{
		auto cppTypeName = PrefixSubset5CppTypeName((PrefixSubset5Classes)type);
		switch((PrefixSubset5Classes)type)
		{
		case PrefixSubset5Classes::CallExpr:
			return vl::glr::AssemblerResolveAmbiguity<prefixsubset5::CallExpr, prefixsubset5::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixSubset5Classes::ConstType:
			return vl::glr::AssemblerResolveAmbiguity<prefixsubset5::ConstType, prefixsubset5::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixSubset5Classes::CtorExpr:
			return vl::glr::AssemblerResolveAmbiguity<prefixsubset5::CtorExpr, prefixsubset5::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixSubset5Classes::FunctionType:
			return vl::glr::AssemblerResolveAmbiguity<prefixsubset5::FunctionType, prefixsubset5::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixSubset5Classes::MemberName:
			return vl::glr::AssemblerResolveAmbiguity<prefixsubset5::MemberName, prefixsubset5::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixSubset5Classes::MulExpr:
			return vl::glr::AssemblerResolveAmbiguity<prefixsubset5::MulExpr, prefixsubset5::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixSubset5Classes::Name:
			return vl::glr::AssemblerResolveAmbiguity<prefixsubset5::Name, prefixsubset5::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixSubset5Classes::PointerType:
			return vl::glr::AssemblerResolveAmbiguity<prefixsubset5::PointerType, prefixsubset5::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixSubset5Classes::QualifiedName:
			return vl::glr::AssemblerResolveAmbiguity<prefixsubset5::QualifiedName, prefixsubset5::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixSubset5Classes::TypeOrExpr:
			return vl::glr::AssemblerResolveAmbiguity<prefixsubset5::TypeOrExpr, prefixsubset5::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixSubset5Classes::TypeOrExprToResolve:
			return vl::glr::AssemblerResolveAmbiguity<prefixsubset5::TypeOrExprToResolve, prefixsubset5::TypeOrExprToResolve>(type, candidates, cppTypeName);
		default:
			return vl::glr::AssemblyThrowTypeNotAllowAmbiguity(type, cppTypeName);
		}
	}
}

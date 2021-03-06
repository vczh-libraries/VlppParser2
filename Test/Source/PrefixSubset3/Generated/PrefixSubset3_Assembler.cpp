/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:PrefixSubset3
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "PrefixSubset3_Assembler.h"

namespace prefixsubset3
{

/***********************************************************************
PrefixSubset3AstInsReceiver : public vl::glr::AstInsReceiverBase
***********************************************************************/

	vl::Ptr<vl::glr::ParsingAstBase> PrefixSubset3AstInsReceiver::CreateAstNode(vl::vint32_t type)
	{
		auto cppTypeName = PrefixSubset3CppTypeName((PrefixSubset3Classes)type);
		switch((PrefixSubset3Classes)type)
		{
		case PrefixSubset3Classes::CallExpr:
			return new prefixsubset3::CallExpr();
		case PrefixSubset3Classes::ConstType:
			return new prefixsubset3::ConstType();
		case PrefixSubset3Classes::CtorExpr:
			return new prefixsubset3::CtorExpr();
		case PrefixSubset3Classes::FunctionType:
			return new prefixsubset3::FunctionType();
		case PrefixSubset3Classes::MemberName:
			return new prefixsubset3::MemberName();
		case PrefixSubset3Classes::MulExpr:
			return new prefixsubset3::MulExpr();
		case PrefixSubset3Classes::Name:
			return new prefixsubset3::Name();
		case PrefixSubset3Classes::PointerType:
			return new prefixsubset3::PointerType();
		case PrefixSubset3Classes::TypeOrExprToResolve:
			return new prefixsubset3::TypeOrExprToResolve();
		default:
			return vl::glr::AssemblyThrowCannotCreateAbstractType(type, cppTypeName);
		}
	}

	void PrefixSubset3AstInsReceiver::SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, vl::Ptr<vl::glr::ParsingAstBase> value)
	{
		auto cppFieldName = PrefixSubset3CppFieldName((PrefixSubset3Fields)field);
		switch((PrefixSubset3Fields)field)
		{
		case PrefixSubset3Fields::CallExpr_args:
			return vl::glr::AssemblerSetObjectField(&prefixsubset3::CallExpr::args, object, field, value, cppFieldName);
		case PrefixSubset3Fields::CallExpr_func:
			return vl::glr::AssemblerSetObjectField(&prefixsubset3::CallExpr::func, object, field, value, cppFieldName);
		case PrefixSubset3Fields::ConstType_type:
			return vl::glr::AssemblerSetObjectField(&prefixsubset3::ConstType::type, object, field, value, cppFieldName);
		case PrefixSubset3Fields::CtorExpr_args:
			return vl::glr::AssemblerSetObjectField(&prefixsubset3::CtorExpr::args, object, field, value, cppFieldName);
		case PrefixSubset3Fields::CtorExpr_type:
			return vl::glr::AssemblerSetObjectField(&prefixsubset3::CtorExpr::type, object, field, value, cppFieldName);
		case PrefixSubset3Fields::FunctionType_args:
			return vl::glr::AssemblerSetObjectField(&prefixsubset3::FunctionType::args, object, field, value, cppFieldName);
		case PrefixSubset3Fields::FunctionType_returnType:
			return vl::glr::AssemblerSetObjectField(&prefixsubset3::FunctionType::returnType, object, field, value, cppFieldName);
		case PrefixSubset3Fields::MemberName_parent:
			return vl::glr::AssemblerSetObjectField(&prefixsubset3::MemberName::parent, object, field, value, cppFieldName);
		case PrefixSubset3Fields::MulExpr_first:
			return vl::glr::AssemblerSetObjectField(&prefixsubset3::MulExpr::first, object, field, value, cppFieldName);
		case PrefixSubset3Fields::MulExpr_second:
			return vl::glr::AssemblerSetObjectField(&prefixsubset3::MulExpr::second, object, field, value, cppFieldName);
		case PrefixSubset3Fields::PointerType_type:
			return vl::glr::AssemblerSetObjectField(&prefixsubset3::PointerType::type, object, field, value, cppFieldName);
		case PrefixSubset3Fields::TypeOrExprToResolve_candidates:
			return vl::glr::AssemblerSetObjectField(&prefixsubset3::TypeOrExprToResolve::candidates, object, field, value, cppFieldName);
		default:
			return vl::glr::AssemblyThrowFieldNotObject(field, cppFieldName);
		}
	}

	void PrefixSubset3AstInsReceiver::SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, const vl::regex::RegexToken& token, vl::vint32_t tokenIndex)
	{
		auto cppFieldName = PrefixSubset3CppFieldName((PrefixSubset3Fields)field);
		switch((PrefixSubset3Fields)field)
		{
		case PrefixSubset3Fields::MemberName_member:
			return vl::glr::AssemblerSetTokenField(&prefixsubset3::MemberName::member, object, field, token, tokenIndex, cppFieldName);
		case PrefixSubset3Fields::Name_name:
			return vl::glr::AssemblerSetTokenField(&prefixsubset3::Name::name, object, field, token, tokenIndex, cppFieldName);
		default:
			return vl::glr::AssemblyThrowFieldNotToken(field, cppFieldName);
		}
	}

	void PrefixSubset3AstInsReceiver::SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, vl::vint32_t enumItem, bool weakAssignment)
	{
		auto cppFieldName = PrefixSubset3CppFieldName((PrefixSubset3Fields)field);
		return vl::glr::AssemblyThrowFieldNotEnum(field, cppFieldName);
	}

	const wchar_t* PrefixSubset3TypeName(PrefixSubset3Classes type)
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

	const wchar_t* PrefixSubset3CppTypeName(PrefixSubset3Classes type)
	{
		const wchar_t* results[] = {
			L"prefixsubset3::CallExpr",
			L"prefixsubset3::ConstType",
			L"prefixsubset3::CtorExpr",
			L"prefixsubset3::FunctionType",
			L"prefixsubset3::MemberName",
			L"prefixsubset3::MulExpr",
			L"prefixsubset3::Name",
			L"prefixsubset3::PointerType",
			L"prefixsubset3::QualifiedName",
			L"prefixsubset3::TypeOrExpr",
			L"prefixsubset3::TypeOrExprToResolve",
		};
		vl::vint index = (vl::vint)type;
		return 0 <= index && index < 11 ? results[index] : nullptr;
	}

	const wchar_t* PrefixSubset3FieldName(PrefixSubset3Fields field)
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

	const wchar_t* PrefixSubset3CppFieldName(PrefixSubset3Fields field)
	{
		const wchar_t* results[] = {
			L"prefixsubset3::CallExpr::args",
			L"prefixsubset3::CallExpr::func",
			L"prefixsubset3::ConstType::type",
			L"prefixsubset3::CtorExpr::args",
			L"prefixsubset3::CtorExpr::type",
			L"prefixsubset3::FunctionType::args",
			L"prefixsubset3::FunctionType::returnType",
			L"prefixsubset3::MemberName::member",
			L"prefixsubset3::MemberName::parent",
			L"prefixsubset3::MulExpr::first",
			L"prefixsubset3::MulExpr::second",
			L"prefixsubset3::Name::name",
			L"prefixsubset3::PointerType::type",
			L"prefixsubset3::TypeOrExprToResolve::candidates",
		};
		vl::vint index = (vl::vint)field;
		return 0 <= index && index < 14 ? results[index] : nullptr;
	}

	vl::Ptr<vl::glr::ParsingAstBase> PrefixSubset3AstInsReceiver::ResolveAmbiguity(vl::vint32_t type, vl::collections::Array<vl::Ptr<vl::glr::ParsingAstBase>>& candidates)
	{
		auto cppTypeName = PrefixSubset3CppTypeName((PrefixSubset3Classes)type);
		switch((PrefixSubset3Classes)type)
		{
		case PrefixSubset3Classes::CallExpr:
			return vl::glr::AssemblerResolveAmbiguity<prefixsubset3::CallExpr, prefixsubset3::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixSubset3Classes::ConstType:
			return vl::glr::AssemblerResolveAmbiguity<prefixsubset3::ConstType, prefixsubset3::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixSubset3Classes::CtorExpr:
			return vl::glr::AssemblerResolveAmbiguity<prefixsubset3::CtorExpr, prefixsubset3::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixSubset3Classes::FunctionType:
			return vl::glr::AssemblerResolveAmbiguity<prefixsubset3::FunctionType, prefixsubset3::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixSubset3Classes::MemberName:
			return vl::glr::AssemblerResolveAmbiguity<prefixsubset3::MemberName, prefixsubset3::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixSubset3Classes::MulExpr:
			return vl::glr::AssemblerResolveAmbiguity<prefixsubset3::MulExpr, prefixsubset3::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixSubset3Classes::Name:
			return vl::glr::AssemblerResolveAmbiguity<prefixsubset3::Name, prefixsubset3::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixSubset3Classes::PointerType:
			return vl::glr::AssemblerResolveAmbiguity<prefixsubset3::PointerType, prefixsubset3::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixSubset3Classes::QualifiedName:
			return vl::glr::AssemblerResolveAmbiguity<prefixsubset3::QualifiedName, prefixsubset3::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixSubset3Classes::TypeOrExpr:
			return vl::glr::AssemblerResolveAmbiguity<prefixsubset3::TypeOrExpr, prefixsubset3::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixSubset3Classes::TypeOrExprToResolve:
			return vl::glr::AssemblerResolveAmbiguity<prefixsubset3::TypeOrExprToResolve, prefixsubset3::TypeOrExprToResolve>(type, candidates, cppTypeName);
		default:
			return vl::glr::AssemblyThrowTypeNotAllowAmbiguity(type, cppTypeName);
		}
	}
}

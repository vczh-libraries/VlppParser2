/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:PrefixMerge1_Lri
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "PrefixMerge1_Lri_Assembler.h"

namespace prefixmerge1_lri
{

/***********************************************************************
PrefixMerge1_LriAstInsReceiver : public vl::glr::AstInsReceiverBase
***********************************************************************/

	vl::Ptr<vl::glr::ParsingAstBase> PrefixMerge1_LriAstInsReceiver::CreateAstNode(vl::vint32_t type)
	{
		auto cppTypeName = PrefixMerge1_LriCppTypeName((PrefixMerge1_LriClasses)type);
		switch((PrefixMerge1_LriClasses)type)
		{
		case PrefixMerge1_LriClasses::CallExpr:
			return new prefixmerge1_lri::CallExpr();
		case PrefixMerge1_LriClasses::ConstType:
			return new prefixmerge1_lri::ConstType();
		case PrefixMerge1_LriClasses::FunctionType:
			return new prefixmerge1_lri::FunctionType();
		case PrefixMerge1_LriClasses::MemberName:
			return new prefixmerge1_lri::MemberName();
		case PrefixMerge1_LriClasses::MulExpr:
			return new prefixmerge1_lri::MulExpr();
		case PrefixMerge1_LriClasses::Name:
			return new prefixmerge1_lri::Name();
		case PrefixMerge1_LriClasses::PointerType:
			return new prefixmerge1_lri::PointerType();
		case PrefixMerge1_LriClasses::TypeOrExprToResolve:
			return new prefixmerge1_lri::TypeOrExprToResolve();
		default:
			return vl::glr::AssemblyThrowCannotCreateAbstractType(type, cppTypeName);
		}
	}

	void PrefixMerge1_LriAstInsReceiver::SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, vl::Ptr<vl::glr::ParsingAstBase> value)
	{
		auto cppFieldName = PrefixMerge1_LriCppFieldName((PrefixMerge1_LriFields)field);
		switch((PrefixMerge1_LriFields)field)
		{
		case PrefixMerge1_LriFields::CallExpr_args:
			return vl::glr::AssemblerSetObjectField(&prefixmerge1_lri::CallExpr::args, object, field, value, cppFieldName);
		case PrefixMerge1_LriFields::CallExpr_func:
			return vl::glr::AssemblerSetObjectField(&prefixmerge1_lri::CallExpr::func, object, field, value, cppFieldName);
		case PrefixMerge1_LriFields::ConstType_type:
			return vl::glr::AssemblerSetObjectField(&prefixmerge1_lri::ConstType::type, object, field, value, cppFieldName);
		case PrefixMerge1_LriFields::FunctionType_args:
			return vl::glr::AssemblerSetObjectField(&prefixmerge1_lri::FunctionType::args, object, field, value, cppFieldName);
		case PrefixMerge1_LriFields::FunctionType_returnType:
			return vl::glr::AssemblerSetObjectField(&prefixmerge1_lri::FunctionType::returnType, object, field, value, cppFieldName);
		case PrefixMerge1_LriFields::MemberName_parent:
			return vl::glr::AssemblerSetObjectField(&prefixmerge1_lri::MemberName::parent, object, field, value, cppFieldName);
		case PrefixMerge1_LriFields::MulExpr_first:
			return vl::glr::AssemblerSetObjectField(&prefixmerge1_lri::MulExpr::first, object, field, value, cppFieldName);
		case PrefixMerge1_LriFields::MulExpr_second:
			return vl::glr::AssemblerSetObjectField(&prefixmerge1_lri::MulExpr::second, object, field, value, cppFieldName);
		case PrefixMerge1_LriFields::PointerType_type:
			return vl::glr::AssemblerSetObjectField(&prefixmerge1_lri::PointerType::type, object, field, value, cppFieldName);
		case PrefixMerge1_LriFields::TypeOrExprToResolve_candidates:
			return vl::glr::AssemblerSetObjectField(&prefixmerge1_lri::TypeOrExprToResolve::candidates, object, field, value, cppFieldName);
		default:
			return vl::glr::AssemblyThrowFieldNotObject(field, cppFieldName);
		}
	}

	void PrefixMerge1_LriAstInsReceiver::SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, const vl::regex::RegexToken& token, vl::vint32_t tokenIndex)
	{
		auto cppFieldName = PrefixMerge1_LriCppFieldName((PrefixMerge1_LriFields)field);
		switch((PrefixMerge1_LriFields)field)
		{
		case PrefixMerge1_LriFields::MemberName_member:
			return vl::glr::AssemblerSetTokenField(&prefixmerge1_lri::MemberName::member, object, field, token, tokenIndex, cppFieldName);
		case PrefixMerge1_LriFields::Name_name:
			return vl::glr::AssemblerSetTokenField(&prefixmerge1_lri::Name::name, object, field, token, tokenIndex, cppFieldName);
		default:
			return vl::glr::AssemblyThrowFieldNotToken(field, cppFieldName);
		}
	}

	void PrefixMerge1_LriAstInsReceiver::SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, vl::vint32_t enumItem, bool weakAssignment)
	{
		auto cppFieldName = PrefixMerge1_LriCppFieldName((PrefixMerge1_LriFields)field);
		return vl::glr::AssemblyThrowFieldNotEnum(field, cppFieldName);
	}

	const wchar_t* PrefixMerge1_LriTypeName(PrefixMerge1_LriClasses type)
	{
		const wchar_t* results[] = {
			L"CallExpr",
			L"ConstType",
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
		return 0 <= index && index < 10 ? results[index] : nullptr;
	}

	const wchar_t* PrefixMerge1_LriCppTypeName(PrefixMerge1_LriClasses type)
	{
		const wchar_t* results[] = {
			L"prefixmerge1_lri::CallExpr",
			L"prefixmerge1_lri::ConstType",
			L"prefixmerge1_lri::FunctionType",
			L"prefixmerge1_lri::MemberName",
			L"prefixmerge1_lri::MulExpr",
			L"prefixmerge1_lri::Name",
			L"prefixmerge1_lri::PointerType",
			L"prefixmerge1_lri::QualifiedName",
			L"prefixmerge1_lri::TypeOrExpr",
			L"prefixmerge1_lri::TypeOrExprToResolve",
		};
		vl::vint index = (vl::vint)type;
		return 0 <= index && index < 10 ? results[index] : nullptr;
	}

	const wchar_t* PrefixMerge1_LriFieldName(PrefixMerge1_LriFields field)
	{
		const wchar_t* results[] = {
			L"CallExpr::args",
			L"CallExpr::func",
			L"ConstType::type",
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
		return 0 <= index && index < 12 ? results[index] : nullptr;
	}

	const wchar_t* PrefixMerge1_LriCppFieldName(PrefixMerge1_LriFields field)
	{
		const wchar_t* results[] = {
			L"prefixmerge1_lri::CallExpr::args",
			L"prefixmerge1_lri::CallExpr::func",
			L"prefixmerge1_lri::ConstType::type",
			L"prefixmerge1_lri::FunctionType::args",
			L"prefixmerge1_lri::FunctionType::returnType",
			L"prefixmerge1_lri::MemberName::member",
			L"prefixmerge1_lri::MemberName::parent",
			L"prefixmerge1_lri::MulExpr::first",
			L"prefixmerge1_lri::MulExpr::second",
			L"prefixmerge1_lri::Name::name",
			L"prefixmerge1_lri::PointerType::type",
			L"prefixmerge1_lri::TypeOrExprToResolve::candidates",
		};
		vl::vint index = (vl::vint)field;
		return 0 <= index && index < 12 ? results[index] : nullptr;
	}

	vl::Ptr<vl::glr::ParsingAstBase> PrefixMerge1_LriAstInsReceiver::ResolveAmbiguity(vl::vint32_t type, vl::collections::Array<vl::Ptr<vl::glr::ParsingAstBase>>& candidates)
	{
		auto cppTypeName = PrefixMerge1_LriCppTypeName((PrefixMerge1_LriClasses)type);
		switch((PrefixMerge1_LriClasses)type)
		{
		case PrefixMerge1_LriClasses::CallExpr:
			return vl::glr::AssemblerResolveAmbiguity<prefixmerge1_lri::CallExpr, prefixmerge1_lri::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixMerge1_LriClasses::ConstType:
			return vl::glr::AssemblerResolveAmbiguity<prefixmerge1_lri::ConstType, prefixmerge1_lri::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixMerge1_LriClasses::FunctionType:
			return vl::glr::AssemblerResolveAmbiguity<prefixmerge1_lri::FunctionType, prefixmerge1_lri::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixMerge1_LriClasses::MemberName:
			return vl::glr::AssemblerResolveAmbiguity<prefixmerge1_lri::MemberName, prefixmerge1_lri::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixMerge1_LriClasses::MulExpr:
			return vl::glr::AssemblerResolveAmbiguity<prefixmerge1_lri::MulExpr, prefixmerge1_lri::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixMerge1_LriClasses::Name:
			return vl::glr::AssemblerResolveAmbiguity<prefixmerge1_lri::Name, prefixmerge1_lri::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixMerge1_LriClasses::PointerType:
			return vl::glr::AssemblerResolveAmbiguity<prefixmerge1_lri::PointerType, prefixmerge1_lri::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixMerge1_LriClasses::QualifiedName:
			return vl::glr::AssemblerResolveAmbiguity<prefixmerge1_lri::QualifiedName, prefixmerge1_lri::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixMerge1_LriClasses::TypeOrExpr:
			return vl::glr::AssemblerResolveAmbiguity<prefixmerge1_lri::TypeOrExpr, prefixmerge1_lri::TypeOrExprToResolve>(type, candidates, cppTypeName);
		case PrefixMerge1_LriClasses::TypeOrExprToResolve:
			return vl::glr::AssemblerResolveAmbiguity<prefixmerge1_lri::TypeOrExprToResolve, prefixmerge1_lri::TypeOrExprToResolve>(type, candidates, cppTypeName);
		default:
			return vl::glr::AssemblyThrowTypeNotAllowAmbiguity(type, cppTypeName);
		}
	}
}
/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:TypeOrExpr
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "PrefixSubset2TypeOrExpr.h"

namespace prefixsubset2
{
/***********************************************************************
Visitor Pattern Implementation
***********************************************************************/

	void QualifiedName::Accept(TypeOrExpr::IVisitor* visitor)
	{
		visitor->Visit(this);
	}

	void Name::Accept(QualifiedName::IVisitor* visitor)
	{
		visitor->Visit(this);
	}

	void MemberName::Accept(QualifiedName::IVisitor* visitor)
	{
		visitor->Visit(this);
	}

	void CallExpr::Accept(TypeOrExpr::IVisitor* visitor)
	{
		visitor->Visit(this);
	}

	void MulExpr::Accept(TypeOrExpr::IVisitor* visitor)
	{
		visitor->Visit(this);
	}

	void ConstType::Accept(TypeOrExpr::IVisitor* visitor)
	{
		visitor->Visit(this);
	}

	void PointerType::Accept(TypeOrExpr::IVisitor* visitor)
	{
		visitor->Visit(this);
	}

	void FunctionType::Accept(TypeOrExpr::IVisitor* visitor)
	{
		visitor->Visit(this);
	}

	void TypeOrExprToResolve::Accept(TypeOrExpr::IVisitor* visitor)
	{
		visitor->Visit(this);
	}
}
namespace vl
{
	namespace reflection
	{
		namespace description
		{
#ifndef VCZH_DEBUG_NO_REFLECTION

			IMPL_TYPE_INFO_RENAME(prefixsubset2::TypeOrExpr, prefixsubset2::TypeOrExpr)
			IMPL_TYPE_INFO_RENAME(prefixsubset2::TypeOrExpr::IVisitor, prefixsubset2::TypeOrExpr::IVisitor)
			IMPL_TYPE_INFO_RENAME(prefixsubset2::QualifiedName, prefixsubset2::QualifiedName)
			IMPL_TYPE_INFO_RENAME(prefixsubset2::QualifiedName::IVisitor, prefixsubset2::QualifiedName::IVisitor)
			IMPL_TYPE_INFO_RENAME(prefixsubset2::Name, prefixsubset2::Name)
			IMPL_TYPE_INFO_RENAME(prefixsubset2::MemberName, prefixsubset2::MemberName)
			IMPL_TYPE_INFO_RENAME(prefixsubset2::CallExpr, prefixsubset2::CallExpr)
			IMPL_TYPE_INFO_RENAME(prefixsubset2::MulExpr, prefixsubset2::MulExpr)
			IMPL_TYPE_INFO_RENAME(prefixsubset2::ConstType, prefixsubset2::ConstType)
			IMPL_TYPE_INFO_RENAME(prefixsubset2::PointerType, prefixsubset2::PointerType)
			IMPL_TYPE_INFO_RENAME(prefixsubset2::FunctionType, prefixsubset2::FunctionType)
			IMPL_TYPE_INFO_RENAME(prefixsubset2::TypeOrExprToResolve, prefixsubset2::TypeOrExprToResolve)

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA

			BEGIN_CLASS_MEMBER(prefixsubset2::TypeOrExpr)
				CLASS_MEMBER_BASE(vl::glr::ParsingAstBase)

			END_CLASS_MEMBER(prefixsubset2::TypeOrExpr)

			BEGIN_CLASS_MEMBER(prefixsubset2::QualifiedName)
				CLASS_MEMBER_BASE(prefixsubset2::TypeOrExpr)

			END_CLASS_MEMBER(prefixsubset2::QualifiedName)

			BEGIN_CLASS_MEMBER(prefixsubset2::Name)
				CLASS_MEMBER_BASE(prefixsubset2::QualifiedName)

				CLASS_MEMBER_CONSTRUCTOR(vl::Ptr<prefixsubset2::Name>(), NO_PARAMETER)

				CLASS_MEMBER_FIELD(name)
			END_CLASS_MEMBER(prefixsubset2::Name)

			BEGIN_CLASS_MEMBER(prefixsubset2::MemberName)
				CLASS_MEMBER_BASE(prefixsubset2::QualifiedName)

				CLASS_MEMBER_CONSTRUCTOR(vl::Ptr<prefixsubset2::MemberName>(), NO_PARAMETER)

				CLASS_MEMBER_FIELD(parent)
				CLASS_MEMBER_FIELD(member)
			END_CLASS_MEMBER(prefixsubset2::MemberName)

			BEGIN_CLASS_MEMBER(prefixsubset2::CallExpr)
				CLASS_MEMBER_BASE(prefixsubset2::TypeOrExpr)

				CLASS_MEMBER_CONSTRUCTOR(vl::Ptr<prefixsubset2::CallExpr>(), NO_PARAMETER)

				CLASS_MEMBER_FIELD(func)
				CLASS_MEMBER_FIELD(args)
			END_CLASS_MEMBER(prefixsubset2::CallExpr)

			BEGIN_CLASS_MEMBER(prefixsubset2::MulExpr)
				CLASS_MEMBER_BASE(prefixsubset2::TypeOrExpr)

				CLASS_MEMBER_CONSTRUCTOR(vl::Ptr<prefixsubset2::MulExpr>(), NO_PARAMETER)

				CLASS_MEMBER_FIELD(first)
				CLASS_MEMBER_FIELD(second)
			END_CLASS_MEMBER(prefixsubset2::MulExpr)

			BEGIN_CLASS_MEMBER(prefixsubset2::ConstType)
				CLASS_MEMBER_BASE(prefixsubset2::TypeOrExpr)

				CLASS_MEMBER_CONSTRUCTOR(vl::Ptr<prefixsubset2::ConstType>(), NO_PARAMETER)

				CLASS_MEMBER_FIELD(type)
			END_CLASS_MEMBER(prefixsubset2::ConstType)

			BEGIN_CLASS_MEMBER(prefixsubset2::PointerType)
				CLASS_MEMBER_BASE(prefixsubset2::TypeOrExpr)

				CLASS_MEMBER_CONSTRUCTOR(vl::Ptr<prefixsubset2::PointerType>(), NO_PARAMETER)

				CLASS_MEMBER_FIELD(type)
			END_CLASS_MEMBER(prefixsubset2::PointerType)

			BEGIN_CLASS_MEMBER(prefixsubset2::FunctionType)
				CLASS_MEMBER_BASE(prefixsubset2::TypeOrExpr)

				CLASS_MEMBER_CONSTRUCTOR(vl::Ptr<prefixsubset2::FunctionType>(), NO_PARAMETER)

				CLASS_MEMBER_FIELD(returnType)
				CLASS_MEMBER_FIELD(args)
			END_CLASS_MEMBER(prefixsubset2::FunctionType)

			BEGIN_CLASS_MEMBER(prefixsubset2::TypeOrExprToResolve)
				CLASS_MEMBER_BASE(prefixsubset2::TypeOrExpr)

				CLASS_MEMBER_CONSTRUCTOR(vl::Ptr<prefixsubset2::TypeOrExprToResolve>(), NO_PARAMETER)

				CLASS_MEMBER_FIELD(candidates)
			END_CLASS_MEMBER(prefixsubset2::TypeOrExprToResolve)

			BEGIN_INTERFACE_MEMBER(prefixsubset2::TypeOrExpr::IVisitor)
				CLASS_MEMBER_METHOD_OVERLOAD(Visit, {L"node"}, void(prefixsubset2::TypeOrExpr::IVisitor::*)(prefixsubset2::TypeOrExprToResolve* node))
				CLASS_MEMBER_METHOD_OVERLOAD(Visit, {L"node"}, void(prefixsubset2::TypeOrExpr::IVisitor::*)(prefixsubset2::QualifiedName* node))
				CLASS_MEMBER_METHOD_OVERLOAD(Visit, {L"node"}, void(prefixsubset2::TypeOrExpr::IVisitor::*)(prefixsubset2::CallExpr* node))
				CLASS_MEMBER_METHOD_OVERLOAD(Visit, {L"node"}, void(prefixsubset2::TypeOrExpr::IVisitor::*)(prefixsubset2::MulExpr* node))
				CLASS_MEMBER_METHOD_OVERLOAD(Visit, {L"node"}, void(prefixsubset2::TypeOrExpr::IVisitor::*)(prefixsubset2::ConstType* node))
				CLASS_MEMBER_METHOD_OVERLOAD(Visit, {L"node"}, void(prefixsubset2::TypeOrExpr::IVisitor::*)(prefixsubset2::PointerType* node))
				CLASS_MEMBER_METHOD_OVERLOAD(Visit, {L"node"}, void(prefixsubset2::TypeOrExpr::IVisitor::*)(prefixsubset2::FunctionType* node))
			END_INTERFACE_MEMBER(prefixsubset2::TypeOrExpr)

			BEGIN_INTERFACE_MEMBER(prefixsubset2::QualifiedName::IVisitor)
				CLASS_MEMBER_METHOD_OVERLOAD(Visit, {L"node"}, void(prefixsubset2::QualifiedName::IVisitor::*)(prefixsubset2::Name* node))
				CLASS_MEMBER_METHOD_OVERLOAD(Visit, {L"node"}, void(prefixsubset2::QualifiedName::IVisitor::*)(prefixsubset2::MemberName* node))
			END_INTERFACE_MEMBER(prefixsubset2::QualifiedName)

#endif

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
			class PrefixSubset2TypeOrExprTypeLoader : public vl::Object, public ITypeLoader
			{
			public:
				void Load(ITypeManager* manager)
				{
					ADD_TYPE_INFO(prefixsubset2::TypeOrExpr)
					ADD_TYPE_INFO(prefixsubset2::TypeOrExpr::IVisitor)
					ADD_TYPE_INFO(prefixsubset2::QualifiedName)
					ADD_TYPE_INFO(prefixsubset2::QualifiedName::IVisitor)
					ADD_TYPE_INFO(prefixsubset2::Name)
					ADD_TYPE_INFO(prefixsubset2::MemberName)
					ADD_TYPE_INFO(prefixsubset2::CallExpr)
					ADD_TYPE_INFO(prefixsubset2::MulExpr)
					ADD_TYPE_INFO(prefixsubset2::ConstType)
					ADD_TYPE_INFO(prefixsubset2::PointerType)
					ADD_TYPE_INFO(prefixsubset2::FunctionType)
					ADD_TYPE_INFO(prefixsubset2::TypeOrExprToResolve)
				}

				void Unload(ITypeManager* manager)
				{
				}
			};
#endif
#endif

			bool PrefixSubset2TypeOrExprLoadTypes()
			{
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
				if (auto manager = GetGlobalTypeManager())
				{
					Ptr<ITypeLoader> loader = new PrefixSubset2TypeOrExprTypeLoader;
					return manager->AddTypeLoader(loader);
				}
#endif
				return false;
			}
		}
	}
}
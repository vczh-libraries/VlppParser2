/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:TypeOrExpr
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "PrefixMerge6_Pm2TypeOrExpr.h"

namespace prefixmerge6_pm2
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

	void CtorExpr::Accept(TypeOrExpr::IVisitor* visitor)
	{
		visitor->Visit(this);
	}

	void MulExpr::Accept(TypeOrExpr::IVisitor* visitor)
	{
		visitor->Visit(this);
	}

	void ThrowExpr::Accept(TypeOrExpr::IVisitor* visitor)
	{
		visitor->Visit(this);
	}

	void CommaExpr::Accept(TypeOrExpr::IVisitor* visitor)
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

			IMPL_TYPE_INFO_RENAME(prefixmerge6_pm2::TypeOrExpr, prefixmerge6_pm2::TypeOrExpr)
			IMPL_TYPE_INFO_RENAME(prefixmerge6_pm2::TypeOrExpr::IVisitor, prefixmerge6_pm2::TypeOrExpr::IVisitor)
			IMPL_TYPE_INFO_RENAME(prefixmerge6_pm2::QualifiedName, prefixmerge6_pm2::QualifiedName)
			IMPL_TYPE_INFO_RENAME(prefixmerge6_pm2::QualifiedName::IVisitor, prefixmerge6_pm2::QualifiedName::IVisitor)
			IMPL_TYPE_INFO_RENAME(prefixmerge6_pm2::Name, prefixmerge6_pm2::Name)
			IMPL_TYPE_INFO_RENAME(prefixmerge6_pm2::MemberName, prefixmerge6_pm2::MemberName)
			IMPL_TYPE_INFO_RENAME(prefixmerge6_pm2::CallExpr, prefixmerge6_pm2::CallExpr)
			IMPL_TYPE_INFO_RENAME(prefixmerge6_pm2::CtorExpr, prefixmerge6_pm2::CtorExpr)
			IMPL_TYPE_INFO_RENAME(prefixmerge6_pm2::MulExpr, prefixmerge6_pm2::MulExpr)
			IMPL_TYPE_INFO_RENAME(prefixmerge6_pm2::ThrowExpr, prefixmerge6_pm2::ThrowExpr)
			IMPL_TYPE_INFO_RENAME(prefixmerge6_pm2::CommaExpr, prefixmerge6_pm2::CommaExpr)
			IMPL_TYPE_INFO_RENAME(prefixmerge6_pm2::ConstType, prefixmerge6_pm2::ConstType)
			IMPL_TYPE_INFO_RENAME(prefixmerge6_pm2::PointerType, prefixmerge6_pm2::PointerType)
			IMPL_TYPE_INFO_RENAME(prefixmerge6_pm2::FunctionType, prefixmerge6_pm2::FunctionType)
			IMPL_TYPE_INFO_RENAME(prefixmerge6_pm2::TypeOrExprToResolve, prefixmerge6_pm2::TypeOrExprToResolve)

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA

			BEGIN_CLASS_MEMBER(prefixmerge6_pm2::TypeOrExpr)
				CLASS_MEMBER_BASE(vl::glr::ParsingAstBase)

			END_CLASS_MEMBER(prefixmerge6_pm2::TypeOrExpr)

			BEGIN_CLASS_MEMBER(prefixmerge6_pm2::QualifiedName)
				CLASS_MEMBER_BASE(prefixmerge6_pm2::TypeOrExpr)

			END_CLASS_MEMBER(prefixmerge6_pm2::QualifiedName)

			BEGIN_CLASS_MEMBER(prefixmerge6_pm2::Name)
				CLASS_MEMBER_BASE(prefixmerge6_pm2::QualifiedName)

				CLASS_MEMBER_CONSTRUCTOR(vl::Ptr<prefixmerge6_pm2::Name>(), NO_PARAMETER)

				CLASS_MEMBER_FIELD(name)
			END_CLASS_MEMBER(prefixmerge6_pm2::Name)

			BEGIN_CLASS_MEMBER(prefixmerge6_pm2::MemberName)
				CLASS_MEMBER_BASE(prefixmerge6_pm2::QualifiedName)

				CLASS_MEMBER_CONSTRUCTOR(vl::Ptr<prefixmerge6_pm2::MemberName>(), NO_PARAMETER)

				CLASS_MEMBER_FIELD(parent)
				CLASS_MEMBER_FIELD(member)
			END_CLASS_MEMBER(prefixmerge6_pm2::MemberName)

			BEGIN_CLASS_MEMBER(prefixmerge6_pm2::CallExpr)
				CLASS_MEMBER_BASE(prefixmerge6_pm2::TypeOrExpr)

				CLASS_MEMBER_CONSTRUCTOR(vl::Ptr<prefixmerge6_pm2::CallExpr>(), NO_PARAMETER)

				CLASS_MEMBER_FIELD(func)
				CLASS_MEMBER_FIELD(args)
			END_CLASS_MEMBER(prefixmerge6_pm2::CallExpr)

			BEGIN_CLASS_MEMBER(prefixmerge6_pm2::CtorExpr)
				CLASS_MEMBER_BASE(prefixmerge6_pm2::TypeOrExpr)

				CLASS_MEMBER_CONSTRUCTOR(vl::Ptr<prefixmerge6_pm2::CtorExpr>(), NO_PARAMETER)

				CLASS_MEMBER_FIELD(type)
				CLASS_MEMBER_FIELD(args)
			END_CLASS_MEMBER(prefixmerge6_pm2::CtorExpr)

			BEGIN_CLASS_MEMBER(prefixmerge6_pm2::MulExpr)
				CLASS_MEMBER_BASE(prefixmerge6_pm2::TypeOrExpr)

				CLASS_MEMBER_CONSTRUCTOR(vl::Ptr<prefixmerge6_pm2::MulExpr>(), NO_PARAMETER)

				CLASS_MEMBER_FIELD(first)
				CLASS_MEMBER_FIELD(second)
			END_CLASS_MEMBER(prefixmerge6_pm2::MulExpr)

			BEGIN_CLASS_MEMBER(prefixmerge6_pm2::ThrowExpr)
				CLASS_MEMBER_BASE(prefixmerge6_pm2::TypeOrExpr)

				CLASS_MEMBER_CONSTRUCTOR(vl::Ptr<prefixmerge6_pm2::ThrowExpr>(), NO_PARAMETER)

				CLASS_MEMBER_FIELD(arg)
			END_CLASS_MEMBER(prefixmerge6_pm2::ThrowExpr)

			BEGIN_CLASS_MEMBER(prefixmerge6_pm2::CommaExpr)
				CLASS_MEMBER_BASE(prefixmerge6_pm2::TypeOrExpr)

				CLASS_MEMBER_CONSTRUCTOR(vl::Ptr<prefixmerge6_pm2::CommaExpr>(), NO_PARAMETER)

				CLASS_MEMBER_FIELD(first)
				CLASS_MEMBER_FIELD(second)
			END_CLASS_MEMBER(prefixmerge6_pm2::CommaExpr)

			BEGIN_CLASS_MEMBER(prefixmerge6_pm2::ConstType)
				CLASS_MEMBER_BASE(prefixmerge6_pm2::TypeOrExpr)

				CLASS_MEMBER_CONSTRUCTOR(vl::Ptr<prefixmerge6_pm2::ConstType>(), NO_PARAMETER)

				CLASS_MEMBER_FIELD(type)
			END_CLASS_MEMBER(prefixmerge6_pm2::ConstType)

			BEGIN_CLASS_MEMBER(prefixmerge6_pm2::PointerType)
				CLASS_MEMBER_BASE(prefixmerge6_pm2::TypeOrExpr)

				CLASS_MEMBER_CONSTRUCTOR(vl::Ptr<prefixmerge6_pm2::PointerType>(), NO_PARAMETER)

				CLASS_MEMBER_FIELD(type)
			END_CLASS_MEMBER(prefixmerge6_pm2::PointerType)

			BEGIN_CLASS_MEMBER(prefixmerge6_pm2::FunctionType)
				CLASS_MEMBER_BASE(prefixmerge6_pm2::TypeOrExpr)

				CLASS_MEMBER_CONSTRUCTOR(vl::Ptr<prefixmerge6_pm2::FunctionType>(), NO_PARAMETER)

				CLASS_MEMBER_FIELD(returnType)
				CLASS_MEMBER_FIELD(args)
			END_CLASS_MEMBER(prefixmerge6_pm2::FunctionType)

			BEGIN_CLASS_MEMBER(prefixmerge6_pm2::TypeOrExprToResolve)
				CLASS_MEMBER_BASE(prefixmerge6_pm2::TypeOrExpr)

				CLASS_MEMBER_CONSTRUCTOR(vl::Ptr<prefixmerge6_pm2::TypeOrExprToResolve>(), NO_PARAMETER)

				CLASS_MEMBER_FIELD(candidates)
			END_CLASS_MEMBER(prefixmerge6_pm2::TypeOrExprToResolve)

			BEGIN_INTERFACE_MEMBER(prefixmerge6_pm2::TypeOrExpr::IVisitor)
				CLASS_MEMBER_METHOD_OVERLOAD(Visit, {L"node"}, void(prefixmerge6_pm2::TypeOrExpr::IVisitor::*)(prefixmerge6_pm2::TypeOrExprToResolve* node))
				CLASS_MEMBER_METHOD_OVERLOAD(Visit, {L"node"}, void(prefixmerge6_pm2::TypeOrExpr::IVisitor::*)(prefixmerge6_pm2::QualifiedName* node))
				CLASS_MEMBER_METHOD_OVERLOAD(Visit, {L"node"}, void(prefixmerge6_pm2::TypeOrExpr::IVisitor::*)(prefixmerge6_pm2::CallExpr* node))
				CLASS_MEMBER_METHOD_OVERLOAD(Visit, {L"node"}, void(prefixmerge6_pm2::TypeOrExpr::IVisitor::*)(prefixmerge6_pm2::CtorExpr* node))
				CLASS_MEMBER_METHOD_OVERLOAD(Visit, {L"node"}, void(prefixmerge6_pm2::TypeOrExpr::IVisitor::*)(prefixmerge6_pm2::MulExpr* node))
				CLASS_MEMBER_METHOD_OVERLOAD(Visit, {L"node"}, void(prefixmerge6_pm2::TypeOrExpr::IVisitor::*)(prefixmerge6_pm2::ThrowExpr* node))
				CLASS_MEMBER_METHOD_OVERLOAD(Visit, {L"node"}, void(prefixmerge6_pm2::TypeOrExpr::IVisitor::*)(prefixmerge6_pm2::CommaExpr* node))
				CLASS_MEMBER_METHOD_OVERLOAD(Visit, {L"node"}, void(prefixmerge6_pm2::TypeOrExpr::IVisitor::*)(prefixmerge6_pm2::ConstType* node))
				CLASS_MEMBER_METHOD_OVERLOAD(Visit, {L"node"}, void(prefixmerge6_pm2::TypeOrExpr::IVisitor::*)(prefixmerge6_pm2::PointerType* node))
				CLASS_MEMBER_METHOD_OVERLOAD(Visit, {L"node"}, void(prefixmerge6_pm2::TypeOrExpr::IVisitor::*)(prefixmerge6_pm2::FunctionType* node))
			END_INTERFACE_MEMBER(prefixmerge6_pm2::TypeOrExpr)

			BEGIN_INTERFACE_MEMBER(prefixmerge6_pm2::QualifiedName::IVisitor)
				CLASS_MEMBER_METHOD_OVERLOAD(Visit, {L"node"}, void(prefixmerge6_pm2::QualifiedName::IVisitor::*)(prefixmerge6_pm2::Name* node))
				CLASS_MEMBER_METHOD_OVERLOAD(Visit, {L"node"}, void(prefixmerge6_pm2::QualifiedName::IVisitor::*)(prefixmerge6_pm2::MemberName* node))
			END_INTERFACE_MEMBER(prefixmerge6_pm2::QualifiedName)

#endif

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
			class PrefixMerge6_Pm2TypeOrExprTypeLoader : public vl::Object, public ITypeLoader
			{
			public:
				void Load(ITypeManager* manager)
				{
					ADD_TYPE_INFO(prefixmerge6_pm2::TypeOrExpr)
					ADD_TYPE_INFO(prefixmerge6_pm2::TypeOrExpr::IVisitor)
					ADD_TYPE_INFO(prefixmerge6_pm2::QualifiedName)
					ADD_TYPE_INFO(prefixmerge6_pm2::QualifiedName::IVisitor)
					ADD_TYPE_INFO(prefixmerge6_pm2::Name)
					ADD_TYPE_INFO(prefixmerge6_pm2::MemberName)
					ADD_TYPE_INFO(prefixmerge6_pm2::CallExpr)
					ADD_TYPE_INFO(prefixmerge6_pm2::CtorExpr)
					ADD_TYPE_INFO(prefixmerge6_pm2::MulExpr)
					ADD_TYPE_INFO(prefixmerge6_pm2::ThrowExpr)
					ADD_TYPE_INFO(prefixmerge6_pm2::CommaExpr)
					ADD_TYPE_INFO(prefixmerge6_pm2::ConstType)
					ADD_TYPE_INFO(prefixmerge6_pm2::PointerType)
					ADD_TYPE_INFO(prefixmerge6_pm2::FunctionType)
					ADD_TYPE_INFO(prefixmerge6_pm2::TypeOrExprToResolve)
				}

				void Unload(ITypeManager* manager)
				{
				}
			};
#endif
#endif

			bool PrefixMerge6_Pm2TypeOrExprLoadTypes()
			{
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
				if (auto manager = GetGlobalTypeManager())
				{
					Ptr<ITypeLoader> loader = new PrefixMerge6_Pm2TypeOrExprTypeLoader;
					return manager->AddTypeLoader(loader);
				}
#endif
				return false;
			}
		}
	}
}
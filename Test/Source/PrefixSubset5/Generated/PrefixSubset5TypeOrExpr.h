/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:TypeOrExpr
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_UNITTEST_PREFIXSUBSET5_TYPEOREXPR_AST
#define VCZH_PARSER2_UNITTEST_PREFIXSUBSET5_TYPEOREXPR_AST

#include "../../../../Source/AstBase.h"
#include "../../../../Source/SyntaxBase.h"

namespace prefixsubset5
{
	class CallExpr;
	class ConstType;
	class CtorExpr;
	class FunctionType;
	class MemberName;
	class MulExpr;
	class Name;
	class PointerType;
	class QualifiedName;
	class TypeOrExpr;
	class TypeOrExprToResolve;

	class TypeOrExpr abstract : public vl::glr::ParsingAstBase, vl::reflection::Description<TypeOrExpr>
	{
	public:
		class IVisitor : public virtual vl::reflection::IDescriptable, vl::reflection::Description<IVisitor>
		{
		public:
			virtual void Visit(TypeOrExprToResolve* node) = 0;
			virtual void Visit(QualifiedName* node) = 0;
			virtual void Visit(CallExpr* node) = 0;
			virtual void Visit(CtorExpr* node) = 0;
			virtual void Visit(MulExpr* node) = 0;
			virtual void Visit(ConstType* node) = 0;
			virtual void Visit(PointerType* node) = 0;
			virtual void Visit(FunctionType* node) = 0;
		};

		virtual void Accept(TypeOrExpr::IVisitor* visitor) = 0;

	};

	class QualifiedName abstract : public TypeOrExpr, vl::reflection::Description<QualifiedName>
	{
	public:
		class IVisitor : public virtual vl::reflection::IDescriptable, vl::reflection::Description<IVisitor>
		{
		public:
			virtual void Visit(Name* node) = 0;
			virtual void Visit(MemberName* node) = 0;
		};

		virtual void Accept(QualifiedName::IVisitor* visitor) = 0;


		void Accept(TypeOrExpr::IVisitor* visitor) override;
	};

	class Name : public QualifiedName, vl::reflection::Description<Name>
	{
	public:
		vl::glr::ParsingToken name;

		void Accept(QualifiedName::IVisitor* visitor) override;
	};

	class MemberName : public QualifiedName, vl::reflection::Description<MemberName>
	{
	public:
		vl::Ptr<QualifiedName> parent;
		vl::glr::ParsingToken member;

		void Accept(QualifiedName::IVisitor* visitor) override;
	};

	class CallExpr : public TypeOrExpr, vl::reflection::Description<CallExpr>
	{
	public:
		vl::Ptr<TypeOrExpr> func;
		vl::collections::List<vl::Ptr<TypeOrExpr>> args;

		void Accept(TypeOrExpr::IVisitor* visitor) override;
	};

	class CtorExpr : public TypeOrExpr, vl::reflection::Description<CtorExpr>
	{
	public:
		vl::Ptr<TypeOrExpr> type;
		vl::collections::List<vl::Ptr<TypeOrExpr>> args;

		void Accept(TypeOrExpr::IVisitor* visitor) override;
	};

	class MulExpr : public TypeOrExpr, vl::reflection::Description<MulExpr>
	{
	public:
		vl::Ptr<TypeOrExpr> first;
		vl::Ptr<TypeOrExpr> second;

		void Accept(TypeOrExpr::IVisitor* visitor) override;
	};

	class ConstType : public TypeOrExpr, vl::reflection::Description<ConstType>
	{
	public:
		vl::Ptr<TypeOrExpr> type;

		void Accept(TypeOrExpr::IVisitor* visitor) override;
	};

	class PointerType : public TypeOrExpr, vl::reflection::Description<PointerType>
	{
	public:
		vl::Ptr<TypeOrExpr> type;

		void Accept(TypeOrExpr::IVisitor* visitor) override;
	};

	class FunctionType : public TypeOrExpr, vl::reflection::Description<FunctionType>
	{
	public:
		vl::Ptr<TypeOrExpr> returnType;
		vl::collections::List<vl::Ptr<TypeOrExpr>> args;

		void Accept(TypeOrExpr::IVisitor* visitor) override;
	};

	class TypeOrExprToResolve : public TypeOrExpr, vl::reflection::Description<TypeOrExprToResolve>
	{
	public:
		vl::collections::List<vl::Ptr<TypeOrExpr>> candidates;

		void Accept(TypeOrExpr::IVisitor* visitor) override;
	};
}
namespace vl
{
	namespace reflection
	{
		namespace description
		{
#ifndef VCZH_DEBUG_NO_REFLECTION
			DECL_TYPE_INFO(prefixsubset5::TypeOrExpr)
			DECL_TYPE_INFO(prefixsubset5::TypeOrExpr::IVisitor)
			DECL_TYPE_INFO(prefixsubset5::QualifiedName)
			DECL_TYPE_INFO(prefixsubset5::QualifiedName::IVisitor)
			DECL_TYPE_INFO(prefixsubset5::Name)
			DECL_TYPE_INFO(prefixsubset5::MemberName)
			DECL_TYPE_INFO(prefixsubset5::CallExpr)
			DECL_TYPE_INFO(prefixsubset5::CtorExpr)
			DECL_TYPE_INFO(prefixsubset5::MulExpr)
			DECL_TYPE_INFO(prefixsubset5::ConstType)
			DECL_TYPE_INFO(prefixsubset5::PointerType)
			DECL_TYPE_INFO(prefixsubset5::FunctionType)
			DECL_TYPE_INFO(prefixsubset5::TypeOrExprToResolve)

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(prefixsubset5::TypeOrExpr::IVisitor)
				void Visit(prefixsubset5::TypeOrExprToResolve* node) override
				{
					INVOKE_INTERFACE_PROXY(Visit, node);
				}

				void Visit(prefixsubset5::QualifiedName* node) override
				{
					INVOKE_INTERFACE_PROXY(Visit, node);
				}

				void Visit(prefixsubset5::CallExpr* node) override
				{
					INVOKE_INTERFACE_PROXY(Visit, node);
				}

				void Visit(prefixsubset5::CtorExpr* node) override
				{
					INVOKE_INTERFACE_PROXY(Visit, node);
				}

				void Visit(prefixsubset5::MulExpr* node) override
				{
					INVOKE_INTERFACE_PROXY(Visit, node);
				}

				void Visit(prefixsubset5::ConstType* node) override
				{
					INVOKE_INTERFACE_PROXY(Visit, node);
				}

				void Visit(prefixsubset5::PointerType* node) override
				{
					INVOKE_INTERFACE_PROXY(Visit, node);
				}

				void Visit(prefixsubset5::FunctionType* node) override
				{
					INVOKE_INTERFACE_PROXY(Visit, node);
				}

			END_INTERFACE_PROXY(prefixsubset5::TypeOrExpr::IVisitor)

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(prefixsubset5::QualifiedName::IVisitor)
				void Visit(prefixsubset5::Name* node) override
				{
					INVOKE_INTERFACE_PROXY(Visit, node);
				}

				void Visit(prefixsubset5::MemberName* node) override
				{
					INVOKE_INTERFACE_PROXY(Visit, node);
				}

			END_INTERFACE_PROXY(prefixsubset5::QualifiedName::IVisitor)

#endif
#endif
			/// <summary>Load all reflectable AST types, only available when <b>VCZH_DEBUG_NO_REFLECTION</b> is off.</summary>
			/// <returns>Returns true if this operation succeeded.</returns>
			extern bool PrefixSubset5TypeOrExprLoadTypes();
		}
	}
}
#endif
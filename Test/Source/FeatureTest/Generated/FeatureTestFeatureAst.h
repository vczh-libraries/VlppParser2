/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:FeatureAst
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_UNITTEST_FEATURETEST_FEATUREAST_AST
#define VCZH_PARSER2_UNITTEST_FEATURETEST_FEATUREAST_AST

#include "../../../../Source/AstBase.h"
#include "../../../../Source/SyntaxBase.h"

namespace featuretest
{
	class Feature;
	class FeatureToResolve;
	class NestedOptionalFeature;
	class OptionalFeature;
	class Plus;

	enum class OptionalProprity
	{
		UNDEFINED_ENUM_ITEM_VALUE = -1,
		Equal = 0,
		PreferTake = 1,
		PreferSkip = 2,
	};

	class Plus : public vl::glr::ParsingAstBase, vl::reflection::Description<Plus>
	{
	public:
	};

	class Feature abstract : public vl::glr::ParsingAstBase, vl::reflection::Description<Feature>
	{
	public:
		class IVisitor : public virtual vl::reflection::IDescriptable, vl::reflection::Description<IVisitor>
		{
		public:
			virtual void Visit(FeatureToResolve* node) = 0;
			virtual void Visit(OptionalFeature* node) = 0;
			virtual void Visit(NestedOptionalFeature* node) = 0;
		};

		virtual void Accept(Feature::IVisitor* visitor) = 0;

	};

	class OptionalFeature : public Feature, vl::reflection::Description<OptionalFeature>
	{
	public:
		OptionalProprity priority = OptionalProprity::UNDEFINED_ENUM_ITEM_VALUE;
		vl::Ptr<Plus> optional;
		vl::collections::List<vl::Ptr<Plus>> loop;

		void Accept(Feature::IVisitor* visitor) override;
	};

	class NestedOptionalFeature : public Feature, vl::reflection::Description<NestedOptionalFeature>
	{
	public:
		vl::Ptr<Plus> optional;
		vl::Ptr<Plus> tail1;
		vl::Ptr<Plus> tail2;
		vl::Ptr<Plus> tail3;
		vl::collections::List<vl::Ptr<Plus>> tails;

		void Accept(Feature::IVisitor* visitor) override;
	};

	class FeatureToResolve : public Feature, vl::reflection::Description<FeatureToResolve>
	{
	public:
		vl::collections::List<vl::Ptr<Feature>> candidates;

		void Accept(Feature::IVisitor* visitor) override;
	};
}
namespace vl
{
	namespace reflection
	{
		namespace description
		{
#ifndef VCZH_DEBUG_NO_REFLECTION
			DECL_TYPE_INFO(featuretest::Plus)
			DECL_TYPE_INFO(featuretest::Feature)
			DECL_TYPE_INFO(featuretest::Feature::IVisitor)
			DECL_TYPE_INFO(featuretest::OptionalProprity)
			DECL_TYPE_INFO(featuretest::OptionalFeature)
			DECL_TYPE_INFO(featuretest::NestedOptionalFeature)
			DECL_TYPE_INFO(featuretest::FeatureToResolve)

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA

			BEGIN_INTERFACE_PROXY_NOPARENT_SHAREDPTR(featuretest::Feature::IVisitor)
				void Visit(featuretest::FeatureToResolve* node) override
				{
					INVOKE_INTERFACE_PROXY(Visit, node);
				}

				void Visit(featuretest::OptionalFeature* node) override
				{
					INVOKE_INTERFACE_PROXY(Visit, node);
				}

				void Visit(featuretest::NestedOptionalFeature* node) override
				{
					INVOKE_INTERFACE_PROXY(Visit, node);
				}

			END_INTERFACE_PROXY(featuretest::Feature::IVisitor)

#endif
#endif
			/// <summary>Load all reflectable AST types, only available when <b>VCZH_DEBUG_NO_REFLECTION</b> is off.</summary>
			/// <returns>Returns true if this operation succeeded.</returns>
			extern bool FeatureTestFeatureAstLoadTypes();
		}
	}
}
#endif
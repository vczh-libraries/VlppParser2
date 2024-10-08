/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:RuleAst
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_PARSERGEN_RULEAST_AST_BUILDER
#define VCZH_PARSER2_PARSERGEN_RULEAST_AST_BUILDER

#include "ParserGenRuleAst.h"

namespace vl::glr::parsergen::builder
{
	class MakeAlternativeSyntax : public vl::glr::ParsingAstBuilder<GlrAlternativeSyntax>
	{
	public:
		MakeAlternativeSyntax& first(const vl::Ptr<GlrSyntax>& value);
		MakeAlternativeSyntax& second(const vl::Ptr<GlrSyntax>& value);
	};

	class MakeAndCondition : public vl::glr::ParsingAstBuilder<GlrAndCondition>
	{
	public:
		MakeAndCondition& first(const vl::Ptr<GlrCondition>& value);
		MakeAndCondition& second(const vl::Ptr<GlrCondition>& value);
	};

	class MakeAssignment : public vl::glr::ParsingAstBuilder<GlrAssignment>
	{
	public:
		MakeAssignment& field(const vl::WString& value);
		MakeAssignment& type(GlrAssignmentType value);
		MakeAssignment& value(const vl::WString& value);
	};

	class MakeCreateClause : public vl::glr::ParsingAstBuilder<GlrCreateClause>
	{
	public:
		MakeCreateClause& assignments(const vl::Ptr<GlrAssignment>& value);
		MakeCreateClause& syntax(const vl::Ptr<GlrSyntax>& value);
		MakeCreateClause& type(const vl::WString& value);
	};

	class MakeLeftRecursionInjectClause : public vl::glr::ParsingAstBuilder<GlrLeftRecursionInjectClause>
	{
	public:
		MakeLeftRecursionInjectClause& continuation(const vl::Ptr<GlrLeftRecursionInjectContinuation>& value);
		MakeLeftRecursionInjectClause& rule(const vl::Ptr<GlrRefSyntax>& value);
	};

	class MakeLeftRecursionInjectContinuation : public vl::glr::ParsingAstBuilder<GlrLeftRecursionInjectContinuation>
	{
	public:
		MakeLeftRecursionInjectContinuation& configuration(GlrLeftRecursionConfiguration value);
		MakeLeftRecursionInjectContinuation& flags(const vl::Ptr<GlrLeftRecursionPlaceholder>& value);
		MakeLeftRecursionInjectContinuation& injectionTargets(const vl::Ptr<GlrLeftRecursionInjectClause>& value);
		MakeLeftRecursionInjectContinuation& type(GlrLeftRecursionInjectContinuationType value);
	};

	class MakeLeftRecursionPlaceholder : public vl::glr::ParsingAstBuilder<GlrLeftRecursionPlaceholder>
	{
	public:
		MakeLeftRecursionPlaceholder& flag(const vl::WString& value);
	};

	class MakeLeftRecursionPlaceholderClause : public vl::glr::ParsingAstBuilder<GlrLeftRecursionPlaceholderClause>
	{
	public:
		MakeLeftRecursionPlaceholderClause& flags(const vl::Ptr<GlrLeftRecursionPlaceholder>& value);
	};

	class MakeLoopSyntax : public vl::glr::ParsingAstBuilder<GlrLoopSyntax>
	{
	public:
		MakeLoopSyntax& delimiter(const vl::Ptr<GlrSyntax>& value);
		MakeLoopSyntax& syntax(const vl::Ptr<GlrSyntax>& value);
	};

	class MakeNotCondition : public vl::glr::ParsingAstBuilder<GlrNotCondition>
	{
	public:
		MakeNotCondition& condition(const vl::Ptr<GlrCondition>& value);
	};

	class MakeOptionalSyntax : public vl::glr::ParsingAstBuilder<GlrOptionalSyntax>
	{
	public:
		MakeOptionalSyntax& priority(GlrOptionalPriority value);
		MakeOptionalSyntax& syntax(const vl::Ptr<GlrSyntax>& value);
	};

	class MakeOrCondition : public vl::glr::ParsingAstBuilder<GlrOrCondition>
	{
	public:
		MakeOrCondition& first(const vl::Ptr<GlrCondition>& value);
		MakeOrCondition& second(const vl::Ptr<GlrCondition>& value);
	};

	class MakePartialClause : public vl::glr::ParsingAstBuilder<GlrPartialClause>
	{
	public:
		MakePartialClause& assignments(const vl::Ptr<GlrAssignment>& value);
		MakePartialClause& syntax(const vl::Ptr<GlrSyntax>& value);
		MakePartialClause& type(const vl::WString& value);
	};

	class MakePrefixMergeClause : public vl::glr::ParsingAstBuilder<GlrPrefixMergeClause>
	{
	public:
		MakePrefixMergeClause& rule(const vl::Ptr<GlrRefSyntax>& value);
	};

	class MakePushConditionSyntax : public vl::glr::ParsingAstBuilder<GlrPushConditionSyntax>
	{
	public:
		MakePushConditionSyntax& switches(const vl::Ptr<GlrSwitchItem>& value);
		MakePushConditionSyntax& syntax(const vl::Ptr<GlrSyntax>& value);
	};

	class MakeRefCondition : public vl::glr::ParsingAstBuilder<GlrRefCondition>
	{
	public:
		MakeRefCondition& name(const vl::WString& value);
	};

	class MakeRefSyntax : public vl::glr::ParsingAstBuilder<GlrRefSyntax>
	{
	public:
		MakeRefSyntax& field(const vl::WString& value);
		MakeRefSyntax& literal(const vl::WString& value);
		MakeRefSyntax& refType(GlrRefType value);
	};

	class MakeReuseClause : public vl::glr::ParsingAstBuilder<GlrReuseClause>
	{
	public:
		MakeReuseClause& assignments(const vl::Ptr<GlrAssignment>& value);
		MakeReuseClause& syntax(const vl::Ptr<GlrSyntax>& value);
	};

	class MakeRule : public vl::glr::ParsingAstBuilder<GlrRule>
	{
	public:
		MakeRule& attParser(const vl::WString& value);
		MakeRule& attPublic(const vl::WString& value);
		MakeRule& clauses(const vl::Ptr<GlrClause>& value);
		MakeRule& name(const vl::WString& value);
		MakeRule& type(const vl::WString& value);
	};

	class MakeSequenceSyntax : public vl::glr::ParsingAstBuilder<GlrSequenceSyntax>
	{
	public:
		MakeSequenceSyntax& first(const vl::Ptr<GlrSyntax>& value);
		MakeSequenceSyntax& second(const vl::Ptr<GlrSyntax>& value);
	};

	class MakeSwitchItem : public vl::glr::ParsingAstBuilder<GlrSwitchItem>
	{
	public:
		MakeSwitchItem& name(const vl::WString& value);
		MakeSwitchItem& value(GlrSwitchValue value);
	};

	class MakeSyntaxFile : public vl::glr::ParsingAstBuilder<GlrSyntaxFile>
	{
	public:
		MakeSyntaxFile& rules(const vl::Ptr<GlrRule>& value);
		MakeSyntaxFile& switches(const vl::Ptr<GlrSwitchItem>& value);
	};

	class MakeTestConditionBranch : public vl::glr::ParsingAstBuilder<GlrTestConditionBranch>
	{
	public:
		MakeTestConditionBranch& condition(const vl::Ptr<GlrCondition>& value);
		MakeTestConditionBranch& syntax(const vl::Ptr<GlrSyntax>& value);
	};

	class MakeTestConditionSyntax : public vl::glr::ParsingAstBuilder<GlrTestConditionSyntax>
	{
	public:
		MakeTestConditionSyntax& branches(const vl::Ptr<GlrTestConditionBranch>& value);
	};

	class MakeUseSyntax : public vl::glr::ParsingAstBuilder<GlrUseSyntax>
	{
	public:
		MakeUseSyntax& name(const vl::WString& value);
	};

}
#endif
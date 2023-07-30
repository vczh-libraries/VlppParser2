#include "SyntaxSymbolWriter.h"
#include "../../Source/Ast/AstSymbol.h"
#include "../../Source/ParserGen_Generated/ParserGen_Assembler.h"
#include "../../Source/ParserGen_Generated/ParserGen_Lexer.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace syntax_writer;

			extern syntax_writer::Token		tok(ParserGenTokens id);
			extern syntax_writer::Token		tok(ParserGenTokens id, ParserGenFields field);

/***********************************************************************
CreateParserGenRuleSyntax
***********************************************************************/

			void CreateParserGenRuleSyntax(AstSymbolManager& ast, SyntaxSymbolManager& manager)
			{
				auto createRule = [&](const wchar_t* ruleName)
				{
					return manager.CreateRule(WString::Unmanaged(ruleName), -1, false, false);
				};

				manager.name = L"RuleParser";

				auto _cond0 = createRule(L"Cond0");
				auto _cond1 = createRule(L"Cond1");
				auto _cond2 = createRule(L"Cond2");
				auto _cond = createRule(L"Cond");
				auto _switchItem = createRule(L"SwitchItem");
				auto _switches = createRule(L"Switches");
				auto _optionalBody = createRule(L"OptionalBody");
				auto _testBranch = createRule(L"TestBranch");
				auto _token = createRule(L"Token");
				auto _syntax0 = createRule(L"Syntax0");
				auto _syntax1 = createRule(L"Syntax1");
				auto _syntax2 = createRule(L"Syntax2");
				auto _syntax = createRule(L"Syntax");
				auto _assignmentOp = createRule(L"AssignmentOp");
				auto _assignment = createRule(L"Assignment");
				auto _clause = createRule(L"Clause");
				auto _placeholder = createRule(L"Placeholder");
				auto _ruleName = createRule(L"RuleName");
				auto _lriConfig = createRule(L"LriConfig");
				auto _lriContinuationBody = createRule(L"LriContinuationBody");
				auto _lriContinuation = createRule(L"LriContinuation");
				auto _lriTarget = createRule(L"LriTarget");
				auto _rule = createRule(L"Rule");
				auto _file = createRule(L"File");

				_switches->isPartial = true;
				_optionalBody->isPartial = true;
				_token->isPartial = true;
				_assignmentOp->isPartial = true;
				_lriConfig->isPartial = true;
				_lriContinuationBody->isPartial = true;

				_file->isParser = true;
				_file->ruleType = dynamic_cast<AstClassSymbol*>(ast.Symbols()[L"SyntaxFile"]);

				using T = ParserGenTokens;
				using C = ParserGenClasses;
				using F = ParserGenFields;

				///////////////////////////////////////////////////////////////////////////////////
				// Condition
				///////////////////////////////////////////////////////////////////////////////////

				// ID:name as RefCondition
				Clause{ _cond0 } = create(tok(T::ID, F::RefCondition_name), C::RefCondition);

				// "(" !Cond ")"
				Clause{ _cond0 } = tok(T::OPEN_ROUND) + use(_cond) + tok(T::CLOSE_ROUND);

				// "!" Cond0:syntax as NotCondition
				Clause{ _cond0 } = create(tok(T::USE) + rule(_cond0, F::NotCondition_condition), C::NotCondition);

				// !Cond0
				Clause{ _cond1 } = use(_cond0);

				// Cond1:first "&&" Cond0:second as AndCondition
				Clause{ _cond1 } = create(rule(_cond1, F::AndCondition_first) + tok(T::AND) + rule(_cond0, F::AndCondition_second), C::AndCondition);

				// !Cond1
				Clause{ _cond2 } = use(_cond1);

				// Cond2:first "||" Cond1:second as OrCondition
				Clause{ _cond2 } = create(rule(_cond2, F::OrCondition_first) + tok(T::OR) + rule(_cond1, F::OrCondition_second), C::OrCondition);

				// !Cond2
				Clause{ _cond } = use(_cond2);

				///////////////////////////////////////////////////////////////////////////////////
				// Switch
				///////////////////////////////////////////////////////////////////////////////////

				// ID:name as SwitchItem {value = True}
				Clause{ _switchItem } = create(tok(T::ID, F::SwitchItem_name), C::SwitchItem).with(F::SwitchItem_value, GlrSwitchValue::True);

				// "!" ID:name as SwitchItem {value = False}
				Clause{ _switchItem } = create(tok(T::USE) + tok(T::ID, F::SwitchItem_name), C::SwitchItem).with(F::SwitchItem_value, GlrSwitchValue::False);

				// "switch" {SwitchItem:switches ; ","} ";" as partial File
				Clause{ _switches } = partial(tok(T::SWITCH) + loop(rule(_switchItem, F::SyntaxFile_switches), tok(T::COMMA)) + tok(T::SEMICOLON));

				///////////////////////////////////////////////////////////////////////////////////
				// Syntax (primitive)
				///////////////////////////////////////////////////////////////////////////////////

				// "[" Syntax:syntax "]" as partial OptionalSyntax
				Clause{ _optionalBody } = partial(tok(T::OPEN_SQUARE) + rule(_syntax, F::OptionalSyntax_syntax) + tok(T::CLOSE_SQUARE));

				// ID:literal as partial RefSyntax {refType = Id}
				Clause{ _token } = partial(tok(T::ID, F::RefSyntax_literal)).with(F::RefSyntax_refType, GlrRefType::Id);

				// STRING:literal as partial RefSyntax {refType = Literal}
				Clause{ _token } = partial(tok(T::STRING, F::RefSyntax_literal)).with(F::RefSyntax_refType, GlrRefType::Literal);

				// CONDITIONAL_LITERAL:literal as partial RefSyntax {refType = ConditionalLiteral}
				Clause{ _token } = partial(tok(T::CONDITIONAL_LITERAL, F::RefSyntax_literal)).with(F::RefSyntax_refType, GlrRefType::ConditionalLiteral);

				// Token [":" ID:field] as RefSyntax
				Clause{ _syntax0 } = create(prule(_token) + opt(tok(T::COLON) + tok(T::ID, F::RefSyntax_field)), C::RefSyntax);

				// "!" ID:name as UseSyntax
				Clause{ _syntax0 } = create(tok(T::USE) + tok(T::ID, F::UseSyntax_name), C::UseSyntax);

				// "{" Syntax:syntax [";" syntax:delimiter] "}" as LoopSyntax
				Clause{ _syntax0 } = create(tok(T::OPEN_CURLY) + rule(_syntax, F::LoopSyntax_syntax) + opt(tok(T::SEMICOLON) + rule(_syntax, F::LoopSyntax_delimiter)) + tok(T::CLOSE_CURLY), C::LoopSyntax);

				// "+" OptionalBody as OptionalSyntax {priority = PreferTake}
				Clause{ _syntax0 } = create(tok(T::POSITIVE) + prule(_optionalBody), C::OptionalSyntax).with(F::OptionalSyntax_priority, GlrOptionalPriority::PreferTake);

				// "-" OptionalBody as OptionalSyntax {priority = PreferSkip}
				Clause{ _syntax0 } = create(tok(T::NEGATIVE) + prule(_optionalBody), C::OptionalSyntax).with(F::OptionalSyntax_priority, GlrOptionalPriority::PreferSkip);

				// OptionalBody as OptionalSyntax {priority = Equal}
				Clause{ _syntax0 } = create(prule(_optionalBody), C::OptionalSyntax).with(F::OptionalSyntax_priority, GlrOptionalPriority::Equal);

				///////////////////////////////////////////////////////////////////////////////////
				// Syntax (conditional)
				///////////////////////////////////////////////////////////////////////////////////

				// "!(" {SwitchItem:switches ; ","} ";" Syntax:syntax ")" as PushConditionSyntax
				Clause{ _syntax0 } = create(tok(T::OPEN_PUSH) + loop(rule(_switchItem, F::PushConditionSyntax_switches), tok(T::COMMA)) + tok(T::SEMICOLON) + rule(_syntax, F::PushConditionSyntax_syntax) + tok(T::CLOSE_ROUND), C::PushConditionSyntax);

				// Condition:condition ":" (Syntax1:syntax | ";") as TestConditionBranch
				Clause{ _testBranch } = create(rule(_cond, F::TestConditionBranch_condition) + tok(T::COLON) + (rule(_syntax1, F::TestConditionBranch_syntax) | tok(T::SEMICOLON)), C::TestConditionBranch);

				// "?(" TestBranch:branches {"|" TestBranch:branches} ")" as TestConditionSyntax
				Clause{ _syntax0 } = create(tok(T::OPEN_TEST) + rule(_testBranch, F::TestConditionSyntax_branches) + loop(tok(T::ALTERNATIVE) + rule(_testBranch, F::TestConditionSyntax_branches)) + tok(T::CLOSE_ROUND), C::TestConditionSyntax);

				///////////////////////////////////////////////////////////////////////////////////
				// Syntax (others)
				///////////////////////////////////////////////////////////////////////////////////

				// "(" !Syntax ")"
				Clause{ _syntax0 } = tok(T::OPEN_ROUND) + use(_syntax) + tok(T::CLOSE_ROUND);

				// !Syntax0
				Clause{ _syntax1 } = use(_syntax0);

				// Syntax1:first Syntax0:second as SequenceSyntax
				Clause{ _syntax1 } = create(rule(_syntax1, F::SequenceSyntax_first) + rule(_syntax0, F::SequenceSyntax_second), C::SequenceSyntax);

				// !Syntax1
				Clause{ _syntax2 } = use(_syntax1);

				// Syntax2:first "|" Syntax1:second as AlternativeSyntax
				Clause{ _syntax2 } = create(rule(_syntax2, F::AlternativeSyntax_first) + tok(T::ALTERNATIVE) + rule(_syntax1, F::AlternativeSyntax_second), C::AlternativeSyntax);

				// !Syntax2
				Clause{ _syntax } = use(_syntax2);

				///////////////////////////////////////////////////////////////////////////////////
				// Clause
				///////////////////////////////////////////////////////////////////////////////////

				// "=" as partial Assignment {type = Strong}
				Clause{ _assignmentOp } = partial(tok(T::ASSIGN)).with(F::Assignment_type, GlrAssignmentType::Strong);

				// "?=" as partial Assignment {type = Weak}
				Clause{ _assignmentOp } = partial(tok(T::WEAK_ASSIGN)).with(F::Assignment_type, GlrAssignmentType::Weak);

				// ID:field AssignmentOp STRING:value as Assignment
				Clause{ _assignment } = create(tok(T::ID, F::Assignment_field) + prule(_assignmentOp) + tok(T::ID, F::Assignment_value), C::Assignment);

				// Syntax:syntax "as" ID:type ["{" {Assignment:assignments ; ","} "}"] as CreateClause
				Clause{ _clause } = create(rule(_syntax, F::CreateClause_syntax) + tok(T::AS) + tok(T::ID, F::CreateClause_type) + opt(tok(T::OPEN_CURLY) + loop(rule(_assignment, F::CreateClause_assignments), tok(T::COMMA)) + tok(T::CLOSE_CURLY)), C::CreateClause);

				// Syntax:syntax "as" "partial" ID:type ["{" {Assignment:assignments ; ","} "}"] as PartialClause
				Clause{ _clause } = create(rule(_syntax, F::PartialClause_syntax) + tok(T::AS) + tok(T::PARTIAL) + tok(T::ID, F::PartialClause_type) + opt(tok(T::OPEN_CURLY) + loop(rule(_assignment, F::PartialClause_assignments), tok(T::COMMA)) + tok(T::CLOSE_CURLY)), C::PartialClause);

				// Syntax:syntax ["{" {Assignment:assignments ; ","} "}"] as ReuseClause
				Clause{ _clause } = create(rule(_syntax, F::ReuseClause_syntax) + opt(tok(T::OPEN_CURLY) + loop(rule(_assignment, F::ReuseClause_assignments), tok(T::COMMA)) + tok(T::CLOSE_CURLY)), C::ReuseClause);

				///////////////////////////////////////////////////////////////////////////////////
				// Clause (left recursive)
				///////////////////////////////////////////////////////////////////////////////////

				// ID:flag as LeftRecursionPlaceholder
				Clause{ _placeholder } = create(tok(T::ID, F::LeftRecursionPlaceholder_flag), C::LeftRecursionPlaceholder);

				// ID:literal as RefSyntax {refType = ID}
				Clause{ _ruleName } = create(tok(T::ID, F::RefSyntax_literal), C::RefSyntax).with(F::RefSyntax_refType, GlrRefType::Id);

				// "left_recursion_placeholder" "(" RuleName:flags {"," ruleName:flags} ")" as LeftRecursionPlaceholderClause
				Clause{ _clause } = create(
						tok(T::LS_PH) + tok(T::OPEN_ROUND)
						+ rule(_placeholder, F::LeftRecursionPlaceholderClause_flags)
						+ loop(tok(T::COMMA) + rule(_placeholder, F::LeftRecursionPlaceholderClause_flags))
						+ tok(T::CLOSE_ROUND),
					C::LeftRecursionPlaceholderClause);

				// "left_recursion_inject" as partial LeftRecursionInjectContinuation {configuration = Single}
				Clause{ _lriConfig } = partial(tok(T::LS_I)).with(F::LeftRecursionInjectContinuation_configuration, GlrLeftRecursionConfiguration::Single);

				// "left_recursion_inject_multiple" as partial LeftRecursionInjectContinuation {configuration = Multiple}
				Clause{ _lriConfig } = partial(tok(T::LS_IM)).with(F::LeftRecursionInjectContinuation_configuration, GlrLeftRecursionConfiguration::Multiple);

				// LriConfig "(" Placeholder:flag {"," Placeholder:flag} ")" LriTarget:injectionTargets {"|" LriTarget:injectionTargets} as partial LeftRecursionInjectContinuation
				Clause{ _lriContinuationBody } = partial(
					prule(_lriConfig) + tok(T::OPEN_ROUND)
					+ rule(_placeholder, F::LeftRecursionInjectContinuation_flags)
					+ loop(tok(T::COMMA) + rule(_placeholder, F::LeftRecursionInjectContinuation_flags))
					+ tok(T::CLOSE_ROUND)
					+ rule(_lriTarget, F::LeftRecursionInjectContinuation_injectionTargets)
					+ loop(tok(T::ALTERNATIVE) + rule(_lriTarget, F::LeftRecursionInjectContinuation_injectionTargets))
					);

				// LriContinuationBody as LeftRecursionInjectionContinuation {type = Required}
				Clause{ _lriContinuation } = create(prule(_lriContinuationBody), C::LeftRecursionInjectContinuation).with(F::LeftRecursionInjectContinuation_type, GlrLeftRecursionInjectContinuationType::Required);

				// "[" LriContinuationBody as LeftRecursionInjectionContinuation "]" {type = Optional}
				Clause{ _lriContinuation } = create(tok(T::OPEN_SQUARE) + prule(_lriContinuationBody) + tok(T::CLOSE_SQUARE), C::LeftRecursionInjectContinuation).with(F::LeftRecursionInjectContinuation_type, GlrLeftRecursionInjectContinuationType::Optional);

				// RuleName:rule as LeftRecursionInjectClause
				Clause{ _lriTarget } = create(rule(_ruleName, F::LeftRecursionInjectClause_rule), C::LeftRecursionInjectClause);

				// "(" RuleName:rule LriContinuation:continuation ")" as LeftRecursionInjectClause
				Clause{ _lriTarget } = create(tok(T::OPEN_ROUND) + rule(_ruleName, F::LeftRecursionInjectClause_rule) + rule(_lriContinuation, F::LeftRecursionInjectClause_continuation) + tok(T::CLOSE_ROUND), C::LeftRecursionInjectClause);

				// "!" RuleName:rule LriContinuation:continuation as LeftRecursionInjectClause
				Clause{ _clause } = create(tok(T::USE) + rule(_ruleName, F::LeftRecursionInjectClause_rule) + rule(_lriContinuation, F::LeftRecursionInjectClause_continuation), C::LeftRecursionInjectClause);

				// "!" "prefix_merge" "(" RuleName:rule ")" as PrefixMergeClause
				Clause{ _clause } = create(tok(T::USE) + tok(T::LS_PM) + tok(T::OPEN_ROUND) + rule(_ruleName, F::PrefixMergeClause_rule) + tok(T::CLOSE_ROUND), C::PrefixMergeClause);

				///////////////////////////////////////////////////////////////////////////////////
				// File
				///////////////////////////////////////////////////////////////////////////////////

				// ["@public"] ["@parser"] ID:name {"::=" Clause:clauses} ";" as Rule
				Clause{ _rule } = create(opt(tok(T::ATT_PUBLIC, F::Rule_attPublic)) + opt(tok(T::ATT_PARSER, F::Rule_attParser)) + tok(T::ID, F::Rule_name) + opt(tok(T::COLON) + tok(T::ID, F::Rule_type)) + loop(tok(T::INFER) + rule(_clause, F::Rule_clauses)) + tok(T::SEMICOLON), C::Rule);

				// [Switches] Rule:rules {Rule:rules} as SyntaxFile
				Clause{ _file } = create(opt(prule(_switches)) + rule(_rule, F::SyntaxFile_rules) + loop(rule(_rule, F::SyntaxFile_rules)), C::SyntaxFile);
			}
		}
	}
}
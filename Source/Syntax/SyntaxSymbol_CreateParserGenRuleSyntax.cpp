#include "SyntaxSymbolWriter.h"
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

			void CreateParserGenRuleSyntax(SyntaxSymbolManager& manager)
			{
				manager.name = L"RuleParser";

				auto _syntax0 = manager.CreateRule(L"Syntax0");
				auto _syntax1 = manager.CreateRule(L"Syntax1");
				auto _syntax2 = manager.CreateRule(L"Syntax2");
				auto _syntax = manager.CreateRule(L"Syntax");
				auto _assignment = manager.CreateRule(L"Assignment");
				auto _clause = manager.CreateRule(L"Clause");
				auto _rule = manager.CreateRule(L"Rule");
				auto _file = manager.CreateRule(L"File");

				manager.parsableRules.Add(_file);
				manager.ruleTypes.Add(_file, L"vl::glr::parsergen::GlrSyntaxFile");

				using T = ParserGenTokens;
				using C = ParserGenClasses;
				using F = ParserGenFields;

				// ID:name [":" ID:field] as RefSyntax
				Clause{ _syntax0 } = create(tok(T::ID, F::RefSyntax_name) + opt(tok(T::COLON) + tok(T::ID, F::RefSyntax_field)), C::RefSyntax);

				// STRING:value as LiteralSyntax
				Clause{ _syntax0 } = create(tok(T::STRING, F::LiteralSyntax_value), C::LiteralSyntax);

				// "!" ID:name as UseSyntax
				Clause{ _syntax0 } = create(tok(T::USE) + tok(T::ID, F::UseSyntax_name), C::UseSyntax);

				// "{" Syntax:syntax [";" syntax:delimiter] "}" as LoopSyntax
				Clause{ _syntax0 } = create(tok(T::OPEN_CURLY) + rule(_syntax, F::LoopSyntax_syntax) + opt(tok(T::SEMICOLON) + rule(_syntax, F::LoopSyntax_delimiter)) + tok(T::CLOSE_CURLY), C::LoopSyntax);

				// "[" Syntax:syntax "]" as OptionalSyntax
				Clause{ _syntax0 } = create(tok(T::OPEN_SQUARE) + rule(_syntax, F::OptionalSyntax_syntax) + tok(T::CLOSE_SQUARE), C::OptionalSyntax);

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

				// ID:field "=" STRING:value as partial Assignment
				Clause{ _assignment } = partial(tok(T::ID, F::Assignment_field) + tok(T::ASSIGN) + tok(T::ID, F::Assignment_value));

				// Syntax:syntax "as" ID:type ["{" {Assignment:assignments} "}"] as CreateClause
				Clause{ _clause } = create(rule(_syntax, F::CreateClause_syntax) + tok(T::AS) + tok(T::ID, F::CreateClause_type) + opt(tok(T::OPEN_CURLY) + loop(rule(_assignment, F::CreateClause_assignments)) + tok(T::CLOSE_CURLY)), C::CreateClause);

				// Syntax:syntax "as" "partial" ID:type ["{" {Assignment:assignments} "}"] as PartialClause
				Clause{ _clause } = create(rule(_syntax, F::PartialClause_syntax) + tok(T::AS) + tok(T::PARTIAL) + tok(T::ID, F::PartialClause_type) + opt(tok(T::OPEN_CURLY) + loop(rule(_assignment, F::PartialClause_assignments)) + tok(T::CLOSE_CURLY)), C::PartialClause);

				// Syntax:syntax ["{" {Assignment:assignments} "}"] as ReuseClause
				Clause{ _clause } = create(rule(_syntax, F::_ReuseClause_syntax) + opt(tok(T::OPEN_CURLY) + loop(rule(_assignment, F::_ReuseClause_assignments)) + tok(T::CLOSE_CURLY)), C::_ReuseClause);

				// ID:name {"::=" Clause:clauses} ";" as Rule
				Clause{ _rule } = create(tok(T::ID, F::Rule_name) + loop(tok(T::INFER) + rule(_clause, F::Rule_clauses)) + tok(T::SEMICOLON), C::Rule);

				// Rule:rules {Rule:rules} as SyntaxFile
				Clause{ _file } = create(rule(_rule, F::SyntaxFile_rules) + loop(rule(_rule, F::SyntaxFile_rules)), C::SyntaxFile);
			}
		}
	}
}
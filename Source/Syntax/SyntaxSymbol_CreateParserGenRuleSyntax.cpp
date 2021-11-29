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

				// ID:name [":" ID:field] as RefSyntax
				Clause{ _syntax0 } = ;

				// STRING:value as LiteralSyntax
				Clause{ _syntax0 } = ;

				// "!" ID:name as UseSyntax
				Clause{ _syntax0 } = ;

				// "{" Syntax:syntax [":" syntax:delimiter] "}" as LoopSyntax
				Clause{ _syntax0 } = ;

				// "[" Syntax:syntax "]" as OptionalSyntax
				Clause{ _syntax0 } = ;

				// !Syntax0
				Clause{ _syntax1 } = ;

				// Syntax1:first Syntax0:second as SequenceSyntax
				Clause{ _syntax1 } = ;

				// !Syntax1
				Clause{ _syntax2 } = ;

				// Syntax2:first "|" Syntax1:second as AlternativeSyntax
				Clause{ _syntax2 } = ;

				// !Syntax2
				Clause{ _syntax } = ;

				// ID:field "=" STRING:value as partial Assignment
				Clause{ _assignment } = ;

				// Syntax:syntax "as" ID:type ["{" {Assignment:assignments} "}"] as CreateClause
				Clause{ _clause } = ;

				// Syntax:syntax "as" "partial" ID:type ["{" {Assignment:assignments} "}"] as PartialClause
				Clause{ _clause } = ;

				// Syntax:syntax ["{" {Assignment:assignments} "}"] as ReuseClause
				Clause{ _clause } = ;

				// ID:name {"::=" Clause:clauses} ";" as Rule
				Clause{ _rule } = ;

				// Rule:rules {Rule:rules}
				Clause{ _file } = ;
			}
		}
	}
}
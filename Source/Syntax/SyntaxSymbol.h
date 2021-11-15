/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_SYNTAX_SYNTAXSYMBOL
#define VCZH_PARSER2_SYNTAX_SYNTAXSYMBOL

#include "../ParserGen/ParserSymbol.h"
#include "../../../Source/SyntaxBase.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			class SyntaxSymbolManager;

/***********************************************************************
SyntaxSymbolManager
***********************************************************************/

			class RuleSymbol : public Object
			{
				friend class SyntaxSymbolManager;
			protected:
				WString						name;

				RuleSymbol(const WString& _name);
			public:

				const WString&				Name();
			};

			class SyntaxSymbolManager : public Object
			{
			protected:
				MappedOwning<RuleSymbol>	rules;
				ParserSymbolManager&		global;

			public:
				SyntaxSymbolManager(ParserSymbolManager& _global);

				RuleSymbol*					CreateRule(const WString& name);

				ParserSymbolManager&		Global() { return global; }
				const auto&					Rules() { return rules.map; }
				const auto&					RuleOrder() { return rules.order; }
			};

			extern void						CreateParserGenSyntax(SyntaxSymbolManager& manager);
		}
	}
}

#endif
/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:PrefixSubset
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_UNITTEST_PREFIXSUBSET_MODULEPARSER_SYNTAX
#define VCZH_PARSER2_UNITTEST_PREFIXSUBSET_MODULEPARSER_SYNTAX

#include "PrefixSubset_Assembler.h"
#include "PrefixSubset_Lexer.h"

namespace prefixsubset
{
	enum class ModuleParserStates
	{
		_Name = 0,
		_ShortType = 6,
		_LongType = 11,
		_Expr = 23,
		Module = 34,
	};

	const wchar_t* ModuleParserRuleName(vl::vint index);
	const wchar_t* ModuleParserStateLabel(vl::vint index);
	const wchar_t* ModuleParserSwitchName(vl::vint index);
	extern void PrefixSubsetModuleParserData(vl::stream::IStream& outputStream);

	class ModuleParser
		: public vl::glr::ParserBase<PrefixSubsetTokens, ModuleParserStates, PrefixSubsetAstInsReceiver>
		, protected vl::glr::automaton::IExecutor::ITypeCallback
	{
	protected:
		vl::vint32_t FindCommonBaseClass(vl::vint32_t class1, vl::vint32_t class2) const override;
	public:
		ModuleParser();

		vl::Ptr<prefixsubset::TypeOrExpr> ParseModule(const vl::WString& input, vl::vint codeIndex = -1) const;
		vl::Ptr<prefixsubset::TypeOrExpr> ParseModule(vl::collections::List<vl::regex::RegexToken>& tokens, vl::vint codeIndex = -1) const;
	};
}
#endif
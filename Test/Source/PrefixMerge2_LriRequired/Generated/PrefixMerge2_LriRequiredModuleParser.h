/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:PrefixMerge2_LriRequired
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_UNITTEST_PREFIXMERGE2_LRIREQUIRED_MODULEPARSER_SYNTAX
#define VCZH_PARSER2_UNITTEST_PREFIXMERGE2_LRIREQUIRED_MODULEPARSER_SYNTAX

#include "PrefixMerge2_LriRequired_Assembler.h"
#include "PrefixMerge2_LriRequired_Lexer.h"

namespace prefixmerge2_lrirequired
{
	enum class ModuleParserStates
	{
		_Name = 0,
		_PrimitiveShared = 6,
		_PrimitiveTypeOnly = 9,
		_PrimitiveExprOnly = 15,
		_LongType = 20,
		_Expr0 = 35,
		_Expr1 = 41,
		_Expr = 49,
		Module = 55,
	};

	const wchar_t* ModuleParserRuleName(vl::vint index);
	const wchar_t* ModuleParserStateLabel(vl::vint index);
	const wchar_t* ModuleParserSwitchName(vl::vint index);
	extern void PrefixMerge2_LriRequiredModuleParserData(vl::stream::IStream& outputStream);

	class ModuleParser
		: public vl::glr::ParserBase<PrefixMerge2_LriRequiredTokens, ModuleParserStates, PrefixMerge2_LriRequiredAstInsReceiver>
		, protected vl::glr::automaton::IExecutor::ITypeCallback
	{
	protected:
		vl::vint32_t FindCommonBaseClass(vl::vint32_t class1, vl::vint32_t class2) const override;
	public:
		ModuleParser();

		vl::Ptr<prefixmerge2_lrirequired::TypeOrExpr> ParseModule(const vl::WString& input, vl::vint codeIndex = -1) const;
		vl::Ptr<prefixmerge2_lrirequired::TypeOrExpr> ParseModule(vl::collections::List<vl::regex::RegexToken>& tokens, vl::vint codeIndex = -1) const;
	};
}
#endif
/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:IfElseSwitch
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_UNITTEST_IFELSESWITCH_AST_ASSEMBLER
#define VCZH_PARSER2_UNITTEST_IFELSESWITCH_AST_ASSEMBLER

#include "IfElseSwitchStatAst.h"

namespace ifelseswitch
{
	enum class IfElseSwitchClasses : vl::vint32_t
	{
		BlockStat = 0,
		DoStat = 1,
		IfStat = 2,
		Module = 3,
		Stat = 4,
	};

	enum class IfElseSwitchFields : vl::vint32_t
	{
		BlockStat_stats = 0,
		IfStat_elseBranch = 1,
		IfStat_thenBranch = 2,
		Module_stat = 3,
	};

	extern const wchar_t* IfElseSwitchTypeName(IfElseSwitchClasses type);
	extern const wchar_t* IfElseSwitchCppTypeName(IfElseSwitchClasses type);
	extern const wchar_t* IfElseSwitchFieldName(IfElseSwitchFields field);
	extern const wchar_t* IfElseSwitchCppFieldName(IfElseSwitchFields field);

	class IfElseSwitchAstInsReceiver : public vl::glr::AstInsReceiverBase
	{
	protected:
		vl::Ptr<vl::glr::ParsingAstBase> CreateAstNode(vl::vint32_t type) override;
		void SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, vl::Ptr<vl::glr::ParsingAstBase> value) override;
		void SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, const vl::regex::RegexToken& token, vl::vint32_t tokenIndex) override;
		void SetField(vl::glr::ParsingAstBase* object, vl::vint32_t field, vl::vint32_t enumItem, bool weakAssignment) override;
		vl::Ptr<vl::glr::ParsingAstBase> ResolveAmbiguity(vl::vint32_t type, vl::collections::Array<vl::Ptr<vl::glr::ParsingAstBase>>& candidates) override;
	};
}
#endif
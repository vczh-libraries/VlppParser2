/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:PrefixSubset
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_UNITTEST_PREFIXSUBSET_LEXER
#define VCZH_PARSER2_UNITTEST_PREFIXSUBSET_LEXER

#include "../../../../Source/AstBase.h"
#include "../../../../Source/SyntaxBase.h"

namespace prefixsubset
{
	enum class PrefixSubsetTokens : vl::vint32_t
	{
		OPEN_ROUND = 0,
		CLOSE_ROUND = 1,
		COMMA = 2,
		DOT = 3,
		CONST = 4,
		ASTERISK = 5,
		ID = 6,
		SPACE = 7,
	};

	constexpr vl::vint PrefixSubsetTokenCount = 8;
	extern bool PrefixSubsetTokenDeleter(vl::vint token);
	extern const wchar_t* PrefixSubsetTokenId(PrefixSubsetTokens token);
	extern const wchar_t* PrefixSubsetTokenDisplayText(PrefixSubsetTokens token);
	extern const wchar_t* PrefixSubsetTokenRegex(PrefixSubsetTokens token);
	extern void PrefixSubsetLexerData(vl::stream::IStream& outputStream);
}
#endif
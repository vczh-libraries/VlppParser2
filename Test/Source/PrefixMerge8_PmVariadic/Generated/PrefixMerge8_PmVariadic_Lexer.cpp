/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:PrefixMerge8_PmVariadic
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "PrefixMerge8_PmVariadic_Lexer.h"

namespace prefixmerge8_pmvariadic
{
	bool PrefixMerge8_PmVariadicTokenDeleter(vl::vint token)
	{
		switch((PrefixMerge8_PmVariadicTokens)token)
		{
		case PrefixMerge8_PmVariadicTokens::SPACE:
			return true;
		default:
			return false;
		}
	}

	const wchar_t* PrefixMerge8_PmVariadicTokenId(PrefixMerge8_PmVariadicTokens token)
	{
		static const wchar_t* results[] = {
			L"OPEN_ROUND",
			L"CLOSE_ROUND",
			L"OPEN_BRACE",
			L"CLOSE_BRACE",
			L"LT",
			L"GT",
			L"COMMA",
			L"VARIADIC",
			L"DOT",
			L"CONST",
			L"ASTERISK",
			L"ID",
			L"SPACE",
		};
		vl::vint index = (vl::vint)token;
		return 0 <= index && index < PrefixMerge8_PmVariadicTokenCount ? results[index] : nullptr;
	}

	const wchar_t* PrefixMerge8_PmVariadicTokenDisplayText(PrefixMerge8_PmVariadicTokens token)
	{
		static const wchar_t* results[] = {
			L"(",
			L")",
			L"{",
			L"}",
			L"<",
			L">",
			L",",
			L"...",
			L".",
			L"const",
			L"*",
			nullptr,
			nullptr,
		};
		vl::vint index = (vl::vint)token;
		return 0 <= index && index < PrefixMerge8_PmVariadicTokenCount ? results[index] : nullptr;
	}

	const wchar_t* PrefixMerge8_PmVariadicTokenRegex(PrefixMerge8_PmVariadicTokens token)
	{
		static const wchar_t* results[] = {
			L"/(",
			L"/)",
			L"/{",
			L"/}",
			L"/<",
			L"/>",
			L",",
			L"...",
			L".",
			L"const",
			L"/*",
			L"[a-zA-Z_]/w*",
			L"/s+",
		};
		vl::vint index = (vl::vint)token;
		return 0 <= index && index < PrefixMerge8_PmVariadicTokenCount ? results[index] : nullptr;
	}

	void PrefixMerge8_PmVariadicLexerData(vl::stream::IStream& outputStream)
	{
		static const vl::vint dataLength = 362; // 2491 bytes before compressing
		static const vl::vint dataBlock = 256;
		static const vl::vint dataRemain = 106;
		static const vl::vint dataSolidRows = 1;
		static const vl::vint dataRows = 2;
		static const char* compressed[] = {
			"\xBB\x09\x00\x00\x62\x01\x00\x00\x15\x00\x01\x9A\x01\x84\x81\x81\x0C\x82\x09\x08\x84\x8A\x0B\x84\x81\x06\x87\x04\xA0\x11\x84\x88\x14\x88\x83\x14\x17\x84\xAA\x1A\x84\x84\x15\x8E\x82\x2E\x20\x84\x90\x14\x81\x1C\x82\x1E\x27\x84\xBE\x0A\x94\x81\x20\x82\x2D\x04\xDF\x31\x84\x81\x34\x82\x30\x82\x63\x38\x84\x84\x34\x85\x34\x82\x37\x3F\x84\xAF\x22\xA4\x80\x38\x82\x39\x04\xF3\x09\xA4\x84\x3C\xA4\x83\x3A\x04\xFA\x04\x9B\x33\xAC\x81\x3E\xAB\x04\x8C\x19\xBB\xA1\x82\xAD\x81\x00\x04\x82\x0D\x81\x87\x04\x80\x04\x82\xFF\x69\xBF\x65\x04\x86\x00\x83\x05\x6F\xF1\x81\x89\x81\x80\xB9\xBA\xB9\x00\x76\xB9\xB8\xBA\xBB\x00\x82\x02\x68\xEA\x82\xC3\xC4\xC5\xC2\xC3\xC3\x88\x89\xCA\xCB\xCC\xC5\xC6\xC7\xC7\x90\x91\xD2\xD3\xC4\xCD\xCA\xCB\xCB\x98\x99\xDA\xDB\xCC\xCD\xCE\xCF\xCF\xA0\xA1\xE2\xC3\xD4\xD7\xCB\x87\x80\xA5\xA9\xD7\xCF\x01\xC4\xC0\xD7\x80\xAF\x80\x31\xD3\xD4\x81\xDA\x07\xDA\xB8\xB0\xF9\xC0\x0A\xD5\xDE\xDE\xCC\xB1\x86\xF4\xDA\xD4\xE2\xDB\xDD\xE1\xC6\xC5\xC1\xE9\xC0\x0C\x81\xE7\x80\xCF\x80\x11\xFF\xD4\xED\xCD\x09\xD6\xD5\xD9\xC2\xF8",
			"\xE2\xC0\xE6\xEE\x82\x12\x47\xE2\xE5\xEA\xEB\xE6\xD7\xF1\xC9\xE9\xDF\xEA\xEB\xDA\xC5\xE9\xF7\xCE\xDC\xDA\xF3\xF4\xFD\xFB\xD4\xF9\x69\xEB\xEA\xE8\xF1\xDB\x08\xF5\xF2\xF8\x7F\x3A\x7F\x80\xE8\x54\x0D\x79\x76\xE6\x43\x64\x80\x7B\x0C\xBB\x74\x79\x62\xFE\x1F\x3F\x04\x40\x05\x55\x80\x00\x58\x01\x62\x51\x42\x5F\x01\x40\x61\x40\x5B\x01\x6E\x51\x43\x59\x77\x41\x4C\x41\x89\x00\x1E\x50\x03\x69\x7B\x68\x8A\x89\x59\x28\xBA\x54\x5C\x00",
		};
		vl::glr::DecompressSerializedData(compressed, true, dataSolidRows, dataRows, dataBlock, dataRemain, outputStream);
	}
}
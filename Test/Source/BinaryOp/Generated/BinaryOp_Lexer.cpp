/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:BinaryOp
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "BinaryOp_Lexer.h"

namespace binaryop
{
	bool BinaryOpTokenDeleter(vl::vint token)
	{
		switch((BinaryOpTokens)token)
		{
		case BinaryOpTokens::SPACE:
			return true;
		default:
			return false;
		}
	}

	const wchar_t* BinaryOpTokenId(BinaryOpTokens token)
	{
		static const wchar_t* results[] = {
			L"ADD",
			L"MUL",
			L"EXP",
			L"ASSIGN",
			L"TRY",
			L"DOLLAR",
			L"OPEN",
			L"CLOSE",
			L"ID",
			L"SPACE",
		};
		vl::vint index = (vl::vint)token;
		return 0 <= index && index < BinaryOpTokenCount ? results[index] : nullptr;
	}

	const wchar_t* BinaryOpTokenDisplayText(BinaryOpTokens token)
	{
		static const wchar_t* results[] = {
			L"+",
			L"*",
			L"^",
			L"=",
			L"?",
			L"$",
			L"(",
			L")",
			nullptr,
			nullptr,
		};
		vl::vint index = (vl::vint)token;
		return 0 <= index && index < BinaryOpTokenCount ? results[index] : nullptr;
	}

	const wchar_t* BinaryOpTokenRegex(BinaryOpTokens token)
	{
		static const wchar_t* results[] = {
			L"/+",
			L"/*",
			L"/^",
			L"/=",
			L"/?",
			L"/$",
			L"/(",
			L"/)",
			L"[a-zA-Z_]/w*",
			L"/s+",
		};
		vl::vint index = (vl::vint)token;
		return 0 <= index && index < BinaryOpTokenCount ? results[index] : nullptr;
	}

	void BinaryOpLexerData(vl::stream::IStream& outputStream)
	{
		static const vl::vint dataLength = 216; // 1086 bytes before compressing
		static const vl::vint dataBlock = 256;
		static const vl::vint dataRemain = 216;
		static const vl::vint dataSolidRows = 0;
		static const vl::vint dataRows = 1;
		static const char* compressed[] = {
			"\x3E\x04\x00\x00\xD0\x00\x00\x00\x0D\x00\x01\x91\x01\x84\x81\x80\x08\x82\x09\x08\x84\x8A\x0B\x84\x80\x81\x80\x0F\x80\x20\x04\x83\x89\x80\x12\x82\x17\x81\xA8\x04\x8B\x89\x81\x14\x82\x1F\x81\xAA\x04\x83\x91\x83\x14\x82\x27\x81\xB0\x04\x89\x1C\x81\x1E\x97\x04\xBF\x31\x84\x81\x24\x82\x2C\x82\x5E\x38\x84\x9F\x2B\x9C\x81\x30\x82\x7A\x0D\x81\x8C\x83\xA2\xA0\x00\x03\x04\x87\x04\x88\x04\x82\x00\x83\x00\x04\xFF\x52\xBF\x74\x04\x81\x02\x85\x01\x83\x18\xA0\x09\x81\x83\xA9\xAF\x60\xE1\xA2\xA3\xB4\xB5\xB2\xB3\xB3\x68\xE9\xAA\xAB\xBC\xB5\xB6\xB7\xB7\x70\xF1\xB2\xB3\xB4\xBD\xBA\xBB\xBB\x78\xF9\xBA\xBB\xBC\xBD\xBB\x05\xA8\x62\xFF\x9E\xB3\xA2\xC0\x01\xC3\xAF\x0C\x04\x89\xC1\x8B\xC0\x01\xC7\xBE\x90\xEE\x87\xC1\xC0\xC2\xA9\xC3\xC9\x52\x8D\xDA\xCA\xC5\xC9\xC8\xB4\x7F\x1F\x11\x85\x85\x80\xA9\x82\xA7\xAC\x04\xD5\x81\x97\xA1\x80\xA5\x80\xA5\x01\xCC\x81\x9D\xA0\x02\xD8\xAE\x82",
		};
		vl::glr::DecompressSerializedData(compressed, true, dataSolidRows, dataRows, dataBlock, dataRemain, outputStream);
	}
}
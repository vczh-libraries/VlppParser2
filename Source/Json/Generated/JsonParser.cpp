/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:Json
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "JsonParser.h"

namespace vl
{
	namespace glr
	{
		namespace json
		{
			void JsonParserData(vl::stream::IStream& outputStream)
			{
				static const vl::vint dataLength = 1778; // 15497 bytes before compressing
				static const vl::vint dataBlock = 256;
				static const vl::vint dataRemain = 242;
				static const vl::vint dataSolidRows = 6;
				static const vl::vint dataRows = 7;
				static const char* compressed[] = {
					"\x89\x3C\x00\x00\xEA\x06\x00\x00\x0C\x00\x01\x82\x80\x06\x03\x82\x81\x82\x06\x89\x82\x87\x0A\x80\x81\x84\x09\x0A\x98\x0A\x9D\x0A\x86\x65\x01\x84\xFF\x19\x9A\x99\x8A\x80\x03\x8D\x8D\x1D\x9D\x97\x89\x83\x96\x81\x93\x81\x02\x0A\xA7\x82\x8F\x8A\x8D\x8F\x96\x1C\x8A\xB0\x9F\x7F\x90\x99\x9B\x96\x37\x9F\x9D\x83\x0A\x92\x84\x03\x9E\x18\xB6\xB2\x82\xA1\xA0\x9F\xA0\xA3\x45\xBA\x87\xAA\xA9\xA0\x93\xA4\xA7\x4D\xAE\x8F\xB2\xA1\xA9\x99\xAA\x9A\x53\xD6\x86\x93\x99\x98\xAA\x83\x02\x40\xDB\x84\xA2\xB8\xA4\xB0\xA6\xB3\x5E\x83\x9C\xB9\xA8\xAF\xA9\xAE\xAA\x68\xF0\xAA\xA3\xB3\xBD\xB1\xBB\xB3\x77\xE9\x87\x81\xB9\xBA\xB8\x96\xBA\x7F\xF6\x81\xD8\xB3\xC2\xBD\x81\xB5\x6E\xEC\xAF\xBA\xAD\xBC\xC5\xC2\xBF\x87\x80\xD1\xC2\xC1\x84\x84\x92\xC5\x98\x89\xDA\xD1\xBE\xC3\xC8\xC2\xCF\x86\x9E\x92\xC2\xD4\xCC\xD3\xCE\xD3\xA1\x88\xDC\xCD\xB6\x80\x04\xBF\xC7\x9D\xA8\xD0\xD2\xD1\xDA\xD5\xC6\xD6\xB7\x8B\xF8\xD0\xDB\xD8\xD3\xD8\xDB\xBC\xBA\xC2\xEA\x89\x07\xD6\xDF\xDA\xA3\xC0\xC7\xFD\xD4\xDC\xE6\xE5\xE6\xC3\x9B\xF9\xD9\xCB\xD1\xE9\xEA\xEA\xD3\xD2\xC1\xF8",
					"\xEB\xEA\xEF\xE4\xEB\xD9\xD7\xC9\x8A\x06\xE0\xE8\xEF\xEF\xE7\xE6\xDE\xEA\xF2\xF5\xF5\xF1\xF7\xDD\xEE\xF1\xF0\xF8\xF3\xF7\x05\xF2\x0E\xA6\x8A\x8D\x05\xF6\x05\xF3\xE7\x0F\x3F\x79\x73\x80\xA5\x50\x05\x79\x04\x02\xA5\x60\x47\x65\xEB\x4F\x7E\x80\x81\xA7\x4F\x85\x6A\x84\x11\x90\x89\x6A\x85\xBF\x58\x88\x73\x84\x1A\x8E\x7D\x84\x78\xF3\x61\x81\x43\x04\x09\x95\x8C\x87\x89\x26\xA9\x87\x84\x8A\xA1\x54\x05\x8B\x8A\x2A\x99\x8C\x8A\x8C\x31\x9B\x85\x8E\x87\x33\xB6\x80\x8F\x8E\x34\xBC\x8A\x8D\x8F\x40\xBF\x82\x90\x8E\x37\xA0\x85\x7E\x7C\x47\xB4\x70\x01\x05\x2F\x81\x94\x92\x8F\x4F\xB3\x66\x05\x93\x43\x86\x9C\x7A\x92\x14\x90\x96\x95\x96\x4E\x96\x97\x04\x95\xCC\x52\x4C\x80\x92\x57\xA2\x85\x99\x92\x66\x9C\x95\x95\x7A\x6A\x91\x9B\x99\x83\x1F\xAC\x93\x69\x06\x60\x96\x9A\x05\x9D\x72\xB9\x8E\x9B\x9E\x5B\xB1\x90\x9D\x69\x1B\x38\x9F\x9D\x91\x79\x84\xA3\xA2\x96\x85\x88\xA7\xA1\x97\x89\x8C\xAB\xA3\x9B\x06\x5C\x05\x78\x45\x63\x83\x4E\x05\x79\x1F\x02\xA6\x40\x08\xE5\x61\x0A\xA6\x92\x22\x25\x73\x0B\xA7\x69\xA3\x64\x09\x79\x25\x25\x76\x09\x79",
					"\x27\x24\xA1\x40\x0A\xAE\xA7\x92\xAD\x0A\xE5\x6A\x01\xAF\x0A\xB1\xAD\x9D\x9E\x9E\xBB\x86\xA4\x9A\xAE\x8D\x90\xA8\x96\x9F\xBF\x84\xB3\x40\x0B\xE5\x6D\x09\xAE\xB1\xA5\xBC\xAA\x42\x0B\xE5\x6F\x05\x78\x0C\xE5\x71\x01\xAE\x0C\xCB\x8E\xB9\x43\x0C\xE5\x74\x09\xB6\xAF\xC3\x9A\xB5\xB2\xB8\xE1\xA0\xBD\xB2\xB9\x01\x75\x0F\xB6\xA2\xE3\xA8\xB2\xAC\xB0\x8F\xBC\x99\x7A\x0D\xEB\x8E\xAD\xB8\xBB\xE5\xB8\xB7\xBA\xBE\x68\x81\x47\x0D\xBD\xC2\xBB\xB6\xBD\xBE\x03\xC2\xC1\xC1\xBF\xEF\x8C\xB8\xC3\x73\x38\x31\xA9\x0C\xC0\xF2\xB1\xBD\xAC\xBF\x09\xE4\xB6\xC1\xC4\xF7\x85\xCB\xC2\xC2\x00\x3A\x00\xC7\xC4\x1A\xD5\xCE\xB8\xBC\x19\xC7\xC0\x03\x0E\x1F\xC1\xB8\xC4\xC1\x26\xE4\xCC\xCB\xCA\x02\x5F\x1A\x41\x4A\x95\xBC\x64\xCE\x7E\x57\x77\xC3\x43\x4A\xCB\x7A\xC2\x40\x4F\xF1\xB3\x46\x66\x41\x3E\xFD\xCA\x40\x42\x25\xF3\x4F\x3F\xD0\x03\x48\xD7\x6C\xCF\x00\x10\xD2\xD0\x98\x60\x57\x53\x4C\xD4\x45\xCD\x7C\xD2\x40\x52\xF0\x45\xD7\x6A\x57\xE1\x47\x57\xD6\x01\x5D\xDD\x4B\xD7\xB7\x73\x48\xD8\x6F\x6A\xEC\x53\x4D\xD6\x6B\xDB\xCA\xD3\x47\x64\xC0\x0F\x4D\xCE\x60",
					"\x99\xD6\xDB\x47\x7A\xEC\x59\xD7\xDD\xAB\x59\xDA\x6D\xD6\x40\xC9\xD3\xD5\x42\x59\xEF\xD7\xD2\xCA\x87\xC6\x4E\xD0\x6F\x71\xCD\x7A\xE1\x42\x80\xF7\x60\xDC\xE3\x30\x75\xD5\xE6\xCD\x0A\x51\xED\x99\xE3\x03\x5B\xEB\x72\xE7\x73\xFC\xD2\xD9\xE0\x79\xE9\xE5\xCE\xD1\x94\xFE\xD7\xEA\xE5\x0A\x45\xEF\xC8\xE8\x02\x49\xE0\x4D\xD6\x8F\xF2\xD5\xE9\x46\x75\xF9\xED\x71\xDF\x78\xEB\xEF\xEE\xEA\x06\x62\xE3\xF3\xEB\xAC\xC6\x42\xEF\xC5\xB4\xC1\x46\xED\x4B\xB8\xD8\xED\x49\xEF\x6C\x64\xEB\xED\x46\xC0\xE8\xE2\xF3\xE4\x38\xDA\xF5\xF2\xEB\xC8\xC7\xFA\xF2\xF4\x74\xF3\xC0\xFA\xE3\xE4\xDB\x44\xF7\x55\xD6\xD7\xFD\xFB\xEE\xEF\xE5\xEF\xDF\xF9\xAF\xED\xD6\xD6\xD7\x6E\xD9\xED\xE4\x5B\xC6\xD7\xFA\x45\xFF\xFE\x7F\x1C\x00\xFE\x75\x6F\xE5\x4B\x3D\x80\x6C\x35\x6E\x6E\x0A\x8C\x7F\xFC\x6D\x23\x80\xD7\x6A\x5D\xF8\x50\x62\x25\xF0\x51\x6E\x82\x82\x71\x76\x05\x94\x85\x3C\xE3\x51\x76\x77\x50\x60\x87\xE2\x42\x7E\x77\xE8\x49\x26\x80\x04\x25\x3E\x08\x89\x25\x6E\xA8\x6D\x82\x21\x2F\x8C\x86\xF9\x32\x87\x84\x18\xA5\x39\x86\xDD\x7B\x65\x79\x04\x75\x3C\xA8\x6E\x31",
					"\x21\xC5\x23\x3E\x4F\x0A\x24\x3D\xF0\x55\x0D\x3C\x1D\x21\x51\xF8\x56\x0B\x86\x85\x12\x51\x7C\x57\x05\x3C\x0A\xF7\x41\x7C\x2C\x3D\x82\x21\x08\x41\x7D\x16\x57\x89\x21\x2C\x88\x7A\x0B\x1D\x8A\x23\x97\x61\x7B\x0B\x30\x89\x23\x8C\xC8\x7C\x09\x13\x89\x2C\x53\xF0\x5D\x0A\x8A\x0A\x39\x51\xF8\x5E\x05\x3C\x1E\x16\x86\x78\x16\x97\x82\xC4\x18\x8B\x83\x94\x61\x78\x85\x1D\x39\x7B\xCB\x79\x81\x73\x9D\x26\x8C\x68\xE9\x3D\x22\xEF\x0A\x97\x87\x19\xA4\x4A\x90\xA7\x61\x8B\xF8\x7B\x85\x86\xE1\x5D\x21\x3F\x03\x28\x86\xCB\x02\x26\x78\x0E\x89\x21\x40\x95\x89\x69\x47\x11\x7E\x92\x83\x0B\x41\x87\x17\x6C\x92\x25\x8A\x24\x45\xED\x5C\x65\x6E\xF7\x21\x7E\x8B\x42\x7C\x48\xC4\x69\x3A\x92\xA1\x76\x80\x1A\xBA\x8B\x3D\x40\x84\x3E\x91\xAF\x62\x4C\x2A\x88\x69\x6B\xAC\x78\x97\x67\x16\x94\x4E\x2B\xA5\x36\x78\x63\x8A\x26\x3F\xE1\x74\x8B\x30\xB2\x55\x85\x5F\x89\x23\x96\xC8\x6A\x8B\x34\xCF\x3F\x86\x44\x80\x8B\x7F\xE1\x6F\x8B\x34\x94\x5E\x9A\x83\x05\x8D\x9C\x03\x27\x99\x40\x0B\x91\x8F\xF2\x2D\x88\x9D\x02\x38\x91\xED\x44\x8A\x21\x6E\xAF\x71\x54\xCB\x8E",
					"\x5D\x2C\x86\x2A\x9D\x90\x1B\x90\x61\xF2\x81\x20\x34\xC8\x73\x55\x7C\x94\x63\xA0\x03\x26\x41\xF8\x7F\x92\x56\x84\x82\x23\x9F\xAF\x67\x56\xF0\x29\x56\x97\x84\x0E\xA5\x6C\x0A\x20\x96\x41\x10\xA7\x75\x56\x42\x78\x56\x15\xA6\x22\x46\xBC\x96\x20\x85\x86\x24\x3F\xE1\x76\x56\x41\xA1\x6E\x8B\x84\x96\x84\xA1\xCF\x2D\xA6\x40\x23\xA7\x74\x98\xA3\x37\x94\x03\x38\x9B\xEB\x38\x5B\xA4\x93\x7A\xA4\x93\x16\x88\x59\x4F\xCC\x36\xA4\x94\x0A\x27\xA7\xCE\x76\x82\xB2\x43\xAD\x22\x9C\xB3\x9F\x9A\x1A\x81\x7C\xB4\x4C\xA7\xA8\x60\x82\x20\xA9\x00\x17\xA1\x48\x52\x54\xAB\x84\x13\x49\xA4\x39\xB5\xA9\xF8\x54\x5D\xAA\x83\x02\x9E\xA8\x50\xBE\xAB\x4B\x9C\x6F\x38\xAC\x85\x92\x21\xD6\x42\x78\xB6\x60\xA2\x20\x69\xA8\xAE\x20\x58\xAC\x9D\x42\x5E\x52\x78\x09\xB0\x99\x20\xE2\x97\xAF\x40\x35\x9F\x7B\xF0\x7F\x5D\xAC\x96\x56\xA9\x40\x04\xB2\xB1\xD3\x4D\x60\xB1\x02\x2F\x9D\x63\x8B\xB1\xAA\xE4\x4F\x60\xB2\x01\x38\x9F\x5F\xCB\x4F\x98\x0B\x9E\x60\xB3\x00\x12\xA3\x66\x8C\xB7\xAA\x0B\xA8\x60\xB4\xAB\x54\xAC\x65\x89\x2F\xAD\x84\x36\x8A\x78\x3D\x0A\xB0\x00\x20",
					"\xA3\xB5\x11\x88\x7F\x07\xE5\x20\x0A\x6C\xAA\xAB\xB3\xAF\x86\x25\x95\xC8\x62\x0A\x1F\x86\x2B\x96\x81\x3E\xB5\x7E\x6C\xA3\x35\x71\xAB\xB8\x78\x9B\xA5\x37\xAC\x40\xA1\x7F\x10\x31\x52\xA9\x69\x90\xB9\xA9\xE1\x64\x09\xAC\x4B\xA6\xBB\xCA\xAF\x75\x08\xB1\x53\xA9\x77\x96\xA6\x82\x23\x31\x54\xAB\xE3\xB8\xAE\x05\xC7\x01\x56\xB2\xA9\xBB\x20\xAD\xA6\x20\x12\x65\x39\x08\xD9\xB1\xAF\xBD\x7B\x21\x7F\x12\x65\x3C\x08\xD9\xB3\xAB\x9A\x76\xA2\x22\x32\xC8\x7E\x09\xE1\x83\x22\x9B\x03\x23\xC3\xEB\x4F\x06\xC0\x81\x01\xB2\xC0\x16\x90\x09\x83\x81\x24\x9D\x81\x1A\xAD\x23\x51\x11\x54\xB7\x42\x75\xC3\xD2\xA1\x7A\x0A\xB1\x5C\xAF\x34\xF2\x86\x21\x4B\x61\x7B\x0A\x13\xC0\x02\x64\x82\x26\xC4\xE4\x54\x09\xC5\x9E\x4B\xA3\x68\x12\x76\x91\x3B\xB9\x89\x83\x48\x99\x8E\x1E\xD5\x3B\x6F\x8F\x25\x93\x9A\x21\x3F\xC7\x64\xD0\x3F\x78\xE8\x77\xC7\xC8\x7C\x96\x82\x26\xB6\x2B\xC8\x98\x00\x95\xC6\x7F\x99\x8F\x93\xC3\x8E\x2C\x08\x9F\x25\x99\x1D\x38\x8D\x96\xC8\x81\x7E\x8E\x1D\xC9\x23\xA4\x83\x95\x1E\x80",
				};
				vl::glr::DecompressSerializedData(compressed, true, dataSolidRows, dataRows, dataBlock, dataRemain, outputStream);
			}

			const wchar_t* ParserRuleName(vl::vint index)
			{
				static const wchar_t* results[] = {
					L"JLiteral",
					L"JField",
					L"JObject",
					L"JArray",
					L"JValue",
					L"JRoot",
				};
				return results[index];
			}

			const wchar_t* ParserStateLabel(vl::vint index)
			{
				static const wchar_t* results[] = {
					L"[0][JLiteral] BEGIN ",
					L"[1][JLiteral] END [ENDING]",
					L"[2][JLiteral]< \"false\" @ >",
					L"[3][JLiteral]< \"null\" @ >",
					L"[4][JLiteral]< \"true\" @ >",
					L"[5][JLiteral]< NUMBER @ >",
					L"[6][JLiteral]< STRING @ >",
					L"[7][JField] BEGIN ",
					L"[8][JField] END [ENDING]",
					L"[9][JField]< STRING \":\" @ JValue >",
					L"[10][JField]< STRING \":\" JValue @ >",
					L"[11][JField]< STRING @ \":\" JValue >",
					L"[12][JObject] BEGIN ",
					L"[13][JObject] END [ENDING]",
					L"[14][JObject]< \"{\" @ { JField ; \",\" } \"}\" >",
					L"[15][JObject]< \"{\" { JField ; \",\" @ } \"}\" >",
					L"[16][JObject]< \"{\" { JField ; \",\" } \"}\" @ >",
					L"[17][JObject]< \"{\" { JField @ ; \",\" } \"}\" >",
					L"[18][JArray] BEGIN ",
					L"[19][JArray] END [ENDING]",
					L"[20][JArray]< \"[\" @ { JValue ; \",\" } \"]\" >",
					L"[21][JArray]< \"[\" { JValue ; \",\" @ } \"]\" >",
					L"[22][JArray]< \"[\" { JValue ; \",\" } \"]\" @ >",
					L"[23][JArray]< \"[\" { JValue @ ; \",\" } \"]\" >",
					L"[24][JValue] BEGIN ",
					L"[25][JValue] END [ENDING]",
					L"[26][JValue]<< !JArray @ >>",
					L"[27][JValue]<< !JLiteral @ >>",
					L"[28][JValue]<< !JObject @ >>",
					L"[29][JRoot] BEGIN ",
					L"[30][JRoot] END [ENDING]",
					L"[31][JRoot]<< !JArray @ >>",
					L"[32][JRoot]<< !JObject @ >>",
				};
				return results[index];
			}

			Parser::Parser()
				: vl::glr::ParserBase<JsonTokens, ParserStates, JsonAstInsReceiver, ParserStateTypes>(&JsonTokenDeleter, &JsonLexerData, &JsonParserData)
			{
			};

			vl::vint32_t Parser::FindCommonBaseClass(vl::vint32_t class1, vl::vint32_t class2)
			{
				return -1;
			};

			vl::Ptr<vl::glr::json::JsonNode> Parser::ParseJRoot(const vl::WString & input, vl::vint codeIndex)
			{
				 return Parse<ParserStates::JRoot>(input, this, codeIndex);
			};
		}
	}
}
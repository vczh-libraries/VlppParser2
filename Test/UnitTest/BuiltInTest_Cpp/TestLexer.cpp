#include "../../Source/BuiltIn-Cpp/Generated/CppParser.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::regex;
using namespace cpp_parser;

#define CPP_KEYWORD_TOKENS(F)							\
	F(ALIGNAS,					alignas)				\
	F(__PTR32,					__ptr32)				\
	F(__PTR64,					__ptr64)				\
	F(OPERATOR,					operator)				\
	F(NEW,						new)					\
	F(DELETE,					delete)					\
	F(CONSTEXPR,				constexpr)				\
	F(CONST,					const)					\
	F(VOLATILE,					volatile)				\
	F(OVERRIDE,					override)				\
	F(NOEXCEPT,					noexcept)				\
	F(THROW,					throw)					\
	F(DECLTYPE,					decltype)				\
	F(__CDECL,					__cdecl)				\
	F(__CLRCALL,				__clrcall)				\
	F(__STDCALL,				__stdcall)				\
	F(__FASTCALL,				__fastcall)				\
	F(__THISCALL,				__thiscall)				\
	F(__VECTORCALL,				__vectorcall)			\
	F(TYPE_AUTO,				auto)					\
	F(TYPE_VOID,				void)					\
	F(TYPE_BOOL,				bool)					\
	F(TYPE_CHAR,				char)					\
	F(TYPE_WCHAR_T,				wchar_t)				\
	F(TYPE_CHAR16_T,			char16_t)				\
	F(TYPE_CHAR32_T,			char32_t)				\
	F(TYPE_SHORT,				short)					\
	F(TYPE_INT,					int)					\
	F(TYPE___INT8,				__int8)					\
	F(TYPE___INT16,				__int16)				\
	F(TYPE___INT32,				__int32)				\
	F(TYPE___INT64,				__int64)				\
	F(TYPE_LONG,				long)					\
	F(TYPE_FLOAT,				float)					\
	F(TYPE_DOUBLE,				double)					\
	F(SIGNED,					signed)					\
	F(UNSIGNED,					unsigned)				\
	F(STATIC,					static)					\
	F(VIRTUAL,					virtual)				\
	F(EXPLICIT,					explicit)				\
	F(INLINE,					inline)					\
	F(__INLINE,					__inline)				\
	F(__FORCEINLINE,			__forceinline)			\
	F(REGISTER,					register)				\
	F(MUTABLE,					mutable)				\
	F(THREAD_LOCAL,				thread_local)			\
	F(DECL_ABSTRACT,			abstract)				\
	F(DECL_FINAL,				final)					\
	F(DECL_CLASS,				class)					\
	F(DECL_STRUCT,				struct)					\
	F(DECL_ENUM,				enum)					\
	F(DECL_UNION,				union)					\
	F(DECL_NAMESPACE,			namespace)				\
	F(DECL_TYPEDEF,				typedef)				\
	F(DECL_USING,				using)					\
	F(DECL_FRIEND,				friend)					\
	F(DECL_EXTERN,				extern)					\
	F(DECL_TEMPLATE,			template)				\
	F(STAT_ASM,					__asm)					\
	F(STAT_RETURN,				return)					\
	F(STAT_BREAK,				break)					\
	F(STAT_CONTINUE,			continue)				\
	F(STAT_GOTO,				goto)					\
	F(STAT_IF,					if)						\
	F(STAT_ELSE,				else)					\
	F(STAT_WHILE,				while)					\
	F(STAT_DO,					do)						\
	F(STAT_FOR,					for)					\
	F(STAT_SWITCH,				switch)					\
	F(STAT_CASE,				case)					\
	F(STAT_DEFAULT,				default)				\
	F(STAT_TRY,					try)					\
	F(STAT_CATCH,				catch)					\
	F(STAT___TRY,				__try)					\
	F(STAT___EXCEPT,			__except)				\
	F(STAT___FINALLY,			__finally)				\
	F(STAT___LEAVE,				__leave)				\
	F(STAT___IF_EXISTS,			__if_exists)			\
	F(STAT___IF_NOT_EXISTS,		__if_not_exists)		\
	F(EXPR_TRUE,				true)					\
	F(EXPR_FALSE,				false)					\
	F(EXPR_THIS,				this)					\
	F(EXPR_NULLPTR,				nullptr)				\
	F(EXPR___NULLPTR,			__nullptr)				\
	F(EXPR_TYPEID,				typeid)					\
	F(EXPR_SIZEOF,				sizeof)					\
	F(EXPR_DYNAMIC_CAST,		dynamic_cast)			\
	F(EXPR_STATIC_CAST,			static_cast)			\
	F(EXPR_CONST_CAST,			const_cast)				\
	F(EXPR_REINTERPRET_CAST,	reinterpret_cast)		\
	F(EXPR_SAFE_CAST,			safe_cast)				\
	F(TYPENAME,					typename)				\
	F(PUBLIC,					public)					\
	F(PROTECTED,				protected)				\
	F(PRIVATE,					private)				\
	F(__PRAGMA,					__pragma)				\
	F(__DECLSPEC,				__declspec)				\
	F(STATIC_ASSERT,			static_assert)			\

vint CheckTokens(List<RegexToken>& tokens)
{
	for (auto&& token : tokens)
	{
		switch ((CppTokens)token.token)
		{
		case CppTokens::LBRACE:
			TEST_ASSERT(token.length == 1 && *token.reading == L'{');
			break;
		case CppTokens::RBRACE:
			TEST_ASSERT(token.length == 1 && *token.reading == L'}');
			break;
		case CppTokens::LBRACKET:
			TEST_ASSERT(token.length == 1 && *token.reading == L'[');
			break;
		case CppTokens::RBRACKET:
			TEST_ASSERT(token.length == 1 && *token.reading == L']');
			break;
		case CppTokens::LPARENTHESIS:
			TEST_ASSERT(token.length == 1 && *token.reading == L'(');
			break;
		case CppTokens::RPARENTHESIS:
			TEST_ASSERT(token.length == 1 && *token.reading == L')');
			break;
		case CppTokens::LT:
			TEST_ASSERT(token.length == 1 && *token.reading == L'<');
			break;
		case CppTokens::GT:
			TEST_ASSERT(token.length == 1 && *token.reading == L'>');
			break;
		case CppTokens::EQ:
			TEST_ASSERT(token.length == 1 && *token.reading == L'=');
			break;
		case CppTokens::NOT:
			TEST_ASSERT(token.length == 1 && *token.reading == L'!');
			break;
		case CppTokens::PERCENT:
			TEST_ASSERT(token.length == 1 && *token.reading == L'%');
			break;
		case CppTokens::COLON:
			TEST_ASSERT(token.length == 1 && *token.reading == L':');
			break;
		case CppTokens::SEMICOLON:
			TEST_ASSERT(token.length == 1 && *token.reading == L';');
			break;
		case CppTokens::DOT:
			TEST_ASSERT(token.length == 1 && *token.reading == L'.');
			break;
		case CppTokens::QUESTIONMARK:
			TEST_ASSERT(token.length == 1 && *token.reading == L'?');
			break;
		case CppTokens::COMMA:
			TEST_ASSERT(token.length == 1 && *token.reading == L',');
			break;
		case CppTokens::MUL:
			TEST_ASSERT(token.length == 1 && *token.reading == L'*');
			break;
		case CppTokens::ADD:
			TEST_ASSERT(token.length == 1 && *token.reading == L'+');
			break;
		case CppTokens::SUB:
			TEST_ASSERT(token.length == 1 && *token.reading == L'-');
			break;
		case CppTokens::DIV:
			TEST_ASSERT(token.length == 1 && *token.reading == L'/');
			break;
		case CppTokens::XOR:
			TEST_ASSERT(token.length == 1 && *token.reading == L'^');
			break;
		case CppTokens::AND:
			TEST_ASSERT(token.length == 1 && *token.reading == L'&');
			break;
		case CppTokens::OR:
			TEST_ASSERT(token.length == 1 && *token.reading == L'|');
			break;
		case CppTokens::REVERT:
			TEST_ASSERT(token.length == 1 && *token.reading == L'~');
			break;
		case CppTokens::SHARP:
			TEST_ASSERT(token.length == 1 && *token.reading == L'#');
			break;
		case CppTokens::INT:
			{
				auto reading = token.reading;
				auto length = token.length;

				while (reading[length - 1] == L'u' || reading[length - 1] == L'U' || reading[length - 1] == L'l' || reading[length - 1] == L'L')
				{
					length--;
				}

				TEST_ASSERT(token.length - length <= 3);
				for (vint i = 0; i < length; i++)
				{
					TEST_ASSERT((L'0' <= reading[i] && reading[i] <= L'9') || reading[i] == L'\'');
				}
			}
			break;
		case CppTokens::HEX:
			{
				auto reading = token.reading;
				auto length = token.length;
				TEST_ASSERT(length > 2);
				TEST_ASSERT(reading[0] == L'0' && (reading[1] == L'x' || reading[1] == L'X'));

				reading += 2;
				length -= 2;

				while (reading[length - 1] == L'u' || reading[length - 1] == L'U' || reading[length - 1] == L'l' || reading[length - 1] == L'L')
				{
					length--;
				}

				TEST_ASSERT(token.length - length <= 5);
				for (vint i = 0; i < length; i++)
				{
					TEST_ASSERT((L'0' <= reading[i] && reading[i] <= L'9') || (L'a' <= reading[i] && reading[i] <= L'f') || (L'A' <= reading[i] && reading[i] <= L'F'));
				}
			}
			break;
		case CppTokens::BIN:
			{
				auto reading = token.reading;
				auto length = token.length;
				TEST_ASSERT(length > 2);
				TEST_ASSERT(reading[0] == L'0' && (reading[1] == L'b' || reading[1] == L'B'));

				reading += 2;
				length -= 2;

				while (reading[length - 1] == L'u' || reading[length - 1] == L'U' || reading[length - 1] == L'l' || reading[length - 1] == L'L')
				{
					length--;
				}

				TEST_ASSERT(token.length - length <= 5);
				for (vint i = 0; i < length; i++)
				{
					TEST_ASSERT(reading[i] == L'0' || reading[i] == L'1');
				}
			}
			break;
		case CppTokens::FLOAT:
			{
				vint _1 = 0, _2 = 0;
				auto reading = token.reading;
				auto length = token.length;

				if (reading[length - 1] == L'f' || reading[length - 1] == L'F' || reading[length - 1] == L'l' || reading[length - 1] == L'L')
				{
					length--;
				}

				while (L'0' <= *reading && *reading <= L'9')
				{
					reading++;
					_1++;
				}
				TEST_ASSERT(*reading++ == L'.');
				while (L'0' <= *reading && *reading <= L'9')
				{
					reading++;
					_2++;
				}
				TEST_ASSERT(_1 > 0 || _2 > 0);

				if (*reading == L'e' || *reading == L'E')
				{
					reading++;
					if (*reading == L'+' || *reading == L'-') reading++;
					while (reading < token.reading + length)
					{
						TEST_ASSERT(L'0' <= *reading && *reading <= L'9');
						reading++;
					}
				}
				else
				{
					TEST_ASSERT(length == _1 + _2 + 1);
				}
			}
			break;
		case CppTokens::FLOATHEX:
			{
				vint _1 = 0, _2 = 0;
				auto reading = token.reading;
				auto length = token.length;
				TEST_ASSERT(length > 2);
				TEST_ASSERT(reading[0] == L'0' && (reading[1] == L'x' || reading[1] == L'X'));

				reading += 2;
				length -= 2;

				if (reading[length - 1] == L'f' || reading[length - 1] == L'F' || reading[length - 1] == L'l' || reading[length - 1] == L'L')
				{
					length--;
				}

				while ((L'0' <= *reading && *reading <= L'9') || (L'a' <= *reading && *reading <= L'f') || (L'A' <= *reading && *reading <= L'F'))
				{
					reading++;
					_1++;
				}
				TEST_ASSERT(*reading++ == L'.');
				while ((L'0' <= *reading && *reading <= L'9') || (L'a' <= *reading && *reading <= L'f') || (L'A' <= *reading && *reading <= L'F'))
				{
					reading++;
					_2++;
				}
				TEST_ASSERT(_1 > 0 || _2 > 0);

				if (*reading == L'e' || *reading == L'E')
				{
					reading++;
					if (*reading == L'+' || *reading == L'-') reading++;
					while (reading < token.reading + length)
					{
						TEST_ASSERT(L'0' <= *reading && *reading <= L'9');
						reading++;
					}
				}
				else
				{
					TEST_ASSERT(length == _1 + _2 + 1);
				}
			}
			break;

#define ASSERT_KEYWORD(NAME, KEYWORD)\
		case CppTokens::NAME:\
			TEST_ASSERT(token.length == wcslen(L#KEYWORD) && wcsncmp(token.reading, L#KEYWORD, wcslen(L#KEYWORD)) == 0);\
			break;\

		CPP_KEYWORD_TOKENS(ASSERT_KEYWORD)

#undef ASSERT_KEYWORD
		case CppTokens::ID:
			{
				auto reading = token.reading;
				TEST_ASSERT((L'a' <= reading[0] && reading[0] <= L'z') || (L'A' <= reading[0] && reading[0] <= L'Z') || reading[0] == L'_');
				for (vint i = 1; i < token.length; i++)
				{
					TEST_ASSERT((L'0' <= reading[i] && reading[i] <= L'9') || (L'a' <= reading[i] && reading[i] <= L'z') || (L'A' <= reading[i] && reading[i] <= L'Z') || reading[i] == L'_');
				}
			}
			break;
		case CppTokens::STRING:
			{
				auto reading = token.reading;
				if (wcsncmp(reading, L"L\"", 2) == 0) reading += 2;
				else if (wcsncmp(reading, L"u\"", 2) == 0) reading += 2;
				else if (wcsncmp(reading, L"U\"", 2) == 0) reading += 2;
				else if (wcsncmp(reading, L"u8\"", 3) == 0) reading += 3;
				else if (wcsncmp(reading, L"\"", 1) == 0) reading += 1;
				else TEST_ASSERT(false);

				while (*reading != L'\"')
				{
					TEST_ASSERT(*reading != 0);
					reading += (*reading == L'\\' ? 2 : 1);
				}
				TEST_ASSERT(token.length == (vint)(reading - token.reading + 1));
			}
			break;
		case CppTokens::CHAR:
			{
				auto reading = token.reading;
				if (wcsncmp(reading, L"L'", 2) == 0) reading += 2;
				else if (wcsncmp(reading, L"u'", 2) == 0) reading += 2;
				else if (wcsncmp(reading, L"U'", 2) == 0) reading += 2;
				else if (wcsncmp(reading, L"u8'", 3) == 0) reading += 3;
				else if (wcsncmp(reading, L"'", 1) == 0) reading += 1;
				else TEST_ASSERT(false);

				while (*reading != L'\'')
				{
					TEST_ASSERT(*reading != 0);
					reading += (*reading == L'\\' ? 2 : 1);
				}
				TEST_ASSERT(token.length == (vint)(reading - token.reading + 1));
			}
			break;
		case CppTokens::SPACE:
			{
				for (vint i = 0; i < token.length; i++)
				{
					auto c = token.reading[i];
					TEST_ASSERT(c == L' ' || c == L'\t' || c == L'\r' || c == L'\n' || c == L'\v' || c == L'\f');
				}
			}
			break;
		case CppTokens::DOCUMENT:
			{
				TEST_ASSERT(token.length >= 3);
				TEST_ASSERT(token.reading[0] == L'/');
				TEST_ASSERT(token.reading[1] == L'/');
				TEST_ASSERT(token.reading[2] == L'/');
				TEST_ASSERT(token.reading[token.length] == L'\r' || token.reading[token.length] == L'\n' || token.reading[token.length] == 0);
			}
			break;
		case CppTokens::COMMENT1:
			{
				TEST_ASSERT(token.length >= 2);
				TEST_ASSERT(token.reading[0] == L'/');
				TEST_ASSERT(token.reading[1] == L'/');
				TEST_ASSERT(token.reading[2] != L'/');
				TEST_ASSERT(token.reading[token.length] == L'\r' || token.reading[token.length] == L'\n' || token.reading[token.length] == 0);
			}
			break;
		case CppTokens::COMMENT2:
			{
				TEST_ASSERT(token.length >= 4);
				TEST_ASSERT(token.reading[0] == L'/');
				TEST_ASSERT(token.reading[1] == L'*');
				TEST_ASSERT(token.reading[token.length - 2] == L'*');
				TEST_ASSERT(token.reading[token.length - 1] == L'/');
				TEST_ASSERT(wcsstr(token.reading, L"*/") == token.reading + token.length - 2);
			}
			break;
		default:
			TEST_ASSERT(false);
		}
	}
	return tokens.Count();
}

TEST_FILE
{
	Ptr<RegexLexer> cppLexer;
	{
		MemoryStream data;
		CppLexerData(data);
		data.SeekFromBegin(0);
		cppLexer = new regex::RegexLexer(data);
	}

	TEST_CASE(L"Punctuators")
	{
		WString input = LR"({}[]()<>=!%:;.?,*+-/^&|~#)";
		List<RegexToken> tokens;
		cppLexer->Parse(input).ReadToEnd(tokens);
		TEST_ASSERT(CheckTokens(tokens) == 25);
	});

	TEST_CASE(L"Numbers")
	{
		WString input = LR"(
123
123'123'123u
123l
123'123'123UL
0x12345678
0xDEADBEEFu
0X12345678l
0XDEADBEEFUL
0b00001111
0b11110000u
0B00001111l
0B11110000UL
123.456
123.f
.456l
123.456e10
123.e+10F
.456e-10L
)";
		List<RegexToken> tokens;
		cppLexer->Parse(input).ReadToEnd(tokens);
		TEST_ASSERT(CheckTokens(tokens) == 37);
	});

	TEST_CASE(L"Strings")
	{
		WString input = LR"(
"abc"
L"\"\"xxxx\"\""
u"xxxx\"\"xxxx"
U"\"\"xxxx\"\""
u8"xxxx\"\"xxxx"
'a'
L'\''
u'\''
U'\''
u8'\''
)";
		List<RegexToken> tokens;
		cppLexer->Parse(input).ReadToEnd(tokens);
		TEST_ASSERT(CheckTokens(tokens) == 21);
	});

	TEST_CASE(L"Comments")
	{
		WString input = LR"(
//
//xxxxx
///
///xxxxx
/**/
/********/
/* xxxxx */
/* xx*xx */
/* xx**x */
/* x***x */
)";
		List<RegexToken> tokens;
		cppLexer->Parse(input).ReadToEnd(tokens);
		TEST_ASSERT(CheckTokens(tokens) == 19);
	});

	TEST_CASE(L"Hello, world!")
	{
		WString input = LR"(
using namespace std;

int main()
{
	cout << "Hello, world!" << endl;
}
)";
		List<RegexToken> tokens;
		cppLexer->Parse(input).ReadToEnd(tokens);
		TEST_ASSERT(CheckTokens(tokens) == 31);
	});
}
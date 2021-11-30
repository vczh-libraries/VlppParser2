#include "../../../Source/ParserGen/Compiler.h"

using namespace vl;
using namespace vl::glr;
using namespace vl::glr::parsergen;

extern void AssertError(ParserSymbolManager& global, ParserError expectedError);

namespace TestError_Lexer_TestObjects
{
	void ExpectError(const WString& lexerCode , ParserError expectedError)
	{
		ParserSymbolManager global;
		LexerSymbolManager lexerManager(global);
		CompileLexer(lexerManager, lexerCode);
		AssertError(global, expectedError);
	}
}
using namespace TestError_Lexer_TestObjects;

TEST_FILE
{
	TEST_CASE(L"DuplicatedClassProp 1")
	{
		const wchar_t* input =
LR"LEXER(
discard
)LEXER";
		ExpectError(input, { ParserErrorType::InvalidTokenDefinition,L"discard" });
	});

	TEST_CASE(L"DuplicatedClassProp 2")
	{
		const wchar_t* input =
LR"LEXER(
ID
)LEXER";
		ExpectError(input, { ParserErrorType::InvalidTokenDefinition,L"ID" });
	});

	TEST_CASE(L"DuplicatedClassProp 3")
	{
		const wchar_t* input =
LR"LEXER(
ID/w+
)LEXER";
		ExpectError(input, { ParserErrorType::InvalidTokenDefinition,L"ID/w+" });
	});

	TEST_CASE(L"DuplicatedToken")
	{
		const wchar_t* input =
LR"LEXER(
ID:/w+
ID:/w+
)LEXER";
		ExpectError(input, { ParserErrorType::DuplicatedToken,L"ID" });
	});

	TEST_CASE(L"DuplicatedTokenByDisplayText")
	{
		const wchar_t* input =
LR"LEXER(
ID1:I[D]
ID2:[I]D
)LEXER";
		ExpectError(input, { ParserErrorType::DuplicatedTokenByDisplayText,L"ID2" });
	});

	TEST_CASE(L"InvalidTokenRegex")
	{
		const wchar_t* input =
LR"LEXER(
ID:/
)LEXER";
		ExpectError(input, { ParserErrorType::InvalidTokenRegex,L"ID",L"Regular expression syntax error: Illegal character escaping. : 1 : /" });
	});

	TEST_CASE(L"TokenRegexNotPure")
	{
		const wchar_t* input =
LR"LEXER(
ID:(<capture>something)
)LEXER";
		ExpectError(input, { ParserErrorType::TokenRegexNotPure,L"ID" });
	});
}
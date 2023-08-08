#include "TestError.h"

namespace TestError_Lexer_TestObjects
{
	void ExpectError(const WString& lexerCode , ParserErrorWithoutLocation expectedError)
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
	TEST_CASE(L"InvalidTokenDefinition 1")
	{
		const wchar_t* input =
LR"LEXER(
discard
)LEXER";
		ExpectError(input, { ParserErrorType::InvalidTokenDefinition,L"discard" });
	});

	TEST_CASE(L"InvalidTokenDefinition 2")
	{
		const wchar_t* input =
LR"LEXER(
ID
)LEXER";
		ExpectError(input, { ParserErrorType::InvalidTokenDefinition,L"ID" });
	});

	TEST_CASE(L"InvalidTokenDefinition 3")
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

	TEST_CASE(L"DuplicatedTokenFragment")
	{
		const wchar_t* input =
LR"LEXER(
$ID:/w+
$ID:/w+
)LEXER";
		ExpectError(input, { ParserErrorType::DuplicatedTokenFragment,L"$ID" });
	});

	TEST_CASE(L"TokenFragmentNotExists")
	{
		const wchar_t* input =
LR"LEXER(
ID:\d{$Unexisting}\d
)LEXER";
		ExpectError(input, { ParserErrorType::TokenFragmentNotExists,L"$Unexisting" });
	});
}
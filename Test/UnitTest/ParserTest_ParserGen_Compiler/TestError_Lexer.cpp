#include "../../../Source/ParserGen/Compiler.h"

using namespace vl;
using namespace vl::glr;
using namespace vl::glr::parsergen;

namespace TestError_Lexer_TestObjects
{
	void AssertError(ParserSymbolManager& global, ParserError expectedError)
	{
		TEST_ASSERT(global.Errors().Count() == 1);
		auto&& error = global.Errors()[0];
		TEST_ASSERT(error.type == expectedError.type);
		TEST_ASSERT(error.arg1 == expectedError.arg1);
		TEST_ASSERT(error.arg2 == expectedError.arg2);
		TEST_ASSERT(error.arg3 == expectedError.arg3);
	}

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
LR"AST(
discard
)AST";
		ExpectError(input, { ParserErrorType::InvalidTokenDefinition,L"discard" });
	});

	TEST_CASE(L"DuplicatedClassProp 2")
	{
		const wchar_t* input =
LR"AST(
ID
)AST";
		ExpectError(input, { ParserErrorType::InvalidTokenDefinition,L"ID" });
	});

	TEST_CASE(L"DuplicatedClassProp 3")
	{
		const wchar_t* input =
LR"AST(
discard:/w+
)AST";
		ExpectError(input, { ParserErrorType::InvalidTokenDefinition,L"discard:/w+" });
	});

	TEST_CASE(L"DuplicatedToken")
	{
		const wchar_t* input =
LR"AST(
ID:/w+
ID:/w+
)AST";
		ExpectError(input, { ParserErrorType::DuplicatedToken,L"ID" });
	});

	TEST_CASE(L"DuplicatedTokenByDisplayText")
	{
		const wchar_t* input =
			LR"AST(
ID1:I[D]
ID2:[I]D
)AST";
		ExpectError(input, { ParserErrorType::DuplicatedTokenByDisplayText,L"ID2" });
	});

	TEST_CASE(L"InvalidTokenRegex")
	{
		const wchar_t* input =
			LR"AST(
ID:/
)AST";
		ExpectError(input, { ParserErrorType::InvalidTokenRegex,L"ID" });
	});

	TEST_CASE(L"TokenRegexNotPure")
	{
		const wchar_t* input =
			LR"AST(
ID:(<capture>something)
)AST";
		ExpectError(input, { ParserErrorType::TokenRegexNotPure,L"ID" });
	});
}
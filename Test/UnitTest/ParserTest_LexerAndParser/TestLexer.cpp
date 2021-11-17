#include "../../Source/Calculator/Parser/Calculator_Lexer.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::stream;
using namespace vl::regex;
using namespace calculator;

TEST_FILE
{
	TEST_CASE(L"Test CalculatorLexerData")
	{
		WString input = LR"(
import max
import sum
export sum(1, 2, max(3, 4))
)";

		const wchar_t* outputs[] = {
			L"import", L"max",
			L"import", L"sum",
			L"export", L"sum", L"(", L"1", L",", L"2", L",", L"max", L"(", L"3", L",", L"4", L")", L")",
		};

		MemoryStream lexerData;
		CalculatorLexerData(lexerData);
		lexerData.SeekFromBegin(0);
		RegexLexer lexer(lexerData);

		List<RegexToken> tokens;
		lexer.Parse(input).ReadToEnd(tokens, CalculatorTokenDeleter);

		TEST_ASSERT(tokens.Count() == sizeof(outputs) / sizeof(*outputs));
		for (vint i = 0; i < tokens.Count(); i++)
		{
			TEST_ASSERT(WString::CopyFrom(tokens[i].reading, tokens[i].length) == outputs[i]);
		}
	});

#undef LEXER
}
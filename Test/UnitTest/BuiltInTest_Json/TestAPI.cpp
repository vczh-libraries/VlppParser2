#include "../../../Source/Json/GlrJson.h"

using namespace vl;
using namespace vl::collections;
using namespace vl::glr::json;

TEST_FILE
{
	TEST_CATEGORY(L"Test Json API")
	{
		const wchar_t* inputs[] =
		{
			L"{ }",
			L"[ ]",
			L"[ 1 ]",
			L"[ 1 , 2 ]",
			L"[ true, false, null, 1, \"abc\" ]",
			L"[ \"\\b\\f\\n\\r\\t\\\\\\\"abc\\u0041\\u0039\" ]",
			L"{ \"name\" : \"vczh\", \"scores\" : [100, 90, 80, {\"a\":\"b\"}], \"IDE\":{\"VC++\":\"Microsoft\"} }",
		};

		const wchar_t* outputPlain[] =
		{
			L"{}",
			L"[]",
			L"[1]",
			L"[1,2]",
			L"[true,false,null,1,\"abc\"]",
			L"[\"\\b\\f\\n\\r\\t\\\\\\\"abcA9\"]",
			L"{\"name\":\"vczh\",\"scores\":[100,90,80,{\"a\":\"b\"}],\"IDE\":{\"VC++\":\"Microsoft\"}}",
		};

		JsonFormatting formatCrlf = { true, false, L"\t"};
		const wchar_t* outputCrlf[] =
		{
			L"{}",
			L"[]",
			L"[\r\n\t1\r\n]",
			L"[\r\n\t1,\r\n\t2\r\n]",
			L"[\r\n\ttrue,\r\n\tfalse,\r\n\tnull,\r\n\t1,\r\n\t\"abc\"\r\n]",
			L"[\r\n\t\"\\b\\f\\n\\r\\t\\\\\\\"abcA9\"\r\n]",

			L"{"										L"\r\n"
			L"\t\"name\":\"vczh\","						L"\r\n"
			L"\t\"scores\":["							L"\r\n"
			L"\t\t100,"									L"\r\n"
			L"\t\t90,"									L"\r\n"
			L"\t\t80,"									L"\r\n"
			L"\t\t{"									L"\r\n"
			L"\t\t\t\"a\":\"b\""						L"\r\n"
			L"\t\t}"									L"\r\n"
			L"\t],"										L"\r\n"
			L"\t\"IDE\":{"								L"\r\n"
			L"\t\t\"VC++\":\"Microsoft\""				L"\r\n"
			L"\t}"										L"\r\n"
			L"}"
		};

		JsonFormatting formatCompact = { true, true, L"  " };
		const wchar_t* outputCompact[] =
		{
			L"{}",
			L"[]",
			L"[1]",
			L"[1, 2]",
			L"[true, false, null, 1, \"abc\"]",
			L"[\"\\b\\f\\n\\r\\t\\\\\\\"abcA9\"]",

			L"{"										L"\r\n"
			L"  \"name\":\"vczh\","						L"\r\n"
			L"  \"scores\":["							L"\r\n"
			L"    100,"									L"\r\n"
			L"    90,"									L"\r\n"
			L"    80,"									L"\r\n"
			L"    {\"a\":\"b\"}"						L"\r\n"
			L"  ],"										L"\r\n"
			L"  \"IDE\":{\"VC++\":\"Microsoft\"}"		L"\r\n"
			L"}"
		};

		Parser parser;
		for (auto [input, i] : indexed(From(inputs)))
		{
			TEST_CATEGORY(outputPlain[i])
			{
				auto ast = JsonParse(WString::Unmanaged(input), parser);
				TEST_CASE(L"Plain")
				{
					auto json = JsonToString(ast);
					TEST_ASSERT(json == outputPlain[i]);
				});
				TEST_CASE(L"Crlf")
				{
					auto json = JsonToString(ast, formatCrlf);
					TEST_ASSERT(json == outputCrlf[i]);
				});
				TEST_CASE(L"Compact")
				{
					auto json = JsonToString(ast, formatCompact);
					TEST_ASSERT(json == outputCompact[i]);
				});
			});
		}
	});
}
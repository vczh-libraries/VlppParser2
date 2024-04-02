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
			L"[1]",
			L"[1,2]",
			L"[true,false,null,1,\"abc\"]",
			L"[\"\\b\\f\\n\\r\\t\\\\\\\"abcA9\"]",
			L"{\"name\":\"vczh\",\"scores\":[100,90,80,{\"a\":\"b\"}],\"IDE\":{\"VC++\":\"Microsoft\"}}",
		};

		JsonFormatting formatCompact = { true, true, L"  " };
		const wchar_t* outputCompact[] =
		{
			L"{}",
			L"[]",
			L"[1]",
			L"[1,2]",
			L"[true,false,null,1,\"abc\"]",
			L"[\"\\b\\f\\n\\r\\t\\\\\\\"abcA9\"]",
			L"{\"name\":\"vczh\",\"scores\":[100,90,80,{\"a\":\"b\"}],\"IDE\":{\"VC++\":\"Microsoft\"}}",
		};

		Parser parser;
		for (auto [input, i] : indexed(From(inputs)))
		{
			TEST_CASE(outputPlain[i])
			{
				auto ast = JsonParse(WString::Unmanaged(input), parser);
				{
					auto json = JsonToString(ast);
					TEST_ASSERT(json == outputPlain[i]);
				}
				{
					auto json = JsonToString(ast, formatCrlf);
					TEST_ASSERT(json == outputCrlf[i]);
				}
				{
					auto json = JsonToString(ast, formatCompact);
					TEST_ASSERT(json == outputCompact[i]);
				}
			});
		}
	});
}
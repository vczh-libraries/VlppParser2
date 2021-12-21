#include "../../../Source/Json/GlrJson.h"

using namespace vl;
using namespace vl::glr::json;

TEST_FILE
{
	TEST_CATEGORY(L"Test Json API")
	{
		const wchar_t* input[] =
		{
			L"{ }",
			L"[ ]",
			L"[ 1 ]",
			L"[ 1 , 2 ]",
			L"[ true, false, null, 1, \"abc\" ]",
			L"[ \"\\b\\f\\n\\r\\t\\\\\\\"abc\\u0041\\u0039\" ]",
			L"{ \"name\" : \"vczh\", \"scores\" : [100, 90, 80, {\"a\":\"b\"}], \"IDE\":{\"VC++\":\"Microsoft\"} }",
		};
		const wchar_t* output[] =
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
		for (vint i = 0; i < sizeof(input) / sizeof(*input); i++)
		{
			TEST_CASE(output[i])
			{
				auto ast = JsonParse(WString::Unmanaged(input[i]), parser);
				auto json = JsonToString(ast);
				TEST_ASSERT(json == output[i]);
			});
		}
	});
}
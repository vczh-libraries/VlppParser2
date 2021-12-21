#include "../../../Source/Xml/GlrXml.h"

using namespace vl;
using namespace vl::glr::xml;

TEST_FILE
{
	TEST_CATEGORY(L"Test Xml API")
	{
		const wchar_t* input[] =
		{
			L"<name />",
			L"<name att1 = \"value1\" att2 = \"value2\" />",
			L"<?xml version = \"1.0\" encoding = \"utf16\" ?>\r\n<!--this is a comment-->\r\n<name att1 = \"value1\" att2 = \"value2\" />",
			L"<button name = \'&lt;&gt;&amp;&apos;&quot;\'> <![CDATA[ButtonText]]> <![CDATA[!]!]]!]>!>!]]> </button>",
			L"<text> This is a single line of text </text>",
			L"<text> normal <b>bold</b> normal <!--comment--> <i>italic</i> normal </text>",
			L"<text> \"normal\" <b>bold</b> \"normal\' <!--comment--> <i>italic</i> \'normal\" </text>",
		};
		const wchar_t* output[] =
		{
			L"<name/>",
			L"<name att1=\"value1\" att2=\"value2\"/>",
			L"<?xml version=\"1.0\" encoding=\"utf16\"?><!--this is a comment--><name att1=\"value1\" att2=\"value2\"/>",
			L"<button name=\"&lt;&gt;&amp;&apos;&quot;\"><![CDATA[ButtonText]]><![CDATA[!]!]]!]>!>!]]></button>",
			L"<text> This is a single line of text </text>",
			L"<text> normal <b>bold</b> normal <!--comment--><i>italic</i> normal </text>",
			L"<text> &quot;normal&quot; <b>bold</b> &quot;normal&apos; <!--comment--><i>italic</i> &apos;normal&quot; </text>",
		};

		Parser parser;
		for (vint i = 0; i < sizeof(input) / sizeof(*input); i++)
		{
			TEST_CASE(output[i])
			{
				auto ast = XmlParseDocument(WString::Unmanaged(input[i]), parser);
				auto xml = XmlToString(ast);
				TEST_ASSERT(xml == output[i]);
			});
		}
	});
}
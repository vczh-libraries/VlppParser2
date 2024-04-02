/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
Parser::ParsingJson_Parser

***********************************************************************/

#ifndef VCZH_PARSER2_BUILTIN_JSON
#define VCZH_PARSER2_BUILTIN_JSON

#include "Generated/JsonParser.h"

namespace vl
{
	namespace glr
	{
		namespace json
		{
			/// <summary>Deserialize JSON from string.</summary>
			/// <returns>The deserialized JSON node.</returns>
			/// <param name="input">The JSON code.</param>
			/// <param name="parser">The generated parser.</param>
			extern Ptr<JsonNode>			JsonParse(const WString& input, const Parser& parser);

			struct JsonFormatting
			{
				bool				spaceAfterColon = false;
				bool				spaceAfterComma = false;
				bool				crlf = false;
				bool				compact = false;		// available when crlf == true
				const wchar_t*		indentation = nullptr;	// available when crlf == true;

				JsonFormatting();
				JsonFormatting(const JsonFormatting&) = default;
				JsonFormatting(JsonFormatting&&) = default;
				JsonFormatting& operator=(const JsonFormatting&) = default;
				JsonFormatting& operator=(JsonFormatting&&) = default;
			};

			/// <summary>Serialize JSON to string.</summary>
			/// <param name="node">The JSON node to serialize.</param>
			/// <param name="writer">The text writer to receive the string.</param>
			extern void						JsonPrint(Ptr<JsonNode> node, stream::TextWriter& writer, JsonFormatting formatting = {});

			/// <summary>Serialize JSON to string.</summary>
			/// <returns>The serialized string.</returns>
			/// <param name="node">The JSON node to serialize.</param>
			extern WString					JsonToString(Ptr<JsonNode> node, JsonFormatting formatting = {});
		}
	}
}

#endif
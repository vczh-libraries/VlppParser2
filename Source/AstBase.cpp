#include "AstBase.h"

namespace vl
{
	namespace glr
	{
/***********************************************************************
JsonVisitorBase
***********************************************************************/

		void JsonVisitorBase::BeginObject()
		{
			writer.WriteString(L"{");
			indentation++;
			indices.Add(0);
		}

		void JsonVisitorBase::BeginField(const WString& field)
		{
			vint last = indices[indices.Count() - 1];
			if (last > 0)
			{
				writer.WriteString(L", ");
			}
			writer.WriteLine(L"");
			WriteIndent();
			writer.WriteChar(L'\"');
			writer.WriteString(field);
			writer.WriteChar(L'\"');
			writer.WriteString(L": ");
		}

		void JsonVisitorBase::EndField()
		{
			indices[indices.Count() - 1]++;
		}

		void JsonVisitorBase::EndObject()
		{
			indices.RemoveAt(indices.Count() - 1);
			indentation--;
			writer.WriteLine(L"");
			WriteIndent();
			writer.WriteString(L"}");
		}

		void JsonVisitorBase::BeginArray()
		{
			writer.WriteString(L"[");
			indices.Add(0);
		}

		void JsonVisitorBase::BeginArrayItem()
		{
			vint last = indices[indices.Count() - 1];
			if (last > 0)
			{
				writer.WriteString(L", ");
			}
		}

		void JsonVisitorBase::EndArrayItem()
		{
			indices[indices.Count() - 1]++;
		}

		void JsonVisitorBase::EndArray()
		{
			indices.RemoveAt(indices.Count() - 1);
			writer.WriteString(L"]");
		}

		void JsonVisitorBase::WriteIndent()
		{
			for (vint i = 0; i < indentation; i++)
			{
				writer.WriteString(L"    ");
			}
		}

		void JsonVisitorBase::WriteRange(const ParsingTextRange& range)
		{
			writer.WriteString(L"{\"start\": {\"row\": ");
			writer.WriteString(itow(range.start.row));
			writer.WriteString(L", \"column\": ");
			writer.WriteString(itow(range.start.column));
			writer.WriteString(L", \"index\": ");
			writer.WriteString(itow(range.start.index));
			writer.WriteString(L"}, \"end\": {\"row\": ");
			writer.WriteString(itow(range.end.row));
			writer.WriteString(L", \"column\": ");
			writer.WriteString(itow(range.end.column));
			writer.WriteString(L", \"index\": ");
			writer.WriteString(itow(range.end.index));
			writer.WriteString(L"}, \"codeIndex\": ");
			writer.WriteString(itow(range.codeIndex));
			writer.WriteString(L"}");
		}

		void JsonVisitorBase::WriteToken(const ParsingToken& token)
		{
			writer.WriteString(L"{ \"value\": ");
			WriteString(token.value);
			writer.WriteString(L", \"tokenIndex\": ");
			writer.WriteString(itow(token.tokenIndex));
			writer.WriteString(L", \"codeRange\": ");
			WriteRange(token.codeRange);
			writer.WriteString(L"}");
		}

		void JsonVisitorBase::WriteType(const WString& type, ParsingAstBase* node)
		{
			BeginField(L"$ast");
			writer.WriteString(L"{ \"type\": ");
			WriteString(type);
			writer.WriteString(L", \"codeRange\": ");
			WriteRange(node->codeRange);
			writer.WriteString(L"}");
			EndField();
		}

		void JsonVisitorBase::WriteString(const WString& text)
		{
			writer.WriteChar(L'\"');
			JsonEscapeString(text, writer);
			writer.WriteChar(L'\"');
		}

		JsonVisitorBase::JsonVisitorBase(stream::StreamWriter& _writer)
			:writer(_writer)
		{
		}

/***********************************************************************
Json Printing
***********************************************************************/

		void JsonEscapeString(const WString& text, stream::TextWriter& writer)
		{
			const wchar_t* reading = text.Buffer();
			while (wchar_t c = *reading++)
			{
				switch (c)
				{
				case L'\"': writer.WriteString(L"\\\""); break;
				case L'\\': writer.WriteString(L"\\\\"); break;
				case L'/': writer.WriteString(L"\\/"); break;
				case L'\b': writer.WriteString(L"\\b"); break;
				case L'\f': writer.WriteString(L"\\f"); break;
				case L'\n': writer.WriteString(L"\\n"); break;
				case L'\r': writer.WriteString(L"\\r"); break;
				case L'\t': writer.WriteString(L"\\t"); break;
				default: writer.WriteChar(c);
				}
			}
		}

		vuint16_t GetHex(wchar_t c)
		{
			if (L'0' <= c && c <= L'9')
			{
				return c - L'0';
			}
			else if (L'A' <= c && c <= L'F')
			{
				return c - L'A';
			}
			else if (L'a' <= c && c <= L'f')
			{
				return c - L'a';
			}
			else
			{
				return 0;
			}
		}

		void JsonUnescapeString(const WString& text, stream::TextWriter& writer)
		{
			const wchar_t* reading = text.Buffer();
			while (wchar_t c = *reading++)
			{
				if (c == L'\\' && *reading)
				{
					switch (c = *reading++)
					{
					case L'b': writer.WriteChar(L'\b'); break;
					case L'f': writer.WriteChar(L'\f'); break;
					case L'n': writer.WriteChar(L'\n'); break;
					case L'r': writer.WriteChar(L'\r'); break;
					case L't': writer.WriteChar(L'\t'); break;
					case L'u':
						{
							wchar_t h1, h2, h3, h4;
							if ((h1 = reading[0]) && (h2 = reading[1]) && (h3 = reading[2]) && (h4 = reading[3]))
							{
								reading += 4;
								wchar_t h = (wchar_t)(vuint16_t)(
									(GetHex(h1) << 12) +
									(GetHex(h2) << 8) +
									(GetHex(h3) << 4) +
									(GetHex(h4) << 0)
									);
								writer.WriteChar(h);
							}
						}
						break;
					default: writer.WriteChar(c);
					}
				}
				else
				{
					writer.WriteChar(c);
				}
			}
		}
	}
}
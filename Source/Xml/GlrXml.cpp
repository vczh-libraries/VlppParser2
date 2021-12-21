#include "GlrXml.h"
#include "Generated/XmlAst_Traverse.h"

namespace vl
{
	namespace glr
	{
		namespace xml
		{
			using namespace stream;
			using namespace collections;
			using namespace regex;

/***********************************************************************
XmlUnescapeVisitor
***********************************************************************/

			class XmlUnescapeVisitor : public traverse_visitor::AstVisitor
			{
			protected:
				List<RegexToken>&					tokens;

			public:
				XmlUnescapeVisitor(List<RegexToken>& _tokens)
					:tokens(_tokens)
				{
				}

			protected:
				void Traverse(XmlAttribute* node) override
				{
					node->value.value = XmlUnescapeValue(node->value.value.Sub(1, node->value.value.Length() - 2));
				}

				void Traverse(XmlCData* node) override
				{
					node->content.value = XmlUnescapeCData(node->content.value);
				}

				void Traverse(XmlComment* node) override
				{
					node->content.value = XmlUnescapeComment(node->content.value);
				}

				void Traverse(XmlElement* node) override
				{
					vint begin = -1;
					vint end = -1;
					for (vint i = node->subNodes.Count() - 1; i >= -1; i--)
					{
						if (i == -1)
						{
							if (end != -1) begin = 0;
						}
						else if (node->subNodes[i].Cast<XmlText>())
						{
							if (end == -1) end = i;
						}
						else
						{
							if (end != -1) begin = i + 1;
						}
						if (begin != -1 && end != -1)
						{
							vint beginTokenIndex = node->subNodes[begin].Cast<XmlText>()->content.index;
							vint endTokenIndex = node->subNodes[end].Cast<XmlText>()->content.index;

							auto& beginToken = tokens[beginTokenIndex];
							auto& endToken = tokens[endTokenIndex];

							auto textBegin = beginToken.reading;
							auto textEnd = endToken.reading + endToken.length;

							if (beginTokenIndex > 0)
							{
								auto& previousToken = tokens[beginTokenIndex];
								textBegin = previousToken.reading + previousToken.length;
							}

							if (endTokenIndex < tokens.Count() - 1)
							{
								auto& nextToken = tokens[endTokenIndex];
								textEnd = nextToken.reading;
							}

							WString text = WString::CopyFrom(textBegin, vint(textEnd - textBegin));
							ParsingTextRange range(&beginToken, &endToken);

							Ptr<XmlText> xmlText = new XmlText;
							xmlText->codeRange = range;
							xmlText->content.codeRange = range;
							xmlText->content.value = XmlUnescapeValue(text);

							node->subNodes.RemoveRange(begin, end - begin + 1);
							node->subNodes.Insert(begin, xmlText);

							begin = -1;
							end = -1;
						}
					}
				}
			};

/***********************************************************************
XmlPrintVisitor
***********************************************************************/

			class XmlPrintVisitor : public Object, public XmlNode::IVisitor
			{
			public:
				TextWriter&					writer;

				XmlPrintVisitor(TextWriter& _writer)
					:writer(_writer)
				{
				}

				void Visit(XmlText* node)
				{
					writer.WriteString(XmlEscapeValue(node->content.value));
				}

				void Visit(XmlCData* node)
				{
					writer.WriteString(XmlEscapeCData(node->content.value));
				}

				void Visit(XmlAttribute* node)
				{
					writer.WriteString(node->name.value);
					writer.WriteString(L"=\"");
					writer.WriteString(XmlEscapeValue(node->value.value));
					writer.WriteString(L"\"");
				}

				void Visit(XmlComment* node)
				{
					writer.WriteString(XmlEscapeComment(node->content.value));
				}

				void Visit(XmlElement* node)
				{
					writer.WriteChar(L'<');
					writer.WriteString(node->name.value);
					for (auto att : node->attributes)
					{
						writer.WriteChar(L' ');
						Visit(att.Obj());
					}
					if(node->subNodes.Count()==0)
					{
						writer.WriteString(L"/>");
					}
					else
					{
						writer.WriteChar(L'>');
						for (auto subNode : node->subNodes)
						{
							subNode->Accept(this);
						}
						writer.WriteString(L"</");
						writer.WriteString(node->name.value);
						writer.WriteChar(L'>');
					}
				}

				void Visit(XmlInstruction* node)
				{
					writer.WriteString(L"<?");
					writer.WriteString(node->name.value);
					for (auto att : node->attributes)
					{
						writer.WriteChar(L' ');
						Visit(att.Obj());
					}
					writer.WriteString(L"?>");
				}

				void Visit(XmlDocument* node)
				{
					for (auto prolog : node->prologs)
					{
						prolog->Accept(this);
					}
					node->rootElement->Accept(this);
				}
			};

/***********************************************************************
Escaping and Unescaping
***********************************************************************/

			WString XmlEscapeValue(const WString& value)
			{
				WString result;
				const wchar_t* reading = value.Buffer();
				while (wchar_t c = *reading++)
				{
					switch (c)
					{
					case L'<':
						result += L"&lt;";
						break;
					case L'>':
						result += L"&gt;";
						break;
					case L'&':
						result += L"&amp;";
						break;
					case L'\'':
						result += L"&apos;";
						break;
					case L'\"':
						result += L"&quot;";
						break;
					default:
						result += WString::FromChar(c);
					}
				}
				return result;
			}

			WString XmlUnescapeValue(const WString& value)
			{
				WString result;
				const wchar_t* reading = value.Buffer();
				while (*reading)
				{
					if (wcsncmp(reading, L"&lt;", 4) == 0)
					{
						result += WString::FromChar(L'<');
						reading += 4;
					}
					else if (wcsncmp(reading, L"&gt;", 4) == 0)
					{
						result += WString::FromChar(L'>');
						reading += 4;
					}
					else if (wcsncmp(reading, L"&amp;", 5) == 0)
					{
						result += WString::FromChar(L'&');
						reading += 5;
					}
					else if (wcsncmp(reading, L"&apos;", 6) == 0)
					{
						result += WString::FromChar(L'\'');
						reading += 6;
					}
					else if (wcsncmp(reading, L"&quot;", 6) == 0)
					{
						result += WString::FromChar(L'\"');
						reading += 6;
					}
					else
					{
						result += WString::FromChar(*reading++);
					}
				}
				return result;
			}

			WString XmlEscapeCData(const WString& value)
			{
				return L"<![CDATA[" + value + L"]]>";
			}

			WString XmlUnescapeCData(const WString& value)
			{
				return value.Sub(9, value.Length() - 12);
			}

			WString XmlEscapeComment(const WString& value)
			{
				return L"<!--" + value + L"-->";
			}

			WString XmlUnescapeComment(const WString& value)
			{
				return value.Sub(4, value.Length() - 7);
			}

/***********************************************************************
Parsing and Printing
***********************************************************************/

			Ptr<XmlDocument> XmlParseDocument(const WString& input, const Parser& parser)
			{
				List<RegexToken> tokens;
				parser.Lexer().Parse(input).ReadToEnd(tokens, parser.LexerDeleter());
				auto ast = parser.ParseXDocument(tokens);
				XmlUnescapeVisitor(tokens).InspectInto(ast.Obj());
				return ast;
			}

			Ptr<XmlElement> XmlParseElement(const WString& input, const Parser& parser)
			{
				List<RegexToken> tokens;
				parser.Lexer().Parse(input).ReadToEnd(tokens, parser.LexerDeleter());
				auto ast = parser.ParseXElement(tokens);
				XmlUnescapeVisitor(tokens).InspectInto(ast.Obj());
				return ast;
			}

			void XmlPrint(Ptr<XmlNode> node, stream::TextWriter& writer)
			{
				XmlPrintVisitor visitor(writer);
				node->Accept(&visitor);
			}

			void XmlPrintContent(Ptr<XmlElement> element, stream::TextWriter& writer)
			{
				XmlPrintVisitor visitor(writer);
				for (auto node : element->subNodes)
				{
					node->Accept(&visitor);
				}
			}

			WString XmlToString(Ptr<XmlNode> node)
			{
				return GenerateToStream([&](StreamWriter& writer)
				{
					XmlPrint(node, writer);
				});
			}

/***********************************************************************
Utility
***********************************************************************/

			Ptr<XmlAttribute> XmlGetAttribute(Ptr<XmlElement> element, const WString& name)
			{
				return XmlGetAttribute(element.Obj(), name);
			}

			Ptr<XmlElement> XmlGetElement(Ptr<XmlElement> element, const WString& name)
			{
				return XmlGetElement(element.Obj(), name);
			}

			collections::LazyList<Ptr<XmlElement>> XmlGetElements(Ptr<XmlElement> element)
			{
				return XmlGetElements(element.Obj());
			}

			collections::LazyList<Ptr<XmlElement>> XmlGetElements(Ptr<XmlElement> element, const WString& name)
			{
				return XmlGetElements(element.Obj(), name);
			}

			WString XmlGetValue(Ptr<XmlElement> element)
			{
				return XmlGetValue(element.Obj());
			}

			Ptr<XmlAttribute> XmlGetAttribute(XmlElement* element, const WString& name)
			{
				for (auto att : element->attributes)
				{
					if (att->name.value == name)
					{
						return att;
					}
				}
				return nullptr;
			}

			Ptr<XmlElement> XmlGetElement(XmlElement* element, const WString& name)
			{
				for (auto node : element->subNodes)
				{
					Ptr<XmlElement> subElement = node.Cast<XmlElement>();
					if (subElement && subElement->name.value == name)
					{
						return subElement;
					}
				}
				return nullptr;
			}

			collections::LazyList<Ptr<XmlElement>> XmlGetElements(XmlElement* element)
			{
				return From(element->subNodes).FindType<XmlElement>();
			}

			collections::LazyList<Ptr<XmlElement>> XmlGetElements(XmlElement* element, const WString& name)
			{
				return XmlGetElements(element)
					.Where([name](auto&& e) {return e->name.value == name; });
			}

			WString XmlGetValue(XmlElement* element)
			{
				WString result;
				for (auto node : element->subNodes)
				{
					if (auto text = node.Cast<XmlText>())
					{
						result += text->content.value;
					}
					else if (auto text = node.Cast<XmlCData>())
					{
						result += text->content.value;
					}
				}
				return result;
			}

/***********************************************************************
XmlElementWriter
***********************************************************************/

			XmlElementWriter::XmlElementWriter(Ptr<XmlElement> _element, const XmlElementWriter* _previousWriter)
				: element(_element)
				, previousWriter(_previousWriter)
			{
			}

			XmlElementWriter::~XmlElementWriter()
			{
			}

			const XmlElementWriter& XmlElementWriter::Attribute(const WString& name, const WString& value)const
			{
				Ptr<XmlAttribute> node = new XmlAttribute;
				node->name.value = name;
				node->value.value = value;
				element->attributes.Add(node);
				return *this;
			}

			XmlElementWriter XmlElementWriter::Element(const WString& name)const
			{
				Ptr<XmlElement> node = new XmlElement;
				node->name.value = name;
				element->subNodes.Add(node);
				return XmlElementWriter(node, this);
			}

			const XmlElementWriter& XmlElementWriter::End()const
			{
				return *previousWriter;
			}

			const XmlElementWriter& XmlElementWriter::Text(const WString& value)const
			{
				Ptr<XmlText> node = new XmlText;
				node->content.value = value;
				element->subNodes.Add(node);
				return *this;
			}

			const XmlElementWriter& XmlElementWriter::CData(const WString& value)const
			{
				Ptr<XmlCData> node = new XmlCData;
				node->content.value = value;
				element->subNodes.Add(node);
				return *this;
			}

			const XmlElementWriter& XmlElementWriter::Comment(const WString& value)const
			{
				Ptr<XmlComment> node = new XmlComment;
				node->content.value = value;
				element->subNodes.Add(node);
				return *this;
			}
		}
	}
}

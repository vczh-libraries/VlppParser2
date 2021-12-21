/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
Parser::ParsingXml

***********************************************************************/

#ifndef VCZH_PARSER2_BUILTIN_XML
#define VCZH_PARSER2_BUILTIN_XML

#include "Generated/XmlParser.h"

namespace vl
{
	namespace glr
	{
		namespace xml
		{
			extern WString							XmlEscapeValue(const WString& value);
			extern WString							XmlUnescapeValue(const WString& value);
			extern WString							XmlEscapeCData(const WString& value);
			extern WString							XmlUnescapeCData(const WString& value);
			extern WString							XmlEscapeComment(const WString& value);
			extern WString							XmlUnescapeComment(const WString& value);

			/// <summary>Deserialize XML document from string.</summary>
			/// <returns>The deserialized XML node.</returns>
			/// <param name="input">The XML code.</param>
			/// <param name="parser">The generated parser.</param>
			extern Ptr<XmlDocument>								XmlParseDocument(const WString& input, Parser& parser);

			/// <summary>Deserialize XML element from string.</summary>
			/// <returns>The deserialized XML node.</returns>
			/// <param name="input">The XML code.</param>
			/// <param name="parser">The generated parser.</param>
			extern Ptr<XmlElement>								XmlParseElement(const WString& input, Parser& parser);

			/// <summary>Serialize XML to string.</summary>
			/// <param name="node">The XML node to serialize.</param>
			/// <param name="writer">The text writer to receive the string.</param>
			extern void											XmlPrint(Ptr<XmlNode> node, stream::TextWriter& writer);

			/// <summary>Serialize sub nodes in an XML element to string.</summary>
			/// <param name="element">The XML element in which sub nodes are to be serialized.</param>
			/// <param name="writer">The text writer to receive the string.</param>
			extern void											XmlPrintContent(Ptr<XmlElement> element, stream::TextWriter& writer);

			/// <summary>Serialize XML to string.</summary>
			/// <returns>The serialized string.</returns>
			/// <param name="node">The XML node to serialize.</param>
			extern WString										XmlToString(Ptr<XmlNode> node);

			/// <summary>Try to read an attribute in an XML element.</summary>
			/// <returns>The expected attribute. Returns null if it doesn't exist.</returns>
			/// <param name="element">The element to find the attribute.</param>
			/// <param name="name">The name of the attribute.</param>
			extern Ptr<XmlAttribute>							XmlGetAttribute(Ptr<XmlElement> element, const WString& name);

			/// <summary>Try to read a sub element in an XML element.</summary>
			/// <returns>The expected sub element. Returns null if it doesn't exist. If there are multiple elements of the expected name, returns the first one.</returns>
			/// <param name="element">The element to find the sub element.</param>
			/// <param name="name">The name of the sub element.</param>
			extern Ptr<XmlElement>								XmlGetElement(Ptr<XmlElement> element, const WString& name);

			/// <summary>Get all sub elements in an XML element.</summary>
			/// <returns>All sub elements in an XML element.</returns>
			/// <param name="element">The container XML element.</param>
			extern collections::LazyList<Ptr<XmlElement>>		XmlGetElements(Ptr<XmlElement> element);

			/// <summary>Try to read sub elements in an XML element.</summary>
			/// <returns>Expected sub elements. All sub elements of the expected name will be returned.</returns>
			/// <param name="element">The element to find sub elements.</param>
			/// <param name="name">The name of sub elements.</param>
			extern collections::LazyList<Ptr<XmlElement>>		XmlGetElements(Ptr<XmlElement> element, const WString& name);

			/// <summary>Serialize contents in an XML element.</summary>
			/// <returns>The serialized contents in an XML element.</returns>
			/// <param name="element">The XML element in which contents are to be serialized.</param>
			extern WString										XmlGetValue(Ptr<XmlElement> element);

			extern Ptr<XmlAttribute>							XmlGetAttribute(XmlElement* element, const WString& name);
			extern Ptr<XmlElement>								XmlGetElement(XmlElement* element, const WString& name);
			extern collections::LazyList<Ptr<XmlElement>>		XmlGetElements(XmlElement* element);
			extern collections::LazyList<Ptr<XmlElement>>		XmlGetElements(XmlElement* element, const WString& name);
			extern WString										XmlGetValue(XmlElement* element);

			class XmlElementWriter : public Object
			{
			protected:
				Ptr<XmlElement>					element;
				const XmlElementWriter*			previousWriter;

			public:
				XmlElementWriter(Ptr<XmlElement> _element, const XmlElementWriter* _previousWriter=0);
				~XmlElementWriter();

				const XmlElementWriter&			Attribute(const WString& name, const WString& value)const;
				XmlElementWriter				Element(const WString& name)const;
				const XmlElementWriter&			End()const;
				const XmlElementWriter&			Text(const WString& value)const;
				const XmlElementWriter&			CData(const WString& value)const;
				const XmlElementWriter&			Comment(const WString& value)const;
			};
		}
	}
}

#endif
/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_ASTBASE
#define VCZH_PARSER2_ASTBASE

#include <VlppReflection.h>

namespace vl
{
	namespace glr
	{
/***********************************************************************
Location
***********************************************************************/

		/// <summary>A type representing text position.</summary>
		struct ParsingTextPos
		{
			static const vint	UnknownValue = -2;
			/// <summary>Character index, begins at 0.</summary>
			vint				index;
			/// <summary>Row number, begins at 0.</summary>
			vint				row;
			/// <summary>Column number, begins at 0.</summary>
			vint				column;

			ParsingTextPos()
				:index(UnknownValue)
				, row(UnknownValue)
				, column(UnknownValue)
			{
			}

			ParsingTextPos(vint _index)
				:index(_index)
				, row(UnknownValue)
				, column(UnknownValue)
			{
			}

			ParsingTextPos(vint _row, vint _column)
				:index(UnknownValue)
				, row(_row)
				, column(_column)
			{
			}

			ParsingTextPos(vint _index, vint _row, vint _column)
				:index(_index)
				, row(_row)
				, column(_column)
			{
			}

			/// <summary>Test if this position a valid position.</summary>
			/// <returns>Returns true if this position is a valid position.</returns>
			bool IsInvalid()const
			{
				return index < 0 && row < 0 && column < 0;
			}

			static vint Compare(const ParsingTextPos& a, const ParsingTextPos& b)
			{
				if (a.IsInvalid() && b.IsInvalid())
				{
					return 0;
				}
				else if (a.IsInvalid())
				{
					return -1;
				}
				else if (b.IsInvalid())
				{
					return 1;
				}
				else if (a.index >= 0 && b.index >= 0)
				{
					return a.index - b.index;
				}
				else if (a.row >= 0 && a.column >= 0 && b.row >= 0 && b.column >= 0)
				{
					if (a.row == b.row)
					{
						return a.column - b.column;
					}
					else
					{
						return a.row - b.row;
					}
				}
				else
				{
					return 0;
				}
			}

			bool operator==(const ParsingTextPos& pos)const { return Compare(*this, pos) == 0; }
			bool operator!=(const ParsingTextPos& pos)const { return Compare(*this, pos) != 0; }
			bool operator<(const ParsingTextPos& pos)const { return Compare(*this, pos) < 0; }
			bool operator<=(const ParsingTextPos& pos)const { return Compare(*this, pos) <= 0; }
			bool operator>(const ParsingTextPos& pos)const { return Compare(*this, pos) > 0; }
			bool operator>=(const ParsingTextPos& pos)const { return Compare(*this, pos) >= 0; }
		};

		/// <summary>A type representing text range.</summary>
		struct ParsingTextRange
		{
			/// <summary>Text position for the first character.</summary>
			ParsingTextPos	start;
			/// <summary>Text position for the last character.</summary>
			ParsingTextPos	end;
			/// <summary>Code index, refer to [F:vl.regex.RegexToken.codeIndex]</summary>
			vint			codeIndex;

			ParsingTextRange()
				:codeIndex(-1)
			{
				end.index = -1;
				end.column = -1;
			}

			ParsingTextRange(const ParsingTextPos& _start, const ParsingTextPos& _end, vint _codeIndex = -1)
				:start(_start)
				, end(_end)
				, codeIndex(_codeIndex)
			{
			}

			ParsingTextRange(const regex::RegexToken* startToken, const regex::RegexToken* endToken)
				:codeIndex(startToken->codeIndex)
			{
				start.index = startToken->start;
				start.row = startToken->rowStart;
				start.column = startToken->columnStart;
				end.index = endToken->start + endToken->length - 1;
				end.row = endToken->rowEnd;
				end.column = endToken->columnEnd;
			}

			bool operator==(const ParsingTextRange& range)const { return start == range.start && end == range.end; }
			bool operator!=(const ParsingTextRange& range)const { return start != range.start || end != range.end; }
			bool Contains(const ParsingTextPos& pos)const { return start <= pos && pos <= end; }
			bool Contains(const ParsingTextRange& range)const { return start <= range.start && range.end <= end; }
		};

		/// <summary>Base type of all strong typed syntax tree. Normally all strong typed syntax tree are generated from a grammar file using ParserGen.exe in Tools project.</summary>
		class ParsingAstBase : public Object, public reflection::Description<ParsingAstBase>
		{
		public:
			/// <summary>Range of all tokens that form this object.</summary>
			ParsingTextRange					codeRange;
		};

		/// <summary>Strong typed token syntax node, for all class fields of type "token" in the grammar file. See [T:vl.parsing.tabling.ParsingTable] for details.</summary>
		struct ParsingToken
		{
		public:
			/// <summary>Range of all tokens that form this object.</summary>
			ParsingTextRange					codeRange;
			/// <summary>Type of the token, representing the index of a regular expression that creates this token in the regular expression list in the grammar file.</summary>
			vint								tokenIndex = -1;
			/// <summary>Content of the token.</summary>
			WString								value;
		};

		class CopyVisitorBase : public Object
		{
		protected:
			Ptr<ParsingAstBase>					result;
		};

		class JsonVisitorBase : public Object
		{
		private:
			stream::StreamWriter&				writer;
			vint								indentation = 0;
			collections::List<vint>				indices;

		protected:

			void								BeginObject();
			void								BeginField(const WString& field);
			void								EndField();
			void								EndObject();
			void								BeginArray();
			void								BeginArrayItem();
			void								EndArrayItem();
			void								EndArray();
			void								WriteIndent();
			void								WriteRange(const ParsingTextRange& range);
			void								WriteToken(const ParsingToken& token);
			void								WriteType(const WString& type, ParsingAstBase* node);
			void								WriteString(const WString& text);
			void								WriteNull();
		public:
			JsonVisitorBase(stream::StreamWriter& _writer);
		};

		extern void								JsonEscapeString(const WString& text, stream::TextWriter& writer);
		extern void								JsonUnescapeString(const WString& text, stream::TextWriter& writer);
	}
}

#endif
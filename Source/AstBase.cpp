#include "AstBase.h"

namespace vl
{
	namespace glr
	{
		using namespace collections;

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
			if (printTokenCodeRange)
			{
				writer.WriteString(L"{ \"value\": ");
				WriteString(token.value);
				writer.WriteString(L", \"tokenIndex\": ");
				writer.WriteString(itow(token.tokenIndex));
				writer.WriteString(L", \"codeRange\": ");
				WriteRange(token.codeRange);
				writer.WriteString(L"}");
			}
			else
			{
				WriteString(token.value);
			}
		}

		void JsonVisitorBase::WriteType(const WString& type, ParsingAstBase* node)
		{
			if (printAstType && printAstCodeRange)
			{
				BeginField(L"$ast");
				writer.WriteString(L"{ \"type\": ");
				WriteString(type);
				writer.WriteString(L", \"codeRange\": ");
				WriteRange(node->codeRange);
				writer.WriteString(L"}");
				EndField();
			}
			else if (printAstType)
			{
				BeginField(L"$ast");
				WriteString(type);
				EndField();
			}
			else if (printAstCodeRange)
			{
				BeginField(L"$ast");
				writer.WriteString(L"{ \"codeRange\": ");
				WriteRange(node->codeRange);
				writer.WriteString(L"}");
				EndField();
			}
		}

		void JsonVisitorBase::WriteString(const WString& text)
		{
			writer.WriteChar(L'\"');
			JsonEscapeString(text, writer);
			writer.WriteChar(L'\"');
		}

		void JsonVisitorBase::WriteNull()
		{
			writer.WriteString(L"null");
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

/***********************************************************************
AstInsReceiverBase
***********************************************************************/

		void AstInsReceiverBase::EnsureContinuable()
		{
			if (corrupted)
			{
				throw AstInsException(
					L"An exception has been thrown therefore this receiver cannot be used anymore.",
					AstInsErrorType::Corrupted
					);
			}
			if (finished)
			{
				throw AstInsException(
					L"The finished instruction has been executed therefore this receiver cannot be used anymore.",
					AstInsErrorType::Finished
					);
			}
		}

		void AstInsReceiverBase::AssignToken(ParsingToken& parsingToken, const regex::RegexToken& regexToken)
		{
			parsingToken.codeRange.start.row = regexToken.rowStart;
			parsingToken.codeRange.start.column = regexToken.columnStart;
			parsingToken.codeRange.end.row = regexToken.rowEnd;
			parsingToken.codeRange.end.column = regexToken.columnEnd;
			parsingToken.codeRange.codeIndex = regexToken.codeIndex;
			parsingToken.tokenIndex = regexToken.token;
			parsingToken.value = WString::CopyFrom(regexToken.reading, regexToken.length);
		}

		void AstInsReceiverBase::Execute(AstIns instruction, const regex::RegexToken& token)
		{
			EnsureContinuable();
			try
			{
				if (created.Count() == 0 && instruction.type != AstInsType::BeginObject)
				{
					throw AstInsException(
						L"There is no created objects.",
						AstInsErrorType::NoRootObject
						);
				}
				switch (instruction.type)
				{
				case AstInsType::Token:
					{
						ObjectOrToken value;
						value.token = token;
						pushed.Add(value);
					}
					break;
				case AstInsType::EnumItem:
					{
						ObjectOrToken value;
						value.enumItem = instruction.param;
						pushed.Add(value);
					}
					break;
				case AstInsType::BeginObject:
					{
						auto value = CreateAstNode(instruction.param);
						value->codeRange.start.row = token.rowStart;
						value->codeRange.start.column = token.columnStart;
						value->codeRange.end.row = token.rowEnd;
						value->codeRange.end.column = token.columnEnd;
						value->codeRange.codeIndex = token.codeIndex;
						created.Add(value);
					}
					break;
				case AstInsType::EndObject:
					{
						if (pushed.Count() > 0)
						{
							throw AstInsException(
								L"There are still unassigned objects leaving when executing EndObject.",
								AstInsErrorType::UnassignedObjectLeaving
								);
						}

						ObjectOrToken value;
						value.object = created[created.Count() - 1];
						created.RemoveAt(created.Count() - 1);

						value.object->codeRange.end.row = token.rowEnd;
						value.object->codeRange.end.column = token.columnEnd;
						pushed.Add(value);
					}
					break;
				case AstInsType::Field:
					{
						if (pushed.Count() == 0)
						{
							throw AstInsException(
								L"There is no pushed value to be assigned to a field.",
								AstInsErrorType::MissingFieldValue
								);
						}

						auto value = pushed[pushed.Count() - 1];
						pushed.RemoveAt(pushed.Count() - 1);
						if (value.object)
						{
							SetField(created[created.Count() - 1].Obj(), instruction.param, value.object);
						}
						else if (value.enumItem != -1)
						{
							SetField(created[created.Count() - 1].Obj(), instruction.param, value.enumItem);
						}
						else
						{
							SetField(created[created.Count() - 1].Obj(), instruction.param, value.token);
						}
					}
					break;
				case AstInsType::ResolveAmbiguity:
					{
						if (instruction.count <= 0 || pushed.Count() < instruction.count)
						{
							throw AstInsException(
								L"There are not enough candidates to create an ambiguity node.",
								AstInsErrorType::MissingAmbiguityCandidate
								);
						}

						for (vint i = 0; i < instruction.count; i++)
						{
							if (!pushed[pushed.Count() - i - 1].object)
							{
								throw AstInsException(
									L"Tokens or enum items cannot be ambiguity candidates.",
									AstInsErrorType::AmbiguityCandidateIsNotObject
									);
							}
						}

						Array<Ptr<ParsingAstBase>> candidates(instruction.count);
						for (vint i = 0; i < instruction.count; i++)
						{
							auto value = pushed[pushed.Count() - 1];
							pushed.RemoveAt(pushed.Count() - 1);
							candidates[i] = value.object;
						}

						ObjectOrToken value;
						value.object = ResolveAmbiguity(instruction.param, candidates);
						pushed.Add(value);
					}
					break;
				}
			}
			catch (const AstInsException&)
			{
				corrupted = true;
				throw;
			}
		}

		Ptr<ParsingAstBase> AstInsReceiverBase::Finished()
		{
			EnsureContinuable();
			try
			{
				if (created.Count() > 1)
				{
					throw AstInsException(
						L"No more instruction but the root object has not been completed yet.",
						AstInsErrorType::InstructionNotComplete
						);
				}

				auto object = created[0];
				created.Clear();
				finished = true;
				return object;
			}
			catch (const AstInsException&)
			{
				corrupted = true;
				throw;
			}
		}
	}
}
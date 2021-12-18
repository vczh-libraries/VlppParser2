#include "AstBase.h"

namespace vl
{
	namespace glr
	{
		using namespace collections;
		using namespace stream;

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
				writer.WriteString(L",");
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

		void AstInsReceiverBase::SetField(ParsingAstBase* object, vint32_t field, const ObjectOrToken& value)
		{
			if (value.object)
			{
				SetField(object, field, value.object);
			}
			else if (value.enumItem != -1)
			{
				SetField(object, field, value.enumItem);
			}
			else
			{
				SetField(object, field, value.token);
			}
		}

		void AstInsReceiverBase::PushCreated(CreatedObject&& createdObject)
		{
			if (created.Count() == 0)
			{
				created.Add(std::move(createdObject));
			}
			else
			{
				auto& top = created[created.Count() - 1];
				if (!top.object && top.delayedFieldAssignments.Count() == 0 && top.pushedCount == createdObject.pushedCount)
				{
					top.object = createdObject.object;
					top.extraEmptyDfaBelow++;
				}
				else
				{
					created.Add(std::move(createdObject));
				}
			}
		}

		const AstInsReceiverBase::CreatedObject& AstInsReceiverBase::TopCreated()
		{
			return created[created.Count() - 1];
		}

		void AstInsReceiverBase::PopCreated()
		{
			auto& top = created[created.Count() - 1];
			if (top.extraEmptyDfaBelow == 0)
			{
				created.RemoveAt(created.Count() - 1);
			}
			else if (top.object)
			{
				top.object = nullptr;
				top.delayedFieldAssignments.Clear();
				top.extraEmptyDfaBelow--;
			}
		}

		void AstInsReceiverBase::DelayAssign(FieldAssignment&& fa)
		{
			created[created.Count() - 1].delayedFieldAssignments.Add(std::move(fa));
		}

		void AstInsReceiverBase::Execute(AstIns instruction, const regex::RegexToken& token)
		{
			EnsureContinuable();
			try
			{
				if (created.Count() == 0 && instruction.type != AstInsType::BeginObject)
				{
					switch (instruction.type)
					{
					case AstInsType::BeginObject:
					case AstInsType::BeginObjectLeftRecursive:
					case AstInsType::DelayFieldAssignment:
					case AstInsType::ResolveAmbiguity:
					case AstInsType::AccumulatedDfa:
						break;
					default:
						throw AstInsException(
							L"There is no created objects.",
							AstInsErrorType::NoRootObject
							);
					}
				}

				vint expectedLeavings = 0;
				if (created.Count() > 0)
				{
					expectedLeavings = TopCreated().pushedCount;
				}

				switch (instruction.type)
				{
				case AstInsType::Token:
					{
						pushed.Add(ObjectOrToken{ token });
					}
					break;
				case AstInsType::EnumItem:
					{
						pushed.Add(ObjectOrToken{ instruction.param });
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
						PushCreated(CreatedObject{ value,pushed.Count() });
					}
					break;
				case AstInsType::BeginObjectLeftRecursive:
					{
						if (pushed.Count() < expectedLeavings + 1)
						{
							throw AstInsException(
								L"There is no pushed value to create left recursive object.",
								AstInsErrorType::MissingLeftRecursiveValue
								);
						}

						auto subValue = pushed[pushed.Count() - 1];
						if (subValue.object)
						{
							auto value = CreateAstNode(instruction.param);
							value->codeRange = subValue.object->codeRange;
							PushCreated(CreatedObject{ value,pushed.Count() - 1 });
						}
						else
						{
							throw AstInsException(
								L"The pushed value to create left recursive object is not an object.",
								AstInsErrorType::LeftRecursiveValueIsNotObject
								);
						}
					}
					break;
				case AstInsType::DelayFieldAssignment:
					{
						PushCreated(CreatedObject{ nullptr,pushed.Count() });
					}
					break;
				case AstInsType::ReopenObject:
					{
						auto& createdObject = created[created.Count() - 1];
						if (createdObject.object)
						{
							throw AstInsException(
								L"DelayFieldAssignment is not submitted before ReopenObject.",
								AstInsErrorType::MissingDfaBeforeReopen
								);
						}
						if (pushed.Count() < expectedLeavings + 1)
						{
							throw AstInsException(
								L"There is no pushed value to reopen.",
								AstInsErrorType::MissingValueToReopen
								);
						}
						if (pushed.Count() > expectedLeavings + 1)
						{
							throw AstInsException(
								L"The value to reopen is not the only unassigned value.",
								AstInsErrorType::TooManyUnassignedValues
								);
						}

						auto value = pushed[pushed.Count() - 1];
						if (value.object)
						{
							pushed.RemoveAt(pushed.Count() - 1);
							createdObject.object = value.object;

							for (auto&& dfa : createdObject.delayedFieldAssignments)
							{
								SetField(createdObject.object.Obj(), dfa.field, dfa.value);
							}
							createdObject.delayedFieldAssignments.Clear();
						}
						else
						{
							throw AstInsException(
								L"The pushed value to reopen is not an object.",
								AstInsErrorType::ReopenedValueIsNotObject
								);
						}
					}
					break;
				case AstInsType::EndObject:
					{
						Ptr<ParsingAstBase> objectToPush;
						{
							auto& createdObject = TopCreated();
							if (!createdObject.object)
							{
								throw AstInsException(
									L"There is no created objects after DelayFieldAssignment.",
									AstInsErrorType::NoRootObjectAfterDfa
									);
							}
							if (pushed.Count() > createdObject.pushedCount)
							{
								throw AstInsException(
									L"There are still values to assign to fields before finishing an object.",
									AstInsErrorType::LeavingUnassignedValues
									);
							}

							objectToPush = createdObject.object;
							PopCreated();
						}

						objectToPush->codeRange.end.row = token.rowEnd;
						objectToPush->codeRange.end.column = token.columnEnd;
						pushed.Add(ObjectOrToken{ objectToPush });
					}
					break;
				case AstInsType::DiscardValue:
					{
						auto& createdObject = TopCreated();
						if (pushed.Count() <= createdObject.pushedCount)
						{
							throw AstInsException(
								L"There is no pushed value to discard.",
								AstInsErrorType::MissingValueToDiscard
								);
						}
						pushed.RemoveAt(pushed.Count() - 1);
					}
					break;
				case AstInsType::Field:
					{
						auto& createdObject = TopCreated();
						if (pushed.Count() <= createdObject.pushedCount)
						{
							throw AstInsException(
								L"There is no pushed value to be assigned to a field.",
								AstInsErrorType::MissingFieldValue
								);
						}

						auto value = pushed[pushed.Count() - 1];
						pushed.RemoveAt(pushed.Count() - 1);

						if (createdObject.object)
						{
							SetField(createdObject.object.Obj(), instruction.param, value);
						}
						else
						{
							DelayAssign({ value,instruction.param });
						}
					}
					break;
				case AstInsType::ResolveAmbiguity:
					{
						if (instruction.count <= 0 || pushed.Count() < expectedLeavings + instruction.count)
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

						pushed.Add(ObjectOrToken{ ResolveAmbiguity(instruction.param, candidates) });
					}
					break;
				case AstInsType::AccumulatedDfa:
					{
						PushCreated(CreatedObject{ nullptr,pushed.Count() });
						created[created.Count() - 1].extraEmptyDfaBelow += instruction.count - 1;
					}
					break;
				case AstInsType::AccumulatedEoRo:
					{
						while (instruction.count > 0)
						{
							auto& createdObject = created[created.Count() - 1];
							if (!createdObject.object)
							{
								throw AstInsException(
									L"There is no created objects after DelayFieldAssignment.",
									AstInsErrorType::NoRootObjectAfterDfa
									);
							}
							if (pushed.Count() > createdObject.pushedCount)
							{
								throw AstInsException(
									L"There are still values to assign to fields before finishing an object.",
									AstInsErrorType::LeavingUnassignedValues
									);
							}

							if (createdObject.extraEmptyDfaBelow >= instruction.count)
							{
								createdObject.extraEmptyDfaBelow -= instruction.count;
								instruction.count = 0;
							}
							else
							{
								instruction.count -= createdObject.extraEmptyDfaBelow + 1;
								createdObject.extraEmptyDfaBelow = 0;
								Execute({ AstInsType::EndObject }, token);
								Execute({ AstInsType::ReopenObject }, token);
							}
						}
					}
					break;
				default:
					CHECK_FAIL(L"vl::glr::AstInsReceiverBase::Execute(AstIns, const regex::RegexToken&)#Unknown Instruction.");
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
				if (created.Count() > 0 || pushed.Count() > 1)
				{
					throw AstInsException(
						L"No more instruction but the root object has not been completed yet.",
						AstInsErrorType::InstructionNotComplete
						);
				}

				auto object = pushed[0].object;
				if (!object)
				{
					throw AstInsException(
						L"No more instruction but the root object has not been completed yet.",
						AstInsErrorType::InstructionNotComplete
						);
				}
				pushed.Clear();
				finished = true;
				return object;
			}
			catch (const AstInsException&)
			{
				corrupted = true;
				throw;
			}
		}

/***********************************************************************
IAstInsReceiver (Code Generation Error Templates)
***********************************************************************/

		Ptr<ParsingAstBase> AssemblyThrowCannotCreateAbstractType(vint32_t type, const wchar_t* cppTypeName)
		{
			if (cppTypeName)
			{
				throw AstInsException(
					WString::Unmanaged(L"Unable to create abstract class \"") +
					WString::Unmanaged(cppTypeName) +
					WString::Unmanaged(L"\"."),
					AstInsErrorType::UnsupportedAbstractType, type);
			}
			else
			{
				throw AstInsException(L"The type id does not exist.", vl::glr::AstInsErrorType::UnknownType, type);
			}
		}

		void AssemblyThrowFieldNotObject(vint32_t field, const wchar_t* cppFieldName)
		{
			if (cppFieldName)
			{
				throw AstInsException(
					WString::Unmanaged(L"Field \"") +
					WString::Unmanaged(cppFieldName) +
					WString::Unmanaged(L"\" is not an object."),
					AstInsErrorType::ObjectTypeMismatchedToField, field);
			}
			else
			{
				throw AstInsException(L"The field id does not exist.", vl::glr::AstInsErrorType::UnknownField, field);
			}
		}

		void AssemblyThrowFieldNotToken(vint32_t field, const wchar_t* cppFieldName)
		{
			if (cppFieldName)
			{
				throw AstInsException(
					WString::Unmanaged(L"Field \"") +
					WString::Unmanaged(cppFieldName) +
					WString::Unmanaged(L"\" is not a token."),
					AstInsErrorType::ObjectTypeMismatchedToField, field);
			}
			else
			{
				throw AstInsException(L"The field id does not exist.", vl::glr::AstInsErrorType::UnknownField, field);
			}
		}

		void AssemblyThrowFieldNotEnum(vint32_t field, const wchar_t* cppFieldName)
		{
			if (cppFieldName)
			{
				throw AstInsException(
					WString::Unmanaged(L"Field \"") +
					WString::Unmanaged(cppFieldName) +
					WString::Unmanaged(L"\" is not an enum item."),
					AstInsErrorType::ObjectTypeMismatchedToField, field);
			}
			else
			{
				throw AstInsException(L"The field id does not exist.", vl::glr::AstInsErrorType::UnknownField, field);
			}
		}

		Ptr<ParsingAstBase> AssemblyThrowTypeNotAllowAmbiguity(vint32_t type, const wchar_t* cppTypeName)
		{
			if (cppTypeName)
			{
				throw AstInsException(
					WString::Unmanaged(L"Type \"") +
					WString::Unmanaged(cppTypeName) +
					WString::Unmanaged(L"\" is not configured to allow ambiguity."),
					AstInsErrorType::UnsupportedAmbiguityType, type);
			}
			else
			{
				throw AstInsException(L"The type id does not exist.", vl::glr::AstInsErrorType::UnknownType, type);
			}
		}

/***********************************************************************
Compression
***********************************************************************/

		void DecompressSerializedData(const char** buffer, bool decompress, vint solidRows, vint rows, vint block, vint remain, stream::IStream& outputStream)
		{
			if (decompress)
			{
				MemoryStream compressedStream;
				DecompressSerializedData(buffer, false, solidRows, rows, block, remain, compressedStream);
				compressedStream.SeekFromBegin(0);
				DecompressStream(compressedStream, outputStream);
			}
			else
			{
				for (vint i = 0; i < rows; i++)
				{
					vint size = i == solidRows ? remain : block;
					outputStream.Write((void*)buffer[i], size);
				}
			}
		}
	}

/***********************************************************************
Reflection
***********************************************************************/

	namespace reflection
	{
		namespace description
		{
#ifndef VCZH_DEBUG_NO_REFLECTION
			IMPL_TYPE_INFO_RENAME(vl::glr::ParsingTextPos, system::ParsingTextPos)
			IMPL_TYPE_INFO_RENAME(vl::glr::ParsingTextRange, system::ParsingTextRange)
			IMPL_TYPE_INFO_RENAME(vl::glr::ParsingToken, system::ParsingToken)
			IMPL_TYPE_INFO_RENAME(vl::glr::ParsingAstBase, system::ParsingAstBase)

#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA

			BEGIN_STRUCT_MEMBER(vl::glr::ParsingTextPos)
				STRUCT_MEMBER(index)
				STRUCT_MEMBER(row)
				STRUCT_MEMBER(column)
			END_STRUCT_MEMBER(vl::glr::ParsingTextPos)

			BEGIN_STRUCT_MEMBER(vl::glr::ParsingTextRange)
				STRUCT_MEMBER(start)
				STRUCT_MEMBER(end)
				STRUCT_MEMBER(codeIndex)
			END_STRUCT_MEMBER(vl::glr::ParsingTextRange)

			BEGIN_STRUCT_MEMBER(vl::glr::ParsingToken)
				STRUCT_MEMBER(codeRange)
				STRUCT_MEMBER(tokenIndex)
				STRUCT_MEMBER(value)
			END_STRUCT_MEMBER(vl::glr::ParsingToken)

			BEGIN_CLASS_MEMBER(vl::glr::ParsingAstBase)
				CLASS_MEMBER_FIELD(codeRange)
			END_CLASS_MEMBER(vl::glr::ParsingAstBase)

			class Parsing2TypeLoader : public vl::Object, public ITypeLoader
			{
			public:
				void Load(ITypeManager* manager)
				{
					ADD_TYPE_INFO(vl::glr::ParsingTextPos)
					ADD_TYPE_INFO(vl::glr::ParsingTextRange)
					ADD_TYPE_INFO(vl::glr::ParsingToken)
					ADD_TYPE_INFO(vl::glr::ParsingAstBase)
				}

				void Unload(ITypeManager* manager)
				{
				}
			};
#endif
#endif
			bool LoadParsing2Types()
			{
#ifdef VCZH_DESCRIPTABLEOBJECT_WITH_METADATA
				if (auto manager = GetGlobalTypeManager())
				{
					Ptr<ITypeLoader> loader = new Parsing2TypeLoader;
					return manager->AddTypeLoader(loader);
				}
#endif
				return false;
			}
		}
	}
}
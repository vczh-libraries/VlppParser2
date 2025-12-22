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

		void AstInsReceiverBase::SetField(ParsingAstBase* object, vint32_t field, const SlotValue& value, bool weakAssignment)
		{
			value.Apply(Overloading(
				[&](const TokenSlot& tokenSlot)
				{
					if (weakAssignment)
					{
						throw AstInsException(
							L"Weak assignment only available for field of enum type",
							AstInsErrorType::FieldWeakAssignmentOnNonEnum,
							field
						);
					}
					SetField(object, field, tokenSlot.token, tokenSlot.index);
				},
				[&](const EnumItemSlot& enumItemSlot)
				{
					SetField(object, field, enumItemSlot.value, weakAssignment);
				},
				[&](const Ptr< ParsingAstBase>& objectSlot)
				{
					if (weakAssignment)
					{
						throw AstInsException(
							L"Weak assignment only available for field of enum type",
							AstInsErrorType::FieldWeakAssignmentOnNonEnum,
							field
						);
					}
					SetField(object, field, objectSlot);
				}
			));
		}

		void AstInsReceiverBase::Execute(AstIns instruction, const regex::RegexToken& token, vint32_t tokenIndex)
		{
			EnsureContinuable();
			try
			{
				switch (instruction.type)
				{
				case AstInsType::Token:
				case AstInsType::EnumItem:
				case AstInsType::StackSlot:
					{
						if (stackFrames.Count() == 0)
						{
							throw AstInsException(
								L"There is no stack frame to store slot values.",
								AstInsErrorType::NoStackFrame
							);
						}
						auto&& frame = stackFrames[stackFrames.Count() - 1];

						SlotValue slotValue;
						switch (instruction.type)
						{
						case AstInsType::Token:
							slotValue = SlotValue(TokenSlot{ token,tokenIndex });
							break;
						case AstInsType::EnumItem:
							slotValue = SlotValue(EnumItemSlot{ instruction.param });
							break;
						case AstInsType::StackSlot:
							{
								if (creatingObjects.Count() == 0)
								{
									throw AstInsException(
										L"There is no creating object to store in a stack slot.",
										AstInsErrorType::NoCreatingObjectForStackSlot
									);
								}
								auto astNode = creatingObjects[creatingObjects.Count() - 1].object;
								creatingObjects.RemoveAt(creatingObjects.Count() - 1);
								slotValue = SlotValue(astNode);

								if (frame.codeRangeStart > astNode->codeRange.start)
								{
									frame.codeRangeStart = astNode->codeRange.start;
								}
							}
							break;
						default:;
						}

						auto keyIndex = frame.slots.Keys().IndexOf(instruction.count);
						if (keyIndex == -1)
						{
							SlotStorage storage;
							storage.value = slotValue;
							frame.slots.Add(instruction.count, storage);
						}
						else
						{
							auto&& storage = const_cast<SlotStorage&>(frame.slots.Values()[keyIndex]);
							if (!storage.additionalValues)
							{
								storage.additionalValues = Ptr(new List<SlotValue>);
							}
							storage.additionalValues->Add(slotValue);
						}
					}
					break;
				case AstInsType::StackBegin:
					{
						stackFrames.Add({ {},ParsingTextPos::Start(&token) });
					}
					break;
				case AstInsType::CreateObject:
					{
						if (stackFrames.Count() == 0)
						{
							throw AstInsException(
								L"There is no stack frame to store slot values.",
								AstInsErrorType::NoStackFrame
							);
						}
						auto&& frame = stackFrames[stackFrames.Count() - 1];

						auto astNode = CreateAstNode(instruction.param);
						astNode->codeRange = { &token,&token };
						if (astNode->codeRange.start > frame.codeRangeStart)
						{
							astNode->codeRange.start = frame.codeRangeStart;
						}

						CreatingObject info;
						info.object = astNode;
						info.type = instruction.param;
						creatingObjects.Add(info);
					}
					break;
				case AstInsType::Field:
				case AstInsType::FieldIfUnassigned:
					{
						if (creatingObjects.Count() == 0)
						{
							throw AstInsException(
								L"There is no creating object to assign fields.",
								AstInsErrorType::NoCreatingObjectForField,
								instruction.param
							);
						}

						if (stackFrames.Count() == 0)
						{
							throw AstInsException(
								L"There is no stack frame to provide values for field assignment.",
								AstInsErrorType::NoStackFrame
							);
						}
						auto&& frame = stackFrames[stackFrames.Count() - 1];

						auto slotKeyIndex = frame.slots.Keys().IndexOf(instruction.count);
						if (slotKeyIndex == -1)
						{
							break;
						}

						auto storage = frame.slots.Values()[slotKeyIndex];
						auto object = creatingObjects[creatingObjects.Count() - 1].object.Obj();

						const bool weakAssignment = instruction.type == AstInsType::FieldIfUnassigned;
						auto assignValue = [&](const SlotValue& slotValue)
						{
							SetField(object, instruction.param, slotValue, weakAssignment);
						};

						SetField(object, instruction.param, storage.value, weakAssignment);
						if (storage.additionalValues)
						{
							for (auto&& additionalValue : *storage.additionalValues.Obj())
							{
								SetField(object, instruction.param, additionalValue, weakAssignment);
							}
						}
					}
					break;
				case AstInsType::StackEnd:
					{
						if (stackFrames.Count() == 0)
						{
							throw AstInsException(
								L"There is no stack frame to end.",
								AstInsErrorType::NoStackFrameForStackEnd
							);
						}
						if (creatingObjects.Count() == 0)
						{
							throw AstInsException(
								L"There is no creating object when ending the current stack frame.",
								AstInsErrorType::NoCreatingObjectForStackEnd
							);
						}

						auto&& frame = stackFrames[stackFrames.Count() - 1];
						auto astNode = creatingObjects[creatingObjects.Count() - 1].object.Obj();

						if (astNode->codeRange.start > frame.codeRangeStart)
						{
							astNode->codeRange.start = frame.codeRangeStart;
						}

						auto codeRangeEnd = ParsingTextPos::End(&token);
						if (astNode->codeRange.end < codeRangeEnd)
						{
							astNode->codeRange.end = codeRangeEnd;
						}

						stackFrames.RemoveAt(stackFrames.Count() - 1);
					}
					break;
				case AstInsType::ResolveAmbiguity:
					{
						if (stackFrames.Count() == 0)
						{
							throw AstInsException(
								L"There is no stack frame to resolve ambiguity.",
								AstInsErrorType::NoStackFrame
							);
						}
						auto&& frame = stackFrames[stackFrames.Count() - 1];

						auto slotKeyIndex = frame.slots.Keys().IndexOf(ResolveAmbiguitySlotIndex);
						if (slotKeyIndex == -1)
						{
							throw AstInsException(
								L"There are not enough candidates to create an ambiguity node.",
								AstInsErrorType::MissingAmbiguityCandidate
							);
						}

						auto storage = frame.slots.Values()[slotKeyIndex];
						vint candidateCount = 1;
						if (storage.additionalValues)
						{
							candidateCount += storage.additionalValues->Count();
						}
						if (candidateCount < 2)
						{
							throw AstInsException(
								L"There are not enough candidates to create an ambiguity node.",
								AstInsErrorType::MissingAmbiguityCandidate
							);
						}

						Array<Ptr<ParsingAstBase>> candidates(candidateCount);
						auto readCandidate = [&](const SlotValue& slotValue, vint index)
						{
							slotValue.Apply(Overloading(
								[&](const TokenSlot&)
								{
									throw AstInsException(
										L"Tokens cannot be ambiguity candidates.",
										AstInsErrorType::AmbiguityCandidateIsNotObject
									);
								},
								[&](const EnumItemSlot&)
								{
									throw AstInsException(
										L"Enum items cannot be ambiguity candidates.",
										AstInsErrorType::AmbiguityCandidateIsNotObject
									);
								},
								[&](const Ptr<ParsingAstBase>& objectSlot)
								{
									candidates[index] = objectSlot;
								}
							));
						};

						readCandidate(storage.value, 0);
						if (storage.additionalValues)
						{
							for (vint i = 0; i < storage.additionalValues->Count(); i++)
							{
								readCandidate(storage.additionalValues->Get(i), i + 1);
							}
						}

						auto resolved = ResolveAmbiguity(instruction.param, candidates);
						CreatingObject info;
						info.object = resolved;
						info.type = instruction.param;
						creatingObjects.Add(info);
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
				if (stackFrames.Count() > 0 || creatingObjects.Count() != 1)
				{
					throw AstInsException(
						L"No more instruction but the root object has not been completed yet.",
						AstInsErrorType::InstructionNotComplete
						);
				}

				auto object = creatingObjects[0].object;
				creatingObjects.RemoveAt(0);
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
					WString::Unmanaged(L"\" cannot be assigned with an object."),
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
					WString::Unmanaged(L"\" cannot be assigned with a token."),
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
					WString::Unmanaged(L"\" cannot be assigned with an enum item."),
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
				STRUCT_MEMBER(index)
				STRUCT_MEMBER(token)
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
					auto loader = Ptr(new Parsing2TypeLoader);
					return manager->AddTypeLoader(loader);
				}
#endif
				return false;
			}
		}
	}
}

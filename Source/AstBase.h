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
ParsingTextPos
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

			static ParsingTextPos Start(const regex::RegexToken* token)
			{
				return { token->start,token->rowStart,token->columnStart };
			}

			static ParsingTextPos End(const regex::RegexToken* token)
			{
				return { token->start + token->length - 1,token->rowEnd,token->columnEnd };
			}

			friend std::strong_ordering operator<=>(const ParsingTextPos& a, const ParsingTextPos& b)
			{
				if (a.IsInvalid() && b.IsInvalid())
				{
					return std::strong_ordering::equal;
				}
				else if (a.IsInvalid())
				{
					return std::strong_ordering::less;
				}
				else if (b.IsInvalid())
				{
					return std::strong_ordering::greater;
				}
				else if (a.index >= 0 && b.index >= 0)
				{
					return a.index <=> b.index;
				}
				else if (a.row >= 0 && a.column >= 0 && b.row >= 0 && b.column >= 0)
				{
					if (a.row == b.row)
					{
						return a.column <=> b.column;
					}
					else
					{
						return a.row <=> b.row;
					}
				}
				else
				{
					return std::strong_ordering::equal;
				}
			}

			friend bool operator==(const ParsingTextPos& a, const ParsingTextPos& b)
			{
				return(a <=> b) == 0;
			}
		};

/***********************************************************************
ParsingTextRange
***********************************************************************/

		/// <summary>A type representing text range.</summary>
		struct ParsingTextRange
		{
			/// <summary>Text position for the first character.</summary>
			ParsingTextPos	start;
			/// <summary>Text position for the last character.</summary>
			ParsingTextPos	end;
			/// <summary>Code index, refer to [F:vl.regex.RegexToken.codeIndex]</summary>
			vint			codeIndex = -1;

			ParsingTextRange() = default;

			ParsingTextRange(const ParsingTextPos& _start, const ParsingTextPos& _end, vint _codeIndex = -1)
				: start(_start)
				, end(_end)
				, codeIndex(_codeIndex)
			{
			}

			ParsingTextRange(const regex::RegexToken* startToken, const regex::RegexToken* endToken)
				: start(ParsingTextPos::Start(startToken))
				, end(ParsingTextPos::End(endToken))
				, codeIndex(startToken->codeIndex)
			{
			}

			bool Contains(const ParsingTextPos& pos)const { return start <= pos && pos <= end; }
			bool Contains(const ParsingTextRange& range)const { return start <= range.start && range.end <= end; }

			friend std::strong_ordering operator<=>(const ParsingTextRange& a, const ParsingTextRange& b)
			{
				std::strong_ordering
					result = a.start <=> b.start; if (result != 0) return result;
				result = a.end <=> b.end; if (result != 0) return result;
				return std::strong_ordering::equal;
			}

			friend bool operator==(const ParsingTextRange& a, const ParsingTextRange& b)
			{
				return (a <=> b) == 0;
			}
		};

/***********************************************************************
ParsingError
***********************************************************************/

		class ParsingAstBase;

		struct ParsingError
		{
			ParsingAstBase*						node = nullptr;
			ParsingTextRange					codeRange;
			WString								message;
		};

/***********************************************************************
AST
***********************************************************************/

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
			/// <summary>Index of the token in the token list provided to the parser.</summary>
			vint								index = -1;
			/// <summary>Type of the token, representing the index of a regular expression that creates this token in the regular expression list in the grammar file.</summary>
			vint								token = -1;
			/// <summary>Content of the token.</summary>
			WString								value;

			operator bool() const { return value.Length() > 0; }
		};

/***********************************************************************
AST (Builder)
***********************************************************************/

		template<typename TAst>
		class ParsingAstBuilder
		{
		protected:
			Ptr<TAst> node{ new TAst };
			ParsingAstBuilder() {}
		public:

			template<typename TExpected>
			operator Ptr<TExpected>() const
			{
				return node;
			}
		};

/***********************************************************************
AST (Visitor)
***********************************************************************/

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
			bool								printTokenCodeRange = true;
			bool								printAstCodeRange = true;
			bool								printAstType = true;

			JsonVisitorBase(stream::StreamWriter& _writer);
		};

		extern void								JsonEscapeString(const WString& text, stream::TextWriter& writer);
		extern void								JsonUnescapeString(const WString& text, stream::TextWriter& writer);

/***********************************************************************
Instructions
***********************************************************************/

		enum class AstInsType
		{
			Token,										// Token(Count)						: Put the current token in the Count-th slot.
			EnumItem,									// EnumItem(Value, Count)			: Put an enum item in the Count-th slot.
			StackBegin,									// StackBegin()						: Begin a new stack frame.
			StackSlot,									// StackSlot(Count)					: Assign the just created object to the Count-th slot. Reset the creating object.
			CreateObject,								// CreateObject(Type)				: Create an AST node, it becomes the creating object. Error if the previous creating object has not been reset.
			Field,										// Field(Field, Count)				: Associate a field name of the creating object with the value in the Count-th slot. Ignored if the Count-th slot is empty.
			FieldIfUnassigned,							// FieldIfUnassigned(Field, Count)	: Like Field(Field) but only take effect if such field has never been assigned.
			StackEnd,									// StackEnd()						: End the current stack frame. Leave the creating object as is.
			ResolveAmbiguity,							// ResolveAmbiguity(Type)			: Combine several values in the 0-th slot to one using an ambiguity node. Type is the type of each value.
		};

		struct AstIns
		{
			AstInsType									type = AstInsType::Token;
			vint32_t									param = -1;
			vint										count = -1;

			std::strong_ordering operator<=>(const AstIns& ins) const
			{
				std::strong_ordering
				result = type <=> ins.type; if (result != 0) return result;
				result = param <=> ins.param; if (result != 0) return result;
				result = count <=> ins.count; if (result != 0) return result;
				return result;
			}

			bool operator==(const AstIns& ins) const
			{
				return (*this <=> ins) == 0;
			}
		};

		enum class AstInsErrorType
		{
			UnknownType,								// UnknownType(Type)					: The type id does not exist.
			UnknownField,								// UnknownField(Field)					: The field id does not exist.
			UnsupportedAbstractType,					// UnsupportedAbstractType(Type)		: Unable to create abstract class.
			FieldNotExistsInType,						// FieldNotExistsInType(Field)			: The type doesn't have such field.
			FieldReassigned,							// FieldReassigned(Field)				: An object is assigned to a field but this field has already been assigned.
			FieldWeakAssignmentOnNonEnum,				// FieldWeakAssignmentOnNonEnum(Field)	: Weak assignment only available for field of enum type.
			ObjectTypeMismatchedToField,				// ObjectTypeMismatchedToField(Field)	: Unable to assign an object to a field because the type does not match.

			UnsupportedAmbiguityType,					// UnsupportedAmbiguityType(Type)		: The type is not configured to allow ambiguity.
			UnexpectedAmbiguousCandidate,				// UnexpectedAmbiguousCandidate(Type)	: The type of the ambiguous candidate is not compatible to the required type.
			MissingAmbiguityCandidate,					// MissingAmbiguityCandidate()			: There are less than two candidates to create an ambiguity node.
			AmbiguityCandidateIsNotObject,				// AmbiguityCandidateIsNotObject()		: Tokens or enum items cannot be ambiguity candidates.

			NoCreatingObjectForField,					// NoCreatingObjectForField()			: Field when no creating object.
			NoCreatingObjectForStackSlot,				// NoCreatingObjectForStackSlot()		: StackSlot when no creating object.
			NoCreatingObjectForStackEnd,				// NoCreatingObjectForStackEnd()		: StackEnd when no creating object.
			CreatingObjectNotReset,						// CreatingObjectNotReset()				: The previous creating object has not been reset.

			InstructionNotComplete,						// InstructionNotComplete()				: No more instruction but the root object has not been completed yet.
			Corrupted,									// Corrupted()							: An exception has been thrown therefore this receiver cannot be used anymore.
			Finished,									// Finished()							: The finished instruction has been executed therefore this receiver cannot be used anymore.
		};

		class AstInsException : public Exception
		{
		public:
			AstInsErrorType								error = AstInsErrorType::Corrupted;
			vint32_t									paramTypeOrField = -1;

			AstInsException(const WString& message, AstInsErrorType _error, vint32_t _typeOrField = -1)
				: Exception(message)
				, error(_error)
				, paramTypeOrField(_typeOrField)
			{
			}
		};

/***********************************************************************
IAstInsReceiver
***********************************************************************/

		class IAstInsReceiver : public virtual Interface
		{
		public:
			virtual void								Execute(AstIns instruction, const regex::RegexToken& token, vint32_t tokenIndex) = 0;
			virtual Ptr<ParsingAstBase>					Finished() = 0;
		};

		class AstInsReceiverBase : public Object, public virtual IAstInsReceiver
		{
		private:
			struct TokenSlot
			{
				regex::RegexToken						token;
				vint32_t								index = -1;

				auto operator<=>(const TokenSlot&) const = default;
			};

			struct EnumItemSlot
			{
				vint32_t								value = -1;

				auto operator<=>(const EnumItemSlot&) const = default;
			};

			using SlotValue = Variant<TokenSlot, EnumItemSlot, Ptr<ParsingAstBase>>;

			struct SlotStorage
			{
				SlotValue								value;
				Ptr<collections::List<SlotValue>>		additionalValues;

				auto operator<=>(const SlotStorage&) const = default;
			};
			using SlotMap = collections::Dictionary<vint32_t, SlotStorage>;

			struct StackFrame
			{
				SlotMap									slots;
			};
			using StackFrameList = collections::List<Ptr<StackFrame>>;

			struct CreatingObject
			{
				Ptr<ParsingAstBase>						object;
				vint32_t								type = -1;
			};

			Nullable<CreatingObject>					creatingObject;
			StackFrameList								stackFrames;
			bool										finished = false;
			bool										corrupted = false;

			void										EnsureContinuable();
			void										SetField(ParsingAstBase* object, vint32_t field, const SlotValue& value, bool weakAssignment);

		protected:
			virtual Ptr<ParsingAstBase>					CreateAstNode(vint32_t type) = 0;
			virtual void								SetField(ParsingAstBase* object, vint32_t field, Ptr<ParsingAstBase> value) = 0;
			virtual void								SetField(ParsingAstBase* object, vint32_t field, const regex::RegexToken& token, vint32_t tokenIndex) = 0;
			virtual void								SetField(ParsingAstBase* object, vint32_t field, vint32_t enumValue, bool weakAssignment) = 0;
			virtual Ptr<ParsingAstBase>					ResolveAmbiguity(vint32_t type, collections::Array<Ptr<ParsingAstBase>>& candidates) = 0;

		public:
			AstInsReceiverBase() = default;
			~AstInsReceiverBase() = default;

			void										Execute(AstIns instruction, const regex::RegexToken& token, vint32_t tokenIndex) override;
			Ptr<ParsingAstBase>							Finished() override;
		};

/***********************************************************************
IAstInsReceiver (Code Generation Templates)
***********************************************************************/

		template<typename TClass, typename TField>
		void AssemblerSetObjectField(Ptr<TField>(TClass::* member), ParsingAstBase* object, vint32_t field, Ptr<ParsingAstBase> value, const wchar_t* cppFieldName)
		{
			auto typedObject = dynamic_cast<TClass*>(object);
			if (!typedObject)
			{
				throw AstInsException(
					WString::Unmanaged(L"Field \"") +
					WString::Unmanaged(cppFieldName) +
					WString::Unmanaged(L"\" does not exist in the current object."),
					AstInsErrorType::FieldNotExistsInType, field);
			}
			if ((typedObject->*member))
			{
				throw AstInsException(
					WString::Unmanaged(L"Field \"") +
					WString::Unmanaged(cppFieldName) +
					WString::Unmanaged(L"\" has already been assigned."),
					AstInsErrorType::FieldReassigned, field);
			}

			auto typedValue = value.Cast<TField>();
			if (!typedValue)
			{
				throw AstInsException(
					WString::Unmanaged(L"Field \"") +
					WString::Unmanaged(cppFieldName) +
					WString::Unmanaged(L"\" cannot be assigned with an uncompatible value."),
					AstInsErrorType::ObjectTypeMismatchedToField, field);
			}
			(typedObject->*member) = typedValue;
		}

		template<typename TClass, typename TField>
		void AssemblerSetObjectField(collections::List<Ptr<TField>>(TClass::* member), ParsingAstBase* object, vint32_t field, Ptr<ParsingAstBase> value, const wchar_t* cppFieldName)
		{
			auto typedObject = dynamic_cast<TClass*>(object);
			if (!typedObject)
			{
				throw AstInsException(
					WString::Unmanaged(L"Field \"") +
					WString::Unmanaged(cppFieldName) +
					WString::Unmanaged(L"\" does not exist in the current object."),
					AstInsErrorType::FieldNotExistsInType, field);
			}

			auto typedValue = value.Cast<TField>();
			if (!typedValue)
			{
				throw AstInsException(
					WString::Unmanaged(L"Field \"") +
					WString::Unmanaged(cppFieldName) +
					WString::Unmanaged(L"\" cannot be assigned with an uncompatible value."),
					AstInsErrorType::ObjectTypeMismatchedToField, field);
			}
			(typedObject->*member).Add(typedValue);
		}

		template<typename TClass>
		void AssemblerSetTokenField(ParsingToken(TClass::* member), ParsingAstBase* object, vint32_t field, const regex::RegexToken& token, vint32_t tokenIndex, const wchar_t* cppFieldName)
		{
			auto typedObject = dynamic_cast<TClass*>(object);
			if (!typedObject)
			{
				throw AstInsException(
					WString::Unmanaged(L"Field \"") +
					WString::Unmanaged(cppFieldName) +
					WString::Unmanaged(L"\" does not exist in the current object."),
					AstInsErrorType::FieldNotExistsInType, field);
			}
			if ((typedObject->*member).value.Length() != 0)
			{
				throw AstInsException(
					WString::Unmanaged(L"Field \"") +
					WString::Unmanaged(cppFieldName) +
					WString::Unmanaged(L"\" has already been assigned."),
					AstInsErrorType::FieldReassigned, field);
			}

			ParsingToken& tokenField = typedObject->*member;
			tokenField.codeRange = { &token,&token };
			tokenField.index = tokenIndex;
			tokenField.token = token.token;
			tokenField.value = WString::CopyFrom(token.reading, token.length);
		}

		template<typename TClass, typename TField>
		void AssemblerSetEnumField(TField(TClass::* member), ParsingAstBase* object, vint32_t field, vint32_t enumItem, bool weakAssignment, const wchar_t* cppFieldName)
		{
			auto typedObject = dynamic_cast<TClass*>(object);
			if (!typedObject)
			{
				throw AstInsException(
					WString::Unmanaged(L"Field \"") +
					WString::Unmanaged(cppFieldName) +
					WString::Unmanaged(L"\" does not exist in the current object."),
					AstInsErrorType::FieldNotExistsInType, field);
			}
			if ((typedObject->*member) != TField::UNDEFINED_ENUM_ITEM_VALUE)
			{
				if (weakAssignment) return;
				throw AstInsException(
					WString::Unmanaged(L"Field \"") +
					WString::Unmanaged(cppFieldName) +
					WString::Unmanaged(L"\" has already been assigned."),
					AstInsErrorType::FieldReassigned, field);
			}
			(typedObject->*member) = (TField)enumItem;
		}

		template<typename TElement, typename TAmbiguity>
		Ptr<ParsingAstBase> AssemblerResolveAmbiguity(vint32_t type, collections::Array<Ptr<ParsingAstBase>>& candidates, const wchar_t* cppTypeName)
		{
			auto ast = Ptr(new TAmbiguity());
			if (candidates.Count() > 0)
			{
				ast->codeRange = candidates[0]->codeRange;
			}
			for (auto candidate : candidates)
			{
				if (auto typedAst = candidate.Cast<TElement>())
				{
					ast->candidates.Add(typedAst);
				}
				else if (auto ambiguityAst = candidate.Cast<TAmbiguity>())
				{
					CopyFrom(ast->candidates, ambiguityAst->candidates, true);
				}
				else
				{
					throw AstInsException(
						WString::Unmanaged(L"The type of the ambiguous candidate is not compatible to the required type \"") +
						WString::Unmanaged(cppTypeName) +
						WString::Unmanaged(L"\"."),
						AstInsErrorType::UnexpectedAmbiguousCandidate, type);
				}
			}
			return ast;
		}

		template<vint32_t Size>
		vint32_t AssemblerFindCommonBaseClass(vint32_t class1, vint32_t class2, vint32_t(&matrix)[Size][Size])
		{
			if (class1 < 0 || class1 >= Size) throw glr::AstInsException(L"The type id does not exist.", glr::AstInsErrorType::UnknownType, class1);
			if (class2 < 0 || class2 >= Size) throw glr::AstInsException(L"The type id does not exist.", glr::AstInsErrorType::UnknownType, class2);
			return matrix[class1][class2];
		}

/***********************************************************************
IAstInsReceiver (Code Generation Error Templates)
***********************************************************************/

		extern Ptr<ParsingAstBase>	AssemblyThrowCannotCreateAbstractType(vint32_t type, const wchar_t* cppTypeName);
		extern void					AssemblyThrowFieldNotObject(vint32_t field, const wchar_t* cppFieldName);
		extern void					AssemblyThrowFieldNotToken(vint32_t field, const wchar_t* cppFieldName);
		extern void					AssemblyThrowFieldNotEnum(vint32_t field, const wchar_t* cppFieldName);
		extern Ptr<ParsingAstBase>	AssemblyThrowTypeNotAllowAmbiguity(vint32_t type, const wchar_t* cppTypeName);

/***********************************************************************
Compression
***********************************************************************/

		extern void			DecompressSerializedData(const char** buffer, bool decompress, vint solidRows, vint rows, vint block, vint remain, stream::IStream& outputStream);
	}

/***********************************************************************
Reflection
***********************************************************************/

	namespace reflection
	{
		namespace description
		{
#ifndef VCZH_DEBUG_NO_REFLECTION
			DECL_TYPE_INFO(vl::glr::ParsingTextPos)
			DECL_TYPE_INFO(vl::glr::ParsingTextRange)
			DECL_TYPE_INFO(vl::glr::ParsingToken)
			DECL_TYPE_INFO(vl::glr::ParsingAstBase)
#endif
			extern bool		LoadParsing2Types();
		}
	}
}

#endif
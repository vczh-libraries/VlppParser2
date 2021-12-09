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
			/// <summary>Type of the token, representing the index of a regular expression that creates this token in the regular expression list in the grammar file.</summary>
			vint								tokenIndex = -1;
			/// <summary>Content of the token.</summary>
			WString								value; 

			operator bool() const { return value.Length() > 0; }
		};

/***********************************************************************
AST (Builder)
***********************************************************************/

		template<typename TAst, typename ...TBase>
		class ParsingAstBuilder : public Object, public TBase...
		{
		protected:
			Ptr<TAst>				node = MakePtr<TAst>();
		public:
			ParsingAstBuilder() : TBase(node.Obj())... {}

			template<typename TExpected>
			auto operator()() const ->
				std::enable_if_t<
					std::is_convertible_v<TAst*, TExpected*>,
					Ptr<TExpected>
					>
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
			Token,										// Token()							: Push the current token as a value.
			EnumItem,									// EnumItem(Value)					: Push an enum item.
			BeginObject,								// BeginObject(Type)				: Begin creating an AST node.
			BeginObjectLeftRecursive,					// BeginObjectLeftRecursive(Type)	: Begin creating an AST node, taking the ownership of the last pushed object.
			ReopenObject,								// ReopenObject()					: Move the last pushed object back to creating status.
			EndObject,									// EndObject()						: Finish creating an AST node, all objects pushed after BeginObject are supposed to be its fields.
			DiscardValue,								// DiscardValue()					: Remove a pushed value.
			Field,										// Field(Field)						: Associate a field name with the top object.
			ResolveAmbiguity,							// ResolveAmbiguity(Type, Count)	: Combine several top objects to one using an ambiguity node. Type is the type of each top object.
		};

		struct AstIns
		{
			AstInsType									type = AstInsType::Token;
			vint32_t									param = -1;
			vint										count = -1;

			vint Compare(const AstIns& ins) const
			{
				auto result = (vint)type - (vint)ins.type;
				if (result != 0) return result;
				result = (vint)param - (vint)ins.param;
				if (result != 0) return result;
				return count - ins.count;
			}

			bool operator==(const AstIns& ins) const { return Compare(ins) == 0; }
			bool operator!=(const AstIns& ins) const { return Compare(ins) != 0; }
			bool operator< (const AstIns& ins) const { return Compare(ins) < 0; }
			bool operator<=(const AstIns& ins) const { return Compare(ins) <= 0; }
			bool operator> (const AstIns& ins) const { return Compare(ins) > 0; }
			bool operator>=(const AstIns& ins) const { return Compare(ins) >= 0; }
		};

		enum class AstInsErrorType
		{
			UnknownType,								// UnknownType(Type)					: The type id does not exist.
			UnknownField,								// UnknownField(Field)					: The field id does not exist.
			UnsupportedAmbiguityType,					// UnsupportedAmbiguityType(Type)		: The type is not configured to allow ambiguity.
			UnexpectedAmbiguousCandidate,				// UnexpectedAmbiguousCandidate(Type)	: The type of the ambiguous candidate is not compatible to the required type.
			FieldNotExistsInType,						// FieldNotExistsInType(Field)			: The type doesn't have such field.
			FieldReassigned,							// FieldReassigned(Field)				: An object is assigned to a field but this field has already been assigned.
			ObjectTypeMismatchedToField,				// ObjectTypeMismatchedToField(Field)	: Unable to assign an object to a field because the type does not match.

			NoRootObject,								// NoRootObject()						: There is no created objects.
			MissingLeftRecursiveValue,					// MissingLeftRecursiveValue()			: There is no pushed value to create left recursive object.
			LeftRecursiveValueIsNotObject,				// LeftRecursiveValueIsNotObject()		: The pushed value to create left recursive object is not an object.
			TooManyUnassignedValues,					// LeavingUnassignedValues()			: The value to reopen is not the only unassigned value.
			MissingValueToReopen,						// MissingValueToReopen()				: There is no pushed value to reopen.
			ReopenedValueIsNotObject,					// ReopenedValueIsNotObject()			: The pushed value to reopen is not an object.
			MissingValueToDiscard,						// MissingValueToDiscard()				: There is no pushed value to discard.
			LeavingUnassignedValues,					// LeavingUnassignedValues()			: There are still values to assign to fields before finishing an object.
			MissingFieldValue,							// MissingFieldValue()					: There is no pushed value to be assigned to a field.
			MissingAmbiguityCandidate,					// MissingAmbiguityCandidate()			: There are not enough candidates to create an ambiguity node.
			AmbiguityCandidateIsNotObject,				// AmbiguityCandidateIsNotObject()		: Tokens or enum items cannot be ambiguity candidates.
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
			virtual void								Execute(AstIns instruction, const regex::RegexToken& token) = 0;
			virtual Ptr<ParsingAstBase>					Finished() = 0;
		};

		class AstInsReceiverBase : public Object, public virtual IAstInsReceiver
		{
		private:
			struct CreatedObject
			{
				Ptr<ParsingAstBase>						object;
				vint									pushedCount;

				explicit CreatedObject(Ptr<ParsingAstBase> _object, vint _pushedCount) : object(_object), pushedCount(_pushedCount) {}
			};

			struct ObjectOrToken
			{
				Ptr<ParsingAstBase>						object;
				vint32_t								enumItem = -1;
				regex::RegexToken						token = {};

				explicit ObjectOrToken(Ptr<ParsingAstBase> _object) : object(_object) {}
				explicit ObjectOrToken(vint32_t _enumItem) : enumItem(_enumItem) {}
				explicit ObjectOrToken(const regex::RegexToken& _token) : token(_token) {}
			};

			collections::List<CreatedObject>			created;
			collections::List<ObjectOrToken>			pushed;
			bool										finished = false;
			bool										corrupted = false;

			void										EnsureContinuable();
		protected:
			virtual Ptr<ParsingAstBase>					CreateAstNode(vint32_t type) = 0;
			virtual void								SetField(ParsingAstBase* object, vint32_t field, Ptr<ParsingAstBase> value) = 0;
			virtual void								SetField(ParsingAstBase* object, vint32_t field, const regex::RegexToken& token) = 0;
			virtual void								SetField(ParsingAstBase* object, vint32_t field, vint32_t enumValue) = 0;
			virtual Ptr<ParsingAstBase>					ResolveAmbiguity(vint32_t type, collections::Array<Ptr<ParsingAstBase>>& candidates) = 0;

		public:
			AstInsReceiverBase() = default;
			~AstInsReceiverBase() = default;

			void										Execute(AstIns instruction, const regex::RegexToken& token) override;
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
				throw vl::glr::AstInsException(
					WString::Unmanaged(L"Field \"") +
					WString::Unmanaged(cppFieldName) +
					WString::Unmanaged(L"\" does not exist in the current object."),
					vl::glr::AstInsErrorType::FieldNotExistsInType, field);
			}
			if ((typedObject->*member))
			{
				throw vl::glr::AstInsException(
					WString::Unmanaged(L"Field \"") +
					WString::Unmanaged(cppFieldName) +
					WString::Unmanaged(L"\" has already been assigned."),
					vl::glr::AstInsErrorType::FieldReassigned, field);
			}

			auto typedValue = value.Cast<TField>();
			if (!typedValue)
			{
				throw vl::glr::AstInsException(
					WString::Unmanaged(L"Field \"") +
					WString::Unmanaged(cppFieldName) +
					WString::Unmanaged(L"\" cannot be assigned with an uncompatible value."),
					vl::glr::AstInsErrorType::ObjectTypeMismatchedToField, field);
			}
			(typedObject->*member) = typedValue;
		}

		template<typename TClass, typename TField>
		void AssemblerSetObjectField(collections::List<Ptr<TField>>(TClass::* member), ParsingAstBase* object, vint32_t field, Ptr<ParsingAstBase> value, const wchar_t* cppFieldName)
		{
			auto typedObject = dynamic_cast<TClass*>(object);
			if (!typedObject)
			{
				throw vl::glr::AstInsException(
					WString::Unmanaged(L"Field \"") +
					WString::Unmanaged(cppFieldName) +
					WString::Unmanaged(L"\" does not exist in the current object."),
					vl::glr::AstInsErrorType::FieldNotExistsInType, field);
			}

			auto typedValue = value.Cast<TField>();
			if (!typedValue)
			{
				throw vl::glr::AstInsException(
					WString::Unmanaged(L"Field \"") +
					WString::Unmanaged(cppFieldName) +
					WString::Unmanaged(L"\" cannot be assigned with an uncompatible value."),
					vl::glr::AstInsErrorType::ObjectTypeMismatchedToField, field);
			}
			(typedObject->*member).Add(typedValue);
		}

		template<typename TClass>
		void AssemblerSetTokenField(ParsingToken(TClass::* member), ParsingAstBase* object, vint32_t field, const regex::RegexToken& token, const wchar_t* cppFieldName)
		{
			auto typedObject = dynamic_cast<TClass*>(object);
			if (!typedObject)
			{
				throw vl::glr::AstInsException(
					WString::Unmanaged(L"Field \"") +
					WString::Unmanaged(cppFieldName) +
					WString::Unmanaged(L"\" does not exist in the current object."),
					vl::glr::AstInsErrorType::FieldNotExistsInType, field);
			}
			if ((typedObject->*member).value.Length() != 0)
			{
				throw vl::glr::AstInsException(
					WString::Unmanaged(L"Field \"") +
					WString::Unmanaged(cppFieldName) +
					WString::Unmanaged(L"\" has already been assigned."),
					vl::glr::AstInsErrorType::FieldReassigned, field);
			}

			ParsingToken& tokenField = typedObject->*member;
			tokenField.codeRange.start.row = token.rowStart;
			tokenField.codeRange.start.column = token.columnStart;
			tokenField.codeRange.end.row = token.rowEnd;
			tokenField.codeRange.end.column = token.columnEnd;
			tokenField.codeRange.codeIndex = token.codeIndex;
			tokenField.tokenIndex = token.token;
			tokenField.value = WString::CopyFrom(token.reading, token.length);
		}

		template<typename TClass, typename TField>
		void AssemblerSetEnumField(TField(TClass::* member), ParsingAstBase* object, vint32_t field, vint32_t enumItem, const wchar_t* cppFieldName)
		{
			auto typedObject = dynamic_cast<TClass*>(object);
			if (!typedObject)
			{
				throw vl::glr::AstInsException(
					WString::Unmanaged(L"Field \"") +
					WString::Unmanaged(cppFieldName) +
					WString::Unmanaged(L"\" does not exist in the current object."),
					vl::glr::AstInsErrorType::FieldNotExistsInType, field);
			}
			if ((typedObject->*member) != TField::UNDEFINED_ENUM_ITEM_VALUE)
			{
				throw vl::glr::AstInsException(
					WString::Unmanaged(L"Field \"") +
					WString::Unmanaged(cppFieldName) +
					WString::Unmanaged(L"\" has already been assigned."),
					vl::glr::AstInsErrorType::FieldReassigned, field);
			}
			(typedObject->*member) = (TField)enumItem;
		}

		template<typename TElement, typename TAmbiguity>
		Ptr<ParsingAstBase> AssemblerResolveAmbiguity(vint32_t type, collections::Array<Ptr<ParsingAstBase>>& candidates, const wchar_t* cppTypeName)
		{
			vl::Ptr<TAmbiguity> ast = new TAmbiguity();
			for (auto candidate : candidates)
			{
				auto typedAst = candidate.Cast<TElement>();
				if (!typedAst)
				{
					throw vl::glr::AstInsException(
						WString::Unmanaged(L"The type of the ambiguous candidate is not compatible to the required type \"") +
						WString::Unmanaged(cppTypeName) +
						WString::Unmanaged(L"\"."),
						vl::glr::AstInsErrorType::UnexpectedAmbiguousCandidate, type);
				}
				ast->candidates.Add(typedAst);
			}
			return ast;
		}

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
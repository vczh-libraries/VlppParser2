/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_PARSERGEN_PARSERSYMBOl
#define VCZH_PARSER2_PARSERGEN_PARSERSYMBOl

#include "../AstBase.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			template<typename T>
			struct MappedOwning
			{
				collections::List<Ptr<T>>				items;
				collections::List<WString>				order;
				collections::Dictionary<WString, T*>	map;

				bool Add(const WString& name, T* item)
				{
					items.Add(item);
					if (map.Keys().Contains(name)) return false;
					order.Add(name);
					map.Add(name, item);
					return true;
				}
			};

			template<typename ...TArgs>
			void Fill(collections::List<WString>& ss, TArgs&& ...args)
			{
				WString items[] = { args... };
				for (auto& item : items)
				{
					ss.Add(item);
				}
			}

/***********************************************************************
ParserSymbolManager
***********************************************************************/

			enum class ParserErrorType
			{
				// AstSymbolManager -------------------------------------------------------------------
				DuplicatedFile,								// (fileName)
				FileDependencyNotExists,					// (fileName, dependency)
				FileCyclicDependency,						// (fileName, dependency)
				DuplicatedSymbol,							// (fileName, symbolName)
				DuplicatedSymbolGlobally,					// (fileName, symbolName, anotherFileName)
				DuplicatedClassProp,						// (fileName, className, propName)
				DuplicatedEnumItem,							// (fileName, enumName, propName)
				BaseClassNotExists,							// (fileName, className, typeName)
				BaseClassNotClass,							// (fileName, className, typeName)
				BaseClassCyclicDependency,					// (fileName, className)
				FieldTypeNotExists,							// (fileName, className, propName)
				FieldTypeNotClass,							// (fileName, className, propName)

				// LexerSymbolManager -----------------------------------------------------------------
				InvalidTokenDefinition,						// (code)
				DuplicatedToken,							// (tokenName)
				DuplicatedTokenByDisplayText,				// (tokenName)
				InvalidTokenRegex,							// (tokenName, errorMessage)
				TokenRegexNotPure,							// (tokenName)
				DuplicatedTokenFragment,					// (fragmentName)
				TokenFragmentNotExists,						// (fragmentName)

				// SyntaxSymbolManager ----------------------------------------------------------------
				DuplicatedRule,								// (ruleName)
				RuleIsIndirectlyLeftRecursive,				// (ruleName)													: Indirect left recursion must be resolved before.
				LeftRecursiveClauseInsidePushCondition,		// (ruleName)													: The left recursive clause is not allowed to begin with a push condition syntax.
				LeftRecursiveClauseInsideTestCondition,		// (ruleName)													: The left recursive clause is not allowed to begin with a test condition syntax.

				// SyntaxAst (ResolveName) ------------------------------------------------------------
				RuleNameConflictedWithToken,				// (ruleName)
				TypeNotExistsInRule,						// (ruleName, name)
				TypeNotClassInRule,							// (ruleName, name)
				TokenOrRuleNotExistsInRule,					// (ruleName, name)
				LiteralNotValidToken,						// (ruleName, name)
				LiteralIsDiscardedToken,					// (ruleName, name)
				ConditionalLiteralNotValidToken,			// (ruleName, name)
				ConditionalLiteralIsDiscardedToken,			// (ruleName, name)
				ConditionalLiteralIsDisplayText,			// (ruleName, name)
				DuplicatedSwitch,							// (switchName)
				UnusedSwitch,								// (switchName)
				SwitchNotExists,							// (ruleName, switchName)

				// SyntaxAst (CalculateTypes) ---------------------------------------------------------
				RuleMixedPartialClauseWithOtherClause,		// (ruleName)
				RuleWithDifferentPartialTypes,				// (ruleName)
				RuleCannotResolveToDeterministicType,		// (ruleName)													: Unable to resolve to one type from clauses (token, type) or (create, partial).
				CyclicDependedRuleTypeIncompatible,			// (ruleName)													: Types of rules are not compatible to each other when they build cyclic dependency by reuse clauses.
				ReuseClauseCannotResolveToDeterministicType,// (ruleName)													: A reuse clause contains multiple use rule but their types are not compatible to each other.
				ReuseClauseContainsNoUseRule,				// (ruleName)													: A reuse clause contains no use rule therefore the type cannot be determined.

				// SyntaxAst (ValidateTypes) ----------------------------------------------------------
				FieldNotExistsInClause,						// (ruleName, clauseType, fieldName)							: The field does not exist in the type of the clause.
				RuleTypeMismatchedToField,					// (ruleName, clauseType, fieldName, fieldRuleType)				: The rule type is not compatible to the assigning field.
				AssignmentToNonEnumField,					// (ruleName, clauseType, fieldName)							: Assignment can only assign fields in enum types.
				EnumItemMismatchedToField,					// (ruleName, clauseType, fieldName, enumItem)					: Try to assign an unexisting or mismatched enum item to a field in an enum type.
				UseRuleWithPartialRule,						// (ruleName, useRuleName)										: A use rule should not be used with a partial rule.
				UseRuleInNonReuseClause,					// (ruleName, useRuleName)										: A use rule should only appear in reuse clause.
				PartialRuleUsedOnField,						// (ruleName, clauseType, partialRuleName, fieldName)			: A partial rule does not create object, it cannot be assigned to a field.
				ClauseTypeMismatchedToPartialRule,			// (ruleName, clauseType, partialRuleName, partialRuleType)		: A clause uses a partial rule of an incompatible type.

				// SyntaxAst (ValidateStructure, counting) --------------------------------------------
				ClauseNotCreateObject,						// (ruleName)													: A reuse clause does not contain use rule in some potential sequences.
				UseRuleUsedInOptionalBody,					// (ruleName, useRuleName)
				UseRuleUsedInLoopBody,						// (ruleName, useRuleName)
				ClauseTooManyUseRule,						// (ruleName)													: Multiple use rules in a potential sequence in a clause.
				NonArrayFieldAssignedInLoop,				// (ruleName, clauseType, fieldName)
				NonLoopablePartialRuleUsedInLoop,			// (ruleName, clauseType, partialRuleName)
				ClauseCouldExpandToEmptySequence,			// (ruleName)
				LoopBodyCouldExpandToEmptySequence,			// (ruleName)
				OptionalBodyCouldExpandToEmptySequence,		// (ruleName)
				NegativeOptionalEndsAClause,				// (ruleName)													: Negative optional syntax cannot ends a clause.
				MultiplePrioritySyntaxInAClause,			// (ruleName)													: Too many syntax with priority in the a clause.
				PushConditionBodyCouldExpandToEmptySequence,// (ruleName)
				TestConditionBodyCouldExpandToEmptySequence,// (ruleName)
				MultipleEmptySyntaxInTestCondition,			// (ruleName)
				TooManyLeftRecursionPlaceholderClauses,		// (ruleName)

				// SyntaxAst (ValidateStructure, relationship) ----------------------------------------
				FieldAssignedMoreThanOnce,					// (ruleName, clauseType, fieldName)
			};

			enum class ParserDefFileType
			{
				Ast,
				Lexer,
				Syntax,
			};

			struct ParserErrorLocation
			{
				ParserDefFileType			type;
				WString						name;
				ParsingTextRange			codeRange;
			};

			struct ParserError
			{
				ParserErrorType				type;
				ParserErrorLocation			location;
				WString						arg1;
				WString						arg2;
				WString						arg3;
				WString						arg4;
			};

			class ParserSymbolManager : public Object
			{
				using ErrorList = collections::List<ParserError>;
				using StringItems = collections::List<WString>;
			protected:

				ErrorList					errors;
			public:
				WString						name;
				StringItems					includes;
				StringItems					cppNss;
				WString						headerGuard;

				const auto&					Errors() { return errors; }

				template<typename ...TArgs>
				void AddError(ParserErrorType type, ParserErrorLocation location, TArgs&& ...args)
				{
					ParserError error;
					error.type = type;
					error.location = location;

					WString sargs[] = { WString(args)... };
					WString* dargs[] = { &error.arg1,&error.arg2,&error.arg3,&error.arg4 };
					constexpr vint sl = sizeof(sargs) / sizeof(*sargs);
					constexpr vint dl = sizeof(dargs) / sizeof(*dargs);
					constexpr vint ml = sl < dl ? sl : dl;
					for (vint i = 0; i < ml; i++)
					{
						*dargs[i] = sargs[i];
					}

					errors.Add(std::move(error));
				}
			};

/***********************************************************************
Utility
***********************************************************************/

			extern void						InitializeParserSymbolManager(ParserSymbolManager& manager);
		}
	}
}

#endif
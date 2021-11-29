/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_PARSERGEN_PARSERSYMBOl
#define VCZH_PARSER2_PARSERGEN_PARSERSYMBOl

#include <Vlpp.h>

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
				DuplicatedEnumItem,							// (fileName, enumName, propName
				BaseClassNotExists,							// (fileName, className)
				BaseClassNotClass,							// (fileName, className)
				BaseClassCyclicDependency,					// (fileName, className)
				FieldTypeNotExists,							// (fileName, className, propName)
				FieldTypeNotClass,							// (fileName, className, propName)

				// LexerSymbolManager -----------------------------------------------------------------
				InvalidTokenDefinition,						// (code)
				DuplicatedToken,							// (tokenName)
				DuplicatedTokenByDisplayText,				// (tokenName)
				InvalidTokenRegex,							// (tokenName, errorMessage)
				TokenRegexNotPure,							// (tokenName)

				// SyntaxSymbolManager ----------------------------------------------------------------
				DuplicatedRule,								// (ruleName)
				RuleIsIndirectlyLeftRecursive,				// (ruleName)								: Indirect left recursion must be resolved before.

				// SyntaxAst to SyntaxSymbolManager ---------------------------------------------------
				// hint is a position in clause like: EXP0 ::= '(' @ !EXP ')'
				RuleNameConflictedWithToken,				// (ruleName)
				TokenOrRuleNotExistsInRule,					// (ruleName, name)
				TypeNotExistsInRule,						// (ruleName, name)
				TypeNotClassInRule,							// (ruleName, name)
				FieldNotExistsInRule,						// (ruleName, typeName, fieldName)
				RuleCannotResolveToDeterministicType,		// (ruleName)								: Unable to resolve to one type from clauses (token, type) or (create, partial).
				RuleTypeMismatchedToField,					// (ruleName, fieldRuleName, fieldName)		: The rule type is not compatible to the assigning field.
				PartialRuleUsedOnField,						// (ruleName, partialRuleName, fieldName)	: A partial rule does not create object, it cannot be assigned to a field.
				ClauseNotCreateObject,						// (ruleName)								: A clause is not a create rule, not a partial rule, and not containing use rule.
				ClauseCouldExpandToEmptySequence,			// (ruleName)
				UseRuleUsedInOptionalBody,					// (ruleName)
				UseRuleUsedInLoopBody,						// (ruleName)
				UseRuleAppearAfterField,					// (ruleName, useRuleName, fieldName)
				UseRuleAppearAfterPartialRule,				// (ruleName, useRuleName, partialRuleName)
				TooManyUseRule,								// (ruleName)
				ClauseMixedUseAndCreateRule,				// (ruleName)								: A clause with a use rule cannot be a create rule or a partial rule.
				ClauseTypeMismatchedToPartialRule,			// (ruleName, partialRuleName)				: A clause uses a partial rule of an incompatible type.
				OptionalBodyCouldExpandToEmptySequence,		// (ruleName)
				LoopBodyCouldExpandToEmptySequence,			// (ruleName)
			};

			struct ParserError
			{
				ParserErrorType				type;
				WString						arg1;
				WString						arg2;
				WString						arg3;
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
				void AddError(ParserErrorType type, TArgs&& ...args)
				{
					ParserError error;
					error.type = type;

					WString sargs[] = { WString(args)... };
					WString* dargs[] = { &error.arg1,&error.arg2,&error.arg3 };
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
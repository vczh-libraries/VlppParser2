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

#define GLR_PARSER_ERROR_LIST(ERROR_ITEM)\
			/* AstSymbolManager */\
			ERROR_ITEM(DuplicatedFile,														fileName)\
			ERROR_ITEM(FileDependencyNotExists,												fileName, dependency)\
			ERROR_ITEM(FileCyclicDependency,												fileName, dependency)\
			ERROR_ITEM(DuplicatedSymbol,													fileName, symbolName)\
			ERROR_ITEM(DuplicatedSymbolGlobally,											fileName, symbolName, anotherFileName)\
			ERROR_ITEM(DuplicatedClassProp,													fileName, className, propName)\
			ERROR_ITEM(DuplicatedEnumItem,													fileName, enumName, propName)\
			ERROR_ITEM(BaseClassNotExists,													fileName, className, typeName)\
			ERROR_ITEM(BaseClassNotClass,													fileName, className, typeName)\
			ERROR_ITEM(BaseClassCyclicDependency,											fileName, className)\
			ERROR_ITEM(FieldTypeNotExists,													fileName, className, propName)\
			ERROR_ITEM(FieldTypeNotClass,													fileName, className, propName)\
			/* LexerSymbolManager */\
			ERROR_ITEM(InvalidTokenDefinition,												code)\
			ERROR_ITEM(DuplicatedToken,														tokenName)\
			ERROR_ITEM(DuplicatedTokenByDisplayText,										tokenName)\
			ERROR_ITEM(InvalidTokenRegex,													tokenName, errorMessage)\
			ERROR_ITEM(TokenRegexNotPure,													tokenName)\
			ERROR_ITEM(DuplicatedTokenFragment,												fragmentName)\
			ERROR_ITEM(TokenFragmentNotExists,												fragmentName)\
			/* SyntaxSymbolManager */\
			ERROR_ITEM(DuplicatedRule,														ruleName)\
			ERROR_ITEM(RuleIsIndirectlyLeftRecursive,										ruleName)													/* Indirect left recursion must be resolved before */\
			ERROR_ITEM(LeftRecursiveClauseInsidePushCondition,								ruleName)													/* The left recursive clause is not allowed to begin with a push condition syntax */\
			ERROR_ITEM(LeftRecursiveClauseInsideTestCondition,								ruleName)													/* The left recursive clause is not allowed to begin with a test condition syntax */\
			ERROR_ITEM(LeftRecursionPlaceholderMixedWithSwitches,							ruleName, placeholder, targetRuleName)\
			ERROR_ITEM(LeftRecursionInjectHasNoContinuation,								ruleName, placeholder, targetRuleName)\
			/* SyntaxAst(ResolveName) */\
			ERROR_ITEM(RuleNameConflictedWithToken,											ruleName)\
			ERROR_ITEM(TypeNotExistsInRule,													ruleName, name)\
			ERROR_ITEM(TypeNotClassInRule,													ruleName, name)\
			ERROR_ITEM(TokenOrRuleNotExistsInRule,											ruleName, name)\
			ERROR_ITEM(LiteralNotValidToken,												ruleName, name)\
			ERROR_ITEM(LiteralIsDiscardedToken,												ruleName, name)\
			ERROR_ITEM(ConditionalLiteralNotValidToken,										ruleName, name)\
			ERROR_ITEM(ConditionalLiteralIsDiscardedToken,									ruleName, name)\
			ERROR_ITEM(ConditionalLiteralIsDisplayText,										ruleName, name)\
			ERROR_ITEM(DuplicatedSwitch,													switchName)\
			ERROR_ITEM(UnusedSwitch,														switchName)\
			ERROR_ITEM(SwitchNotExists,														ruleName, switchName)\
			ERROR_ITEM(SyntaxInvolvesPrefixMergeWithIllegalRuleName,						ruleName)													/* A syntax uses prefix_merge should not use rule name that has _LRI/_LRIP/LRI_/LRIP_ */\
			ERROR_ITEM(SyntaxInvolvesPrefixMergeWithIllegalPlaceholderName,					ruleName, placeholderName)									/* A syntax uses prefix_merge should not use placeholder name that has _LRI/_LRIP/LRI_/LRIP_ */\
			/* SyntaxAst(CalculateTypes) */\
			ERROR_ITEM(RuleMixedPartialClauseWithOtherClause,								ruleName)\
			ERROR_ITEM(RuleWithDifferentPartialTypes,										ruleName, ruleType, newType)\
			ERROR_ITEM(RuleExplicitTypeIsNotCompatibleWithClauseType,						ruleName, ruleType, newType)								/* The type of the rule is explicitly specified, but it is incompatible with its clauses */\
			ERROR_ITEM(RuleCannotResolveToDeterministicType,								ruleName, ruleType, newType)								/* Unable to resolve to one type from clauses (token, type) or (create, partial) */\
			ERROR_ITEM(CyclicDependedRuleTypeIncompatible,									ruleName, ruleTypes)										/* Types of rules are not compatible to each other when they build cyclic dependency by reuse clauses */\
			ERROR_ITEM(ReuseClauseCannotResolveToDeterministicType,							ruleName, ruleTypes)										/* A reuse clause contains multiple use rule but their types are not compatible to each other */\
			ERROR_ITEM(ReuseClauseContainsNoUseRule,										ruleName)													/* A reuse clause contains no use rule therefore the type cannot be determined */\
			/* SyntaxAst(ValidateSwitchesAndConditions, condition) */\
			ERROR_ITEM(PushedSwitchIsNotTested,												ruleName, switchName)\
			ERROR_ITEM(PrefixMergeAffectedBySwitches,										ruleName, prefixMergeRule, switchName)\
			/* SyntaxAst(ValidateTypes) */\
			ERROR_ITEM(FieldNotExistsInClause,												ruleName, clauseType, fieldName)							/* The field does not exist in the type of the clause */\
			ERROR_ITEM(RuleTypeMismatchedToField,											ruleName, clauseType, fieldName, fieldRuleType)				/* The rule type is not compatible to the assigning field */\
			ERROR_ITEM(AssignmentToNonEnumField,											ruleName, clauseType, fieldName)							/* Assignment can only assign fields in enum types */\
			ERROR_ITEM(EnumItemMismatchedToField,											ruleName, clauseType, fieldName, enumItem)					/* Try to assign an unexisting or mismatched enum item to a field in an enum type */\
			ERROR_ITEM(UseRuleWithPartialRule,												ruleName, useRuleName)										/* A use rule should not be used with a partial rule */\
			ERROR_ITEM(UseRuleInNonReuseClause,												ruleName, useRuleName)										/* A use rule should only appear in reuse clause */\
			ERROR_ITEM(PartialRuleUsedOnField,												ruleName, clauseType, partialRuleName, fieldName)			/* A partial rule does not create object, it cannot be assigned to a field */\
			ERROR_ITEM(ClauseTypeMismatchedToPartialRule,									ruleName, clauseType, partialRuleName, partialRuleType)		/* A clause uses a partial rule of an incompatible type */\
			ERROR_ITEM(LeftRecursionPlaceholderNotFoundInRule,								ruleName, placeholder, targetRuleName)						/* left_recursion_inject injects to a rule which doesn't accept the specified placeholder */\
			ERROR_ITEM(LeftRecursionPlaceholderNotUnique,									ruleName, placeholder, targetRuleName)						/* left_recursion_inject injects to a rule which has multiple places accepting the specified placeholder */\
			ERROR_ITEM(LeftRecursionInjectTargetIsPrefixOfAnotherSameEnding,				ruleName, placeholder, targetPrefixName, targetRuleName)	/* left_recursion_inject injects into two targets, A is a prefix of B, and both injection could end with the same target C, C could be B */\
			ERROR_ITEM(LeftRecursionPlaceholderTypeMismatched,								ruleName, placeholder, targetRuleName, placeholderRuleName)\
			ERROR_ITEM(PartialRuleInLeftRecursionInject,									ruleName, partialRuleName)\
			ERROR_ITEM(PartialRuleInPrefixMerge,											ruleName, partialRuleName)\
			/* SyntaxAst(ValidateStructure, counting) */\
			ERROR_ITEM(ClauseNotCreateObject,												ruleName)													/* A reuse clause does not contain use rule in some potential sequences */\
			ERROR_ITEM(UseRuleUsedInOptionalBody,											ruleName, useRuleName)\
			ERROR_ITEM(UseRuleUsedInLoopBody,												ruleName, useRuleName)\
			ERROR_ITEM(ClauseTooManyUseRule,												ruleName)													/* Multiple use rules in a potential sequence in a clause */\
			ERROR_ITEM(NonArrayFieldAssignedInLoop,											ruleName, clauseType, fieldName)\
			ERROR_ITEM(NonLoopablePartialRuleUsedInLoop,									ruleName, clauseType, partialRuleName)\
			ERROR_ITEM(ClauseCouldExpandToEmptySequence,									ruleName)\
			ERROR_ITEM(LoopBodyCouldExpandToEmptySequence,									ruleName)\
			ERROR_ITEM(OptionalBodyCouldExpandToEmptySequence,								ruleName)\
			ERROR_ITEM(NegativeOptionalEndsAClause,											ruleName)													/* Negative optional syntax cannot ends a clause */\
			ERROR_ITEM(MultiplePrioritySyntaxInAClause,										ruleName)													/* Too many syntax with priority in the a clause */\
			ERROR_ITEM(PushConditionBodyCouldExpandToEmptySequence,							ruleName)\
			ERROR_ITEM(TestConditionBodyCouldExpandToEmptySequence,							ruleName)\
			ERROR_ITEM(MultipleEmptySyntaxInTestCondition,									ruleName)\
			ERROR_ITEM(TooManyLeftRecursionPlaceholderClauses,								ruleName)\
			/* SyntaxAst(ValidateStructure, relationship) */\
			ERROR_ITEM(FieldAssignedMoreThanOnce,											ruleName, clauseType, fieldName)\
			/* SyntaxAst(ValidateStructure, prefix_merge) */\
			ERROR_ITEM(RuleMixedPrefixMergeWithClauseNotSyntacticallyBeginWithARule,		ruleName)													/* If a rule has prefix_merge clause, than all other clause must syntactically begins with a rule */\
			ERROR_ITEM(RuleMixedPrefixMergeWithClauseNotBeginWithIndirectPrefixMerge,		ruleName, startRule)										/* If a rule has prefix_merge clause, than all other clause must directly or indirectly starts with prefix_merge */\
			ERROR_ITEM(RuleIndirectlyBeginsWithPrefixMergeMixedLeftRecursionMarkers,		ruleName, prefixMergeRule, leftRecursionMarkerRule)\
			ERROR_ITEM(RuleIndirectlyBeginsWithPrefixMergeMixedNonSimpleUseClause,			ruleName, prefixMergeRule)									/* If a rule indirectly begins with prefix_merge, then all clause must be, either a simple use clause begins with prefix_merge, or a clause not begins with prefix_merge */\
			/* SyntaxAst(RewriteSyntax, prefix_merge) */\
			ERROR_ITEM(PrefixExtractionAffectedRuleReferencedAnother,						ruleName, conflictedRule, prefixRule)						/* During left_recursion_inject clause generation, if prefix extracted affected the process, all !prefixRule clauses where prefixRule is the prefix of conflictedRule in any !conflictedRule clauses, prefixRule should not be affected */\
			ERROR_ITEM(PrefixExtractionAffectedBySwitches,									ruleName, conflictedRule, switchName)						/* During left_recursion_inject clause generation, if prefix extracted affected the process, !prefixRule should not be affected by any switch */

			enum class ParserErrorType
			{
#define ParserErrorType_EnumItem(NAME, ...) NAME,
				GLR_PARSER_ERROR_LIST(ParserErrorType_EnumItem)
#undef ParserErrorType_EnumItem
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

				const auto&					Errors() const { return errors; }

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
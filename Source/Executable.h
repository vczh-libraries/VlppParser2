/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_EXECUTABLE
#define VCZH_PARSER2_EXECUTABLE

#include "AstBase.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
/***********************************************************************
Executable
***********************************************************************/

			struct InstructionArray
			{
				vint32_t							start = -1;
				vint32_t							count = 0;
			};

			struct StringLiteral
			{
				vint32_t							start = -1;
				vint32_t							count = 0;
			};

			struct ReturnIndexArray
			{
				vint32_t							start = -1;
				vint32_t							count = -1;
			};

			struct EdgeArray
			{
				vint32_t							start = -1;
				vint32_t							count = 0;
			};

			enum class EdgePriority
			{
				NoCompetition,
				HighPriority,
				LowPriority,
			};

			enum class ReturnRuleType
			{
				Field,
				Partial,
				Discard,
				Reuse,
			};

			struct ReturnDesc
			{
				vint32_t							consumedRule = -1;
				vint32_t							returnState = -1;
				EdgePriority						priority = EdgePriority::NoCompetition;
				ReturnRuleType						ruleType = ReturnRuleType::Field;
				InstructionArray					insAfterInput;
			};

			struct EdgeDesc
			{
				vint32_t							fromState = -1;
				vint32_t							toState = -1;
				StringLiteral						condition;
				EdgePriority						priority = EdgePriority::NoCompetition;
				InstructionArray					insBeforeInput;
				InstructionArray					insAfterInput;
				ReturnIndexArray					returnIndices;
			};

			struct StateDesc
			{
				vint32_t							rule = -1;
				vint32_t							clause = -1;
				bool								endingState = false;
			};

			struct Executable
			{
				static constexpr vint32_t			EndOfInputInput = -1;
				static constexpr vint32_t			EndingInput = 0;
				static constexpr vint32_t			LeftrecInput = 1;
				static constexpr vint32_t			TokenBegin = 2;

				vint32_t							tokenCount = 0;
				vint32_t							ruleCount = 0;
				collections::Array<vint32_t>		ruleStartStates;		// ruleStartStates[rule] = the start state of this rule.
				collections::Array<EdgeArray>		transitions;			// transitions[state * (TokenBegin + tokenCount) + input] = edges from state with specified input.
				collections::Array<AstIns>			astInstructions;		// referenced by EdgeDesc::insBeforeInput and EdgeDesc::insAfterInput
				collections::Array<vint32_t>		returnIndices;			// referenced by ReturnIndexArray
				collections::Array<ReturnDesc>		returns;				// referenced by Executable::returnIndices
				collections::Array<EdgeDesc>		edges;					// referenced by EdgeArray
				collections::Array<StateDesc>		states;					// referenced by returnState/fromState/toState
				WString								stringLiteralBuffer;	// referenced by StringLiteral

				Executable() = default;
				Executable(stream::IStream& inputStream);

				void								Serialize(stream::IStream& outputStream);

				vint32_t GetTransitionIndex(vint32_t state, vint32_t input)
				{
					return state * (TokenBegin + tokenCount) + input;
				}
			};

			struct Metadata
			{
				collections::Array<WString>			ruleNames;
				collections::Array<WString>			stateLabels;
			};

/***********************************************************************
IExecutor
***********************************************************************/

			class ExecutorException : public Exception
			{
			public:
				vint32_t							tokenIndex1 = -1;
				vint32_t							tokenIndex2 = -1;

				ExecutorException(const WString& message, vint32_t _tokenIndex1, vint32_t _tokenIndex2)
					: Exception(message)
					, tokenIndex1(_tokenIndex1)
					, tokenIndex2(_tokenIndex2)
				{
				}
			};

			struct Trace;

			class IExecutor : public virtual Interface
			{
			public:
				class ITypeCallback : public virtual Interface
				{
				public:
					virtual WString					GetClassName(vint32_t classIndex) const = 0;
					virtual vint32_t				FindCommonBaseClass(vint32_t class1, vint32_t class2) const = 0;
				};

				virtual void						Initialize(vint32_t startState) = 0;
				virtual bool						Input(vint32_t currentTokenIndex, regex::RegexToken* token, regex::RegexToken* lookAhead) = 0;
				virtual bool						EndOfInput(bool& ambiguityInvolved) = 0;
				virtual void						PrepareTraceRoute() = 0;
				virtual void						ResolveAmbiguity() = 0;
				virtual Ptr<ParsingAstBase>			ExecuteTrace(IAstInsReceiver& receiver, collections::List<regex::RegexToken>& tokens) = 0;
			};

			extern Ptr<IExecutor>					CreateExecutor(Executable& executable, const IExecutor::ITypeCallback* typeCallback, vint _blockSize);
		}
	}
}

#endif
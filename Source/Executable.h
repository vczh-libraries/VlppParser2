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

			struct ReturnDesc
			{
				vint32_t							consumedRule = -1;
				vint32_t							returnState = -1;
				EdgePriority						priority = EdgePriority::NoCompetition;
				InstructionArray					insAfterInput;
			};

			struct EdgeDesc
			{
				vint32_t							fromState = -1;
				vint32_t							toState = -1;
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
				collections::Array<AstIns>			instructions;			// referenced by InstructionArray
				collections::Array<vint32_t>		returnIndices;			// referenced by ReturnIndexArray
				collections::Array<ReturnDesc>		returns;				// referenced by Executable::returnIndices
				collections::Array<EdgeDesc>		edges;					// referenced by EdgeArray
				collections::Array<StateDesc>		states;					// refereced by returnState/fromState/toState

				Executable() = default;
				Executable(stream::IStream& inputStream);

				void								Serialize(stream::IStream& outputStream);
			};

			struct Metadata
			{
				collections::Array<WString>			ruleNames;
				collections::Array<WString>			stateLabels;
			};
		}
	}
}

#endif
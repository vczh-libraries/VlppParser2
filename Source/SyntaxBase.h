/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_SYNTAXBASE
#define VCZH_PARSER2_SYNTAXBASE

#include "AstBase.h"

namespace vl
{
	namespace glr
	{
/***********************************************************************
Automaton
***********************************************************************/

		namespace automaton
		{
			struct InstructionArray
			{
				vint								start = -1;
				vint								count = 0;
			};

			struct ReturnArray
			{
				vint								start = -1;
				vint								count = -1;
			};

			struct EdgeArray
			{
				vint								start = -1;
				vint								count = 0;
			};

			struct ReturnDesc
			{
				vint								returnState = -1;
				InstructionArray					insAfterInput;
			};

			struct EdgeDesc
			{
				vint								fromState = -1;
				vint								toState = -1;
				InstructionArray					insBeforeInput;
				InstructionArray					insAfterInput;
				ReturnArray							returns;
			};

			struct StateDesc
			{
				vint								rule = -1;
				bool								endingState = false;
			};

			struct Executable
			{
				static constexpr vint				EndingInput = 0;
				static constexpr vint				LeftrecInput = 1;
				static constexpr vint				TokenBegin = 2;

				vint								tokenCount = 0;
				vint								ruleCount = 0;
				collections::Array<vint>			ruleStartStates;		// ruleStartStates[rule] = the start state of this rule.
				collections::Array<EdgeArray>		transitions;			// transitions[state * (TokenBegin + tokenCount) + input] = edges from state with specified input.
				collections::Array<AstIns>			instructions;			// referenced by InstructionArray
				collections::Array<ReturnDesc>		returns;				// referenced by ReturnArray
				collections::Array<EdgeDesc>		edges;					// referenced by EdgeArray
				collections::Array<StateDesc>		states;
			};

			struct Metadata
			{
				collections::Array<WString>			ruleNames;
				collections::Array<WString>			stateLabels;
			};

/***********************************************************************
Execution
***********************************************************************/
		}
	}
}

#endif
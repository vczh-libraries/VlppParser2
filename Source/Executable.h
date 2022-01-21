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

			enum class SwitchInsType
			{
				SwitchPushFrame,	// SwitchPushFrame()			: Push a new switch value frame, inheriting all values from the previous frame
				SwitchWriteTrue,	// SwitchWriteTrue(switchId)	: Write TRUE to a switch in the current frame
				SwitchWriteFalse,	// SwitchWriteFalse(switchId)	: Write FALSE to a switch in the current frame
				SwitchPopFrame,		// SwitchPopFrame()				: Pop the last frame

				ConditionRead,		// ConditionRead(switchId)		: Push the value of a switch to the stack
				ConditionNot,		// ConditionNot()				: Pop one value from the stack, perform NOT, push the result
				ConditionAnd,		// ConditionAnd()				: Pop two values from the stack, perform AND, push the result
				ConditionOr,		// ConditionOr()				: Pop two values from the stack, perform OR, push the result
				ConditionTest,		// ConditionTest()				: Pop one value from the stack, if it is FALSE, fail the current transition
			};

			struct SwitchIns
			{
				SwitchInsType								type = SwitchInsType::ConditionTest;
				vint32_t									param = -1;

				vint Compare(const SwitchIns& ins) const
				{
					auto result = (vint)type - (vint)ins.type;
					if (result != 0) return result;
					return (vint)param - (vint)ins.param;
				}

				bool operator==(const SwitchIns& ins) const { return Compare(ins) == 0; }
				bool operator!=(const SwitchIns& ins) const { return Compare(ins) != 0; }
				bool operator< (const SwitchIns& ins) const { return Compare(ins) < 0; }
				bool operator<=(const SwitchIns& ins) const { return Compare(ins) <= 0; }
				bool operator> (const SwitchIns& ins) const { return Compare(ins) > 0; }
				bool operator>=(const SwitchIns& ins) const { return Compare(ins) >= 0; }
			};

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
				StringLiteral						condition;
				EdgePriority						priority = EdgePriority::NoCompetition;
				InstructionArray					insSwitch;
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
				collections::Array<SwitchIns>		switchInstructions;		// referenced by EdgeDesc::insSwitch
				collections::Array<vint32_t>		returnIndices;			// referenced by ReturnIndexArray
				collections::Array<ReturnDesc>		returns;				// referenced by Executable::returnIndices
				collections::Array<EdgeDesc>		edges;					// referenced by EdgeArray
				collections::Array<StateDesc>		states;					// referenced by returnState/fromState/toState
				WString								stringLiteralBuffer;	// referenced by StringLiteral
				collections::Array<bool>			switchDefaultValues;	// default value of all switches

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
				collections::Array<WString>			switchNames;
			};
		}
	}
}

#endif
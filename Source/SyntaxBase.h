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
				vint								consumedRule = -1;
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

			template<typename T>
			class AllocateOnly : public Object
			{
			protected:
				vint											blockSize;
				vint											remains;
				collections::List<Ptr<collections::Array<T>>>	buffers;

			public:
				AllocateOnly(vint _blockSize = 65536)
					: blockSize(_blockSize)
					, remains(0)
				{
				}

				T* Get(vint index)
				{
					vint row = index / blockSize;
					vint column = index % blockSize;
					CHECK_ERROR(0 <= row && row < buffers.Count(), L"vl::glr::automaton::AllocateOnly<T>::Get(vint)#Index out of range.");
					if (row == buffers.Count() - 1)
					{
						CHECK_ERROR(0 <= column && column < (blockSize - remains), L"vl::glr::automaton::AllocateOnly<T>::Get(vint)#Index out of range.");
					}
					else
					{
						CHECK_ERROR(0 <= column && column < blockSize, L"vl::glr::automaton::AllocateOnly<T>::Get(vint)#Index out of range.");
					}
					return &buffers[row]->operator[](column);
				}

				vint Allocate()
				{
					if (remains == 0)
					{
						buffers.Add(new collections::Array<T>(blockSize));
						remains = blockSize;
					}
					vint index = blockSize * (buffers.Count() - 1) + (blockSize - remains);
					buffers[buffers.Count() - 1]->operator[](blockSize - remains).allocatedIndex = index;
					remains--;
					return index;
				}

				void Clear()
				{
					remains = 0;
					buffers.Clear();
				}
			};

			struct ReturnStack
			{
				vint	allocatedIndex;
				vint	previous;
				vint	state;
			};

			struct Trace
			{
				vint	allocatedIndex;
				vint	previous;
				vint	state;
				vint	returnStack;
				vint	byEdge;
				vint	byInput;
				vint	traceBeginObject;
				vint	traceAfterBranch;
			};

			class TraceManager : public Object
			{
			protected:
				AllocateOnly<ReturnStack>			returnStacks;
				AllocateOnly<Trace>					traces;
				collections::List<Trace*>			traces1;
				collections::List<Trace*>			traces2;

			public:
				vint								concurrentCount = 0;
				collections::List<Trace*>*			concurrentTraces = &traces1;
				collections::List<Trace*>*			backupTraces = &traces2;

				ReturnStack*						GetReturnStack(vint index);
				ReturnStack*						AllocateReturnStack();
				Trace*								GetTrace(vint index);
				Trace*								AllocateTrace();
				void								Swap();
				void								Initialize(vint startState);
			};
		}
	}
}

#endif
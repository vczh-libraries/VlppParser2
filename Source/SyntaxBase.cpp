#include "SyntaxBase.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
			using namespace collections;
			using namespace stream;

/***********************************************************************
TraceManager
***********************************************************************/

			void TraceManager::BeginSwap()
			{
				concurrentCount = 0;
			}

			void TraceManager::AddTrace(Trace* trace)
			{
				if (concurrentCount < backupTraces->Count())
				{
					backupTraces->Set(concurrentCount, trace);
				}
				else
				{
					backupTraces->Add(trace);
				}
				concurrentCount++;
			}

			void TraceManager::EndSwap()
			{
				auto t = concurrentTraces;
				concurrentTraces = backupTraces;
				backupTraces = t;
			}

			TraceManager::TraceManager(Executable& _executable)
				:executable(_executable)
			{
			}

			ReturnStack* TraceManager::GetReturnStack(vint index)
			{
				return returnStacks.Get(index);
			}

			ReturnStack* TraceManager::AllocateReturnStack()
			{
				auto returnStack = returnStacks.Get(returnStacks.Allocate());
				returnStack->previous = -1;
				returnStack->returnIndex = -1;
				return returnStack;
			}

			Trace* TraceManager::GetTrace(vint index)
			{
				return traces.Get(index);
			}

			Trace* TraceManager::AllocateTrace()
			{
				auto trace = traces.Get(traces.Allocate());
				trace->previous = -1;
				trace->state = -1;
				trace->returnStack = -1;
				trace->byEdge = -1;
				trace->byInput = -1;
				trace->previousTokenIndex = -1;
				trace->currentTokenIndex = -1;
				trace->traceBeginObject = -1;
				trace->traceAfterBranch = -1;
				return trace;
			}

			void TraceManager::Initialize(vint startState)
			{
				returnStacks.Clear();
				traces.Clear();
				traces1.Clear();
				traces2.Clear();
				concurrentTraces = &traces1;
				backupTraces = &traces2;

				auto trace = AllocateTrace();
				trace->state = startState;
				concurrentCount = 1;
				concurrentTraces->Add(trace);
			}

			void TraceManager::Input(vint currentTokenIndex, vint token)
			{
				vint previousTokenIndex = currentTokenIndex - 1;
				vint input = Executable::TokenBegin + token;
				BeginSwap();
				for (auto trace : *concurrentTraces)
				{
					vint transactionIndex = trace->state * (Executable::TokenBegin + executable.tokenCount) + input;
					auto&& edgeArray = executable.transitions[transactionIndex];
					for (vint edgeRef = 0; edgeRef < edgeArray.count; edgeRef++)
					{
						auto& edgeDesc = executable.edges[edgeArray.start + edgeRef];
					}
				}
				EndSwap();
			}

			void TraceManager::EndOfInput()
			{
			}
		}
	}
}
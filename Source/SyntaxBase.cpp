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

			ReturnStack* TraceManager::GetReturnStack(vint index)
			{
				return returnStacks.Get(index);
			}

			ReturnStack* TraceManager::AllocateReturnStack()
			{
				return returnStacks.Get(returnStacks.Allocate());
			}

			Trace* TraceManager::GetTrace(vint index)
			{
				return traces.Get(index);
			}

			Trace* TraceManager::AllocateTrace()
			{
				return traces.Get(traces.Allocate());
			}

			void TraceManager::Initialize(vint startState)
			{
				returnStacks.Clear();
				traces.Clear();
				traces1.Clear();
				traces2.Clear();

				auto trace = AllocateTrace();
				trace->previous = -1;
				trace->state = startState;
				trace->returnStack = -1;
				trace->byEdge = -1;
				trace->byInput = -1;
				trace->previousTokenIndex = -1;
				trace->currentTokenIndex = -1;
				trace->traceBeginObject = -1;
				trace->traceAfterBranch = -1;

				concurrentCount = 1;
				concurrentTraces->Add(trace);
			}

			void TraceManager::Input(vint tokenIndex, vint input)
			{
			}

			void TraceManager::EndOfInput()
			{
			}
		}
	}
}
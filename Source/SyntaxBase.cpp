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

			void TraceManager::Swap()
			{
				auto t = concurrentTraces;
				concurrentTraces = backupTraces;
				backupTraces = t;
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
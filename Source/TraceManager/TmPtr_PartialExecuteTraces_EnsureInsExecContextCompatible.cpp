#include "TraceManager.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
/***********************************************************************
EnsureInsExecContextCompatible
***********************************************************************/

			void TraceManager::EnsureInsExecContextCompatible(Trace* baselineTrace, Trace* commingTrace)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::EnsureInsExecContextCompatible(Trace*, Trace*)#"
				auto&& contextComming = GetTraceExec(baselineTrace->traceExecRef)->context;
				auto&& contextBaseline = GetTraceExec(commingTrace->traceExecRef)->context;
				auto error = []()
				{
					CHECK_FAIL(ERROR_MESSAGE_PREFIX L"Execution results of traces to merge are different.");
				};

				// check if the two lriStored be both empty or non-empty
				if ((contextBaseline.lriStoredObjects == nullref) != (contextComming.lriStoredObjects == nullref)) error();

				// check if the two objectStack have the same depth
				if ((contextBaseline.objectStack == nullref) != (contextComming.objectStack == nullref)) error();
				if (contextBaseline.objectStack != nullref)
				{
					auto stackBaseline = GetInsExec_ObjectStack(contextBaseline.objectStack);
					auto stackComming = GetInsExec_ObjectStack(contextComming.objectStack);
					if (stackBaseline->pushedCount != stackComming->pushedCount) error();
				}

				// check if the two createStack have the same depth
				// check each corresponding createStack have the same stackBase
				auto stack1 = contextBaseline.createStack;
				auto stack2 = contextComming.createStack;
				while (stack1 != stack2)
				{
					if (stack1 == nullref || stack2 == nullref) error();

					auto stackObj1 = GetInsExec_CreateStack(stack1);
					auto stackObj2 = GetInsExec_CreateStack(stack2);

					if (stackObj1->stackBase != stackObj2->stackBase) error();

					stack1 = stackObj1->previous;
					stack2 = stackObj2->previous;
				}
#undef ERROR_MESSAGE_PREFIX
			}
		}
	}
}
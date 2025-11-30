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

				// check if the two objectStack have the same depth
				if ((contextBaseline.objectStack == nullref) != (contextComming.objectStack == nullref)) error();
				if (contextBaseline.objectStack != nullref)
				{
					auto objectStackBaseline = GetInsExec_StackArrayRefLink(contextBaseline.objectStack);
					auto objectStackComming = GetInsExec_StackArrayRefLink(contextComming.objectStack);
					if (objectStackBaseline->currentDepth != objectStackComming->currentDepth) error();
				}

				// check if the two createStack have the same depth
				// check each corresponding createStack have the same stackBase
				auto stackBaseline = contextBaseline.createStack;
				auto stackComming = contextComming.createStack;
				while (stackBaseline != stackComming)
				{
					if (stackBaseline == nullref || stackComming == nullref) error();

					auto stackObjBaseline = GetInsExec_StackArrayRefLink(stackBaseline);
					auto stackObjComming = GetInsExec_StackArrayRefLink(stackComming);

					if (stackObjBaseline->objectStackDepthForCreateStack != stackObjComming->objectStackDepthForCreateStack) error();

					stackBaseline = stackObjBaseline->previous;
					stackComming = stackObjComming->previous;
				}
#undef ERROR_MESSAGE_PREFIX
			}
		}
	}
}
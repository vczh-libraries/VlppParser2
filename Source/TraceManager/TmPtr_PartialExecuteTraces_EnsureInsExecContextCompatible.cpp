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
#define TRACE_MAMAGER_PHRASE L"PrepareTraceRoute/EnsureInsExecContextCompatible"
				auto&& contextComming = GetTraceExec(baselineTrace->traceExecRef)->context;
				auto&& contextBaseline = GetTraceExec(commingTrace->traceExecRef)->context;

				// check if the two objectStack have the same depth
				if ((contextBaseline.objectStack == nullref) != (contextComming.objectStack == nullref))
				{
					throw TraceException(*this, baselineTrace, commingTrace, TRACE_MAMAGER_PHRASE, L"Execution results of traces to merge do not have the same depth of objectStack.");
				}
				if (contextBaseline.objectStack != nullref)
				{
					auto objectStackBaseline = GetInsExec_StackArrayRefLink(contextBaseline.objectStack);
					auto objectStackComming = GetInsExec_StackArrayRefLink(contextComming.objectStack);
					if (objectStackBaseline->currentDepth != objectStackComming->currentDepth)
					{
						throw TraceException(*this, baselineTrace, commingTrace, TRACE_MAMAGER_PHRASE, L"Execution results of traces to merge do not have the same depth of objectStack.");
					}
				}

				// check if the two createStack have the same depth
				// check each corresponding createStack have the same stackBase
				auto stackBaseline = contextBaseline.createStack;
				auto stackComming = contextComming.createStack;
				while (stackBaseline != stackComming)
				{
					if (stackBaseline == nullref || stackComming == nullref)
					{
						throw TraceException(*this, baselineTrace, commingTrace, TRACE_MAMAGER_PHRASE, L"Execution results of traces to merge do not have the same depth of createStack.");
					}

					auto stackObjBaseline = GetInsExec_StackArrayRefLink(stackBaseline);
					auto stackObjComming = GetInsExec_StackArrayRefLink(stackComming);

					if (stackObjBaseline->objectStackDepthForCreateStack != stackObjComming->objectStackDepthForCreateStack)
					{
						throw TraceException(*this, baselineTrace, commingTrace, TRACE_MAMAGER_PHRASE, L"Execution results of traces to merge do not have the same depth of objectStack between one slice of both createStack.");
					}

					stackBaseline = stackObjBaseline->previous;
					stackComming = stackObjComming->previous;
				}
#undef TRACE_MAMAGER_PHRASE
			}
		}
	}
}
#include "TraceManager.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
/***********************************************************************
ResolveAmbiguity
***********************************************************************/

			void TraceManager::ResolveAmbiguity()
			{
				CHECK_ERROR(state == TraceManagerState::PreparedTraceRoute, L"vl::glr::automaton::TraceManager::ResolveAmbiguity()#Wrong timing to call this function.");
				state = TraceManagerState::ResolvedAmbiguity;

				CheckMergeTraces();
				BuildExecutionOrder();
			}

#undef NEW_MERGE_STACK_MAGIC_COUNTER
		}
	}
}
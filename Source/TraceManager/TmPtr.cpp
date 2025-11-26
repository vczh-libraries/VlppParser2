#include "TraceManager.h"
#include "TraceManager_Common.h"

#if defined VCZH_MSVC && defined _DEBUG
#define VCZH_DO_DEBUG_CHECK
#endif

namespace vl
{
	namespace glr
	{
		namespace automaton
		{

/***********************************************************************
PrepareTraceRoute
***********************************************************************/

			void TraceManager::PrepareTraceRoute()
			{
				CHECK_ERROR(state == TraceManagerState::Finished, L"vl::glr::automaton::TraceManager::PrepareTraceRoute()#Wrong timing to call this function.");
				state = TraceManagerState::PreparedTraceRoute;

				AllocateExecutionData();
				BuildAmbiguityStructures();
				PartialExecuteTraces();
			}
		}
	}
}

#if defined VCZH_MSVC && defined _DEBUG
#undef VCZH_DO_DEBUG_CHECK
#endif
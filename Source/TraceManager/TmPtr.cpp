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
DebugCheckTraceExecData
***********************************************************************/

#ifdef VCZH_DO_DEBUG_CHECK
			void TraceManager::DebugCheckTraceExecData()
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::DebugCheckTraceExecData()#"
				IterateSurvivedTraces(
					[this](Trace* trace, Trace* predecessor, vint32_t visitCount, vint32_t predecessorCount)
					{
						if (predecessorCount <= 1)
						{
							auto traceExec = GetTraceExec(trace->traceExecRef);
							for (vint32_t insRef = 0; insRef < traceExec->insExecRefs.count; insRef++)
							{
								auto&& ins = ReadInstruction(insRef, traceExec->insLists);
								auto insExec = GetInsExec(traceExec->insExecRefs.start + insRef);

								// ensure BO/BOLR/DFA are closed
								switch (ins.type)
								{
								case AstInsType::BeginObject:
								case AstInsType::BeginObjectLeftRecursive:
								case AstInsType::DelayFieldAssignment:
									CHECK_ERROR(insExec->eoInsRefs != nullref, ERROR_MESSAGE_PREFIX L"Internal error: BO/BOLA/DFA not closed.");
									break;
								}

								// ensure DFA are associated with objects closed
								switch (ins.type)
								{
								case AstInsType::DelayFieldAssignment:
									CHECK_ERROR(insExec->objRefs != nullref, ERROR_MESSAGE_PREFIX L"Internal error: DFA not associated.");
									break;
								}
							}
						}
					}
				);
#undef ERROR_MESSAGE_PREFIX
			}
#endif

/***********************************************************************
PrepareTraceRoute
***********************************************************************/

			void TraceManager::PrepareTraceRoute()
			{
				CHECK_ERROR(state == TraceManagerState::Finished, L"vl::glr::automaton::TraceManager::PrepareTraceRoute()#Wrong timing to call this function.");
				state = TraceManagerState::PreparedTraceRoute;

				AllocateExecutionData();
				PartialExecuteTraces();
				BuildAmbiguityStructures();
#ifdef VCZH_DO_DEBUG_CHECK
				DebugCheckTraceExecData();
#endif
			}
		}
	}
}

#if defined VCZH_MSVC && defined _DEBUG
#undef VCZH_DO_DEBUG_CHECK
#endif
#include "TraceManager.h"
#include "TraceManager_Common.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
/***********************************************************************
PartialExecuteTraces
***********************************************************************/

			void TraceManager::PartialExecuteTraces()
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::PartialExecuteTraces()#"
				IterateSurvivedTraces(
					[this](Trace* trace, Trace* predecessor, vint32_t visitCount, vint32_t predecessorCount)
					{
						if (predecessorCount <= 1)
						{
							PartialExecuteOrdinaryTrace(trace);
						}
						else
						{
							if (visitCount > 1)
							{
								EnsureInsExecContextCompatible(predecessor, GetTrace(trace->predecessors.first));
							}

							if (visitCount == predecessorCount)
							{
								MergeInsExecContext(trace);
							}
						}
					}
				);

				CalculateObjectFirstInstruction();
#undef ERROR_MESSAGE_PREFIX
			}
		}
	}
}
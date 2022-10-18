#include "TraceManager.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
/***********************************************************************
IterateSurvivedTraces
***********************************************************************/

			template<typename TCallback>
			void TraceManager::IterateSurvivedTraces(TCallback&& callback)
			{
				Trace* lastTrace = nullptr;
				collections::List<Trace*> traces;
				traces.Add(initialTrace);

				while (traces.Count() > 0)
				{
					auto current = traces[traces.Count() - 1];
					traces.RemoveAt(traces.Count() - 1);

					if (current->iterateCounter == current->predecessorCount)
					{
						current->iterateCounter = 0;
					}

					current->iterateCounter++;
					callback(
						current,
						(
							current->predecessorCount == 0 ? nullptr :
							current->predecessorCount == 1 ? GetTrace(current->predecessors.first) :
							lastTrace
						),
						current->iterateCounter,
						current->predecessorCount
						);
					lastTrace = current;
					if (current->iterateCounter < current->predecessorCount) continue;

					auto successorId = current->successors.last;
					while (successorId != nullref)
					{
						auto successor = GetTrace(successorId);
						successorId = successor->successors.siblingPrev;
						traces.Add(successor);
					}
				}
			}
		}
	}
}
#include "TraceManager.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{

/***********************************************************************
GetCurrentSuccessorInReturnStack
***********************************************************************/

			ReturnStackSuccessors* TraceManager::GetCurrentSuccessorInReturnStack(Ref<ReturnStack> base, vint32_t currentTokenIndex)
			{
				auto& cache = !base ? initialReturnStackCache : GetReturnStack(base)->cache;
				if (cache.successors.tokenIndex == currentTokenIndex)
				{
					return &cache.successors;
				}
				if (cache.lastSuccessors.tokenIndex == currentTokenIndex)
				{
					return &cache.lastSuccessors;
				}

				CHECK_ERROR(currentTokenIndex > cache.successors.tokenIndex, L"vl::glr::automaton::TraceManager::GetCurrentSuccessorInReturnStack(vint32_t, vint32_t)#ReturnStackSuccessors::tokenIndex corrupted.");
				cache.lastSuccessors = cache.successors;
				cache.successors = {};
				cache.successors.tokenIndex = currentTokenIndex;
				return &cache.successors;
			}

/***********************************************************************
PushReturnStack
***********************************************************************/

			ReturnStack* TraceManager::PushReturnStack(Ref<ReturnStack> base, vint32_t returnIndex, Ref<Trace> fromTrace, vint32_t currentTokenIndex, bool allowReuse)
			{
				auto siblings = allowReuse ? GetCurrentSuccessorInReturnStack(base, currentTokenIndex) : nullptr;

				if (siblings)
				{
					auto successorId = siblings->first;
					while (successorId)
					{
						auto successor = GetReturnStack(successorId);
						successorId = successor->cache.next;

						if (successor->returnIndex == returnIndex && successor->fromTrace == fromTrace)
						{
							return successor;
						}
					}
				}

				auto returnStack = AllocateReturnStack();
				returnStack->previous = base;
				returnStack->returnIndex = returnIndex;
				returnStack->fromTrace = fromTrace;
				returnStack->cache.tokenIndex = currentTokenIndex;

				if (siblings)
				{
					if (!siblings->first)
					{
						siblings->first = returnStack;
						siblings->last = returnStack;
					}
					else
					{
						GetReturnStack(siblings->last)->cache.next = returnStack;
						returnStack->cache.prev = siblings->last;
						siblings->last = returnStack;
					}
				}
				return returnStack;
			}
		}
	}
}
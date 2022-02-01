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

			ReturnStackSuccessors* TraceManager::GetCurrentSuccessorInReturnStack(vint32_t base, vint32_t currentTokenIndex)
			{
				auto& cache = base == -1 ? initialReturnStackCache : GetReturnStack(base)->cache;
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

			ReturnStack* TraceManager::PushReturnStack(vint32_t base, vint32_t returnIndex, vint32_t currentTokenIndex)
			{
				auto siblings = GetCurrentSuccessorInReturnStack(base, currentTokenIndex);

				{
					vint32_t successorId = siblings->first;
					while (successorId != -1)
					{
						auto successor = GetReturnStack(successorId);
						successorId = successor->cache.next;

						if (successor->returnIndex == returnIndex)
						{
							return successor;
						}
					}
				}

				auto returnStack = AllocateReturnStack();
				returnStack->previous = base;
				returnStack->returnIndex = returnIndex;
				returnStack->cache.tokenIndex = currentTokenIndex;

				{
					if (siblings->first == -1)
					{
						siblings->first = returnStack->allocatedIndex;
						siblings->last = returnStack->allocatedIndex;
					}
					else
					{
						GetReturnStack(siblings->last)->cache.next = returnStack->allocatedIndex;
						returnStack->cache.prev = siblings->last;
						siblings->last = returnStack->allocatedIndex;
					}
				}
				return returnStack;
			}
		}
	}
}
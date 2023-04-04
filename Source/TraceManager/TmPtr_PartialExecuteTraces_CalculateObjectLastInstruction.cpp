#include "TraceManager.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
			using namespace collections;

#define NEW_MERGE_STACK_MAGIC_COUNTER (void)(MergeStack_MagicCounter++)

/***********************************************************************
CalculateObjectLastInstruction
***********************************************************************/

			void TraceManager::CalculateObjectLastInstruction()
			{
				// check all individual objects
				{
					auto objRef = firstObject;
					while (objRef != nullref)
					{
						auto ieObject = GetInsExec_Object(objRef);
						objRef = ieObject->previous;
					}
				}
			}

#undef NEW_MERGE_STACK_MAGIC_COUNTER
		}
	}
}
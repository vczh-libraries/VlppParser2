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
CalculateObjectFirstInstruction
***********************************************************************/

			void TraceManager::InjectFirstInstruction(Ref<Trace> trace, vint32_t ins, Ref<InsExec_ObjRefLink> injectTargets, vuint64_t magicInjection)
			{
				auto objLinkRef = injectTargets;
				while (objLinkRef != nullref)
				{
					auto objLink = GetInsExec_ObjRefLink(objLinkRef);
					objLinkRef = objLink->previous;
					auto ieObject = GetInsExec_Object(objLink->id);

					if (ieObject->mergeCounter == magicInjection) continue;
					ieObject->mergeCounter = magicInjection;

					if (
						trace < ieObject->topTrace ||
						(trace == ieObject->topTrace && ins < ieObject->topIns)
						)
					{
						// there will be only one top create instruction per object
						// even when object relationship is partial ordered
						// TODO: prove it
						ieObject->topTrace = trace;
						ieObject->topIns = ins;
						InjectFirstInstruction(trace, ins, ieObject->injectObjectIds, magicInjection);
					}
				}
			}

			void TraceManager::CalculateObjectFirstInstruction()
			{
				// check all individual objects
				{
					auto objRef = firstObject;
					while (objRef != nullref)
					{
						auto ieObject = GetInsExec_Object(objRef);
						objRef = ieObject->previous;

						// set the top local trace to its create trace
						ieObject->topLocalTrace = ieObject->bo_bolr_Trace;
						ieObject->topLocalIns = ieObject->bo_bolr_Ins;

						// check all DFA instructions
						auto insRefLinkId = ieObject->dfaInsRefs;
						while (insRefLinkId != nullref)
						{
							auto insRefLink = GetInsExec_InsRefLink(insRefLinkId);
							insRefLinkId = insRefLink->previous;
							if (
								insRefLink->trace < ieObject->topLocalTrace ||
								(insRefLink->trace == ieObject->topLocalTrace && insRefLink->ins < ieObject->topLocalIns)
								)
							{
								// there will be only one top local create instruction per object
								// even when object relationship is partial ordered
								// TODO: prove it
								ieObject->topLocalTrace = insRefLink->trace;
								ieObject->topLocalIns = insRefLink->ins;
							}
						}

						// set the top trace to its top local trace
						ieObject->topTrace = ieObject->topLocalTrace;
						ieObject->topIns = ieObject->topLocalIns;
					}
				}

				// check all inject into targets
				{
					auto objRef = firstObject;
					while (objRef != nullref)
					{
						auto ieObject = GetInsExec_Object(objRef);
						objRef = ieObject->previous;

						NEW_MERGE_STACK_MAGIC_COUNTER;
						auto magicInjection = MergeStack_MagicCounter;
						ieObject->mergeCounter = magicInjection;
						InjectFirstInstruction(ieObject->topTrace, ieObject->topIns, ieObject->injectObjectIds, magicInjection);
					}
				}
			}

#undef NEW_MERGE_STACK_MAGIC_COUNTER
		}
	}
}
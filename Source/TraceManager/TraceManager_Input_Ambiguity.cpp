#include "TraceManager.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{

/***********************************************************************
AreTwoTraceEqual
***********************************************************************/

			bool TraceManager::AreTwoEndingInputTraceEqual(vint32_t state, vint32_t returnStack, vint32_t executedReturnStack, vint32_t acId, vint32_t switchValues, Trace* candidate)
			{
				// two traces equal to each other if
				//   1) they are in the same state
				//   2) they have the same executedReturnStack (and therefore the same returnStack)
				//   3) they are attending same competitions
				//   4) they have the same switchValues
				//   5) the candidate has an ending input

				if (state != candidate->state) return false;
				if (acId != candidate->competitionRouting.attendingCompetitions) return false;
				if (switchValues != candidate->switchValues) return false;
				if (candidate->byInput != Executable::EndingInput) return false;

				if (executedReturnStack != candidate->executedReturnStack) return false;
				if (returnStack != candidate->returnStack) return false;
				return true;
			}

/***********************************************************************
GetInstructionPostfix
***********************************************************************/

			vint32_t TraceManager::GetInstructionPostfix(EdgeDesc& oldEdge, EdgeDesc& newEdge)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::GetInstructionPostfix(EdgeDesc&, EdgeDesc&)#"
				// given two equal traces, calculate their common instruction postfix length in insBeforeInput
				// EndObject is the last instruction of the prefix

				// EndObject may not be the first instruction in both edges
				// and instructions before EndObject could be different
				// the most common case is different {field = value} before EndObject
				// if the ambiguity is created by two left recursive clauses which consume the same series of tokens

				CHECK_ERROR(oldEdge.insAfterInput.count == 0, ERROR_MESSAGE_PREFIX L"EndingInput edge is not allowed to have insAfterInput.");
				CHECK_ERROR(newEdge.insAfterInput.count == 0, ERROR_MESSAGE_PREFIX L"EndingInput edge is not allowed to have insAfterInput.");

				// find the first EndObject instruction
				vint32_t i1 = -1;
				vint32_t i2 = -1;

				for (vint32_t insRef = 0; insRef < oldEdge.insBeforeInput.count; insRef++)
				{
					auto&& ins = executable.astInstructions[oldEdge.insBeforeInput.start + insRef];
					if (ins.type == AstInsType::EndObject)
					{
						i1 = insRef;
						break;
					}
				}

				for (vint32_t insRef = 0; insRef < newEdge.insBeforeInput.count; insRef++)
				{
					auto&& ins = executable.astInstructions[newEdge.insBeforeInput.start + insRef];
					if (ins.type == AstInsType::EndObject)
					{
						i2 = insRef;
						break;
					}
				}

				CHECK_ERROR(i1 != -1, ERROR_MESSAGE_PREFIX L"EndObject from oldEdge not found.");
				CHECK_ERROR(i2 != -1, ERROR_MESSAGE_PREFIX L"EndObject from newEdge not found.");

				// ensure they have the same instruction postfix starting from EndObject
				CHECK_ERROR(oldEdge.insBeforeInput.count - i1 == newEdge.insBeforeInput.count - i2, L"Two instruction postfix after EndObject not equal.");

				vint32_t postfix = oldEdge.insBeforeInput.count - i1 - 1;
				for (vint32_t postfixRef = 0; postfixRef < postfix; postfixRef++)
				{
					auto&& ins1 = executable.astInstructions[oldEdge.insBeforeInput.start + i1 + 1 + postfixRef];
					auto&& ins2 = executable.astInstructions[newEdge.insBeforeInput.start + i2 + 1 + postfixRef];
					CHECK_ERROR(ins1 == ins2, L"Two instruction postfix after EndObject not equal.");
				}
				return postfix;
#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
MergeTwoEndingInputTrace
***********************************************************************/

			void TraceManager::MergeTwoEndingInputTrace(
				Trace* trace,
				Trace* ambiguityTraceToMerge,
				vint32_t currentTokenIndex,
				vint32_t input,
				vint32_t byEdge,
				EdgeDesc& edgeDesc,
				vint32_t state,
				vint32_t returnStack,
				vint32_t attendingCompetitions,
				vint32_t carriedCompetitions,
				vint32_t executedReturnStack)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::MergeTwoEndingInputTrace(...)#"
				// goal of this function is to create a structure
				// ? -----+-> AMBIGUITY
				//        |
				// TRACE -+

				// but AMBIGUITY or the virtual new trace may not begin with EndObject
				// the structure will have to be twisted so that
				// 1) AMBIGUITY begins with EndObject
				// 2) instructions before EndObject in AMBIGUITY is stored in the FORMER trace
				// 3) instructions before EndObject in the virtual new trace is stored in the NEW trace
				// which results in such structure
				// ? -> FORMER --+-> AMBIGUITY
				//               |
				// TRACE -> NEW -+

				// FORMER and NEW are only created when corresponding instruction prefix exist
				// this function handle every possible case to keep the structure small and correct

				// if ambiguity resolving happens
				// find the instruction postfix
				// the instruction prefix ends at EndObject of a trace
				// and both instruction postfix should equal

				// old == ambiguityTraceToMerge
				// new == the trace that is being created (could skip)
				auto& oldEdge = executable.edges[ambiguityTraceToMerge->byEdge];
				vint32_t oldInsCount = oldEdge.insBeforeInput.count + oldEdge.insAfterInput.count;
				vint32_t newInsCount = edgeDesc.insBeforeInput.count + edgeDesc.insAfterInput.count;
				vint32_t returnInsCount = 0;
				vint32_t postfix = GetInstructionPostfix(oldEdge, edgeDesc);

				// if two state can merge
				// then executedReturnStack == ambiguityTraceToMerge.executedReturnStack
				// so two ReturnDesc.insAfterInput.count are identical
				// and also instructions
				if (executedReturnStack != -1)
				{
					auto rs = GetReturnStack(executedReturnStack);
					auto& rd = executable.returns[rs->returnIndex];
					returnInsCount = rd.insAfterInput.count;
					postfix += returnInsCount;
					oldInsCount += returnInsCount;
					newInsCount += returnInsCount;
				}

				// a trace needs to be cut if EndObject is not its first instruction
				bool needCut = oldInsCount > postfix + 1 || newInsCount > postfix + 1;

				if (ambiguityTraceToMerge->ambiguityMergeInsPostfix == -1 && needCut)
				{
					// create an extra trace between ambiguityTraceToMerge and its predecessors
					// ? -> FORMER -> AMBIGUITY

					Trace* firstFormer = nullptr;
					Trace* lastFormer = nullptr;
					vint32_t predecessorId = ambiguityTraceToMerge->predecessors.first;
					while (predecessorId != -1)
					{
						auto predecessor = GetTrace(predecessorId);
						predecessorId = predecessor->predecessors.siblingNext;

						auto formerTrace = AllocateTrace();
						{
							vint32_t formerId = formerTrace->allocatedIndex;
							*formerTrace = *ambiguityTraceToMerge;
							formerTrace->allocatedIndex = formerId;
						}

						// connect predecessor and formerTrace
						formerTrace->predecessors.first = predecessor->allocatedIndex;
						formerTrace->predecessors.last = predecessor->allocatedIndex;
						formerTrace->predecessors.siblingPrev = -1;
						formerTrace->predecessors.siblingNext = -1;

						// connect ambiguityTraceToMerge and formerTrace
						if (firstFormer == nullptr)
						{
							firstFormer = formerTrace;
							lastFormer = formerTrace;
						}
						else
						{
							lastFormer->predecessors.siblingNext = formerTrace->allocatedIndex;
							formerTrace->predecessors.siblingPrev = lastFormer->allocatedIndex;
							lastFormer = formerTrace;
						}

						// executedReturnStack is from the EndObject instruction
						// which is available in the instruction postfix
						// so formerTrace->executedReturnStack should be -1 and keep the previous return stack
						formerTrace->executedReturnStack = -1;
						formerTrace->returnStack = predecessor->returnStack;

						// ambiguity is filled by PrepareTraceRoute, skipped
						// runtimeRouting.holdingCompetition always belong to the second trace
						// runtimeRouting.attendingCompetitions is inherited
						// runtimeRouting.carriedCompetitions is inherited
						formerTrace->competitionRouting = {};
						formerTrace->competitionRouting.attendingCompetitions = ambiguityTraceToMerge->competitionRouting.attendingCompetitions;
						formerTrace->competitionRouting.carriedCompetitions = ambiguityTraceToMerge->competitionRouting.carriedCompetitions;

						// both traces need to have the same postfix
						// since formerTrace doesn't have executedReturnStack but ambiguityTraceToMerge has
						// the amount of returnInsCount need to cut from the postfix
						formerTrace->ambiguityBranchInsPostfix = postfix - returnInsCount;
					}

					ambiguityTraceToMerge->ambiguityMergeInsPostfix = postfix;
					ambiguityTraceToMerge->predecessors.first = firstFormer->allocatedIndex;
					ambiguityTraceToMerge->predecessors.last = lastFormer->allocatedIndex;
				}

				if (needCut)
				{
					// if EndObject is not the first instruction of the new trace
					// create a new trace with the instruction prefix
					// (? | FORMER) -+-> AMBIGUITY
					//               |
					// TRACE -> NEW -+

					auto newTrace = AllocateTrace();
					AddTraceToCollection(newTrace, trace, &Trace::predecessors);
					newTrace->state = state;
					newTrace->returnStack = returnStack;
					newTrace->switchValues = ambiguityTraceToMerge->switchValues;
					newTrace->byEdge = byEdge;
					newTrace->byInput = input;
					newTrace->currentTokenIndex = currentTokenIndex;

					// 1) executedReturnStack == ambiguityTraceToMerge->executedReturnStack is ensured
					//    in order to prevent executedReturnStack from being executed twice
					//    no need to assign executedReturnStack to newTrace which cause
					// 2) acid == ambiguityTraceToMerge->runtimeRouting.attendingCompetitions is ensured
					//    newTrace is supposed to inherit this value from ambiguityTraceToMerge
					newTrace->competitionRouting.attendingCompetitions = attendingCompetitions;
					newTrace->competitionRouting.carriedCompetitions = carriedCompetitions;

					// both traces need to have the same postfix
					// since newTrace doesn't have executedReturnStack but ambiguityTraceToMerge has
					// the amount of returnInsCount need to cut from the postfix
					newTrace->ambiguityBranchInsPostfix = postfix - returnInsCount;

					AddTraceToCollection(ambiguityTraceToMerge, newTrace, &Trace::predecessors);
				}
				else
				{
					// otherwise, no need to create the new trace
					// (? | FORMER) -+-> AMBIGUITY
					//               |
					// TRACE --------+

					AddTraceToCollection(ambiguityTraceToMerge, trace, &Trace::predecessors);
				}
#undef ERROR_MESSAGE_PREFIX
			}
		}
	}
}
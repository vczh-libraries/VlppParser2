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

			bool TraceManager::AreTwoEndingInputTraceEqual(vint32_t state, vint32_t returnStack, vint32_t executedReturnStack, vint32_t acId, Trace* candidate)
			{
				if (state == 19 && candidate->allocatedIndex == 23)
				{
					int a = 0;
				}
				// two traces equal to each other if
				//   1) they are in the same state
				//   2) they have the same ReturnStack before executing thie EndingInput transitions
				//   3) they are attending same competitions
				//   4) the candidate has an ending input
				//   5) they have the same return stack
				// TODO: verify if we can do "acId == candidate->runtimeRouting.attendingCompetitions" or not

				if (state != candidate->state) return false;
				if (acId != candidate->competitionRouting.attendingCompetitions) return false;
				if (candidate->byInput != Executable::EndingInput) return false;

				if (executedReturnStack != candidate->executedReturnStack)
				{
					if (executedReturnStack == -1) return false;
					if (candidate->executedReturnStack == -1) return false;

					auto rs1 = GetReturnStack(executedReturnStack);
					auto rs2 = GetReturnStack(candidate->executedReturnStack);
					if (rs1->returnIndex != rs2->returnIndex) return false;
				}

				// two traces could be merged into one ambiguity resolving trace if
				//   1) they have the same ReturnStack object before executing the EndingInput transition
				//      because two EndingObject are required to be tracable to the BeginObject instruction in the same trace
				//      the ReturnStack object before executing the EndingInput transition
				//      is the ReturnStack object in that common trace
				//      which is something after executing a transition
				//   2) they share the same state after executing EndObject
				auto r1 = returnStack;
				auto r2 = candidate->returnStack;
				if (r1 != r2) return false;

				return true;
			}

/***********************************************************************
GetInstructionPostfix
***********************************************************************/

			vint32_t TraceManager::GetInstructionPostfix(EdgeDesc& oldEdge, EdgeDesc& newEdge)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::GetInstructionPostfix(EdgeDesc&, EdgeDesc&)#"
				// given two equal traces, calculate their common instruction postfix length
				// EndObject is the first instruction of the postfix

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
					auto&& ins = executable.instructions[oldEdge.insBeforeInput.start + insRef];
					if (ins.type == AstInsType::EndObject)
					{
						i1 = insRef;
						break;
					}
				}

				for (vint32_t insRef = 0; insRef < newEdge.insBeforeInput.count; insRef++)
				{
					auto&& ins = executable.instructions[newEdge.insBeforeInput.start + insRef];
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

				vint32_t postfix = oldEdge.insBeforeInput.count - i1;
				for (vint32_t postfixRef = 0; postfixRef < postfix; postfixRef++)
				{
					auto&& ins1 = executable.instructions[oldEdge.insBeforeInput.start + i1 + postfixRef];
					auto&& ins2 = executable.instructions[newEdge.insBeforeInput.start + i2 + postfixRef];
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
				// if ambiguity resolving happens
				// find the instruction postfix
				// the instruction postfix starts from EndObject of a trace
				// and both instruction postfix should equal
				auto& oldEdge = executable.edges[ambiguityTraceToMerge->byEdge];
				vint32_t postfix = GetInstructionPostfix(oldEdge, edgeDesc);

				if (ambiguityTraceToMerge->ambiguityInsPostfix == -1)
				{
					if (oldEdge.insBeforeInput.count == postfix)
					{
						// if EndObject is the first instruction
						// no need to insert another trace
						ambiguityTraceToMerge->ambiguityInsPostfix = postfix;
					}
					else
					{
						// if EndObject is not the first instruction
						// insert another trace before ambiguityTraceMerge
						// and ambiguityTraceMerge should not have had multiple predecessors at this moment
						CHECK_ERROR(ambiguityTraceToMerge->predecessors.first == ambiguityTraceToMerge->predecessors.last, ERROR_MESSAGE_PREFIX L"An ambiguity resolving traces should have been cut.");

						auto formerTrace = AllocateTrace();
						{
							vint32_t formerId = formerTrace->allocatedIndex;
							*formerTrace = *ambiguityTraceToMerge;
							formerTrace->allocatedIndex = formerId;
						}

						// executedReturnStack is from the EndObject instruction
						// which is available in the instruction postfix
						// so formerTrace->executedReturnStack should be -1 and keep the previous return stack
						formerTrace->executedReturnStack = -1;
						if (ambiguityTraceToMerge->predecessors.first != -1)
						{
							auto predecessor = GetTrace(ambiguityTraceToMerge->predecessors.first);
							formerTrace->returnStack = predecessor->returnStack;
						}

						// ambiguity is filled by PrepareTraceRoute, skipped
						// runtimeRouting.holdingCompetition always belong to the second trace
						// runtimeRouting.attendingCompetitions is inherited
						// runtimeRouting.carriedCompetitions is inherited
						formerTrace->competitionRouting = {};
						formerTrace->competitionRouting.attendingCompetitions = ambiguityTraceToMerge->competitionRouting.attendingCompetitions;
						formerTrace->competitionRouting.carriedCompetitions = ambiguityTraceToMerge->competitionRouting.carriedCompetitions;

						// both traces need to have the same ambiguityInsPostfix
						formerTrace->ambiguityInsPostfix = postfix;
						ambiguityTraceToMerge->ambiguityInsPostfix = postfix;

						// connect two traces
						// formerTrace has already copied predecessors, skipped
						// successors of both traces are filled byPrepareTraceRoute, skipped
						// insert formerTrace before ambiguityTraceToMerge because
						// we don't successors of ambiguityTraceToMerge, cannot redirect their predecessors
						ambiguityTraceToMerge->predecessors.first = formerTrace->allocatedIndex;
						ambiguityTraceToMerge->predecessors.last = formerTrace->allocatedIndex;
					}
				}

				if (edgeDesc.insBeforeInput.count == postfix)
				{
					// if EndObject is the first instruction of the new trace
					// then no need to create the new trace
					AddTraceToCollection(ambiguityTraceToMerge, trace, &Trace::predecessors);
				}
				else
				{
					// otherwise, create a new trace with the instruction prefix
					auto newTrace = AllocateTrace();
					AddTraceToCollection(newTrace, trace, &Trace::predecessors);
					newTrace->state = state;
					newTrace->returnStack = returnStack;
					newTrace->byEdge = byEdge;
					newTrace->byInput = input;
					newTrace->currentTokenIndex = currentTokenIndex;

					// executedReturnStack == ambiguityTraceToMerge->executedReturnStack is ensured
					// so no need to assign executedReturnStack to newTrace
					// acid == ambiguityTraceToMerge->runtimeRouting.attendingCompetitions is ensure
					//   this is affected by TODO: in TraceManager::AreTwoEndingInputTraceEqual
					// and ambiguityTraceToMerge is supposed to inherit this value
					newTrace->competitionRouting.attendingCompetitions = attendingCompetitions;
					newTrace->competitionRouting.carriedCompetitions = carriedCompetitions;

					newTrace->ambiguityInsPostfix = postfix;

					AddTraceToCollection(ambiguityTraceToMerge, newTrace, &Trace::predecessors);
				}
#undef ERROR_MESSAGE_PREFIX
			}
		}
	}
}
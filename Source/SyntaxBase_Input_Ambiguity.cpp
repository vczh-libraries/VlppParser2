#include "SyntaxBase.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
			// The following code is useful only when it is proven that
			// AreTwoEndingInputTraceEqual could not just compare two returnStack object
			// The code should be deleted when I have enough confidence
			//
			// bool TraceManager::AreReturnDescEqual(vint32_t ri1, vint32_t ri2)
			// {
			// 	// two returns equal to each other if
			// 	//   1) they shares the same id, so we are comparing a return with itself
			// 	//   2) they have exactly the same data
			// 
			// 	// we cannot just compare ri1 == ri2 because
			// 	// if two alternative branches ends with the same rule and same instructions
			// 	// then two ReturnDesc will have the same data in it
			// 	// TODO: verify (this function could be deleted if AreReturnStackEqual doesn't need it anymore)
			// 
			// 	if (ri1 == ri2) return true;
			// 	auto& rd1 = executable.returns[ri1];
			// 	auto& rd2 = executable.returns[ri2];
			// 	if (rd1.returnState != rd2.returnState) return false;
			// 	if (rd1.insAfterInput.count != rd2.insAfterInput.count) return false;
			// 	for (vint insRef = 0; insRef < rd1.insAfterInput.count; insRef++)
			// 	{
			// 		auto& ins1 = executable.instructions[rd1.insAfterInput.start + insRef];
			// 		auto& ins2 = executable.instructions[rd2.insAfterInput.start + insRef];
			// 		if (ins1 != ins2) return false;
			// 	}
			// 	return true;
			// }
			// 
			// bool TraceManager::AreReturnStackEqual(vint32_t r1, vint32_t r2)
			// {
			// 	return r1 == r2;
			// 
			// 	// two return stacks equal to each other if
			// 	//   1) they shares the same id, so we are comparing a return stack with itself
			// 	//   2) both top returns equal, and both remaining return stack equals
			// 
			// 	// could we just compare r1 == r2 (TODO: verify)
			// 	// Ambiguity resolving requires different branchs should share
			// 	// BeginObject, BeginObjectLeftRecursive and EndObject in exactly the same place (trace + ins)
			// 	// so their return stack should just be the same object
			// 
			// 	// TODO: is it possible that we must (or not just could) compare r1 == r2?
			// 	// try to build this case
			// 
			// 	while (true)
			// 	{
			// 		if (r1 == r2) return true;
			// 		if (r1 == -1 || r2 == -1) return false;
			// 		auto rs1 = GetReturnStack(r1);
			// 		auto rs2 = GetReturnStack(r2);
			// 		if (!AreReturnDescEqual(rs1->returnIndex, rs2->returnIndex))
			// 		{
			// 			return false;
			// 		}
			// 		r1 = rs1->previous;
			// 		r2 = rs2->previous;
			// 	}
			// }

/***********************************************************************
AreTwoTraceEqual
***********************************************************************/

			bool TraceManager::AreTwoEndingInputTraceEqual(vint32_t state, vint32_t returnStack, vint32_t executedReturn, vint32_t acId, Trace* candidate)
			{
				// two traces equal to each other if
				//   1) they are in the same state
				//   2) they have the same executedReturn
				//   3) they are attending same competitions
				//   4) the candidate has an ending input
				//   5) they have the same return stack
				// TODO: verify if we can do "acId == candidate->runtimeRouting.attendingCompetitions" or not
				if (state == candidate->state &&
					executedReturn == candidate->executedReturn &&
					acId == candidate->runtimeRouting.attendingCompetitions &&
					candidate->byInput == Executable::EndingInput)
				{
					// we compare if they have executed the same return edge
					// and than compare if the remaining ReturnStack objects are the same object (not content)
					// two traces could be merged into one ambiguity resolving trace if they share the same state after executing EndObject
					// executedReturn is executed by EndObject
					// returnStack here is the same returnStack when BeginObject for this EndObject was executed
					// since it requires both trace to share the same BeginObject in the same trace
					// than two ReturnStack object should also be the same
					auto r1 = returnStack;
					auto r2 = candidate->returnStack;
					if (r1 == r2)
					{
						return true;
					}
				}
				return false;
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
		}
	}
}
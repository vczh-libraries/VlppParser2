#include "TraceManager.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
/***********************************************************************
TraceManager::ExecuteTrace
***********************************************************************/

			struct TraceManagerSubmitter
			{
				AstIns					submittedInstruction;
				regex::RegexToken*		submittedToken = nullptr;
				bool					submitted = false;
				IAstInsReceiver*		receiver = nullptr;

				void Submit(AstIns& ins, regex::RegexToken& token)
				{
					// ReopenObject and EndObject cancel each other
					// so we don't execute adjacent {EndObject, ReopenObject} or {ReopenObject, EndObject}
					// when an instruction is submitted
					// the previous instruction is executed and the current one is put on wait
					// it gives us a chance to detect the pattern and cancel both instructions

					if (submitted)
					{
						if (
							(submittedInstruction.type == AstInsType::EndObject && ins.type == AstInsType::ReopenObject) ||
							(submittedInstruction.type == AstInsType::ReopenObject && ins.type == AstInsType::EndObject))
						{
							submitted = false;
							submittedToken = nullptr;
							return;
						}
					}

					ExecuteSubmitted();
					submittedInstruction = ins;
					submittedToken = &token;
					submitted = true;
				}

				void ExecuteSubmitted()
				{
					if (submitted)
					{
						receiver->Execute(submittedInstruction, *submittedToken);
						submittedToken = nullptr;
						submitted = false;
					}
				}
			};

			Ptr<ParsingAstBase> TraceManager::ExecuteTrace(Trace* trace, IAstInsReceiver& receiver, collections::List<regex::RegexToken>& tokens)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::ExecuteTrace(Trace*, IAstInsReceiver&, List<RegexToken>&)#"
				CHECK_ERROR(state == TraceManagerState::PreparedTraceRoute, ERROR_MESSAGE_PREFIX L"Wrong timing to call this function.");

				TraceManagerSubmitter submitter;
				submitter.receiver = &receiver;

				// execute from the root trace

				vint32_t startIns = 0;
				while (trace)
				{
					TraceInsLists insLists;
					ReadInstructionList(trace, insLists);

					// if the current trace is an ambiguity resolving trace
					// we check if all predecessors has been visited
					// if yes, we continue
					// if no, we jump to the BeginObject and repeat it again

					if (trace->ambiguity.traceBeginObject != -1 && trace->ambiguityRouting.predecessorCount == -1)
					{
						// we need to know how many predecessors there
						// the number is calculated and cached when an ambiguity resolving trace is visited for the first time
						trace->ambiguityRouting.predecessorCount = 0;
						auto predecessorId = trace->predecessors.first;
						while (predecessorId != -1)
						{
							trace->ambiguityRouting.predecessorCount++;
							predecessorId = GetTrace(predecessorId)->predecessors.siblingNext;
						}
					}

					if (trace->ambiguity.traceBeginObject != -1)
					{
						if (0 <= trace->ambiguity.insEndObject && trace->ambiguity.insEndObject < insLists.c3)
						{
							// execute the EndObject instruction if insEndObject exists
							auto& ins = ReadInstruction(trace->ambiguity.insEndObject, insLists);
							auto& token = tokens[trace->currentTokenIndex];
							submitter.Submit(ins, token);
						}
						else
						{
							// otherwise this must be the trace created by CreateLastMergingTrace
							CHECK_ERROR(insLists.c3 == 0 && trace->successors.first == -1 && trace->successors.last == -1, ERROR_MESSAGE_PREFIX L"Instruction index out of range.");
						}

						// for any ambiguity resolving trace
						// we check all predecessors has been visited
						trace->ambiguityRouting.branchVisited++;
						auto traceBeginObject = GetTrace(trace->ambiguity.traceBeginObject);

						if (trace->ambiguityRouting.branchVisited == trace->ambiguityRouting.predecessorCount)
						{
							// if all predecessors has been visited
							// we reset the number to 0
							// because TraceManager::ExecuteTrace could be called multiple time
							trace->ambiguityRouting.branchVisited = 0;
							{
								// submit a ResolveAmbiguity instruction
								auto& token = tokens[trace->currentTokenIndex];
								AstIns insResolve = { AstInsType::ResolveAmbiguity,trace->ambiguity.ambiguityType,trace->ambiguityRouting.predecessorCount };
								submitter.Submit(insResolve, token);
							}

							// execute all instructions after EndObject
							// these part should not be repeated
							for (vint32_t i = trace->ambiguity.insEndObject + 1; i < insLists.c3; i++)
							{
								auto& ins = ReadInstruction(i, insLists);
								auto& token = tokens[trace->currentTokenIndex];
								submitter.Submit(ins, token);
							}
						}
						else
						{
							// if there are unvisited predecessors
							// we jump to the BeginObject instruction and repeat it again
							startIns = trace->ambiguity.insBeginObject;
							trace = traceBeginObject;
							goto FOUND_NEXT_TRACE;
						}
					}
					else
					{
						// otherwise, just submit instructions
						vint32_t endIns = insLists.c3 - 1;
						if (trace->ambiguityInsPostfix != -1)
						{
							endIns = insLists.c1 - trace->ambiguityInsPostfix - 1;
						}

						for (vint32_t i = startIns; i <= endIns; i++)
						{
							auto& ins = ReadInstruction(i, insLists);
							auto& token = tokens[trace->currentTokenIndex];
							submitter.Submit(ins, token);
						}
					}
					startIns = 0;

					if (trace->successors.first == -1)
					{
						trace = nullptr;
					}
					else if (trace->successors.first == trace->successors.last)
					{
						trace = GetTrace(trace->successors.first);
					}
					else
					{
						// if there are multiple successors
						// whenever this trace is visited
						// we pick a different successor to continue
						auto nextSuccessorId = trace->successors.first;
						Trace* successor = nullptr;
						for (vint i = 0; i <= trace->ambiguityRouting.branchVisited; i++)
						{
							CHECK_ERROR(nextSuccessorId != -1, ERROR_MESSAGE_PREFIX L"branchVisited corrupted.");
							successor = GetTrace(nextSuccessorId);
							nextSuccessorId = successor->successors.siblingNext;
						}

						if (nextSuccessorId == -1)
						{
							trace->ambiguityRouting.branchVisited = 0;
						}
						else
						{
							trace->ambiguityRouting.branchVisited++;
						}

						trace = successor;
					}
				FOUND_NEXT_TRACE:;
				}

				submitter.ExecuteSubmitted();
				return receiver.Finished();
#undef ERROR_MESSAGE_PREFIX
			}
		}
	}
}
#include "SyntaxBase.h"

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
				AstIns*					submittedInstruction = nullptr;
				regex::RegexToken*		submittedToken = nullptr;
				IAstInsReceiver*		receiver = nullptr;

				void Submit(AstIns& ins, regex::RegexToken& token)
				{
					if (submittedInstruction && submittedInstruction->type == AstInsType::EndObject)
					{
						if (ins.type == AstInsType::ReopenObject)
						{
							submittedInstruction = nullptr;
							submittedToken = nullptr;
							return;
						}
					}

					ExecuteSubmitted();
					submittedInstruction = &ins;
					submittedToken = &token;
				}

				void ExecuteSubmitted()
				{
					if (submittedInstruction)
					{
						receiver->Execute(*submittedInstruction, *submittedToken);
						submittedInstruction = nullptr;
						submittedToken = nullptr;
					}
				}
			};

			Ptr<ParsingAstBase> TraceManager::ExecuteTrace(Trace* trace, IAstInsReceiver& receiver, collections::List<regex::RegexToken>& tokens)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::automaton::TraceManager::ExecuteTrace(Trace*, IAstInsReceiver&, List<RegexToken>&)#"
				CHECK_ERROR(state == TraceManagerState::PreparedTraceRoute, ERROR_MESSAGE_PREFIX L"Wrong timing to call this function.");

				TraceManagerSubmitter submitter;
				submitter.receiver = &receiver;

				vint32_t startIns = 0;
				while (trace)
				{
					TraceInsLists insLists;
					ReadInstructionList(trace, insLists);

					vint32_t maxIns = insLists.c3 - 1;
					if (trace->ambiguity.traceBeginObject != -1)
					{
						maxIns = trace->ambiguity.insEndObject;

						if (trace->runtimeRouting.predecessorCount == -1)
						{
							trace->runtimeRouting.predecessorCount = 0;
							auto predecessorId = trace->predecessors.first;
							while (predecessorId != -1)
							{
								trace->runtimeRouting.predecessorCount++;
								predecessorId = GetTrace(predecessorId)->predecessors.siblingNext;
							}
						}
					}

					for (vint32_t i = startIns; i <= maxIns; i++)
					{
						auto& ins = ReadInstruction(i, insLists);
						auto& token = tokens[trace->currentTokenIndex];
						submitter.Submit(ins, token);
					}

					startIns = 0;
					if (trace->ambiguity.traceBeginObject != -1)
					{
						trace->runtimeRouting.branchVisited++;
						auto traceBeginObject = GetTrace(trace->ambiguity.traceBeginObject);

						if (trace->runtimeRouting.branchVisited == trace->runtimeRouting.predecessorCount)
						{
							trace->runtimeRouting.branchVisited = 0;
							{
								TraceInsLists beginInsLists;
								ReadInstructionList(traceBeginObject, beginInsLists);
								auto& beginIns = ReadInstruction(trace->ambiguity.insBeginObject, beginInsLists);
								auto& token = tokens[trace->currentTokenIndex];

								AstIns insResolve = { AstInsType::ResolveAmbiguity,beginIns.param,trace->runtimeRouting.predecessorCount };
								submitter.Submit(insResolve, token);
							}

							for (vint32_t i = maxIns + 1; i < insLists.c3; i++)
							{
								auto& ins = ReadInstruction(i, insLists);
								auto& token = tokens[trace->currentTokenIndex];
								submitter.Submit(ins, token);
							}
						}
						else
						{
							startIns = trace->ambiguity.insBeginObject;
							trace = traceBeginObject;
							goto FOUND_NEXT_TRACE;
						}
					}

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
						auto nextSuccessorId = trace->successors.first;
						Trace* successor = nullptr;
						for (vint i = 0; i <= trace->runtimeRouting.branchVisited; i++)
						{
							CHECK_ERROR(nextSuccessorId != -1, ERROR_MESSAGE_PREFIX L"branchVisited corrupted.");
							successor = GetTrace(nextSuccessorId);
							nextSuccessorId = successor->successors.siblingNext;
						}

						if (nextSuccessorId == -1)
						{
							trace->runtimeRouting.branchVisited = 0;
						}
						else
						{
							trace->runtimeRouting.branchVisited++;
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
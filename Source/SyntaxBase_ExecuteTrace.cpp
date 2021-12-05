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

				baseVisitCount += maxTraceVisitCount;

				TraceManagerSubmitter submitter;
				submitter.receiver = &receiver;

				vint startIns = 0;
				while (trace)
				{
					TraceInsLists insLists;
					ReadInstructionList(trace, insLists);

					vint maxIns = insLists.c3 - 1;
					if (trace->ambiguity.traceBeginObject != -1)
					{
						maxIns = trace->ambiguity.insEndObject;

						CHECK_ERROR(trace->runtimeRouting.expectedVisitCount > 0, ERROR_MESSAGE_PREFIX L"expectedVisitCount for this merging trace is not properly initialized.");
						if (trace->runtimeRouting.visitedCount < baseVisitCount)
						{
							trace->runtimeRouting.visitedCount = baseVisitCount;
						}
						trace->runtimeRouting.visitedCount++;
					}

					for (vint i = startIns; i <= maxIns; i++)
					{
						auto& ins = ReadInstruction(i, insLists);
						auto& token = tokens[trace->currentTokenIndex];
						submitter.Submit(ins, token);
					}

					startIns = 0;
					if (trace->ambiguity.traceBeginObject != -1)
					{
						auto traceBeginObject = GetTrace(trace->ambiguity.traceBeginObject);
						if (trace->runtimeRouting.visitedCount - baseVisitCount == trace->runtimeRouting.expectedVisitCount)
						{
							{
								TraceInsLists beginInsLists;
								ReadInstructionList(traceBeginObject, beginInsLists);
								auto& beginIns = ReadInstruction(trace->ambiguity.insBeginObject, beginInsLists);
								auto& token = tokens[trace->currentTokenIndex];

								AstIns insResolve = { AstInsType::ResolveAmbiguity,beginIns.param,trace->runtimeRouting.expectedVisitCount };
								submitter.Submit(insResolve, token);
							}

							for (vint i = maxIns + 1; i < insLists.c3; i++)
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
						vint successorId = trace->successors.first;
						while (successorId != -1)
						{
							auto successor = GetTrace(successorId);
							CHECK_ERROR(successor->runtimeRouting.expectedVisitCount > 0, ERROR_MESSAGE_PREFIX L"expectedVisitCount for this branch is not properly initialized.");
							if (successor->runtimeRouting.visitedCount < baseVisitCount)
							{
								successor->runtimeRouting.visitedCount = baseVisitCount;
							}

							vint visitedCount = successor->runtimeRouting.visitedCount - baseVisitCount;
							if (visitedCount < successor->runtimeRouting.expectedVisitCount)
							{
								successor->runtimeRouting.visitedCount++;
								trace = successor;
								goto FOUND_NEXT_TRACE;
							}
							else
							{
								successorId = successor->successors.siblingNext;
							}
						}
						CHECK_FAIL(ERROR_MESSAGE_PREFIX L"All branches have been executed, the merging trace should not have jumped back here.");
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
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
				CHECK_ERROR(state == TraceManagerState::PreparedTraceRoute, L"vl::glr::automaton::TraceManager::ExecuteTrace(Trace*, IAstInsReceiver&, List<RegexToken>&)#Wrong timing to call this function.");

				baseVisitCount += maxTraceVisitCount;

				TraceManagerSubmitter submitter;
				submitter.receiver = &receiver;

				while (trace)
				{
					if (trace->byEdge != -1)
					{
						auto& edgeDesc = executable.edges[trace->byEdge];
						for (vint insRef = 0; insRef < edgeDesc.insBeforeInput.count; insRef++)
						{
							vint insIndex = edgeDesc.insBeforeInput.start + insRef;
							auto& ins = executable.instructions[insIndex];
							auto& token = tokens[trace->currentTokenIndex];
							submitter.Submit(ins, token);
						}
						for (vint insRef = 0; insRef < edgeDesc.insAfterInput.count; insRef++)
						{
							vint insIndex = edgeDesc.insAfterInput.start + insRef;
							auto& ins = executable.instructions[insIndex];
							auto& token = tokens[trace->currentTokenIndex];
							submitter.Submit(ins, token);
						}
					}

					if (trace->executedReturn != -1)
					{
						auto& returnDesc = executable.returns[trace->executedReturn];
						for (vint insRef = 0; insRef < returnDesc.insAfterInput.count; insRef++)
						{
							vint insIndex = returnDesc.insAfterInput.start + insRef;
							auto& ins = executable.instructions[insIndex];
							auto& token = tokens[trace->currentTokenIndex];
							submitter.Submit(ins, token);
						}
					}

					if (trace->successors.first == -1)
					{
						trace = nullptr;
					}
					else
					{
						CHECK_ERROR(trace->successors.first == trace->successors.last, L"vl::glr::automaton::TraceManager::ExecuteTrace(Trace*, IAstInsReceiver&, List<RegexToken>&)#Ambiguous trace not implemented.");
						trace = GetTrace(trace->successors.first);
					}
				}

				submitter.ExecuteSubmitted();
				return receiver.Finished();
			}
		}
	}
}
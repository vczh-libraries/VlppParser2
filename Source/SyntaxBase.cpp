#include "SyntaxBase.h"

namespace vl
{
	namespace stream
	{
		namespace internal
		{
			using namespace glr;
			using namespace glr::automaton;

			SERIALIZE_ENUM(AstInsType)

			BEGIN_SERIALIZATION(AstIns)
				SERIALIZE(type)
				SERIALIZE(param)
				SERIALIZE(count)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(InstructionArray)
				SERIALIZE(start)
				SERIALIZE(count)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(ReturnIndexArray)
				SERIALIZE(start)
				SERIALIZE(count)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(EdgeArray)
				SERIALIZE(start)
				SERIALIZE(count)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(ReturnDesc)
				SERIALIZE(consumedRule)
				SERIALIZE(returnState)
				SERIALIZE(insAfterInput)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(EdgeDesc)
				SERIALIZE(fromState)
				SERIALIZE(toState)
				SERIALIZE(insBeforeInput)
				SERIALIZE(insAfterInput)
				SERIALIZE(returnIndices)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(StateDesc)
				SERIALIZE(rule)
				SERIALIZE(endingState)
			END_SERIALIZATION

			BEGIN_SERIALIZATION(Executable)
				SERIALIZE(tokenCount)
				SERIALIZE(ruleCount)
				SERIALIZE(ruleStartStates)
				SERIALIZE(transitions)
				SERIALIZE(instructions)
				SERIALIZE(returnIndices)
				SERIALIZE(returns)
				SERIALIZE(edges)
				SERIALIZE(states)
			END_SERIALIZATION
		}
	}

	namespace glr
	{
		namespace automaton
		{
			using namespace collections;
			using namespace stream;

/***********************************************************************
Executable
***********************************************************************/

			Executable::Executable(stream::IStream& inputStream)
			{
				internal::ContextFreeReader reader(inputStream);
				reader << *this;
			}

			void Executable::Serialize(stream::IStream& outputStream)
			{
				internal::ContextFreeWriter writer(outputStream);
				writer << *this;
			}

/***********************************************************************
TraceManager
***********************************************************************/

			void TraceManager::BeginSwap()
			{
				concurrentCount = 0;
			}

			void TraceManager::AddTrace(Trace* trace)
			{
				if (concurrentCount < backupTraces->Count())
				{
					backupTraces->Set(concurrentCount, trace);
				}
				else
				{
					backupTraces->Add(trace);
				}
				concurrentCount++;
			}

			void TraceManager::EndSwap()
			{
				auto t = concurrentTraces;
				concurrentTraces = backupTraces;
				backupTraces = t;
			}

			TraceManager::TraceManager(Executable& _executable)
				:executable(_executable)
			{
			}

			ReturnStack* TraceManager::GetReturnStack(vint index)
			{
				return returnStacks.Get(index);
			}

			ReturnStack* TraceManager::AllocateReturnStack()
			{
				auto returnStack = returnStacks.Get(returnStacks.Allocate());
				returnStack->previous = -1;
				returnStack->returnIndex = -1;
				return returnStack;
			}

			Trace* TraceManager::GetTrace(vint index)
			{
				return traces.Get(index);
			}

			Trace* TraceManager::AllocateTrace()
			{
				auto trace = traces.Get(traces.Allocate());
				trace->previous = -1;
				trace->selectedNext = -1;
				trace->state = -1;
				trace->returnStack = -1;
				trace->executedReturn = -1;
				trace->byEdge = -1;
				trace->byInput = -1;
				trace->previousTokenIndex = -1;
				trace->currentTokenIndex = -1;
				trace->traceBeginObject = -1;
				trace->traceAfterBranch = -1;
				return trace;
			}

			void TraceManager::Initialize(vint startState)
			{
				returnStacks.Clear();
				traces.Clear();
				traces1.Clear();
				traces2.Clear();
				concurrentTraces = &traces1;
				backupTraces = &traces2;

				auto trace = AllocateTrace();
				trace->state = startState;
				concurrentCount = 1;
				concurrentTraces->Add(trace);
			}

/***********************************************************************
TraceManager::Input
***********************************************************************/

			Trace* TraceManager::WalkAlongSingleEdge(
				vint previousTokenIndex,
				vint currentTokenIndex,
				vint input,
				Trace* trace,
				vint byEdge,
				EdgeDesc& edgeDesc
			)
			{
				auto newTrace = AllocateTrace();
				AddTrace(newTrace);

				newTrace->previous = trace->allocatedIndex;
				newTrace->state = edgeDesc.toState;
				newTrace->returnStack = trace->returnStack;
				newTrace->byEdge = byEdge;
				newTrace->byInput = input;
				newTrace->previousTokenIndex = previousTokenIndex;
				newTrace->currentTokenIndex = currentTokenIndex;

				for (vint returnRef = 0; returnRef < edgeDesc.returnIndices.count; returnRef++)
				{
					vint returnIndex = executable.returnIndices[edgeDesc.returnIndices.start + returnRef];
					auto returnStack = AllocateReturnStack();
					returnStack->previous = newTrace->returnStack;
					returnStack->returnIndex = returnIndex;
					newTrace->returnStack = returnStack->allocatedIndex;
				}

				return newTrace;
			}

			void TraceManager::WalkAlongTokenEdges(
				vint previousTokenIndex,
				vint currentTokenIndex,
				vint input,
				Trace* trace,
				EdgeArray& edgeArray
			)
			{
				for (vint edgeRef = 0; edgeRef < edgeArray.count; edgeRef++)
				{
					vint byEdge = edgeArray.start + edgeRef;
					auto& edgeDesc = executable.edges[edgeArray.start + edgeRef];
					auto newTrace = WalkAlongSingleEdge(previousTokenIndex, currentTokenIndex, input, trace, byEdge, edgeDesc);
					WalkAlongEpsilonEdges(previousTokenIndex, currentTokenIndex, newTrace);
				}
			}

			void TraceManager::WalkAlongEpsilonEdges(
				vint previousTokenIndex,
				vint currentTokenIndex,
				Trace* trace
			)
			{
				{
					vint transactionIndex = trace->state * (Executable::TokenBegin + executable.tokenCount) + Executable::LeftrecInput;
					auto&& edgeArray = executable.transitions[transactionIndex];
					WalkAlongLeftrecEdges(previousTokenIndex, currentTokenIndex, trace, edgeArray);
				}
				{
					vint transactionIndex = trace->state * (Executable::TokenBegin + executable.tokenCount) + Executable::EndingInput;
					auto&& edgeArray = executable.transitions[transactionIndex];
					WalkAlongEndingEdges(previousTokenIndex, currentTokenIndex, trace, edgeArray);
				}
			}

			void TraceManager::WalkAlongLeftrecEdges(
				vint previousTokenIndex,
				vint currentTokenIndex,
				Trace* trace,
				EdgeArray& edgeArray
			)
			{
				for (vint edgeRef = 0; edgeRef < edgeArray.count; edgeRef++)
				{
					vint byEdge = edgeArray.start + edgeRef;
					auto& edgeDesc = executable.edges[edgeArray.start + edgeRef];
					WalkAlongSingleEdge(previousTokenIndex, currentTokenIndex, Executable::LeftrecInput, trace, byEdge, edgeDesc);
				}
			}

			void TraceManager::WalkAlongEndingEdges(
				vint previousTokenIndex,
				vint currentTokenIndex,
				Trace* trace,
				EdgeArray& edgeArray
			)
			{
				for (vint edgeRef = 0; edgeRef < edgeArray.count; edgeRef++)
				{
					vint byEdge = edgeArray.start + edgeRef;
					auto& edgeDesc = executable.edges[edgeArray.start + edgeRef];
					auto newTrace = WalkAlongSingleEdge(previousTokenIndex, currentTokenIndex, Executable::EndingInput, trace, byEdge, edgeDesc);

					if (newTrace->returnStack != -1)
					{
						auto returnStack = GetReturnStack(newTrace->returnStack);
						newTrace->returnStack = returnStack->previous;
						newTrace->executedReturn = returnStack->returnIndex;

						auto& returnDesc = executable.returns[newTrace->executedReturn];
						newTrace->state = returnDesc.returnState;
						WalkAlongEpsilonEdges(previousTokenIndex, currentTokenIndex, newTrace);
					}
				}
			}

			void TraceManager::Input(vint currentTokenIndex, vint token)
			{
				vint traceCount = concurrentCount;
				vint previousTokenIndex = currentTokenIndex - 1;
				vint input = Executable::TokenBegin + token;
				BeginSwap();
				for (vint traceIndex = 0; traceIndex < traceCount; traceIndex++)
				{
					auto trace = concurrentTraces->Get(traceIndex);
					vint transactionIndex = trace->state * (Executable::TokenBegin + executable.tokenCount) + input;
					auto&& edgeArray = executable.transitions[transactionIndex];
					WalkAlongTokenEdges(previousTokenIndex, currentTokenIndex, input, trace, edgeArray);
				}
				EndSwap();
			}

			void TraceManager::EndOfInput()
			{
				vint traceCount = concurrentCount;
				BeginSwap();
				for (vint traceIndex = 0; traceIndex < traceCount; traceIndex++)
				{
					auto trace = concurrentTraces->Get(traceIndex);
					auto& stateDesc = executable.states[trace->state];
					if (trace->returnStack == -1 && stateDesc.endingState)
					{
						AddTrace(trace);
					}
				}
				EndSwap();
			}

/***********************************************************************
TraceManager::PrepareTraceRoute
***********************************************************************/

			Trace* TraceManager::PrepareTraceRoute()
			{
				CHECK_ERROR(concurrentCount == 1, L"vl::glr::automaton::TraceManager::PrepareTraceRoute()#Too many finite traces.");
				auto trace = concurrentTraces->Get(0);
				while (trace->previous != -1)
				{
					auto previous = GetTrace(trace->previous);
					CHECK_ERROR(previous->selectedNext == -1, L"vl::glr::automaton::TraceManager::PrepareTraceRoute()#Trace::selectedNext has been assigned.");
					previous->selectedNext = trace->allocatedIndex;
					trace = previous;
				}
				return trace;
			}

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

					if (trace->selectedNext == -1)
					{
						trace = nullptr;
					}
					else
					{
						trace = GetTrace(trace->selectedNext);
					}
				}

				submitter.ExecuteSubmitted();
				return receiver.Finished();
			}
		}
	}
}
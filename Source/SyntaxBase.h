/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_SYNTAXBASE
#define VCZH_PARSER2_SYNTAXBASE

#include "AstBase.h"

namespace vl
{
	namespace glr
	{
/***********************************************************************
Automaton
***********************************************************************/

		namespace automaton
		{
			struct InstructionArray
			{
				vint								start = -1;
				vint								count = 0;
			};

			struct ReturnIndexArray
			{
				vint								start = -1;
				vint								count = -1;
			};

			struct EdgeArray
			{
				vint								start = -1;
				vint								count = 0;
			};

			enum class EdgePriority
			{
				NoCompetition,
				HighPriority,
				LowPriority,
			};

			struct ReturnDesc
			{
				vint								consumedRule = -1;
				vint								returnState = -1;
				EdgePriority						priority = EdgePriority::NoCompetition;
				InstructionArray					insAfterInput;
			};

			struct EdgeDesc
			{
				vint								fromState = -1;
				vint								toState = -1;
				EdgePriority						priority = EdgePriority::NoCompetition;
				InstructionArray					insBeforeInput;
				InstructionArray					insAfterInput;
				ReturnIndexArray					returnIndices;
			};

			struct StateDesc
			{
				vint								rule = -1;
				bool								endingState = false;
			};

			struct Executable
			{
				static constexpr vint				EndOfInputInput = -1;
				static constexpr vint				EndingInput = 0;
				static constexpr vint				LeftrecInput = 1;
				static constexpr vint				TokenBegin = 2;

				vint								tokenCount = 0;
				vint								ruleCount = 0;
				collections::Array<vint>			ruleStartStates;		// ruleStartStates[rule] = the start state of this rule.
				collections::Array<EdgeArray>		transitions;			// transitions[state * (TokenBegin + tokenCount) + input] = edges from state with specified input.
				collections::Array<AstIns>			instructions;			// referenced by InstructionArray
				collections::Array<vint>			returnIndices;			// referenced by ReturnIndexArray
				collections::Array<ReturnDesc>		returns;				// referenced by Executable::returnIndices
				collections::Array<EdgeDesc>		edges;					// referenced by EdgeArray
				collections::Array<StateDesc>		states;					// refereced by returnState/fromState/toState

				Executable() = default;
				Executable(stream::IStream& inputStream);

				void								Serialize(stream::IStream& outputStream);
			};

			struct Metadata
			{
				collections::Array<WString>			ruleNames;
				collections::Array<WString>			stateLabels;
			};

/***********************************************************************
Execution
***********************************************************************/

			template<typename T>
			class AllocateOnly : public Object
			{
			protected:
				vint											blockSize;
				vint											remains;
				collections::List<Ptr<collections::Array<T>>>	buffers;

			public:
				AllocateOnly(vint _blockSize = 65536)
					: blockSize(_blockSize)
					, remains(0)
				{
				}

				T* Get(vint index)
				{
					vint row = index / blockSize;
					vint column = index % blockSize;
					CHECK_ERROR(0 <= row && row < buffers.Count(), L"vl::glr::automaton::AllocateOnly<T>::Get(vint)#Index out of range.");
					if (row == buffers.Count() - 1)
					{
						CHECK_ERROR(0 <= column && column < (blockSize - remains), L"vl::glr::automaton::AllocateOnly<T>::Get(vint)#Index out of range.");
					}
					else
					{
						CHECK_ERROR(0 <= column && column < blockSize, L"vl::glr::automaton::AllocateOnly<T>::Get(vint)#Index out of range.");
					}
					return &buffers[row]->operator[](column);
				}

				vint Allocate()
				{
					if (remains == 0)
					{
						buffers.Add(new collections::Array<T>(blockSize));
						remains = blockSize;
					}
					vint index = blockSize * (buffers.Count() - 1) + (blockSize - remains);
					buffers[buffers.Count() - 1]->operator[](blockSize - remains).allocatedIndex = index;
					remains--;
					return index;
				}

				void Clear()
				{
					remains = 0;
					buffers.Clear();
				}
			};

			struct ReturnStack
			{
				vint					allocatedIndex = -1;		// id of this ReturnStack
				vint					previous = -1;				// id of the previous ReturnStack
				vint					returnIndex = -1;			// index of ReturnDesc
			};

			struct TraceCollection
			{
				vint					first = -1;					// first trace in the collection
				vint					last = -1;					// last trace in the collection
				vint					siblingPrev = -1;			// previous trace in the collection of the owned trace
				vint					siblingNext = -1;			// next trace in the collection of the owned trace
			};

			struct TraceAmbiguity
			{
				vint					insEndObject = -1;			// the index of the first EndObject instruction
																	// in {byEdge.insBeforeInput, byEdge.insAfterInput, executedReturn.insAfterInput} combined

				vint					traceBeginObject = -1;		// id of the trace containing BeginObject or BeginObjectLeftRecursive
																	// that ends by the above EndObject

				vint					insBeginObject = -1;		// the index of the BeginObject or BeginObjectLeftRecursive instruction
																	// from traceBeginObject
																	// in {byEdge.insBeforeInput, byEdge.insAfterInput, executedReturn.insAfterInput} combined
			};

			struct RuntimeRouting
			{
				vint					predecessorCount = -1;		// the number of predecessors

				vint					branchVisited = 0;			// the number of visited branches in the current loop.
																	// if these branches are contained in a larger ambiguity resolving loop, all branches could be visited multiple times
																	// (filled by ExecuteTrace)
			};

			struct Trace
			{
				vint					allocatedIndex = -1;		// id of this Trace
				TraceCollection			predecessors;				// id of the predecessor Trace
				TraceCollection			successors;					// successors (filled by PrepareTraceRoute)

				vint					state = -1;					// id of the current StateDesc
				vint					returnStack = -1;			// id of the current ReturnStack
				vint					executedReturn = -1;		// id of the executed ReturnDesc
				vint					byEdge = -1;				// id of the last EdgeDesc that make this trace
				vint					byInput = -1;				// the last input that make this trace
				vint					previousTokenIndex = -1;	// the index of the token before byInput
				vint					currentTokenIndex = -1;		// the index of the token that is byInput

				TraceAmbiguity			ambiguity;					// where to end resolving ambiguity in instructions from this trace
																	// this member is useful when it has multiple predecessors
																	// (filled by PrepareTraceRoute)

				RuntimeRouting			runtimeRouting;				// a data structure guiding instruction execution when a trace need to be executed multiple times
																	// this member is useful when it has multiple predecessors or successors
			};

			enum class TraceManagerState
			{
				Uninitialized,
				WaitingForInput,
				Finished,
				PreparedTraceRoute,
			};

			struct TraceInsLists
			{
				InstructionArray					edgeInsBeforeInput;
				InstructionArray					edgeInsAfterInput;
				InstructionArray					returnInsAfterInput;
				vint								c1;
				vint								c2;
				vint								c3;
			};

			class TraceManager : public Object
			{
			protected:
				TraceManagerState					state = TraceManagerState::Uninitialized;
				Executable&							executable;
				AllocateOnly<ReturnStack>			returnStacks;
				AllocateOnly<Trace>					traces;

				collections::List<Trace*>			traces1;
				collections::List<Trace*>			traces2;

				Trace*								rootTrace = nullptr;

				void								BeginSwap();
				void								AddTrace(Trace* trace);
				void								EndSwap();
				void								AddTraceToCollection(Trace* owner, Trace* element, TraceCollection(Trace::* collection));

				Trace*								WalkAlongSingleEdge(vint previousTokenIndex, vint currentTokenIndex, vint input, Trace* trace, vint byEdge, EdgeDesc& edgeDesc);
				void								WalkAlongTokenEdges(vint previousTokenIndex, vint currentTokenIndex, vint input, Trace* trace, EdgeArray& edgeArray);
				void								WalkAlongEpsilonEdges(vint previousTokenIndex, vint currentTokenIndex, Trace* trace);
				void								WalkAlongLeftrecEdges(vint previousTokenIndex, vint currentTokenIndex, Trace* trace, EdgeArray& edgeArray);
				void								WalkAlongEndingEdges(vint previousTokenIndex, vint currentTokenIndex, Trace* trace, EdgeArray& edgeArray);

				void								ReadInstructionList(Trace* trace, TraceInsLists& insLists);
				AstIns&								ReadInstruction(vint instruction, TraceInsLists& insLists);
				bool								RunInstruction(vint instruction, TraceInsLists& insLists, vint& objectCount);
				void								FindBalancedBeginObject(Trace*& trace, vint& instruction, vint& objectCount);
				void								FillAmbiguityInfoForMergingTrace(Trace* trace);
				void								FillAmbiguityInfoForPredecessorTraces(Trace* trace);
			public:
				TraceManager(Executable& _executable);

				vint								concurrentCount = 0;
				collections::List<Trace*>*			concurrentTraces = nullptr;
				collections::List<Trace*>*			backupTraces = nullptr;

				ReturnStack*						GetReturnStack(vint index);
				ReturnStack*						AllocateReturnStack();
				Trace*								GetTrace(vint index);
				Trace*								AllocateTrace();

				void								Initialize(vint startState);
				void								Input(vint currentTokenIndex, vint token);
				void								EndOfInput();
				Trace*								PrepareTraceRoute();
				Ptr<ParsingAstBase>					ExecuteTrace(Trace* trace, IAstInsReceiver& receiver, collections::List<regex::RegexToken>& tokens);
			};
		}

/***********************************************************************
Parser
***********************************************************************/

		template<
			typename TTokens,
			typename TStates,
			typename TReceiver,
			template<TStates> class TStateTypes
		>
		class ParserBase : public Object
		{
			static_assert(std::is_enum_v<TTokens>);
			static_assert(std::is_enum_v<TStates>);
			static_assert(std::is_convertible_v<TReceiver*, IAstInsReceiver*>);

			using Deleter = bool(*)(vint);
			using TokenList = collections::List<regex::RegexToken>;
			using Callback = void(TokenList&, automaton::Executable&, automaton::TraceManager&, automaton::Trace*);
		protected:
			Deleter									deleter;
			Ptr<regex::RegexLexer>					lexer;
			Ptr<automaton::Executable>				executable;

		public:
			Event<Callback>							OnEndOfInput;

			ParserBase(
				Deleter _deleter,
				void(*_lexerData)(stream::IStream&),
				void(*_parserData)(stream::IStream&)
			) : deleter(_deleter)
			{
				{
					stream::MemoryStream data;
					_lexerData(data);
					data.SeekFromBegin(0);
					lexer = new regex::RegexLexer(data);
				}
				{
					stream::MemoryStream data;
					_parserData(data);
					data.SeekFromBegin(0);
					executable = new automaton::Executable(data);
				}
			}

			template<TStates State>
			auto Parse(const WString& input, vint codeIndex = -1)
			{
				#define ERROR_MESSAGE_PREFIX L"vl::glr::ParserBase<...>::ParseWithReceiver<TReceiver2>(const WString&, TState, TReceiver2&, vint)#"

				TokenList tokens;
				lexer->Parse(input, {}, codeIndex).ReadToEnd(tokens, deleter);

				automaton::TraceManager tm(*executable.Obj());
				tm.Initialize((vint)State);
				for (vint i = 0; i < tokens.Count(); i++)
				{
					auto&& token = tokens[i];
					tm.Input(i, token.token);
					// TODO: log errors instead of crashing (failed to parse)
					CHECK_ERROR(tm.concurrentCount > 0, ERROR_MESSAGE_PREFIX L"Error happens during parsing.");
				}

				tm.EndOfInput();
				auto rootTrace = tm.PrepareTraceRoute();
				OnEndOfInput(tokens, *executable.Obj(), tm, rootTrace);
				// TODO: log errors instead of crashing (input not complete, unresolvable ambiguity)
				CHECK_ERROR(tm.concurrentCount == 1, ERROR_MESSAGE_PREFIX L"Ambiguity not fully resolved.");
				CHECK_ERROR(executable->states[tm.concurrentTraces->Get(0)->state].endingState, ERROR_MESSAGE_PREFIX L"Input is incomplete.");

				TReceiver receiver;
				auto ast = tm.ExecuteTrace(rootTrace, receiver, tokens);
				auto typedAst = ast.Cast<typename TStateTypes<State>::Type>();
				CHECK_ERROR(typedAst, ERROR_MESSAGE_PREFIX L"#Unexpected type of the created AST.");
				return typedAst;

				#undef ERROR_MESSAGE_PREFIX
			}
		};
	}
}

#endif
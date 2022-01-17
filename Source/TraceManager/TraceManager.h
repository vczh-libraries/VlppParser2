/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_TRACEMANAGER_TRACEMANAGER
#define VCZH_PARSER2_TRACEMANAGER_TRACEMANAGER

#include "../Executable.h"

namespace vl
{
	namespace glr
	{
		namespace automaton
		{
/***********************************************************************
AllocateOnly<T>
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

				T* Get(vint32_t index)
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

				vint32_t Allocate()
				{
					if (remains == 0)
					{
						buffers.Add(new collections::Array<T>(blockSize));
						remains = blockSize;
					}
					vint index = blockSize * (buffers.Count() - 1) + (blockSize - remains);
					buffers[buffers.Count() - 1]->operator[](blockSize - remains).allocatedIndex = (vint32_t)index;
					remains--;
					return (vint32_t)index;
				}

				void Clear()
				{
					remains = 0;
					buffers.Clear();
				}
			};

/***********************************************************************
TraceManager (Data Structures)
***********************************************************************/

			struct ReturnStackSuccessors
			{
				vint32_t				createdTokenIndex = -1;		// index of the token when this ReturnStack is created
				vint32_t				successorTokenIndex = -1;	// index of the token when this ReturnStack has its first successor
																	// the following members records all successors
																	// that is created at the token index

				vint32_t				first = -1;					// first successor
				vint32_t				last = -1;					// last successor
				vint32_t				prev = -1;					// previous successor of ReturnStack::previous
				vint32_t				next = -1;					// next successor of ReturnStack::previous
			};

			struct ReturnStack
			{
				vint32_t				allocatedIndex = -1;		// id of this ReturnStack
				vint32_t				previous = -1;				// id of the previous ReturnStack
				vint32_t				returnIndex = -1;			// index of ReturnDesc
				ReturnStackSuccessors	successors;
			};

			struct TraceCollection
			{
				vint32_t				first = -1;					// first trace in the collection
				vint32_t				last = -1;					// last trace in the collection
				vint32_t				siblingPrev = -1;			// previous trace in the collection of the owned trace
				vint32_t				siblingNext = -1;			// next trace in the collection of the owned trace
			};

			struct TraceAmbiguity
			{
				vint32_t				insEndObject = -1;			// the index of the first EndObject instruction
																	// in {byEdge.insBeforeInput, byEdge.insAfterInput, executedReturnStack.returnIndex.insAfterInput} combined
																	// when this member is valid, the trace should satisfies:
																	// trace.ambiguity.insEndObject == trace.byEdge.insBeforeInput.count - trace.ambiguityInsPostfix

				vint32_t				traceBeginObject = -1;		// id of the trace containing BeginObject or DelayFieldAssignment
																	// that ends by the above EndObject

				vint32_t				insBeginObject = -1;		// the index of the BeginObject instruction
																	// from traceBeginObject
																	// in {byEdge.insBeforeInput, byEdge.insAfterInput, executedReturnStack.returnIndex.insAfterInput} combined
																	// if insBeginObject is larger than the number of instructions in traceBeginObject
																	// then the branches begin from the (insBeginObject - instruction-count(traceBeginObject))-th instruction (starting from 0) in all successors

				vint32_t				ambiguityType = -1;			// when the BeginObject creates an object that later be consumed by BeginObjectLeftRecursive
																	// than the correct type is the type in BeginObjectLeftRecursive
			};

			enum class CompetitionStatus
			{
				Holding,
				HighPriorityWin,
				LowPriorityWin,
			};

			struct Competition
			{
				vint32_t				allocatedIndex = -1;
				vint32_t				nextActiveCompetition = -1;				// next active Competition
				vint32_t				nextHoldCompetition = -1;				// next Competition hold by this trace

				CompetitionStatus		status = CompetitionStatus::Holding;	// if predecessors from this trace have different priority, the competition begins
																				// when the competition is over, it will be changed to HighPriorityWin or LowPriorityWin
																				// if all candidates fail, it could be Holding forever

				vint32_t				currentTokenIndex = -1;		// currentTokenIndex from the trace that creates this competition
				vint32_t				ruleId = -1;				// the rule id of state, when an edge starts this competition
				vint32_t				clauseId = -1;				// the clause id of the state, when an edge starts this competition
																	// an state must be picked up and ensure that, the syntax creating the priority and the state belong to the same clause

				vint32_t				highCounter = 0;			// temporary counter for all existing high bets
																	// in the current step of input
				vint32_t				lowCounter = 0;				// temporary counter for all existing low bets
																	// in the current step of input
			};

			struct AttendingCompetitions
			{
				vint32_t				allocatedIndex = -1;		// id of this AttendingCompetitions
				vint32_t				nextActiveAC = -1;			// the next AttendingCompetitions for RuntimeRouting::attendingCompetitions
				vint32_t				nextCarriedAC = -1;			// the next AttendingCompetitions for RuntimeRouting::carriedCompetitions
				vint32_t				competition = -1;			// the id of the Competition
				bool					forHighPriority = false;	// bet of this competition

				vint32_t				returnStack = -1;			// the ReturnStack object for the competition
																	// if the competition is attended by a ReturnDesc
																	// then the ReturnStack object is the one before a ReturnDesc transition happens

				bool					closed = false;				// true if the competition has been closed
																	// this flag is not always updated for discarded AttendingCompetitions objects
			};

			struct CompetitionRouting
			{
				vint32_t				holdingCompetitions = -1;	// the id of the active Competition

				vint32_t				attendingCompetitions = -1;	// a linked list containing all AttendingCompetitions that this trace is attending
																	// predecessors could share and modify the same linked list
																	// if a competition is over, node could be removed from the linked list
																	// one competition only creates two AttendingCompetitions, traces with the same bet share the object

				vint32_t				carriedCompetitions = -1;	// all attended competitions regardless of the status of the competition
			};

			struct AmbiguityRouting
			{
				vint32_t				predecessorCount = -1;		// the number of predecessors
																	// (filled by ExecuteTrace)

				vint32_t				branchVisited = 0;			// the number of visited branches in the current loop.
																	// if these branches are contained in a larger ambiguity resolving loop, all branches could be visited multiple times
																	// (filled by ExecuteTrace)
			};

			struct Trace
			{
				vint32_t				allocatedIndex = -1;		// id of this Trace
				TraceCollection			predecessors;				// id of the predecessor Trace
				TraceCollection			successors;					// successors (filled by PrepareTraceRoute)

				vint32_t				state = -1;					// id of the current StateDesc
				vint32_t				returnStack = -1;			// id of the current ReturnStack
				vint32_t				executedReturnStack = -1;	// id of the executed ReturnStack that contains the ReturnDesc being executed
				vint32_t				byEdge = -1;				// id of the last EdgeDesc that make this trace
				vint32_t				byInput = -1;				// the last input that make this trace
				vint32_t				currentTokenIndex = -1;		// the index of the token that is byInput

				TraceAmbiguity			ambiguity;					// where to end resolving ambiguity in instructions from this trace
																	// this member is useful when it has multiple predecessors
																	// (filled by PrepareTraceRoute)

				vint32_t				ambiguityBranchInsPostfix = -1;		// this member is useful when it is not -1 and the trace has multiple successors
																			// specifying the length of the postfix
																			// of {byEdge.insBeforeInput, byEdge.insAfterInput, executedReturnStack.returnIndex.insAfterInput} combined
																			// only execute the specified prefix of instructions
																			// usually EndObject is the last instruction in the prefix

				vint32_t				ambiguityMergeInsPostfix = -1;		// this member is useful when it is not -1 and the trace has multiple predecessors
																			// specifying the length of the postfix
																			// of {byEdge.insBeforeInput, byEdge.insAfterInput, executedReturnStack.returnIndex.insAfterInput} combined
																			// only execute the specified postfix of instructions
																			// usually EndObject is the last instruction in the prefix

				CompetitionRouting		competitionRouting;			// a data structure carrying priority and competition information

				AmbiguityRouting		ambiguityRouting;			// a data structure guiding instruction execution when a trace need to be executed multiple times
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
				vint32_t							c1;
				vint32_t							c2;
				vint32_t							c3;
			};

/***********************************************************************
TraceManager
***********************************************************************/

			class TraceManager : public Object
			{
			public:
				class ITypeCallback : public virtual Interface
				{
				public:
					virtual vint32_t				FindCommonBaseClass(vint32_t class1, vint32_t class2) const= 0;
				};

			protected:
				Executable&							executable;
				const ITypeCallback*				typeCallback = nullptr;

				TraceManagerState					state = TraceManagerState::Uninitialized;
				AllocateOnly<ReturnStack>			returnStacks;
				AllocateOnly<Trace>					traces;
				AllocateOnly<Competition>			competitions;
				AllocateOnly<AttendingCompetitions>	attendingCompetitions;

				collections::List<Trace*>			traces1;
				collections::List<Trace*>			traces2;

				Trace*								initialTrace = nullptr;
				vint32_t							activeCompetitions = -1;
				ReturnStackSuccessors				initialReturnStackSuccessors;

				void								BeginSwap();
				void								AddTrace(Trace* trace);
				void								EndSwap();
				void								AddTraceToCollection(Trace* owner, Trace* element, TraceCollection(Trace::* collection));

				// Ambiguity
				bool								AreTwoEndingInputTraceEqual(vint32_t state, vint32_t returnStack, vint32_t executedReturnStack, vint32_t acId, Trace* candidate);
				vint32_t							GetInstructionPostfix(EdgeDesc& oldEdge, EdgeDesc& newEdge);
				void								MergeTwoEndingInputTrace(
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
														vint32_t executedReturnStack);

				// Competition
				void								AttendCompetition(Trace* trace, vint32_t& newAttendingCompetitions, vint32_t& newCarriedCompetitions, vint32_t returnStack, vint32_t ruleId, vint32_t clauseId, bool forHighPriority);
				ReturnStack*						PushReturnStack(vint32_t base, vint32_t returnIndex, vint32_t currentTokenIndex);
				void								AttendCompetitionIfNecessary(Trace* trace, vint32_t currentTokenIndex, EdgeDesc& edgeDesc, vint32_t& newAttendingCompetitions, vint32_t& newCarriedCompetitions, vint32_t& newReturnStack);
				void								CheckAttendingCompetitionsOnEndingEdge(Trace* trace, EdgeDesc& edgeDesc, vint32_t acId, vint32_t returnStack);
				void								CheckBackupTracesBeforeSwapping(vint32_t currentTokenIndex);

				// Walk
				bool								IsQualifiedTokenForCondition(regex::RegexToken* token, StringLiteral condition);
				bool								IsQualifiedTokenForEdgeArray(regex::RegexToken* token, EdgeArray& edgeArray);
				Trace*								WalkAlongSingleEdge(vint32_t currentTokenIndex, vint32_t input, Trace* trace, vint32_t byEdge, EdgeDesc& edgeDesc);
				void								WalkAlongLeftrecEdges(vint32_t currentTokenIndex, regex::RegexToken* lookAhead, Trace* trace, EdgeArray& edgeArray);
				void								WalkAlongEpsilonEdges(vint32_t currentTokenIndex, regex::RegexToken* lookAhead, Trace* trace);
				void								WalkAlongTokenEdges(vint32_t currentTokenIndex, vint32_t input, regex::RegexToken* token, regex::RegexToken* lookAhead, Trace* trace, EdgeArray& edgeArray);

				// PrepareTraceRoute
				struct SharedBeginObject
				{
					Trace*							traceBeginObject = nullptr;
					vint32_t						insBeginObject = -1;
					vint32_t						type = -1;
				};

				void								ReadInstructionList(Trace* trace, TraceInsLists& insLists);
				AstIns&								ReadInstruction(vint32_t instruction, TraceInsLists& insLists);
				bool								RunInstruction(vint32_t instruction, TraceInsLists& insLists, vint32_t& objectCount, vint32_t& reopenCount);
				void								AdjustToRealTrace(SharedBeginObject& shared);

				void								FindBalancedBoOrBolr(SharedBeginObject& balanced, vint32_t& objectCount, vint32_t& reopenCount);
				void								FindBalancedBoOrDfa(Trace* trace, vint32_t objectCount, SharedBeginObject& branch);

				void								MergeAmbiguityType(vint32_t& ambiguityType, vint32_t branchType);
				SharedBeginObject					MergeSharedBeginObjectsSingleRoot(Trace* trace, collections::Dictionary<Trace*, SharedBeginObject>& predecessorToBranches);
				SharedBeginObject					MergeSharedBeginObjectsMultipleRoot(Trace* trace, collections::Dictionary<Trace*, SharedBeginObject>& predecessorToBranches);
				SharedBeginObject					MergeSharedBeginObjectsPartialMultipleRoot(Trace* trace, vint32_t ambiguityType, collections::Group<Trace*, Trace*>& beginToPredecessors, collections::Dictionary<Trace*, SharedBeginObject>& predecessorToBranches);

				SharedBeginObject					FillAmbiguityInfoForMergingTrace(Trace* trace);
				void								FillAmbiguityInfoForPredecessorTraces(Trace* trace);
				void								CreateLastMergingTrace(Trace* rootTraceCandidate);
			public:
				TraceManager(Executable& _executable, const ITypeCallback* _typeCallback = nullptr);

				vint32_t							concurrentCount = 0;
				collections::List<Trace*>*			concurrentTraces = nullptr;
				collections::List<Trace*>*			backupTraces = nullptr;

				ReturnStack*						GetReturnStack(vint32_t index);
				ReturnStack*						AllocateReturnStack();
				Trace*								GetTrace(vint32_t index);
				Trace*								AllocateTrace();
				Competition*						GetCompetition(vint32_t index);
				Competition*						AllocateCompetition();
				AttendingCompetitions*				GetAttendingCompetitions(vint32_t index);
				AttendingCompetitions*				AllocateAttendingCompetitions();

				void								Initialize(vint32_t startState);
				void								Input(vint32_t currentTokenIndex, regex::RegexToken* token, regex::RegexToken* lookAhead);
				void								EndOfInput();
				Trace*								PrepareTraceRoute();
				Ptr<ParsingAstBase>					ExecuteTrace(Trace* trace, IAstInsReceiver& receiver, collections::List<regex::RegexToken>& tokens);
			};
		}
	}
}

#endif
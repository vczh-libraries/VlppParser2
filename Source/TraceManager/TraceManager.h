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
				vint32_t				tokenIndex = -1;			// index of the token when successors in this list are created
																	// the following members records all successors
																	// that is created at the token index

				vint32_t				first = -1;					// first successor
				vint32_t				last = -1;					// last successor
			};

			struct ReturnStackCache
			{
				ReturnStackSuccessors	lastSuccessors;				// the value of successors before the current one is changed
				ReturnStackSuccessors	successors;					// successors of ReturnStack for a token
				vint32_t				tokenIndex = -1;			// index of the token when this ReturnStack is created.
				vint32_t				prev = -1;					// previous successor of ReturnStack::previous
				vint32_t				next = -1;					// next successor of ReturnStack::previous
			};

			struct ReturnStack
			{
				vint32_t				allocatedIndex = -1;		// id of this ReturnStack
				vint32_t				previous = -1;				// id of the previous ReturnStack
				vint32_t				returnIndex = -1;			// index of the ReturnDesc
				vint32_t				fromTrace = -1;				// id of the Trace which has a transition containing this ReturnStack
				ReturnStackCache		cache;
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

			struct AmbiguityRouting
			{
				vint32_t				predecessorCount = -1;		// the number of predecessors
																	// (filled by ExecuteTrace)

				vint32_t				branchVisited = 0;			// the number of visited branches in the current loop.
																	// if these branches are contained in a larger ambiguity resolving loop, all branches could be visited multiple times
																	// (filled by ExecuteTrace)
			};

			struct Switches
			{
				vint32_t				allocatedIndex = -1;
				vint32_t				previous = -1;				// id of the previous Switches in the stack
				vint32_t				firstChild = -1;			// id of the first Swtiches that was ever pushed after the current one
				vint32_t				nextSibling = -1;			// id of the next Switches in all Switches that were ever pushed after the previous one
				vuint32_t				values[2] = { 0 };			// switch values, temporary set to 64 slots
			};

/***********************************************************************
TraceManager (Data Structures -- Trace)
***********************************************************************/

			struct TraceCollection
			{
				vint32_t				first = -1;					// first trace in the collection
				vint32_t				last = -1;					// last trace in the collection
				vint32_t				siblingPrev = -1;			// previous trace in the collection of the owned trace
				vint32_t				siblingNext = -1;			// next trace in the collection of the owned trace
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

			struct Trace
			{
				vint32_t				allocatedIndex = -1;		// id of this Trace
				TraceCollection			predecessors;				// id of the predecessor Trace
				TraceCollection			successors;					// successors (filled by EndOfInput)

				vint32_t				state = -1;					// id of the current StateDesc
				vint32_t				returnStack = -1;			// id of the current ReturnStack
				vint32_t				executedReturnStack = -1;	// id of the executed ReturnStack that contains the ReturnDesc being executed
				vint32_t				switchValues = -1;			// the id of switch values, it will be -1 if no switch is defined for this parser
				vint32_t				byEdge = -1;				// id of the last EdgeDesc that make this trace
				vint32_t				byInput = -1;				// the last input that make this trace
				vint32_t				currentTokenIndex = -1;		// the index of the token that is byInput
				CompetitionRouting		competitionRouting;			// a data structure carrying priority and competition information
			};

/***********************************************************************
TraceManager
***********************************************************************/

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

			class TraceManager : public Object, public virtual IExecutor
			{
			protected:
				Executable&							executable;
				const ITypeCallback*				typeCallback = nullptr;
				vint32_t							maxSwitchValues = 0;

				TraceManagerState					state = TraceManagerState::Uninitialized;
				AllocateOnly<ReturnStack>			returnStacks;
				AllocateOnly<Trace>					traces;
				AllocateOnly<Competition>			competitions;
				AllocateOnly<AttendingCompetitions>	attendingCompetitions;
				AllocateOnly<Switches>				switches;

				collections::List<Trace*>			traces1;
				collections::List<Trace*>			traces2;

				Trace*								initialTrace = nullptr;
				vint32_t							activeCompetitions = -1;
				vint32_t							rootSwitchValues = -1;
				ReturnStackCache					initialReturnStackCache;

				collections::List<bool>				temporaryConditionStack;
				vint32_t							temporaryConditionStackSize = 0;

				void								BeginSwap();
				void								AddTrace(Trace* trace);
				void								EndSwap();
				void								AddTraceToCollection(Trace* owner, Trace* element, TraceCollection(Trace::* collection));

				// Ambiguity
				bool								AreTwoEndingInputTraceEqual(vint32_t state, vint32_t returnStack, vint32_t executedReturnStack, vint32_t acId, vint32_t switchValues, Trace* candidate);
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
				void								AttendCompetitionIfNecessary(Trace* trace, vint32_t currentTokenIndex, EdgeDesc& edgeDesc, vint32_t& newAttendingCompetitions, vint32_t& newCarriedCompetitions, vint32_t& newReturnStack);
				void								CheckAttendingCompetitionsOnEndingEdge(Trace* trace, EdgeDesc& edgeDesc, vint32_t acId, vint32_t returnStack);
				void								CheckBackupTracesBeforeSwapping(vint32_t currentTokenIndex);

				// ReturnStack
				ReturnStackSuccessors*				GetCurrentSuccessorInReturnStack(vint32_t base, vint32_t currentTokenIndex);
				ReturnStack*						PushReturnStack(vint32_t base, vint32_t returnIndex, vint32_t fromTrace, vint32_t currentTokenIndex, bool allowReuse);

				// Walk
				bool								IsQualifiedTokenForCondition(regex::RegexToken* token, StringLiteral condition);
				bool								IsQualifiedTokenForEdgeArray(regex::RegexToken* token, EdgeArray& edgeArray);
				vint32_t							PushSwitchFrame(Switches* currentSV, vuint32_t* values);
				vint32_t							RunEdgeConditionChecking(vint32_t currentSwitchValues, EdgeDesc& edgeDesc);
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
				TraceManager(Executable& _executable, const ITypeCallback* _typeCallback);

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
				Switches*							GetSwitches(vint32_t index);

				void								Initialize(vint32_t startState) override;
				bool								Input(vint32_t currentTokenIndex, regex::RegexToken* token, regex::RegexToken* lookAhead) override;
				Trace*								EndOfInput() override;
				void								PrepareTraceRoute() override;
				Ptr<ParsingAstBase>					ExecuteTrace(Trace* trace, IAstInsReceiver& receiver, collections::List<regex::RegexToken>& tokens) override;
			};
		}
	}
}

#endif
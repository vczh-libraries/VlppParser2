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

			struct WithMagicCounter
			{
				vuint64_t							mergeCounter = 0;		// a temporary counter for internal use
			};

			struct NullRef {};
			constexpr auto nullref = NullRef{};

			template<typename T>
			struct Ref
			{
				vint32_t		handle = -1;

				Ref() = default;
				Ref(NullRef) :handle(-1) {}
				Ref(T* obj) :handle(obj == nullptr ? -1 : obj->allocatedIndex) {}
				Ref(const Ref<T>& ref) :handle(ref.handle) {}
				explicit Ref(vint32_t _handle) :handle(_handle) {}

				__forceinline bool operator==(NullRef) const { return handle == -1; }
				__forceinline bool operator==(const Ref<T>& ref) const { return handle == ref.handle; }
				__forceinline std::strong_ordering operator<=>(const Ref<T>& ref) const = default;

				__forceinline Ref& operator=(const Ref<T>& ref) { handle = ref.handle; return *this; }
				__forceinline Ref& operator=(T* obj) { handle = obj == nullptr ? -1 : obj->allocatedIndex; return *this; }
				__forceinline Ref& operator=(NullRef) { handle = -1; return *this; }

				__forceinline std::strong_ordering operator<=>(vint32_t) = delete;
				__forceinline bool operator==(vint32_t) = delete;
				__forceinline Ref& operator=(vint32_t) = delete;
			};

			template<typename T>
			struct Allocatable
			{
				vint32_t		allocatedIndex = -1;
			};

			template<typename T>
			class AllocateOnly : public Object
			{
				static_assert(std::is_base_of_v<Allocatable<T>, T>, "T in AllocateOnly<T> does not inherit from Allocatable<T>.");
			protected:
				vint											blockSize;
				vint											remains;
				collections::List<Ptr<collections::Array<T>>>	buffers;

			public:
				AllocateOnly(vint _blockSize)
					: blockSize(_blockSize)
					, remains(0)
				{
				}

				T* Get(Ref<T> index)
				{
					vint row = index.handle / blockSize;
					vint column = index.handle % blockSize;
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

				Ref<T> Allocate()
				{
					if (remains == 0)
					{
						buffers.Add(Ptr(new collections::Array<T>(blockSize)));
						remains = blockSize;
					}
					vint index = blockSize * (buffers.Count() - 1) + (blockSize - remains);
					buffers[buffers.Count() - 1]->operator[](blockSize - remains).allocatedIndex = (vint32_t)index;
					remains--;
					return Ref<T>((vint32_t)index);
				}

				void Clear()
				{
					remains = 0;
					buffers.Clear();
				}
			};

			struct ReturnStack;
			struct Trace;
			struct TraceExec;

/***********************************************************************
TraceManager (Data Structures)
***********************************************************************/

			struct ReturnStackSuccessors
			{
				vint32_t				tokenIndex = -1;			// index of the token when successors in this list are created
																	// the following members records all successors
																	// that is created at the token index

				Ref<ReturnStack>		first;						// first successor
				Ref<ReturnStack>		last;						// last successor
			};

			struct ReturnStackCache
			{
				ReturnStackSuccessors	lastSuccessors;				// the value of successors before the current one is changed
				ReturnStackSuccessors	successors;					// successors of ReturnStack for a token
				vint32_t				tokenIndex = -1;			// index of the token when this ReturnStack is created.
				Ref<ReturnStack>		prev;						// previous successor of ReturnStack::previous
				Ref<ReturnStack>		next;						// next successor of ReturnStack::previous
			};

			struct ReturnStack : Allocatable<ReturnStack>
			{
				Ref<ReturnStack>		previous;					// id of the previous ReturnStack
				vint32_t				returnIndex = -1;			// index of the ReturnDesc
				Ref<Trace>				fromTrace;					// id of the Trace which has a transition containing this ReturnStack
				ReturnStackCache		cache;
			};

			enum class CompetitionStatus
			{
				Holding,
				HighPriorityWin,
				LowPriorityWin,
			};

			struct Competition : Allocatable<Competition>
			{
				Ref<Competition>		nextActiveCompetition;					// next active Competition
				Ref<Competition>		nextHoldCompetition;					// next Competition hold by this trace

				CompetitionStatus		status = CompetitionStatus::Holding;	// if predecessors from this trace have different priority, the competition begins
																				// when the competition is over, it will be changed to HighPriorityWin or LowPriorityWin
																				// if all candidates fail, it could be Holding forever

				vint32_t				currentTokenIndex = -1;					// currentTokenIndex from the trace that creates this competition
				vint32_t				ruleId = -1;							// the rule id of state, when an edge starts this competition
				vint32_t				clauseId = -1;							// the clause id of the state, when an edge starts this competition
																				// an state must be picked up and ensure that, the syntax creating the priority and the state belong to the same clause

				vint32_t				highCounter = 0;						// temporary counter for all existing high bets
																				// in the current step of input
				vint32_t				lowCounter = 0;							// temporary counter for all existing low bets
																				// in the current step of input
			};

			struct AttendingCompetitions : Allocatable<AttendingCompetitions>
			{
				Ref<AttendingCompetitions>	nextActiveAC;				// the next AttendingCompetitions for RuntimeRouting::attendingCompetitions
				Ref<AttendingCompetitions>	nextCarriedAC;				// the next AttendingCompetitions for RuntimeRouting::carriedCompetitions
				Ref<Competition>			competition;				// the id of the Competition
				bool						forHighPriority = false;	// bet of this competition

				Ref<ReturnStack>			returnStack;				// the ReturnStack object for the competition
																		// if the competition is attended by a ReturnDesc
																		// then the ReturnStack object is the one before a ReturnDesc transition happens

				bool						closed = false;				// true if the competition has been closed
																		// this flag is not always updated for discarded AttendingCompetitions objects
			};

/***********************************************************************
TraceManager (Data Structures -- Input/EndOfInput)

For a trace like:
  A
 / \
B   C
 \ /
  D

A.successors.(first .. last) = {B,C}
B.successors.siblingNext = C
C.successors.siblingPrev = B
(B, C).successors.(first .. last) = {D}

predecessors are for reverse relationships.
Such data structure makes many-to-many relationships impossible to represent.
***********************************************************************/

			struct TraceCollection
			{
				Ref<Trace>					first;						// first trace in the collection
				Ref<Trace>					last;						// last trace in the collection
				Ref<Trace>					siblingPrev;				// previous trace in the collection of the owned trace
				Ref<Trace>					siblingNext;				// next trace in the collection of the owned trace
			};

			struct CompetitionRouting
			{
				Ref<Competition>			holdingCompetitions;		// the id of the active Competition

				Ref<AttendingCompetitions>	attendingCompetitions;		// a linked list containing all AttendingCompetitions that this trace is attending
																		// predecessors could share and modify the same linked list
																		// if a competition is over, node could be removed from the linked list
																		// one competition only creates two AttendingCompetitions, traces with the same bet sharing the object

				Ref<AttendingCompetitions>	carriedCompetitions;		// all attended competitions regardless of the status of the competition
			};

			struct Trace : Allocatable<Trace>
			{
				TraceCollection			predecessors;				// ids of predecessor Trace

				// (filled by EndOfInput)
				TraceCollection			successors;					// ids of successor Trace
				vint32_t				predecessorCount = 0;
				vint32_t				successorCount = 0;

				// if state == -1
				// it means this is an ambiguity resolving trace
				// all merged traces are in predecessors

				vint32_t				state = -1;					// id of the current StateDesc
				Ref<ReturnStack>		returnStack;				// id of the current ReturnStack
				Ref<ReturnStack>		executedReturnStack;		// id of the executed ReturnStack that contains the ReturnDesc being executed
				vint32_t				byEdge = -1;				// id of the last EdgeDesc that make this trace
				vint32_t				byInput = -1;				// the last input that make this trace
				vint32_t				currentTokenIndex = -1;		// the index of the token that is byInput
				CompetitionRouting		competitionRouting;			// a data structure carrying priority and competition information

				// (filled by PrepareTraceRoute)
				Ref<TraceExec>			traceExecRef;				// the allocated TraceExec
				vint32_t				iterateCounter = 0;			// a temporary counter for IterateSurvivedTraces internal use
			};

/***********************************************************************
TraceManager (Data Structures -- PrepareTraceRoute)
***********************************************************************/

			struct InsExec_Stack;

			struct InsRef
			{
				Ref<Trace>							trace;
				vint32_t							ins = -1;

				__forceinline std::strong_ordering operator<=>(const InsRef& ref) const = default;
			};

			struct InsExec_InsRefLink : Allocatable<InsExec_InsRefLink>
			{
				Ref<InsExec_InsRefLink>				previous;
				InsRef								insRef;
			};

			struct InsExec_StackRefLink : Allocatable<InsExec_StackRefLink>
			{
				Ref<InsExec_StackRefLink>			previous;
				Ref<InsExec_Stack>					id;
			};

			struct InsExec_StackArrayRefLink : Allocatable<InsExec_StackArrayRefLink>, WithMagicCounter
			{
				Ref<InsExec_StackArrayRefLink>		previous;
				Ref<InsExec_StackRefLink>			ids;

				// The current depth of the link. The first one is 0.
				vint								currentDepth = -1;

				// Available when the link is in InsExec_Context::createStack
				// It records the InsExec_Context::objectStack depth when the link is created.
				vint								objectStackDepthForCreateStack = -1;
			};

			struct InsExec_StackSummarizing
			{
				// The earliest StackBegin instructions including in useFromStacks
				InsRef								earliestLocalInsRef;

				// The earliest StackBegin instructions including in useFromStacks and fieldStacks
				InsRef								earliestInsRef;

				// All CreateObject instructions including in useFromStacks
				Ref<InsExec_InsRefLink>				indirectCreateObjectInsRefs;
			};

			struct InsExec_Stack : Allocatable<InsExec_Stack>, WithMagicCounter
			{
				// previous allocated object
				Ref<InsExec_Stack>					previous;

				// owner-field relationships
				Ref<InsExec_StackRefLink>			fieldStacks;

				// useFrom-useBy relationships
				Ref<InsExec_StackRefLink>			useFromStacks;

				// Key instructions in this stack
				InsRef								beginInsRef;
				Ref<InsExec_InsRefLink>				createObjectInsRefs;
				Ref<InsExec_InsRefLink>				endWithCreateInsRefs;
				Ref<InsExec_InsRefLink>				endWithReuseInsRefs;

				InsExec_StackSummarizing			summarizing;
			};

			struct InsExec_Context
			{
				Ref<InsExec_StackArrayRefLink>		objectStack;			// Stack of created objects
				Ref<InsExec_StackArrayRefLink>		createStack;			// Stack of opening objects
			};

			struct InsExec : WithMagicCounter
			{
				// Stack operated by StackBegin/StackEnd/CreateObject
				Ref<InsExec_StackRefLink>			operatingStacks;

				// Context before executing this instruction
				InsExec_Context						contextBeforeExecution;
			};

/***********************************************************************
TraceManager (Data Structures -- ResolveAmbiguity)

A branch begins from:
  The initial trace
  Successors of a branch trace (a trace with multiple successors)
  A merge trace (a trace with multiple predecessors)
branchData.forwardTrace points to the nearest beginning of a trace.

Here a demos of which traces are beginnings:
  A*
  |
  B
 / \
C*  D*
|   |
E   F
 \ /
  G*(cfb->B)
  |
  H

For any merge trace, its branchData.commonForwardBranch points to the latest forwardTrace that all comming branches share.
It does not necessary equal to branchData.forwardTrace of all predecessors as their values might be different.

All branch traces can be found beginning from TraceManager::firstBranchTrace following nextBranchTrace.
All merge traces can be found beginning from TraceManager::firstMergeTrace following nextMergeTrace.
Traversing through branchData.forwardTrace and branchData.commonForwardBranch will skip all branches going forward.
***********************************************************************/

			// TraceAmbiguity describes where an ambiguity resolving begins and ends
			struct TraceAmbiguity : Allocatable<TraceAmbiguity>
			{
				// all objects to merge, they all have valid createObjectInsRef 
				Ref<InsExec_StackRefLink>			bottomCreateObjectStacks;

				// if multiple TraceAmbiguity are assigned to the same place
				// it records the one it overrides
				Ref<TraceAmbiguity>					overridedAmbiguity;

				// the trace where ambiguity resolution begins
				// prefix is the number of instructions before SB
				// if prefix + 1 is larger than instructions in firstTrace
				// then StackBegin is in all successors
				// these instructions create topObjectIds
				Ref<Trace>							firstTrace;
				vint32_t							prefix = -1;

				// the trace when ambiguity resolution ends
				// postfix is the number of instructions after SE
				// if lastTrace is a merge trace
				// then StackEnd is in all predecessors
				// these instructions end bottomObjectIds
				Ref<Trace>							lastTrace;
				vint32_t							postfix = -1;
			};

			struct TraceAmbiguityLink : Allocatable<TraceAmbiguityLink>
			{
				Ref<TraceAmbiguityLink>				previous;
				Ref<TraceAmbiguity>					ambiguity;
			};

			struct TraceInsLists
			{
				InstructionArray					edgeInsAfterInput;
				InstructionArray					returnInsAfterInput;
				vint32_t							countAfterInput;
				vint32_t							countAll;
			};

			struct TraceBranchData : WithMagicCounter
			{
				// it stores the first trace of non branching path that this trace is in
				// such trace could be:
				//   the initial trace
				//   successors of a branch trace
				//   a merge trace
				Ref<Trace>							forwardTrace;

				// for merge trace, it stores the latest forwardTrace that all comming branches share
				Ref<Trace>							commonForwardBranch;
			};

			// TraceExec stores all ambiguity awared data for a trace
			struct TraceExec : Allocatable<TraceExec>
			{
				Ref<Trace>							traceId;
				TraceInsLists						insLists;				// instruction list of this trace
				InstructionArray					insExecRefs;			// allocated InsExec for instructions

				InsExec_Context						context;				// context after executing all instructions
				TraceBranchData						branchData;				// branch shapes

				// linked list of branch traces, in a global depth-first order, from TraceManager::firstBranchTrace
				Ref<Trace>							nextBranchTrace;

				// linked list of merge traces, in a global depth-first order, from TraceManager::firstMergeTrace
				Ref<Trace>							nextMergeTrace;

				// linked list of ambiguity critical trace
				// it is stored in a trace whose forwardTrace is itself
				// record all traces with the same forwardTrace value, order by trace id ascending
				//   a branch trace
				//   a predecessor of a merge trace
				//   a trace pointed by TraceAmbiguity::firstTrace
				Ref<Trace>							nextAmbiguityCriticalTrace;

				// TraceAmbiguity associated to the trace
				// it could be associated to
				//   TraceAmbiguity::firstTrace (order by prefix ascending)
				//   TraceAmbiguity::lastTrace  (order by postfix ascending)
				//   the merge trace that create this TraceAmbiguity
				// ambiguityBegins will contain multiple TraceAmbiguity when
				//   multiple ambiguity begins in different group of successors
				//   there is also a possibility when all ambiguities don't cover all successors

				Ref<TraceAmbiguity>					ambiguityDetected;		// The TraceAmbiguity whose lastTrace is this trace
																			// Referring to the last StackEnd of all predecessors
																			// Or the opening object at the end of the trace

				Ref<TraceAmbiguityLink>				ambiguityBegins;		// All TraceAmbiguity whose firstTrace is this trace
																			// All TraceAmbiguity in this list are grouped by lastTrace (using TraceAmbiguity::overridedAmbiguity)
																			// To traverse all of them, begins from each TraceAmbiguity in this list, and go through TraceAmbiguity::overridedAmbiguity

				// when this trace is a successor of a branch trace
				// and such branch trace has non-empty ambiguityBegins
				// ambiguityCoveredInForward points to the ambiguity which begins in the current trace
				Ref<TraceAmbiguity>					ambiguityCoveredInForward;
			};

/***********************************************************************
TraceManager (Data Structures -- BuildExecutionOrder)
***********************************************************************/

			struct ExecutionStep;

			enum class ExecutionType
			{
				Empty,
				Instruction,
				RA_Begin,
				RA_Branch,
				RA_End,
			};

			struct ExecutionStep : Allocatable<ExecutionStep>
			{
				struct ETI
				{
					vint32_t						startTrace;
					vint32_t						startIns;
					vint32_t						endTrace;
					vint32_t						endIns;
				};

				struct ETRA
				{
					vint32_t						count;
					vint32_t						type;
					vint32_t						trace;
				};

				ExecutionType						type = ExecutionType::Instruction;

				// for steps that ready to execute
				// "next" means the next step to execute
				// for steps that returns from BuildStepTree
				// "next" in a leaf step points to the next leaf step
				Ref<ExecutionStep>					next;

				// for steps that returns from BuildStepTree
				// "next" is the parent step in the tree
				Ref<ExecutionStep>					parent;

				vint32_t							copyCount = 0;
				vint32_t							visitCount = 0;

				union
				{
					ETI								et_i;
					ETRA							et_ra;
				};
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
				ResolvedAmbiguity,
			};

			struct WalkingTrace
			{
				Trace*								currentTrace;
				Trace*								stateTrace;

				operator bool() const
				{
					return currentTrace && stateTrace;
				}
			};

			class TraceManager : public Object, public virtual IExecutor
			{
			protected:
				Executable&									executable;
				const ITypeCallback*						typeCallback = nullptr;

				TraceManagerState							state = TraceManagerState::Uninitialized;
				AllocateOnly<ReturnStack>					returnStacks;
				AllocateOnly<Trace>							traces;
				AllocateOnly<Competition>					competitions;
				AllocateOnly<AttendingCompetitions>			attendingCompetitions;

				collections::List<Trace*>					traces1;
				collections::List<Trace*>					traces2;

				Trace*										initialTrace = nullptr;
				Ref<Competition>							activeCompetitions;
				ReturnStackCache							initialReturnStackCache;

				collections::List<bool>						temporaryConditionStack;
				vint32_t									temporaryConditionStackSize = 0;

				void										BeginSwap();
				void										AddTrace(Trace* trace);
				void										EndSwap();
				void										AddTraceToCollection(Trace* owner, Trace* element, TraceCollection(Trace::* collection));

				// Ambiguity
				Trace*										EnsureTraceWithValidStates(Trace* trace);
				bool										AreTwoEndingInputTraceEqual(Trace* newTrace, Trace* candidate);
				Trace*										MergeTwoEndingInputTrace(Trace* newTrace, Trace* candidate);
				void										TryMergeSurvivingTraces();

				// Competition
				void										AttendCompetition(Trace* trace, Ref<AttendingCompetitions>& newAttendingCompetitions, Ref<AttendingCompetitions>& newCarriedCompetitions, Ref<ReturnStack> returnStack, vint32_t ruleId, vint32_t clauseId, bool forHighPriority);
				void										AttendCompetitionIfNecessary(Trace* trace, vint32_t currentTokenIndex, EdgeDesc& edgeDesc, Ref<AttendingCompetitions>& newAttendingCompetitions, Ref<AttendingCompetitions>& newCarriedCompetitions, Ref<ReturnStack>& newReturnStack);
				void										CheckAttendingCompetitionsOnEndingEdge(Trace* trace, EdgeDesc& edgeDesc, Ref<AttendingCompetitions> acId, Ref<ReturnStack> returnStack);
				bool										CheckBackupTracesBeforeSwapping(vint32_t currentTokenIndex);

				// ReturnStack
				ReturnStackSuccessors*						GetCurrentSuccessorInReturnStack(Ref<ReturnStack> base, vint32_t currentTokenIndex);
				ReturnStack*								PushReturnStack(Ref<ReturnStack> base, vint32_t returnIndex, Ref<Trace> fromTrace, vint32_t currentTokenIndex, bool allowReuse);

				// Walk
				bool										IsQualifiedTokenForCondition(regex::RegexToken* token, StringLiteral condition);
				bool										IsQualifiedTokenForEdgeArray(regex::RegexToken* token, EdgeArray& edgeArray);
				WalkingTrace								WalkAlongSingleEdge(vint32_t currentTokenIndex, vint32_t input, WalkingTrace trace, vint32_t byEdge, EdgeDesc& edgeDesc);
				void										WalkAlongLeftrecEdges(vint32_t currentTokenIndex, regex::RegexToken* lookAhead, WalkingTrace trace, EdgeArray& edgeArray);
				void										WalkAlongEpsilonEdges(vint32_t currentTokenIndex, regex::RegexToken* lookAhead, WalkingTrace trace);
				void										WalkAlongTokenEdges(vint32_t currentTokenIndex, vint32_t input, regex::RegexToken* token, regex::RegexToken* lookAhead, WalkingTrace trace, EdgeArray& edgeArray);

				// EndOfInput
				void										FillSuccessorsAfterEndOfInput(bool& ambiguityInvolved);

			protected:
				// Common
				vuint64_t									MergeStack_MagicCounter = 0;

				template<typename TCallback>
				void										IterateSurvivedTraces(TCallback&& callback);

			public:
				void										ReadInstructionList(Trace* trace, TraceInsLists& insLists);
				AstIns&										ReadInstruction(vint32_t instruction, TraceInsLists& insLists);

			protected:
				// PrepareTraceRoute
				AllocateOnly<TraceExec>						traceExecs;
				collections::Array<InsExec>					insExecs;
				AllocateOnly<InsExec_Stack>					insExec_Stacks;
				AllocateOnly<InsExec_InsRefLink>			insExec_InsRefLinks;
				AllocateOnly<InsExec_StackRefLink>			insExec_StackRefLinks;
				AllocateOnly<InsExec_StackArrayRefLink>		insExec_StackArrayRefLinks;

				// phase: AllocateExecutionData
				void										AllocateExecutionData();

				// phase: BuildAmbiguityStructures
				Trace* StepForward(Trace* trace);
				void										BuildAmbiguityStructures();

				// phase: PartialExecuteTraces - PartialExecuteOrdinaryTrace
				InsExec_Stack*								NewStack();
				void										PushInsRefLink(Ref<InsExec_InsRefLink>& link, InsRef insRef);
				void										PushStackRefLink(Ref<InsExec_StackRefLink>& link, Ref<InsExec_Stack> id);
				void										PushStackArrayRefLink(Ref<InsExec_StackArrayRefLink>& arrayLink, Ref<InsExec_Stack> id);
				void										PushStackArrayRefLink(Ref<InsExec_StackArrayRefLink>& arrayLink, Ref<InsExec_StackRefLink> link);
				Ref<InsExec_InsRefLink>						JoinInsRefLink(Ref<InsExec_InsRefLink> first, Ref<InsExec_InsRefLink> second);
				Ref<InsExec_StackRefLink>					JoinStackRefLink(Ref<InsExec_StackRefLink> first, Ref<InsExec_StackRefLink> second);
				void										PartialExecuteOrdinaryTrace(Trace* trace);

				// phase: PartialExecuteTraces - EnsureInsExecContextCompatible
				void										EnsureInsExecContextCompatible(Trace* baselineTrace, Trace* commingTrace);

				// phase: PartialExecuteTraces - MergeInsExecContext
				template<Ref<InsExec_StackArrayRefLink> (InsExec_Context::*stack), typename TMerge>
				Ref<InsExec_StackArrayRefLink>				MergeStack(Trace* mergeTrace, TMerge&& merge);
				void										MergeInsExecContext(Trace* mergeTrace);

				// phase: PartialExecuteTraces
				void										PartialExecuteTraces();

				// phase: SummarizeInstructionRange
				template<typename TCallback>
				void										IterateStackWithDependency(Ref<InsExec_StackRefLink>(InsExec_Stack::* dependencies), TCallback&& callback);
				bool										UpdateTopTrace(InsRef& topInsRef, InsRef newInsRef);
				void										CollectInsRefs(collections::SortedList<InsRef>& insRefs, Ref<InsExec_InsRefLink> link);
				void										SummarizeEarilestLocalInsRefs();
				void										SummarizeEarilestInsRefs();
				void										SummarizeInstructionRange();

			protected:
				// ResolveAmbiguity
				Ref<Trace>									firstBranchTrace;
				Ref<Trace>									firstMergeTrace;
				Ref<InsExec_Stack>							firstStack;
				Ref<ExecutionStep>							firstStep;
				AllocateOnly<TraceAmbiguity>				traceAmbiguities;
				AllocateOnly<TraceAmbiguityLink>			traceAmbiguityLinks;
				AllocateOnly<ExecutionStep>					executionSteps;

				// phase: CheckMergeTraces
				template<typename TCallback>
				bool										EnumerateObjects(Ref<InsExec_StackRefLink> stackRefLinkStartSet, bool withCounter, TCallback&& callback);
				template<typename TCallback>
				bool										EnumerateBottomInstructions(InsExec_Stack* ieObject, TCallback&& callback);
				bool										ComparePrefix(TraceExec* baselineTraceExec, TraceExec* commingTraceExec, vint32_t prefix);
				bool										ComparePostfix(TraceExec* baselineTraceExec, TraceExec* commingTraceExec, vint32_t postfix);
				template<typename TCallback>
				bool										CheckAmbiguityResolution(TraceAmbiguity* ta, collections::List<Ref<InsExec_StackRefLink>>& visitingIds, TCallback&& callback);
				bool										CheckMergeTrace(TraceAmbiguity* ta, Trace* trace, TraceExec* traceExec, collections::List<Ref<InsExec_StackRefLink>>& visitingIds);
				void										LinkAmbiguityCriticalTrace(Ref<Trace> traceId);
				void										CheckTraceAmbiguity(TraceAmbiguity* ta);
				void										MarkAmbiguityCoveredForward(Trace* currentTrace, TraceAmbiguity* ta, Trace* firstTrace, TraceExec* firstTraceExec);
				void										CategorizeTraceAmbiguities(Trace* trace, TraceExec* traceExec);
				void										CheckMergeTraces();

				// phase: BuildExecutionOrder
#define DEFINE_EXECUTION_STEP_CONTEXT						ExecutionStep*& root, ExecutionStep*& firstLeaf, ExecutionStep*& currentStep, ExecutionStep*& currentLeaf
				void										MarkNewLeafStep(ExecutionStep* step, ExecutionStep*& firstLeaf, ExecutionStep*& currentLeaf);
				void										AppendStepLink(ExecutionStep* first, ExecutionStep* last, DEFINE_EXECUTION_STEP_CONTEXT);
				void										AppendStepsBeforeAmbiguity(Trace* startTrace, vint32_t startIns, TraceAmbiguity* ta, DEFINE_EXECUTION_STEP_CONTEXT);
				void										AppendStepsAfterAmbiguity(Trace*& startTrace, vint32_t& startIns, TraceAmbiguity* ta, DEFINE_EXECUTION_STEP_CONTEXT);
				void										AppendStepsForAmbiguity(TraceAmbiguity* ta, bool checkCoveredMark, DEFINE_EXECUTION_STEP_CONTEXT);
				void										AppendStepsBeforeBranch(Trace* startTrace, vint32_t startIns, Trace* branchTrace, TraceExec* branchTraceExec, DEFINE_EXECUTION_STEP_CONTEXT);
				void										BuildStepTree(Trace* startTrace, vint32_t startIns, Trace* endTrace, vint32_t endIns, ExecutionStep*& root, ExecutionStep*& firstLeaf, ExecutionStep* currentStep, ExecutionStep*& currentLeaf, bool ambiguityBranch);
				void										ConvertStepTreeToLink(ExecutionStep* root, ExecutionStep* firstLeaf, ExecutionStep*& first, ExecutionStep*& last);
				void										BuildAmbiguousStepLink(TraceAmbiguity* ta, bool checkCoveredMark, ExecutionStep*& first, ExecutionStep*& last);
				void										BuildExecutionOrder();
#undef DEFINE_EXECUTION_STEP_CONTEXT

			public:
				TraceManager(Executable& _executable, const ITypeCallback* _typeCallback, vint blockSize);

				vint32_t						concurrentCount = 0;
				collections::List<Trace*>*		concurrentTraces = nullptr;
				collections::List<Trace*>*		backupTraces = nullptr;

				ReturnStack*					GetReturnStack(Ref<ReturnStack> index);
				ReturnStack*					AllocateReturnStack();
				Trace*							GetTrace(Ref<Trace> index);
				Trace*							AllocateTrace();
				Competition*					GetCompetition(Ref<Competition> index);
				Competition*					AllocateCompetition();
				AttendingCompetitions*			GetAttendingCompetitions(Ref<AttendingCompetitions> index);
				AttendingCompetitions*			AllocateAttendingCompetitions();

				InsExec*						GetInsExec(vint32_t index);
				InsExec_Stack*					GetInsExec_Stack(Ref<InsExec_Stack> index);
				InsExec_InsRefLink*				GetInsExec_InsRefLink(Ref<InsExec_InsRefLink> index);
				InsExec_StackRefLink*			GetInsExec_StackRefLink(Ref<InsExec_StackRefLink> index);
				InsExec_StackArrayRefLink*		GetInsExec_StackArrayRefLink(Ref<InsExec_StackArrayRefLink> index);
				TraceExec*						GetTraceExec(Ref<TraceExec> index);
				TraceAmbiguity*					GetTraceAmbiguity(Ref<TraceAmbiguity> index);
				TraceAmbiguityLink*				GetTraceAmbiguityLink(Ref<TraceAmbiguityLink> index);
				ExecutionStep*					GetExecutionStep(Ref<ExecutionStep> index);

				void							Initialize(vint32_t startState) override;
				Trace*							GetInitialTrace();
				ExecutionStep*					GetInitialExecutionStep();

				bool							Input(vint32_t currentTokenIndex, regex::RegexToken* token, regex::RegexToken* lookAhead) override;
				bool							EndOfInput(bool& ambiguityInvolved) override;

				void							PrepareTraceRoute() override;
				void							ResolveAmbiguity() override;

			protected:
				void							ExecuteSingleTrace(IAstInsReceiver& receiver, Trace* trace, vint32_t firstIns, vint32_t lastIns, TraceInsLists& insLists, collections::List<regex::RegexToken>& tokens);
				void							ExecuteSingleStep(IAstInsReceiver& receiver, ExecutionStep* step, collections::List<regex::RegexToken>& tokens);
			public:
				Ptr<ParsingAstBase>				ExecuteTrace(IAstInsReceiver& receiver, collections::List<regex::RegexToken>& tokens) override;
			};

			class TraceException : public Exception
			{
			public:
				TraceException(TraceManager& tm, InsRef insRef, const wchar_t* phrase, const wchar_t* message)
					: Exception(
						WString::Unmanaged(L"[") + 
						WString::Unmanaged(phrase) +
						WString::Unmanaged(L"] at ") +
						itow(insRef.trace.handle) + WString::Unmanaged(L"@") + itow(insRef.ins) +
						WString::Unmanaged(L" : ") +
						WString::Unmanaged(message)
					)
				{
				}

				TraceException(TraceManager& tm, Trace* trace1, Trace* trace2, const wchar_t* phrase, const wchar_t* message)
					: Exception(
						WString::Unmanaged(L"[") +
						WString::Unmanaged(phrase) +
						WString::Unmanaged(L"] at trace ") +
						itow(trace1->allocatedIndex) +
						(trace2 == nullptr ? WString::Empty : WString::Unmanaged(L" and ") + itow(trace2->allocatedIndex)) +
						WString::Unmanaged(L" : ") +
						WString::Unmanaged(message)
					)
				{
				}

				TraceException(TraceManager& tm, TraceAmbiguity* ta1, TraceAmbiguity* ta2, const wchar_t* phrase, const wchar_t* message)
					: Exception(
						WString::Unmanaged(L"[") +
						WString::Unmanaged(phrase) +
						WString::Unmanaged(L"] at trace ambiguity ") +
						itow(ta1->firstTrace.handle) + WString::Unmanaged(L"@") + itow(ta1->prefix) + WString::Unmanaged(L"..") +
						itow(ta1->lastTrace.handle) + WString::Unmanaged(L"@-") + itow(ta1->postfix) +
						WString::Unmanaged(L" and ") +
						itow(ta2->firstTrace.handle) + WString::Unmanaged(L"@") + itow(ta2->prefix) + WString::Unmanaged(L"..") +
						itow(ta2->lastTrace.handle) + WString::Unmanaged(L"@-") + itow(ta2->postfix) +
						WString::Unmanaged(L" : ") +
						WString::Unmanaged(message)
					)
				{
				}

				TraceException(TraceManager& tm, InsExec_Stack* stack, const wchar_t* phrase, const wchar_t* message)
					: Exception(
						WString::Unmanaged(L"[") +
						WString::Unmanaged(phrase) +
						WString::Unmanaged(L"] at trace ") +
						itow(stack->allocatedIndex) +
						WString::Unmanaged(L" : ") +
						WString::Unmanaged(message)
					)
				{
				}

				TraceException(TraceManager& tm, const wchar_t* phrase, const wchar_t* message)
					: Exception(
						WString::Unmanaged(L"[") +
						WString::Unmanaged(phrase) +
						WString::Unmanaged(L"] : ") +
						WString::Unmanaged(message)
					)
				{
				}
			};
		}
	}
}

#endif
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
				__forceinline bool operator!=(NullRef) const { return handle != -1; }
				__forceinline bool operator==(const Ref<T>& ref) const { return handle == ref.handle; }
				__forceinline bool operator!=(const Ref<T>& ref) const { return handle != ref.handle; }
				__forceinline bool operator> (const Ref<T>& ref) const { return handle >  ref.handle; }
				__forceinline bool operator>=(const Ref<T>& ref) const { return handle >= ref.handle; }
				__forceinline bool operator< (const Ref<T>& ref) const { return handle <  ref.handle; }
				__forceinline bool operator<=(const Ref<T>& ref) const { return handle <= ref.handle; }

				__forceinline Ref& operator=(const Ref<T>& ref) { handle = ref.handle; return *this; }
				__forceinline Ref& operator=(T* obj) { handle = obj == nullptr ? -1 : obj->allocatedIndex; return *this; }
				__forceinline Ref& operator=(NullRef) { handle = -1; return *this; }

				__forceinline bool operator==(vint32_t) = delete;
				__forceinline bool operator!=(vint32_t) = delete;
				__forceinline bool operator> (vint32_t) = delete;
				__forceinline bool operator>=(vint32_t) = delete;
				__forceinline bool operator< (vint32_t) = delete;
				__forceinline bool operator<=(vint32_t) = delete;
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
						buffers.Add(new collections::Array<T>(blockSize));
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
			struct InsExec_Object;

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

			struct Switches : Allocatable<Switches>
			{
				Ref<Switches>			previous;					// id of the previous Switches in the stack
				Ref<Switches>			firstChild;					// id of the first Swtiches that was ever pushed after the current one
				Ref<Switches>			nextSibling;				// id of the next Switches in all Switches that were ever pushed after the previous one
				vuint32_t				values[2] = { 0 };			// switch values, temporary set to 64 slots
			};

/***********************************************************************
TraceManager (Data Structures -- Input/EndOfInput)
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
																		// one competition only creates two AttendingCompetitions, traces with the same bet share the object

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
				Ref<Switches>			switchValues;				// the id of switch values, it will be -1 if no switch is defined for this parser
				vint32_t				byEdge = -1;				// id of the last EdgeDesc that make this trace
				vint32_t				byInput = -1;				// the last input that make this trace
				vint32_t				currentTokenIndex = -1;		// the index of the token that is byInput
				CompetitionRouting		competitionRouting;			// a data structure carrying priority and competition information

				// (filled by PrepareTraceRoute)
				Ref<TraceExec>			traceExecRef;				// the allocated TraceExec
				vint32_t				iterateCounter = 0;			// a temporary counter for IterateSurvivedTraces internal use
			};

/***********************************************************************
TraceManager (Data Structures -- PrepareTraceRoute/ResolveAmbiguity)
***********************************************************************/

			struct InsExec_InsRefLink : Allocatable<InsExec_InsRefLink>
			{
				Ref<InsExec_InsRefLink>				previous;
				Ref<Trace>							trace;
				vint32_t							ins = -1;
			};

			struct InsExec_ObjRefLink : Allocatable<InsExec_ObjRefLink>
			{
				Ref<InsExec_ObjRefLink>				previous;
				Ref<InsExec_Object>					id;
			};

			struct InsExec_Object : Allocatable<InsExec_Object>, WithMagicCounter
			{
				// lrObjectIds are objects it takes while being created by BOLR
				Ref<InsExec_ObjRefLink>				lrObjectIds;

				// instruction that creates this object
				Ref<Trace>							bo_bolr_Trace;
				vint32_t							bo_bolr_Ins = -1;

				// DelayFieldAssignment instructions that associates to the current object
				Ref<InsExec_InsRefLink>				dfaInsRefs;
			};

			struct InsExec_ObjectStack : Allocatable<InsExec_ObjectStack>, WithMagicCounter
			{
				Ref<InsExec_ObjectStack>			previous;
				Ref<InsExec_ObjRefLink>				objectIds;
				vint32_t							pushedCount = -1;		// number for InsExec_CreateStack::stackBase
			};

			struct InsExec_CreateStack : Allocatable<InsExec_CreateStack>, WithMagicCounter
			{
				Ref<InsExec_CreateStack>			previous;
				vint32_t							stackBase = -1;			// the number of objects in the object stack that is frozen

				// All InsExec_InsRefLink that create the current InsExec_CreateStack
				Ref<InsExec_InsRefLink>				createInsRefs;

				// InsExec_ObjRefLink assigned by BO/BOLA/RO
				Ref<InsExec_ObjRefLink>				objectIds;
			};

			struct InsExec_Context
			{
				Ref<InsExec_ObjectStack>			objectStack;			// InsExec_ObjectStack after executing instructions
				Ref<InsExec_CreateStack>			createStack;			// InsExec_CreateStack after executing instructions
				Ref<InsExec_ObjRefLink>				lriStoredObjects;		// LriStore stored InsExec_ObjRefLink after executing instructions
			};

			struct InsExec : WithMagicCounter
			{
				// BO/BOLR:
				//   the created object
				Ref<InsExec_Object>					createdObjectId;

				// DFA:
				//   all associated objects
				// EO:
				//   all ended objects
				Ref<InsExec_ObjRefLink>				objRefs;

				// InsExec_InsRefLink
				// BO/BOLR/DFA:
				//   EndingObject instructions that close objects or create stack created by the current instruction
				Ref<InsExec_InsRefLink>				eoInsRefs;

				// context before executing the current instruction
				InsExec_Context						contextBeforeExecution;
			};

			struct TraceAmbiguity : Allocatable<TraceAmbiguity>
			{
				// all objects to merge
				Ref<InsExec_ObjRefLink>				bottomObjectIds;

				// if multiple TraceAmbiguity are assigned to the same place
				// it records the one it overrides
				Ref<TraceAmbiguity>					overridedAmbiguity;

				// the trace where ambiguity resolution begins
				// prefix is the number of instructions before BO/DFA
				// if prefix + 1 is larger than instructions in firstTrace
				// then BO/DFA is in all successors
				// these instructions create topObjectIds
				Ref<Trace>							firstTrace;
				vint32_t							prefix = -1;

				// the trace when ambiguity resolution ends
				// postfix is the number of instructions after EO
				// if lastTrace is a merge trace
				// then EO is in all predecessors
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
				InstructionArray					edgeInsBeforeInput;
				InstructionArray					edgeInsAfterInput;
				InstructionArray					returnInsAfterInput;
				vint32_t							c1;
				vint32_t							c2;
				vint32_t							c3;
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

			struct TraceExec : Allocatable<TraceExec>
			{
				Ref<Trace>							traceId;
				TraceInsLists						insLists;				// instruction list of this trace
				InstructionArray					insExecRefs;			// allocated InsExec for instructions

				InsExec_Context						context;
				TraceBranchData						branchData;

				// linked list of branch traces
				Ref<Trace>							nextBranchTrace;

				// linked list of merge traces
				Ref<Trace>							nextMergeTrace;

				// linked list of ambiguity critical trace (order by trace id ascending)
				// the linked list begins from a trace whose forwardTrace is itself
				// record all traces that is
				//   a branch trace
				//   a predecessor of a merge trace
				//   a trace pointed by TraceAmbiguity::firstTrace
				Ref<Trace>							nextAmbiguityCriticalTrace;

				// TraceAmbiguity associated to the trace
				// it could be associated to
				//   firstTrace (order by prefix ascending)
				//   lastTrace  (order by postfix ascending)
				//   the merge trace that create this TraceAmbiguity
				// ambiguityBegins will contain multiple TraceAmbiguity when
				//   multiple ambiguity begins in different group of successors
				//   there is also a possibility when all ambiguities don't cover all successors
				Ref<TraceAmbiguity>					ambiguityDetected;
				Ref<TraceAmbiguityLink>				ambiguityBegins;

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
				ResolveAmbiguity,
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

			struct TraceManagerSubmitter;

			class TraceManager : public Object, public virtual IExecutor
			{
			protected:
				Executable&									executable;
				const ITypeCallback*						typeCallback = nullptr;
				vint32_t									maxSwitchValues = 0;

				TraceManagerState							state = TraceManagerState::Uninitialized;
				AllocateOnly<ReturnStack>					returnStacks;
				AllocateOnly<Trace>							traces;
				AllocateOnly<Competition>					competitions;
				AllocateOnly<AttendingCompetitions>			attendingCompetitions;
				AllocateOnly<Switches>						switches;

				collections::List<Trace*>					traces1;
				collections::List<Trace*>					traces2;

				Trace*										initialTrace = nullptr;
				Ref<Competition>							activeCompetitions;
				Ref<Switches>								rootSwitchValues;
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
				void										MergeTwoEndingInputTrace(Trace* newTrace, Trace* candidate);

				// Competition
				void										AttendCompetition(Trace* trace, Ref<AttendingCompetitions>& newAttendingCompetitions, Ref<AttendingCompetitions>& newCarriedCompetitions, Ref<ReturnStack> returnStack, vint32_t ruleId, vint32_t clauseId, bool forHighPriority);
				void										AttendCompetitionIfNecessary(Trace* trace, vint32_t currentTokenIndex, EdgeDesc& edgeDesc, Ref<AttendingCompetitions>& newAttendingCompetitions, Ref<AttendingCompetitions>& newCarriedCompetitions, Ref<ReturnStack>& newReturnStack);
				void										CheckAttendingCompetitionsOnEndingEdge(Trace* trace, EdgeDesc& edgeDesc, Ref<AttendingCompetitions> acId, Ref<ReturnStack> returnStack);
				void										CheckBackupTracesBeforeSwapping(vint32_t currentTokenIndex);

				// ReturnStack
				ReturnStackSuccessors*						GetCurrentSuccessorInReturnStack(Ref<ReturnStack> base, vint32_t currentTokenIndex);
				ReturnStack*								PushReturnStack(Ref<ReturnStack> base, vint32_t returnIndex, Ref<Trace> fromTrace, vint32_t currentTokenIndex, bool allowReuse);

				// Walk
				bool										IsQualifiedTokenForCondition(regex::RegexToken* token, StringLiteral condition);
				bool										IsQualifiedTokenForEdgeArray(regex::RegexToken* token, EdgeArray& edgeArray);
				Ref<Switches>								PushSwitchFrame(Switches* currentSV, vuint32_t* values);
				Ref<Switches>								RunEdgeConditionChecking(Ref<Switches> currentSwitchValues, EdgeDesc& edgeDesc);
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
				void										ReadInstructionList(Trace* trace, TraceInsLists& insLists);
				AstIns&										ReadInstruction(vint32_t instruction, TraceInsLists& insLists);

			protected:
				// PrepareTraceRoute
				AllocateOnly<TraceExec>						traceExecs;
				collections::Array<InsExec>					insExecs;
				AllocateOnly<InsExec_Object>				insExec_Objects;
				AllocateOnly<InsExec_InsRefLink>			insExec_InsRefLinks;
				AllocateOnly<InsExec_ObjRefLink>			insExec_ObjRefLinks;
				AllocateOnly<InsExec_ObjectStack>			insExec_ObjectStacks;
				AllocateOnly<InsExec_CreateStack>			insExec_CreateStacks;

				// phase: AllocateExecutionData
				void										AllocateExecutionData();

				// phase: PartialExecuteTraces - PartialExecuteOrdinaryTrace
				InsExec_Object*								NewObject();
				vint32_t									GetStackBase(InsExec_Context& context);
				vint32_t									GetStackTop(InsExec_Context& context);
				void										PushInsRefLink(Ref<InsExec_InsRefLink>& link, Ref<Trace> trace, vint32_t ins);
				void										PushObjRefLink(Ref<InsExec_ObjRefLink>& link, Ref<InsExec_Object> id);
				Ref<InsExec_InsRefLink>						JoinInsRefLink(Ref<InsExec_InsRefLink> first, Ref<InsExec_InsRefLink> second);
				Ref<InsExec_ObjRefLink>						JoinObjRefLink(Ref<InsExec_ObjRefLink> first, Ref<InsExec_ObjRefLink> second);
				InsExec_ObjectStack*						PushObjectStackSingle(InsExec_Context& context, Ref<InsExec_Object> objectId);
				InsExec_ObjectStack*						PushObjectStackMultiple(InsExec_Context& context, Ref<InsExec_ObjRefLink> linkId);
				InsExec_CreateStack*						PushCreateStack(InsExec_Context& context);
				void										PartialExecuteOrdinaryTrace(Trace* trace);

				// phase: PartialExecuteTraces - EnsureInsExecContextCompatible
				void										EnsureInsExecContextCompatible(Trace* baselineTrace, Trace* commingTrace);

				// phase: PartialExecuteTraces - MergeInsExecContext
				void										PushInsRefLinkWithCounter(Ref<InsExec_InsRefLink>& link, Ref<InsExec_InsRefLink> comming);
				void										PushObjRefLinkWithCounter(Ref<InsExec_ObjRefLink>& link, Ref<InsExec_ObjRefLink> comming);
				template<typename T, T* (TraceManager::*get)(Ref<T>), Ref<T> (InsExec_Context::*stack), typename TMerge>
				Ref<T>										MergeStack(Trace* mergeTrace, AllocateOnly<T>& allocator, TMerge&& merge);
				void										MergeInsExecContext(Trace* mergeTrace);

				// phase: PartialExecuteTraces
				void										PartialExecuteTraces();

				// phase: BuildAmbiguityStructures
				Trace*										StepForward(Trace* trace);
				void										BuildAmbiguityStructures();

#if defined VCZH_MSVC && defined _DEBUG
				// phase: DebugCheckTraceExecData
				void										DebugCheckTraceExecData();
#endif

			protected:
				// ResolveAmbiguity
				Ref<Trace>									firstBranchTrace;
				Ref<Trace>									firstMergeTrace;
				Ref<ExecutionStep>							firstStep;
				AllocateOnly<TraceAmbiguity>				traceAmbiguities;
				AllocateOnly<TraceAmbiguityLink>			traceAmbiguityLinks;
				AllocateOnly<ExecutionStep>					executionSteps;

				// phase: CheckMergeTraces
				template<typename TCallback>
				bool										SearchForObjects(Ref<InsExec_ObjRefLink> objRefLinkStartSet, bool withCounter, TCallback&& callback);
				template<typename TCallback>
				bool										SearchForAllLevelObjectsWithCounter(InsExec_Object* startObject, collections::List<Ref<InsExec_ObjRefLink>>& visitingIds, TCallback&& callback);
#if defined VCZH_MSVC && defined _DEBUG
				void										EnsureSameForwardTrace(Ref<Trace> currentTraceId, Ref<Trace> forwardTraceId);
#endif
				template<typename TCallback>
				bool										SearchForTopCreateInstructions(InsExec_Object* ieObject, TCallback&& callback);
				template<typename TCallback>
				bool										SearchForTopCreateInstructionsInAllLevelsWithCounter(InsExec_Object* startObject, collections::List<Ref<InsExec_ObjRefLink>>& visitingIds, TCallback&& callback);
				template<typename TCallback>
				bool										SearchForEndObjectInstructions(Trace* createTrace, vint32_t createIns, TCallback&& callback);
				bool										ComparePrefix(TraceExec* baselineTraceExec, TraceExec* commingTraceExec, vint32_t prefix);
				bool										ComparePostfix(TraceExec* baselineTraceExec, TraceExec* commingTraceExec, vint32_t postfix);
				template<typename TCallback>
				bool										CheckAmbiguityResolution(TraceAmbiguity* ta, collections::List<Ref<InsExec_ObjRefLink>>& visitingIds, TCallback&& callback);
				bool										CheckMergeTrace(TraceAmbiguity* ta, Trace* trace, TraceExec* traceExec, collections::List<Ref<InsExec_ObjRefLink>>& visitingIds);
				void										LinkAmbiguityCriticalTrace(Ref<Trace> traceId);
				void										CheckTraceAmbiguity(TraceAmbiguity* ta);
#if defined VCZH_MSVC && defined _DEBUG
				void										DebugCheckTraceAmbiguitiesInSameTrace(Trace* trace, TraceExec* traceExec);
#endif
				void										MarkAmbiguityCoveredForward(Trace* currentTrace, TraceAmbiguity* ta, Trace* firstTrace, TraceExec* firstTraceExec);
				void										CategorizeTraceAmbiguities(Trace* trace, TraceExec* traceExec);
				void										CheckMergeTraces();

				// phase: BuildExecutionOrder
				void										MarkNewLeafStep(ExecutionStep* step, ExecutionStep*& firstLeaf, ExecutionStep*& currentLeaf);
				void										AppendStepLink(ExecutionStep* first, ExecutionStep* last, bool leapNode, ExecutionStep*& root, ExecutionStep*& firstLeaf, ExecutionStep*& currentStep, ExecutionStep*& currentLeaf);
				void										AppendStepsBeforeAmbiguity(Trace* startTrace, vint32_t startIns, TraceAmbiguity* ta, ExecutionStep*& root, ExecutionStep*& firstLeaf, ExecutionStep*& currentStep, ExecutionStep*& currentLeaf);
				void										AppendStepsAfterAmbiguity(Trace*& startTrace, vint32_t& startIns, TraceAmbiguity* ta, ExecutionStep*& root, ExecutionStep*& firstLeaf, ExecutionStep*& currentStep, ExecutionStep*& currentLeaf);
				void										BuildStepTree(Trace* startTrace, vint32_t startIns, Trace* endTrace, vint32_t endIns, ExecutionStep*& root, ExecutionStep*& firstLeaf, ExecutionStep* currentStep, ExecutionStep*& currentLeaf);
				void										ConvertStepTreeToLink(ExecutionStep* root, ExecutionStep* firstLeaf, ExecutionStep*& first, ExecutionStep*& last);
				void										BuildAmbiguousStepLink(TraceAmbiguity* ta, ExecutionStep*& first, ExecutionStep*& last);
				void										BuildExecutionOrder();

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
				Switches*						GetSwitches(Ref<Switches> index);

				InsExec*						GetInsExec(vint32_t index);
				InsExec_Object*					GetInsExec_Object(Ref<InsExec_Object> index);
				InsExec_InsRefLink*				GetInsExec_InsRefLink(Ref<InsExec_InsRefLink> index);
				InsExec_ObjRefLink*				GetInsExec_ObjRefLink(Ref<InsExec_ObjRefLink> index);
				InsExec_ObjectStack*			GetInsExec_ObjectStack(Ref<InsExec_ObjectStack> index);
				InsExec_CreateStack*			GetInsExec_CreateStack(Ref<InsExec_CreateStack> index);
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
				void							ExecuteSingleTrace(TraceManagerSubmitter& submitter, Trace* trace, vint32_t firstIns, vint32_t lastIns, TraceInsLists& insLists, collections::List<regex::RegexToken>& tokens);
				void							ExecuteSingleStep(TraceManagerSubmitter& submitter, ExecutionStep* step, collections::List<regex::RegexToken>& tokens);
			public:
				Ptr<ParsingAstBase>				ExecuteTrace(IAstInsReceiver& receiver, collections::List<regex::RegexToken>& tokens) override;
			};
		}
	}
}

#endif
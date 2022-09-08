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
				AllocateOnly(vint _blockSize)
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

			struct Switches
			{
				vint32_t				allocatedIndex = -1;
				vint32_t				previous = -1;				// id of the previous Switches in the stack
				vint32_t				firstChild = -1;			// id of the first Swtiches that was ever pushed after the current one
				vint32_t				nextSibling = -1;			// id of the next Switches in all Switches that were ever pushed after the previous one
				vuint32_t				values[2] = { 0 };			// switch values, temporary set to 64 slots
			};

/***********************************************************************
TraceManager (Data Structures -- Input/EndOfInput)
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
				TraceCollection			predecessors;				// ids of predecessor Trace

				// (filled by EndOfInput)
				TraceCollection			successors;					// ids of successor Trace
				vint32_t				predecessorCount = 0;
				vint32_t				successorCount = 0;

				// if state == -1
				// it means this is an ambiguity resolving trace
				// all merged traces are in predecessors

				vint32_t				state = -1;					// id of the current StateDesc
				vint32_t				returnStack = -1;			// id of the current ReturnStack
				vint32_t				executedReturnStack = -1;	// id of the executed ReturnStack that contains the ReturnDesc being executed
				vint32_t				switchValues = -1;			// the id of switch values, it will be -1 if no switch is defined for this parser
				vint32_t				byEdge = -1;				// id of the last EdgeDesc that make this trace
				vint32_t				byInput = -1;				// the last input that make this trace
				vint32_t				currentTokenIndex = -1;		// the index of the token that is byInput
				CompetitionRouting		competitionRouting;			// a data structure carrying priority and competition information

				// (filled by PrepareTraceRoute)
				vint32_t				traceExecRef = -1;			// the allocated TraceExec
				vint32_t				iterateCounter = 0;			// a temporary counter for IterateSurvivedTraces internal use
			};

/***********************************************************************
TraceManager (Data Structures -- PrepareTraceRoute/ResolveAmbiguity)
***********************************************************************/

			struct InsExec_InsRefLink
			{
				vint32_t							allocatedIndex = -1;
				vint32_t							previous = -1;
				vint32_t							trace = -1;
				vint32_t							ins = -1;
			};

			struct InsExec_ObjRefLink
			{
				vint32_t							allocatedIndex = -1;
				vint32_t							previous = -1;
				vint32_t							id = -1;
			};

			struct InsExec_Context
			{
				vint32_t							objectStack = -1;		// InsExec_ObjectStack after executing instructions
				vint32_t							createStack = -1;		// InsExec_CreatedStack after executing instructions
				vint32_t							lriStoredObjects = -1;	// LriStore stored InsExec_ObjRefLink after executing instructions
			};

			struct InsExec
			{
				// BO/BOLR:
				//   the created object
				vint32_t							createdObjectId = -1;

				// InsExec_ObjRefLink
				// DFA:
				//   all associated objects
				// EO:
				//   all ended objects
				vint32_t							objRefs = -1;

				// InsExec_InsRefLink
				// BO/BOLR/DFA:
				//   EndingObject instructions that close objects or create stack created by the current instruction
				vint32_t							eoInsRefs = -1;

				// context before executing the current instruction
				InsExec_Context						contextBeforeExecution;

				vuint64_t							mergeCounter = 0;		// a temporary counter for MergeStack internal use
			};

			struct InsExec_Object
			{
				vint32_t							allocatedIndex = -1;

				// InsExec_ObjRefLink
				// lrObjectIds are objects it takes while being created by BOLR
				vint32_t							lrObjectIds = -1;

				// instruction that creates this object
				vint32_t							bo_bolr_Trace = -1;
				vint32_t							bo_bolr_Ins = -1;

				// InsExec_InsRefLink
				// DelayFieldAssignment instructions that associates to the current object
				vint32_t							dfaInsRefs = -1;

				vuint64_t							mergeCounter = 0;		// a temporary counter for MergeStack internal use
			};

			struct InsExec_ObjectStack
			{
				vint32_t							allocatedIndex = -1;
				vint32_t							previous = -1;
				vint32_t							objectIds = -1;			// InsExec_ObjRefLink
				vint32_t							pushedCount = -1;		// number for InsExec_CreateStack::stackBase

				vuint64_t							mergeCounter = 0;		// a temporary counter for MergeStack internal use
			};

			struct InsExec_CreateStack
			{
				vint32_t							allocatedIndex = -1;
				vint32_t							previous = -1;
				vint32_t							stackBase = -1;			// the number of objects in the object stack that is frozen

				// All InsExec_InsRefLink that create the current InsExec_CreateStack
				vint32_t							createInsRefs = -1;

				// InsExec_ObjRefLink assigned by BO/BOLA/RO
				vint32_t							objectIds = -1;

				vuint64_t							mergeCounter = 0;		// a temporary counter for MergeStack internal use
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

			struct TraceBranchData
			{
				// for ordinary trace, it stores the first trace of the most inner branch that this trace is in
				// for merge trace, it stores the latest trace that all comming branches share
				vint32_t							forwardTrace = -1;

				// the depth of nested branches
				// it is 0 for initialTrace
				vint32_t							branchDepth = -1;
			};

			struct TraceAmbiguity
			{
				vint32_t							allocatedIndex = -1;

				// InsExec_ObjRefLink containing all objects to resolve
				vint32_t							objectIdsToMerge = -1;

				// the trace where ambiguity resolution begins
				// prefix is the number of instructions before BO/DFA
				// if prefix + 1 is larger than instructions in firstTrace
				// then BO/DFA is in all successors
				vint32_t							firstTrace = -1;
				vint32_t							prefix = -1;

				// the trace when ambiguity resolution ends
				// postfix is the number of instructions after EO
				// if postfix + 1 is larger than instructions in lastTrace
				// then EO is in all predecessors
				vint32_t							lastTrace = -1;
				vint32_t							postfix = -1;
			};

			struct TraceExec
			{
				vint32_t							allocatedIndex = -1;
				vint32_t							traceId = -1;
				TraceInsLists						insLists;				// instruction list of this trace
				InstructionArray					insExecRefs;			// allocated InsExec for instructions

				InsExec_Context						context;
				TraceBranchData						branchData;

				// linked list of merge traces
				vint32_t							previousMergeTrace = -1;
				vint32_t							nextMergeTrace = -1;

				// TraceAmbiguity associated to the trace
				// it could be associated to
				//   firstTrace
				//   lastTrace
				//   the merge trace that create this TraceAmbiguity
				vint32_t							ambiguity = -1;
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
				vint32_t									activeCompetitions = -1;
				vint32_t									rootSwitchValues = -1;
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
				void										AttendCompetition(Trace* trace, vint32_t& newAttendingCompetitions, vint32_t& newCarriedCompetitions, vint32_t returnStack, vint32_t ruleId, vint32_t clauseId, bool forHighPriority);
				void										AttendCompetitionIfNecessary(Trace* trace, vint32_t currentTokenIndex, EdgeDesc& edgeDesc, vint32_t& newAttendingCompetitions, vint32_t& newCarriedCompetitions, vint32_t& newReturnStack);
				void										CheckAttendingCompetitionsOnEndingEdge(Trace* trace, EdgeDesc& edgeDesc, vint32_t acId, vint32_t returnStack);
				void										CheckBackupTracesBeforeSwapping(vint32_t currentTokenIndex);

				// ReturnStack
				ReturnStackSuccessors*						GetCurrentSuccessorInReturnStack(vint32_t base, vint32_t currentTokenIndex);
				ReturnStack*								PushReturnStack(vint32_t base, vint32_t returnIndex, vint32_t fromTrace, vint32_t currentTokenIndex, bool allowReuse);

				// Walk
				bool										IsQualifiedTokenForCondition(regex::RegexToken* token, StringLiteral condition);
				bool										IsQualifiedTokenForEdgeArray(regex::RegexToken* token, EdgeArray& edgeArray);
				vint32_t									PushSwitchFrame(Switches* currentSV, vuint32_t* values);
				vint32_t									RunEdgeConditionChecking(vint32_t currentSwitchValues, EdgeDesc& edgeDesc);
				WalkingTrace								WalkAlongSingleEdge(vint32_t currentTokenIndex, vint32_t input, WalkingTrace trace, vint32_t byEdge, EdgeDesc& edgeDesc);
				void										WalkAlongLeftrecEdges(vint32_t currentTokenIndex, regex::RegexToken* lookAhead, WalkingTrace trace, EdgeArray& edgeArray);
				void										WalkAlongEpsilonEdges(vint32_t currentTokenIndex, regex::RegexToken* lookAhead, WalkingTrace trace);
				void										WalkAlongTokenEdges(vint32_t currentTokenIndex, vint32_t input, regex::RegexToken* token, regex::RegexToken* lookAhead, WalkingTrace trace, EdgeArray& edgeArray);

				// EndOfInput
				void										FillSuccessorsAfterEndOfInput(bool& ambiguityInvolved);

			protected:
				// Common
				template<typename TCallback>
				void										IterateSurvivedTraces(TCallback&& callback);
				void										ReadInstructionList(Trace* trace, TraceInsLists& insLists);
				AstIns& ReadInstruction(vint32_t instruction, TraceInsLists& insLists);

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
				void										PushInsRefLink(vint32_t& link, vint32_t trace, vint32_t ins);
				void										PushObjRefLink(vint32_t& link, vint32_t id);
				vint32_t									JoinInsRefLink(vint32_t first, vint32_t second);
				vint32_t									JoinObjRefLink(vint32_t first, vint32_t second);
				InsExec_ObjectStack*						PushObjectStackSingle(InsExec_Context& context, vint32_t objectId);
				InsExec_ObjectStack*						PushObjectStackMultiple(InsExec_Context& context, vint32_t linkId);
				InsExec_CreateStack*						PushCreateStack(InsExec_Context& context);
				void										PartialExecuteOrdinaryTrace(Trace* trace);

				// phase: PartialExecuteTraces - EnsureInsExecContextCompatible
				void										EnsureInsExecContextCompatible(Trace* baselineTrace, Trace* commingTrace);

				// phase: PartialExecuteTraces - MergeInsExecContext
				void										PushInsRefLinkWithCounter(vint32_t& link, vint32_t comming);
				void										PushObjRefLinkWithCounter(vint32_t& link, vint32_t comming);
				template<typename T, T* (TraceManager::*get)(vint32_t), vint32_t (InsExec_Context::*stack), typename TMerge>
				vint32_t									MergeStack(Trace* mergeTrace, AllocateOnly<T>& allocator, TMerge&& merge);
				void										MergeInsExecContext(Trace* mergeTrace);

				// phase: PartialExecuteTraces
				void										PartialExecuteTraces();

				// phase: BuildAmbiguityStructures
				void										BuildAmbiguityStructures();

#if defined VCZH_MSVC && defined _DEBUG
				// phase: DebugCheckTraceExecData
				void										DebugCheckTraceExecData();
#endif

			protected:
				// ResolveAmbiguity
				vint32_t									firstMergeTrace = -1;
				vint32_t									lastMergeTrace = -1;
				AllocateOnly<TraceAmbiguity>				traceAmbiguities;

				// phase: CheckMergeTraces
				template<typename TCallback>
				void										SearchForTopObjectsWithCounter(vint32_t objRefLinkStartSet, collections::List<vint32_t>& visitingIds, TCallback&& callback);
				template<typename TCallback>
				void										SearchForTopCreateInstructions(InsExec_Object* ieObject, TCallback&& callback);
				template<typename TCallback>
				void										SearchForEndObjectInstructions(Trace* createTrace, vint32_t createIns, TCallback&& callback);
				bool										ComparePrefix(Trace* baselineTrace, Trace* commingTrace, vint32_t prefix);
				bool										ComparePostfix(Trace* baselineTrace, Trace* commingTrace, vint32_t postfix);
				template<typename TCallback>
				bool										CheckAmbiguityResolution(TraceAmbiguity* ta, TCallback&& callback);
				void										CheckMergeTrace(TraceAmbiguity* ta, Trace* trace, TraceExec* traceExec, collections::List<vint32_t>& visitingIds);
				void										CheckMergeTraces();
			public:
				TraceManager(Executable& _executable, const ITypeCallback* _typeCallback, vint blockSize);

				vint32_t						concurrentCount = 0;
				collections::List<Trace*>*		concurrentTraces = nullptr;
				collections::List<Trace*>*		backupTraces = nullptr;

				ReturnStack*					GetReturnStack(vint32_t index);
				ReturnStack*					AllocateReturnStack();
				Trace*							GetTrace(vint32_t index);
				Trace*							AllocateTrace();
				Competition*					GetCompetition(vint32_t index);
				Competition*					AllocateCompetition();
				AttendingCompetitions*			GetAttendingCompetitions(vint32_t index);
				AttendingCompetitions*			AllocateAttendingCompetitions();
				Switches*						GetSwitches(vint32_t index);
				InsExec*						GetInsExec(vint32_t index);
				InsExec_Object*					GetInsExec_Object(vint32_t index);
				InsExec_InsRefLink*				GetInsExec_InsRefLink(vint32_t index);
				InsExec_ObjRefLink*				GetInsExec_ObjRefLink(vint32_t index);
				InsExec_ObjectStack*			GetInsExec_ObjectStack(vint32_t index);
				InsExec_CreateStack*			GetInsExec_CreateStack(vint32_t index);
				TraceExec*						GetTraceExec(vint32_t index);
				TraceAmbiguity*					GetTraceAmbiguity(vint32_t index);

				void							Initialize(vint32_t startState) override;
				Trace*							GetInitialTrace();

				bool							Input(vint32_t currentTokenIndex, regex::RegexToken* token, regex::RegexToken* lookAhead) override;
				bool							EndOfInput(bool& ambiguityInvolved) override;

				void							PrepareTraceRoute() override;
				void							ResolveAmbiguity() override;
				Ptr<ParsingAstBase>				ExecuteTrace(IAstInsReceiver& receiver, collections::List<regex::RegexToken>& tokens) override;
			};
		}
	}
}

#endif
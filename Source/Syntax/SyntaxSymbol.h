/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_SYNTAX_SYNTAXSYMBOL
#define VCZH_PARSER2_SYNTAX_SYNTAXSYMBOL

#include "../ParserGen_Global/ParserSymbol.h"
#include "../SyntaxBase.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			class AstSymbolManager;
			class AstClassSymbol;
			class StateSymbol;
			class EdgeSymbol;
			class RuleSymbol;
			class SyntaxSymbolManager;
			class CompactSyntaxBuilder;

/***********************************************************************
StateSymbol
***********************************************************************/

			class StateSymbol : public Object
			{
				friend class EdgeSymbol;
				friend class SyntaxSymbolManager;
				friend class CompactSyntaxBuilder;

				using EdgeList = collections::List<EdgeSymbol*>;
			protected:
				SyntaxSymbolManager*		ownerManager;
				RuleSymbol*					rule;
				EdgeList					inEdges;
				EdgeList					outEdges;

				StateSymbol(RuleSymbol* _rule);
			public:
				WString						label;
				bool						endingState = false;

				SyntaxSymbolManager*		Owner() { return ownerManager; }
				RuleSymbol*					Rule() { return rule; }
				const EdgeList&				InEdges() { return inEdges; }
				const EdgeList&				OutEdges() { return outEdges; }

				void						GetOutEdgesInStableOrder(collections::List<StateSymbol*>& orderedStates, EdgeList& orderedEdges);
			};

/***********************************************************************
EdgeSymbol
***********************************************************************/

			enum class EdgeInputType
			{
				Epsilon,		// No input is needed to execute this edge.
				Ending,			// An epsilon edge that reduces the current rule.
				LeftRec,		// An epsilon edge that reduces the current rule, which is the first input of one of its left recursive clause.
				Token,			// An token is read to execute this edge.
				Rule,			// A rule is reduced to execute this edge.
			};

			struct EdgeInput
			{
				EdgeInputType						type = EdgeInputType::Epsilon;

				// Token
				vint32_t							token = -1;
				Nullable<WString>					condition;

				// Rule
				automaton::ReturnRuleType			ruleType = automaton::ReturnRuleType::Field;
				RuleSymbol*							rule = nullptr;

				auto operator<=>(const EdgeInput&) const = default;
			};

			struct EdgeCompetition
			{
				vint32_t					competitionId = -1;
				bool						highPriority = false;

				auto operator<=>(const EdgeCompetition&) const = default;
			};

			class EdgeSymbol : public Object
			{
				friend class SyntaxSymbolManager;
				friend class CompactSyntaxBuilder;

				using InsList = collections::List<AstIns>;
				using EdgeList = collections::List<EdgeSymbol*>;
				using CompetitionList = collections::List<EdgeCompetition>;
			protected:
				SyntaxSymbolManager*		ownerManager;
				StateSymbol*				fromState;
				StateSymbol*				toState;

				EdgeSymbol(StateSymbol* _from, StateSymbol* _to);
			public:
				EdgeInput					input;											// Input of this edge.
				CompetitionList				competitions;									// Competitions this edge involves in.
																							// (filled by BuildCompactNFA)
																							// If any important edge forms a cross referenced NFA edge, it becomes important too.
				InsList						insAfterInput;									// Instructions to execute after pushing the value from a token or a reduced rule.
				EdgeList					returnEdges;									// Edges of rule reduction.
																							// InsBeforeInput will be copied to a cross-referenced edge.
																							// When a reduction is done, only insAfterInput need to execute.

				SyntaxSymbolManager*		Owner() { return ownerManager; }
				StateSymbol*				From() { return fromState; }
				StateSymbol*				To() { return toState; }
			};

/***********************************************************************
RuleSymbol
***********************************************************************/

			class RuleSymbol : public Object
			{
				friend class SyntaxSymbolManager;

				using StateList = collections::List<StateSymbol*>;
			protected:
				SyntaxSymbolManager*		ownerManager;
				WString						name;

				RuleSymbol(SyntaxSymbolManager* _ownerManager, const WString& _name, vint _fileIndex);
			public:
				StateList					startStates;
				vint						fileIndex = -1;
				bool						isPublic = false;
				bool						isParser = false;
				bool						isPartial = false;
				bool						assignedNonArrayField = false;
				AstClassSymbol*				ruleType = nullptr;

				SyntaxSymbolManager*		Owner() { return ownerManager; }
				const WString&				Name() { return name; }
			};

/***********************************************************************
SyntaxSymbolManager
***********************************************************************/

			enum class SyntaxPhase
			{
				EpsilonNFA,					// An automaton that has edges of Epsilon, Token, Rule.

				CompactNFA,					// Epsilon edges are eliminated by compressing multiple edges into one.
											// Epsilon edges to the ending state will be compressed to an Ending edge.
											// The first edge of Rule in left-recursive clauses becomes a LeftRec edge, with its fromState changed to the ending state.
											// fromState and toState of non-LeftRec edges belong to the same clause.

				CrossReferencedNFA,			// Edges of Rule are compressed to an edge that pointing towards states in other clauses.
											// Multiple edges of rule are stored in returnEdges in the order of execution.
											// insBeforeInput of an edge contains insBeforeInput from its returnEdges.
											// returnEdges of an edge will be pushed to a stack when it is executed.
											// Executing an Ending edge pops a returnEdges and execute its insAfterInput only.
											// automaton::Executable is exactly the same to CrossReferencedNFA, stored a more cache friendly way.
			};

			struct PrefixMergeCache;

			class SyntaxSymbolManager : public Object
			{
				using StateList = collections::List<Ptr<StateSymbol>>;
				using EdgeList = collections::List<Ptr<EdgeSymbol>>;
				using StartEndStatePair = collections::Pair<StateSymbol*, StateSymbol*>;
			protected:
				MappedOwning<RuleSymbol>	rules;
				StateList					states;
				EdgeList					edges;
				ParserSymbolManager&		global;
				SyntaxPhase					phase = SyntaxPhase::EpsilonNFA;

			protected:
				struct IncrementalChange
				{
					StateList								createdStates;
					EdgeList								createdEdges;
					collections::SortedList<StateSymbol*>	reusedStates;
					collections::SortedList<EdgeSymbol*>	reusedEdges;
				};

				static StartEndStatePair		EliminateEpsilonEdges(RuleSymbol* rule, StateList& newStates, EdgeList& newEdges);
				static void						BuildLeftRecEdge(EdgeSymbol* newEdge, EdgeSymbol* endingEdge, EdgeSymbol* lrecPrefixEdge);
				static void						EliminateLeftRecursion(RuleSymbol* rule, StateSymbol* startState, StateSymbol* endState, StateList& newStates, EdgeList& newEdges);
				static void						MergeEdgesWithSameInput(RuleSymbol* rule, StateSymbol* startState, StateList& newStates, EdgeList& newEdges);

				static Ptr<PrefixMergeCache>	CreatePrefixMerge();
				static void						PrefixMergeCrossReference(PrefixMergeCache* cache, RuleSymbol* rule, StateSymbol* startState, StateList& newStates, EdgeList& newEdges);

				static void						ApplyIncrementalChange(const IncrementalChange& ic, StateList& newStates, EdgeList& newEdges);
				void							CheckIndirectLeftRecursion(StateSymbol* startState, collections::List<EdgeSymbol*>& accumulatedEdges);
				void							BuildCompactNFAInternal();

				void							FixCrossReferencedRuleEdge(StateSymbol* startState, collections::Group<StateSymbol*, EdgeSymbol*>& orderedEdges, collections::List<EdgeSymbol*>& accumulatedEdges);
				void							BuildCrossReferencedNFAInternal();

			public:
				SyntaxSymbolManager(ParserSymbolManager& _global);

				WString							name;
				vint32_t						usedCompetitionIds = 0;

				RuleSymbol*						CreateRule(const WString& name, vint fileIndex, bool isPublic, bool isParser, ParsingTextRange codeRange = {});
				void							RemoveRule(const WString& name);

				StateSymbol*					CreateState(RuleSymbol* rule);
				EdgeSymbol*						CreateEdge(StateSymbol* from, StateSymbol* to);

				void							BuildCompactNFA();
				void							BuildCrossReferencedNFA();
				void							BuildAutomaton(vint tokenCount, automaton::Executable& executable, automaton::Metadata& metadata);
				void							GetStatesInStableOrder(collections::List<StateSymbol*>& order);
				WString							GetStateGlobalLabel(StateSymbol* state, vint index);

				const ParserSymbolManager&		Global() const { return global; }
				const auto&						Rules() const { return rules.map; }
				const auto&						RuleOrder() { return rules.order; }
				SyntaxPhase						Phase() { return phase; }

				template<typename ...TArgs>
				void AddError(ParserErrorType type, ParsingTextRange codeRange, TArgs&&... args) const
				{
					global.AddError(type, { ParserDefFileType::Syntax,name,codeRange }, std::forward<TArgs&&>(args)...);
				}
			};

			extern void							CreateParserGenTypeSyntax(AstSymbolManager& ast, SyntaxSymbolManager& manager);
			extern void							CreateParserGenRuleSyntax(AstSymbolManager& ast, SyntaxSymbolManager& manager);
		}
	}
}

#endif
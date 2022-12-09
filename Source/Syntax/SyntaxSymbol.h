/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_SYNTAX_SYNTAXSYMBOL
#define VCZH_PARSER2_SYNTAX_SYNTAXSYMBOL

#include "../ParserGen/ParserSymbol.h"
#include "../SyntaxBase.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
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
				vint32_t					clauseId;
				EdgeList					inEdges;
				EdgeList					outEdges;

				StateSymbol(RuleSymbol* _rule, vint32_t _clauseId);
			public:
				WString						label;
				bool						endingState = false;

				SyntaxSymbolManager*		Owner() { return ownerManager; }
				RuleSymbol*					Rule() { return rule; }
				vint32_t					ClauseId() { return clauseId; }
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
				LrPlaceholder,	// A left recursion placeholder is consumed to execute this edge. EdgeInput::token is the index of SyntaxSymbolManager::lrpFlags.
				LrInject,		// A left recursion injection.
			};

			struct EdgeInput
			{
				EdgeInputType						type = EdgeInputType::Epsilon;
				vint32_t							token = -1;										// useful when type == Token
				Nullable<WString>					condition;										// useful when type == Token

				collections::SortedList<vint32_t>	flags;											// usefule when type == LrPlaceholder or LrInject

				automaton::ReturnRuleType			ruleType = automaton::ReturnRuleType::Field;	// useful when type == Rule or LrInject
				RuleSymbol*							rule = nullptr;									// useful when type == Rule or LrInject

				EdgeInput& operator=(EdgeInput& input)
				{
					type = input.type;
					token = input.token;
					condition = input.condition;
					CopyFrom(flags, input.flags);
					ruleType = input.ruleType;
					rule = input.rule;
					return *this;
				}
			};

			enum class EdgeImportancy
			{
				NoCompetition,
				HighPriority,
				LowPriority,
			};

			class EdgeSymbol : public Object
			{
				friend class SyntaxSymbolManager;
				friend class CompactSyntaxBuilder;

				using InsList = collections::List<AstIns>;
				using EdgeList = collections::List<EdgeSymbol*>;
			protected:
				SyntaxSymbolManager*		ownerManager;
				StateSymbol*				fromState;
				StateSymbol*				toState;

				EdgeSymbol(StateSymbol* _from, StateSymbol* _to);
			public:
				EdgeInput					input;											// Input of this edge.
				bool						important = false;								// true and false are the only two priorites of edges.
				EdgeImportancy				importancy = EdgeImportancy::NoCompetition;		// important -> HighPriority, !important with important sibling -> LowPriority.
																							// (filled by BuildCompactNFA)
																							// If any important edge forms a cross referenced NFA edge, it becomes important too.
				InsList						insBeforeInput;									// Instructions to execute before pushing the value from a token or a reduced rule.
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
				using NameList = collections::SortedList<WString>;
			protected:
				SyntaxSymbolManager*		ownerManager;
				WString						name;
				vint32_t					currentClauseId = -1;

				RuleSymbol(SyntaxSymbolManager* _ownerManager, const WString& _name);
			public:
				StateList					startStates;
				bool						isPartial = false;
				bool						assignedNonArrayField = false;
				AstClassSymbol*				ruleType = nullptr;
				NameList					lrFlags;

				SyntaxSymbolManager*		Owner() { return ownerManager; }
				const WString&				Name() { return name; }
				void						NewClause() { currentClauseId++; }
				vint32_t					CurrentClauseId() { return currentClauseId; }
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

			class SyntaxSymbolManager : public Object
			{
				using StateList = collections::List<Ptr<StateSymbol>>;
				using EdgeList = collections::List<Ptr<EdgeSymbol>>;
				using RuleTypeMap = collections::Dictionary<RuleSymbol*, WString>;
				using RuleList = collections::List<RuleSymbol*>;
				using LrpFlagList = collections::SortedList<WString>;
			protected:
				MappedOwning<RuleSymbol>	rules;
				StateList					states;
				EdgeList					edges;
				ParserSymbolManager&		global;
				SyntaxPhase					phase = SyntaxPhase::EpsilonNFA;

				void						BuildLeftRecEdge(EdgeSymbol* newEdge, EdgeSymbol* endingEdge, EdgeSymbol* lrecPrefixEdge);
				void						EliminateLeftRecursion(RuleSymbol* rule, StateSymbol* startState, StateSymbol* endState, StateList& newStates, EdgeList& newEdges);
				void						EliminateSingleRulePrefix(RuleSymbol* rule, StateSymbol* startState, StateSymbol* endState, StateList& newStates, EdgeList& newEdges);
				StateSymbol*				EliminateEpsilonEdges(RuleSymbol* rule, StateList& newStates, EdgeList& newEdges);
				void						BuildCompactNFAInternal();

				void						FixCrossReferencedRuleEdge(StateSymbol* startState, collections::Group<StateSymbol*, EdgeSymbol*>& orderedEdges, collections::List<EdgeSymbol*>& accumulatedEdges);
				void						FixLeftRecursionInjectEdge(StateSymbol* startState, EdgeSymbol* injectEdge);
				void						BuildCrossReferencedNFAInternal();
			public:
				SyntaxSymbolManager(ParserSymbolManager& _global);

				WString						name;
				RuleTypeMap					ruleTypes;
				RuleList					parsableRules;
				LrpFlagList					lrpFlags;

				RuleSymbol*					CreateRule(const WString& name, ParsingTextRange codeRange = {});
				void						RemoveRule(const WString& name);

				StateSymbol*				CreateState(RuleSymbol* rule, vint32_t clauseId);
				EdgeSymbol*					CreateEdge(StateSymbol* from, StateSymbol* to);

				void						BuildCompactNFA();
				void						BuildCrossReferencedNFA();
				void						BuildAutomaton(vint tokenCount, automaton::Executable& executable, automaton::Metadata& metadata);
				void						GetStatesInStableOrder(collections::List<StateSymbol*>& order);
				WString						GetStateGlobalLabel(StateSymbol* state, vint index);

				const ParserSymbolManager&	Global() const { return global; }
				const auto&					Rules() const { return rules.map; }
				const auto&					RuleOrder() { return rules.order; }
				SyntaxPhase					Phase() { return phase; }

				template<typename ...TArgs>
				void AddError(ParserErrorType type, ParsingTextRange codeRange, TArgs&&... args) const
				{
					global.AddError(type, { ParserDefFileType::Syntax,name,codeRange }, std::forward<TArgs&&>(args)...);
				}
			};

			extern void						CreateParserGenTypeSyntax(SyntaxSymbolManager& manager);
			extern void						CreateParserGenRuleSyntax(SyntaxSymbolManager& manager);
		}
	}
}

#endif
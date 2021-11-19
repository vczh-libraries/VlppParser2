/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_SYNTAX_SYNTAXSYMBOL
#define VCZH_PARSER2_SYNTAX_SYNTAXSYMBOL

#include "../ParserGen/ParserSymbol.h"
#include "../../../Source/SyntaxBase.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
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
			};

/***********************************************************************
EdgeSymbol
***********************************************************************/

			enum class EdgeInputType
			{
				Epsilon,
				Ending,
				LeftRec,
				Token,
				Rule,
			};

			struct EdgeInput
			{
				EdgeInputType				type = EdgeInputType::Epsilon;
				vint32_t					token = -1;
				RuleSymbol*					rule = nullptr;
			};

			class EdgeSymbol : public Object
			{
				friend class SyntaxSymbolManager;
				friend class CompactSyntaxBuilder;

				using InsList = collections::List<AstIns>;
			protected:
				SyntaxSymbolManager*		ownerManager;
				StateSymbol*				fromState;
				StateSymbol*				toState;

				EdgeSymbol(StateSymbol* _from, StateSymbol* _to);
			public:
				EdgeInput					input;
				InsList						insBeforeInput;
				InsList						insAfterInput;

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

				RuleSymbol(SyntaxSymbolManager* _ownerManager, const WString& _name);
			public:
				StateList					startStates;
				
				SyntaxSymbolManager*		Owner() { return ownerManager; }
				const WString&				Name() { return name; }
			};

/***********************************************************************
SyntaxSymbolManager
***********************************************************************/

			enum class SyntaxPhase
			{
				EpsilonNFA,
				CompactNFA,
				CrossReferencedNFA,
			};

			class SyntaxSymbolManager : public Object
			{
				using StateList = collections::List<Ptr<StateSymbol>>;
				using EdgeList = collections::List<Ptr<EdgeSymbol>>;
			protected:
				MappedOwning<RuleSymbol>	rules;
				StateList					states;
				EdgeList					edges;
				ParserSymbolManager&		global;
				SyntaxPhase					phase = SyntaxPhase::EpsilonNFA;

				void						EliminateLeftRecursion(RuleSymbol* rule, StateSymbol* startState, StateSymbol* endState, StateList& newStates, EdgeList& newEdges);
				StateSymbol*				EliminateEpsilonEdges(RuleSymbol* rule, StateList& newStates, EdgeList& newEdges);
				void						BuildCompactNFAInternal();

				void						BuildCrossReferencedNFAInternal();
			public:
				SyntaxSymbolManager(ParserSymbolManager& _global);

				RuleSymbol*					CreateRule(const WString& name);
				StateSymbol*				CreateState(RuleSymbol* rule);
				EdgeSymbol*					CreateEdge(StateSymbol* from, StateSymbol* to);

				void						BuildCompactNFA();
				void						BuildCrossReferencedNFA();

				ParserSymbolManager&		Global() { return global; }
				const auto&					Rules() { return rules.map; }
				const auto&					RuleOrder() { return rules.order; }
				SyntaxPhase					Phase() { return phase; }
			};

			extern void						CreateParserGenSyntax(SyntaxSymbolManager& manager);
		}
	}
}

#endif
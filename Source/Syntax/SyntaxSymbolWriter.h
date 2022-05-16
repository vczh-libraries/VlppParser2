/***********************************************************************
Author: Zihan Chen (vczh)
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#ifndef VCZH_PARSER2_SYNTAX_SYNTAXWRITER
#define VCZH_PARSER2_SYNTAX_SYNTAXWRITER

#include "SyntaxSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{

/***********************************************************************
AutomatonBuilder
***********************************************************************/

			class AutomatonBuilder
			{
			public:
				struct StatePair
				{
					StateSymbol* begin;
					StateSymbol* end;
				};

			protected:
				using StatePosMap = collections::Dictionary<StateSymbol*, vint>;
				using StateBuilder = Func<StatePair()>;
				using AssignmentBuilder = Func<StatePair(StatePair)>;

			protected:
				RuleSymbol*					ruleSymbol;

				WString						clauseDisplayText;
				StatePosMap					startPoses;
				StatePosMap					endPoses;

				StateSymbol* CreateState()
				{
					return ruleSymbol->Owner()->CreateState(ruleSymbol, ruleSymbol->CurrentClauseId());
				}

				EdgeSymbol* CreateEdge(StateSymbol* from, StateSymbol* to)
				{
					return ruleSymbol->Owner()->CreateEdge(from, to);
				}

				void						PrintCondition(collections::List<automaton::SwitchIns>& insSwitch);
				StatePair					BuildRuleSyntaxInternal(RuleSymbol* rule, vint32_t field, automaton::ReturnRuleType ruleType);
			public:
				AutomatonBuilder(RuleSymbol* _ruleSymbol);

				StatePair					BuildTokenSyntax(vint32_t tokenId, const WString& displayText, Nullable<WString> condition, vint32_t field);
				StatePair					BuildFieldRuleSyntax(RuleSymbol* rule, vint32_t field);
				StatePair					BuildPartialRuleSyntax(RuleSymbol* rule);
				StatePair					BuildDiscardRuleSyntax(RuleSymbol* rule);
				StatePair					BuildUseSyntax(RuleSymbol* rule);
				StatePair					BuildLoopSyntax(const StateBuilder& loopBody, const StateBuilder& loopDelimiter, bool hasDelimiter);
				StatePair					BuildOptionalSyntax(bool preferTake, bool preferSkip, const StateBuilder& optionalBody);
				StatePair					BuildSequenceSyntax(collections::List<StateBuilder>& elements);
				StatePair					BuildAlternativeSyntax(collections::List<StateBuilder>& elements);
				StatePair					BuildPushConditionSyntax(collections::Dictionary<vint32_t, bool>& switches, const StateBuilder& pushBody);
				StatePair					BuildTestConditionBranch(collections::List<automaton::SwitchIns>& insSwitch);
				StatePair					BuildTestConditionBranch(collections::List<automaton::SwitchIns>& insSwitch, const StateBuilder& branchBody);

				StatePair					BuildClause(const StateBuilder& compileSyntax);
				StatePair					BuildAssignment(StatePair pair, vint32_t enumItem, vint32_t field, bool weakAssignment);
				StatePair					BuildCreateClause(vint32_t classId, const StateBuilder& compileSyntax);
				StatePair					BuildPartialClause(const StateBuilder& compileSyntax);
				StatePair					BuildReuseClause(const StateBuilder& compileSyntax);

				StatePair					BuildLrpClause(collections::List<vint32_t>& flags, const Func<WString(vint32_t)>& flagName);
				StatePair					BuildLriSyntax(vint32_t flag, RuleSymbol* rule);
				StatePair					BuildLriSkip();
				StatePair					BuildLriContinuation(bool optional, collections::List<StateBuilder>&& continuations);
				StatePair					BuildLriClauseSyntax(StateBuilder useOrLriSyntax, bool optional, vint32_t flag, collections::List<RuleSymbol*>& targetRules);
			};

			namespace syntax_writer
			{

/***********************************************************************
Clause
***********************************************************************/

				struct Token
				{
					vint32_t				id;
					vint32_t				field;
					WString					display;
				};

				struct Rule
				{
					static const vint		Partial = -2;
					static const vint		Discard = -1;

					RuleSymbol*				rule;
					vint32_t				field;
				};

				struct Use
				{
					RuleSymbol*				rule;
				};

				template<typename T>
				struct Loop
				{
					T						body;
				};

				template<typename T, typename U>
				struct LoopSep
				{
					T						body;
					U						delimiter;
				};

				template<typename T>
				struct Opt
				{
					T						body;
				};

				template<typename T, typename U>
				struct Seq
				{
					T						first;
					U						second;
				};

				template<typename T, typename U>
				struct Alt
				{
					T						first;
					U						second;
				};

				template<typename T>
				struct With
				{
					T						body;
					vint32_t				field;
					vint32_t				enumItem;
				};

				template<typename T>
				struct Create
				{
					T						body;
					vint32_t				type;

					template<typename F, typename E>
					Create<With<T>> with(F field, E enumItem)
					{
						return { { body,(vint32_t)field,(vint32_t)enumItem }, type };
					}
				};

				template<typename T>
				struct Partial
				{
					T						body;

					template<typename F, typename E>
					Partial<With<T>> with(F field, E enumItem)
					{
						return { { body,(vint32_t)field,(vint32_t)enumItem } };
					}
				};

				template<typename T>
				struct Reuse
				{
					T						body;

					template<typename F, typename E>
					Reuse<With<T>> with(F field, E enumItem)
					{
						return { { body,(vint32_t)field,(vint32_t)enumItem } };
					}
				};

/***********************************************************************
Verification
***********************************************************************/

				template<typename T>
				struct IsClause_ { static constexpr bool Value = false; };

				template<>
				struct IsClause_<Token> { static constexpr bool Value = true; };

				template<>
				struct IsClause_<Rule> { static constexpr bool Value = true; };

				template<>
				struct IsClause_<Use> { static constexpr bool Value = true; };

				template<typename T>
				struct IsClause_<Loop<T>> { static constexpr bool Value = IsClause_<T>::Value; };

				template<typename T, typename U>
				struct IsClause_<LoopSep<T, U>> { static constexpr bool Value = IsClause_<T>::Value && IsClause_<U>::Value; };

				template<typename T>
				struct IsClause_<Opt<T>> { static constexpr bool Value = IsClause_<T>::Value; };

				template<typename T, typename U>
				struct IsClause_<Seq<T, U>> { static constexpr bool Value = IsClause_<T>::Value && IsClause_<U>::Value; };

				template<typename T, typename U>
				struct IsClause_<Alt<T, U>> { static constexpr bool Value = IsClause_<T>::Value && IsClause_<U>::Value; };

				template<typename T>
				struct IsClause_<With<T>> { static constexpr bool Value = IsClause_<T>::Value; };

				template<typename T>
				struct IsClause_<Create<T>> { static constexpr bool Value = IsClause_<T>::Value; };

				template<typename T>
				struct IsClause_<Partial<T>> { static constexpr bool Value = IsClause_<T>::Value; };

				template<typename T>
				struct IsClause_<Reuse<T>> { static constexpr bool Value = IsClause_<T>::Value; };

				template<typename T>
				constexpr bool IsClause = IsClause_<T>::Value;

/***********************************************************************
Operators
***********************************************************************/

				template<typename T>
				inline Token tok(T id, const WString& display)
				{
					return { (vint32_t)id,-1,display };
				}

				template<typename T, typename F>
				inline Token tok(T id, const WString& display, F field = -1)
				{
					return { (vint32_t)id,(vint32_t)field,display };
				}

				inline Rule drule(RuleSymbol* r)
				{
					CHECK_ERROR(!r->isPartial, L"vl::glr::parsergen::syntax_writer::drule(RuleSymbol*)#Rule should not be a partial rule.");
					return { r,Rule::Discard };
				}

				inline Rule prule(RuleSymbol* r)
				{
					CHECK_ERROR(r->isPartial, L"vl::glr::parsergen::syntax_writer::prule(RuleSymbol*)#Rule should be a partial rule.");
					return { r,Rule::Partial };
				}

				template<typename F>
				inline Rule rule(RuleSymbol* r, F field)
				{
					CHECK_ERROR(!r->isPartial, L"vl::glr::parsergen::syntax_writer::rule(RuleSymbol*, F)#Rule should not be a partial rule.");
					return { r,(vint32_t)field };
				}

				inline Use use(RuleSymbol* r)
				{
					CHECK_ERROR(!r->isPartial, L"vl::glr::parsergen::syntax_writer::use(RuleSymbol*)#Rule should not be a partial rule.");
					return { r };
				}

				template<typename C>
				inline std::enable_if_t<IsClause<C>, Loop<C>> loop(const C& clause)
				{
					return { clause };
				}

				template<typename C1, typename C2>
				inline std::enable_if_t<IsClause<C1> && IsClause<C2>, LoopSep<C1, C2>> loop(const C1& c1, const C2& c2)
				{
					return { c1,c2 };
				}

				template<typename C>
				inline std::enable_if_t<IsClause<C>, Opt<C>> opt(const C& clause)
				{
					return { clause };
				}

				template<typename C1, typename C2>
				inline std::enable_if_t<IsClause<C1> && IsClause<C2>, Seq<C1, C2>> operator+(const C1& c1, const C2& c2)
				{
					return { c1,c2 };
				}

				template<typename C1, typename C2>
				inline std::enable_if_t<IsClause<C1>&& IsClause<C2>, Alt<C1, C2>> operator|(const C1& c1, const C2& c2)
				{
					return { c1,c2 };
				}

				template<typename C, typename T>
				inline std::enable_if_t<IsClause<C>, Create<C>> create(const C& clause, T type)
				{
					return { clause,(vint32_t)type };
				}

				template<typename C>
				inline std::enable_if_t<IsClause<C>, Partial<C>> partial(const C& clause)
				{
					return { clause };
				}

				template<typename C>
				inline std::enable_if_t<IsClause<C>, Reuse<C>> reuse(const C& clause)
				{
					return { clause };
				}

/***********************************************************************
Builder
***********************************************************************/

				struct Clause
				{
					using StatePair = AutomatonBuilder::StatePair;
				private:
					AutomatonBuilder				builder;

					StatePair Build(const Token& clause)
					{
						return builder.BuildTokenSyntax(clause.id, clause.display, {}, clause.field);
					}

					StatePair Build(const Rule& clause)
					{
						switch (clause.field)
						{
						case Rule::Partial:
							return builder.BuildPartialRuleSyntax(clause.rule);
						case Rule::Discard:
							return builder.BuildDiscardRuleSyntax(clause.rule);
						default:
							return builder.BuildFieldRuleSyntax(clause.rule, clause.field);
						}
					}

					StatePair Build(const Use& clause)
					{
						return builder.BuildUseSyntax(clause.rule);
					}

					template<typename C>
					StatePair Build(const Loop<C>& clause)
					{
						return builder.BuildLoopSyntax(
							[this, &clause]() { return Build(clause.body); },
							{},
							false
							);
					}

					template<typename C1, typename C2>
					StatePair Build(const LoopSep<C1, C2>& clause)
					{
						return builder.BuildLoopSyntax(
							[this, &clause]() { return Build(clause.body); },
							[this, &clause]() { return Build(clause.delimiter); },
							true
							);
					}

					template<typename C>
					StatePair Build(const Opt<C>& clause)
					{
						return builder.BuildOptionalSyntax(
							false,
							false,
							[this, &clause]() { return Build(clause.body); }
							);
					}

					template<typename C>
					void CollectSeq(const C& clause, collections::List<Func<StatePair()>>& elements)
					{
						elements.Add([this, &clause] { return Build(clause); });
					}

					template<typename C1, typename C2>
					void CollectSeq(const Seq<C1, C2>& clause, collections::List<Func<StatePair()>>& elements)
					{
						CollectSeq(clause.first, elements);
						CollectSeq(clause.second, elements);
					}

					template<typename C1, typename C2>
					StatePair Build(const Seq<C1, C2>& clause)
					{
						collections::List<Func<StatePair()>> elements;
						CollectSeq(clause, elements);
						return builder.BuildSequenceSyntax(elements);
					}

					template<typename C>
					void CollectAlt(const C& clause, collections::List<Func<StatePair()>>& elements)
					{
						elements.Add([this, &clause] { return Build(clause); });
					}

					template<typename C1, typename C2>
					void CollectAlt(const Alt<C1, C2>& clause, collections::List<Func<StatePair()>>& elements)
					{
						CollectAlt(clause.first, elements);
						CollectAlt(clause.second, elements);
					}

					template<typename C1, typename C2>
					StatePair Build(const Alt<C1, C2>& clause)
					{
						collections::List<Func<StatePair()>> elements;
						CollectAlt(clause, elements);
						return builder.BuildAlternativeSyntax(elements);
					}

					template<typename C>
					StatePair Build(const With<C>& clause)
					{
						return builder.BuildAssignment(Build(clause.body), clause.enumItem, clause.field, false);
					}

					template<typename C>
					StatePair Build(const Create<C>& clause)
					{
						return builder.BuildCreateClause(
							clause.type,
							[this, &clause]() { return Build(clause.body); }
							);
					}

					template<typename C>
					StatePair Build(const Partial<C>& clause)
					{
						return builder.BuildPartialClause(
							[this, &clause]() { return Build(clause.body); }
							);
					}

					template<typename C>
					StatePair Build(const Reuse<C>& clause)
					{
						return builder.BuildReuseClause(
							[this, &clause]() { return Build(clause.body); }
							);
					}

					template<typename C>
					void Assign(const C& clause)
					{
						builder.BuildClause(
							[this, &clause]() { return Build(clause); }
							);
					}
				public:
					Clause(RuleSymbol* _ruleSymbol) : builder(_ruleSymbol) {}

					template<typename C>
					Clause& operator=(const Create<C>& clause)
					{
						Assign(clause);
						return *this;
					}

					template<typename C>
					Clause& operator=(const Partial<C>& clause)
					{
						Assign(clause);
						return *this;
					}

					template<typename C>
					Clause& operator=(const Reuse<C>& clause)
					{
						Assign(clause);
						return *this;
					}

					template<typename C>
					Clause& operator=(const C& clause)
					{
						return operator=(reuse(clause));
					}
				};
			}
		}
	}
}

#endif
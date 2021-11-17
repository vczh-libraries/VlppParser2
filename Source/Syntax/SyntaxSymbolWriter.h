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

				template<typename T>
				struct With
				{
					T						body;
					vint32_t				field;
					vint32_t				enumItem;

					template<typename F, typename E>
					With<With<T>> with(F field, E enumItem)
					{
						return{ *this,(vint32_t)field,(vint32_t)enumItem };
					}
				};

				template<typename T>
				struct Create
				{
					T						body;
					vint32_t				type;

					template<typename F, typename E>
					With<Create<T>> with(F field, E enumItem)
					{
						return{ *this,(vint32_t)field,(vint32_t)enumItem };
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

				template<typename T>
				struct IsClause_<Create<T>> { static constexpr bool Value = IsClause_<T>::Value; };

				template<typename T>
				struct IsClause_<With<T>> { static constexpr bool Value = IsClause_<T>::Value; };

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

				inline Rule rule(RuleSymbol* r)
				{
					return { r };
				}

				template<typename F>
				inline Rule rule(RuleSymbol* r, F field)
				{
					return { r,(vint32_t)field };
				}

				inline Use use(RuleSymbol* r)
				{
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

				template<typename C, typename T>
				inline std::enable_if_t<IsClause<C>, Create<C>> create(const C& clause, T type)
				{
					return { clause,(vint32_t)type };
				}

/***********************************************************************
Builder
***********************************************************************/

				struct Clause
				{
					using StatePosMap = collections::Dictionary<StateSymbol*, vint>;
				private:
					struct StatePair
					{
						StateSymbol*			begin;
						StateSymbol*			end;
					};

					RuleSymbol*					ruleSymbol;
					WString						clauseDisplayText;
					StatePosMap					startPoses;
					StatePosMap					endPoses;

					StateSymbol* CreateState()
					{
						return ruleSymbol->Owner()->CreateState(ruleSymbol);
					}

					EdgeSymbol* CreateEdge(StateSymbol* from, StateSymbol* to)
					{
						return ruleSymbol->Owner()->CreateEdge(from, to);
					}

					StatePair Build(const Token& clause)
					{
						StatePair pair;
						pair.begin = CreateState();
						pair.end = CreateState();
						startPoses.Add(pair.begin, clauseDisplayText.Length());

						auto edge = CreateEdge(pair.begin, pair.end);
						edge->input.type = EdgeInputType::Token;
						edge->input.token = clause.id;
						if (clause.field != -1)
						{
							edge->insAfter.Add({ AstInsType::Field,clause.field });
						}

						clauseDisplayText += clause.display;
						endPoses.Add(pair.end, clauseDisplayText.Length());
						return pair;
					}

					StatePair Build(const Rule& clause)
					{
						StatePair pair;
						pair.begin = CreateState();
						pair.end = CreateState();
						startPoses.Add(pair.begin, clauseDisplayText.Length());

						auto edge = CreateEdge(pair.begin, pair.end);
						edge->input.type = EdgeInputType::Rule;
						edge->input.rule = clause.rule;
						if (clause.field != -1)
						{
							edge->insAfter.Add({ AstInsType::Field,clause.field });
						}

						clauseDisplayText += clause.rule->Name();
						endPoses.Add(pair.end, clauseDisplayText.Length());
						return pair;
					}

					StatePair Build(const Use& clause)
					{
						StatePair pair;
						pair.begin = CreateState();
						pair.end = CreateState();
						startPoses.Add(pair.begin, clauseDisplayText.Length());

						auto edge = CreateEdge(pair.begin, pair.end);
						edge->input.type = EdgeInputType::Rule;
						edge->input.rule = clause.rule;
						edge->insAfter.Add({ AstInsType::ReopenObject });

						clauseDisplayText += L"!" + clause.rule->Name();
						endPoses.Add(pair.end, clauseDisplayText.Length());
						return pair;
					}

					template<typename C>
					StatePair Build(const Loop<C>& clause)
					{
						StatePair pair;
						pair.begin = CreateState();
						pair.end = CreateState();
						startPoses.Add(pair.begin, clauseDisplayText.Length());

						clauseDisplayText += L"{ ";
						auto bodyPair = Build(clause.body);
						clauseDisplayText += L" }";

						CreateEdge(pair.begin, bodyPair.begin);
						CreateEdge(bodyPair.end, pair.end);
						CreateEdge(pair.end, pair.begin);

						endPoses.Add(pair.end, clauseDisplayText.Length());
						return pair;
					}

					template<typename C1, typename C2>
					StatePair Build(const LoopSep<C1, C2>& clause)
					{
						throw 0;
					}

					template<typename C>
					StatePair Build(const Opt<C>& clause)
					{
						throw 0;
					}

					template<typename C1, typename C2>
					StatePair Build(const Seq<C1, C2>& clause)
					{
						throw 0;
					}

					template<typename C>
					StatePair Build(const Create<C>& clause)
					{
						throw 0;
					}

					template<typename C>
					StatePair Build(const With<C>& clause)
					{
						throw 0;
					}
				public:
					Clause(RuleSymbol* _ruleSymbol) : ruleSymbol(_ruleSymbol) {}

					template<typename C>
					std::enable_if_t<IsClause<C>, Clause&> operator=(const C& clause)
					{
						auto pair = Build(clause);
						ruleSymbol->startStates.Add(pair.begin);
						return *this;
					}
				};
			}
		}
	}
}

#endif
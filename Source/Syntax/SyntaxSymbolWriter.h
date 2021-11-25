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

				template<typename T>
				struct IsClause_<With<T>> { static constexpr bool Value = IsClause_<T>::Value; };

				template<typename T>
				struct IsClause_<Create<T>> { static constexpr bool Value = IsClause_<T>::Value; };

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

						{
							auto edge = CreateEdge(pair.begin, pair.end);
							edge->input.type = EdgeInputType::Token;
							edge->input.token = clause.id;
							if (clause.field != -1)
							{
								edge->insAfterInput.Add({ AstInsType::Token });
								edge->insAfterInput.Add({ AstInsType::Field,clause.field });
							}
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

						{
							auto edge = CreateEdge(pair.begin, pair.end);
							edge->input.type = EdgeInputType::Rule;
							edge->input.rule = clause.rule;
							if (clause.field != -1)
							{
								edge->insAfterInput.Add({ AstInsType::Field,clause.field });
							}
							else
							{
								edge->insAfterInput.Add({ AstInsType::DiscardValue });
							}
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

						{
							auto edge = CreateEdge(pair.begin, pair.end);
							edge->input.type = EdgeInputType::Rule;
							edge->input.rule = clause.rule;
							edge->insAfterInput.Add({ AstInsType::ReopenObject });
						}

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
						CreateEdge(pair.begin, pair.end);
						CreateEdge(bodyPair.end, bodyPair.begin);

						endPoses.Add(pair.end, clauseDisplayText.Length());
						return pair;
					}

					template<typename C1, typename C2>
					StatePair Build(const LoopSep<C1, C2>& clause)
					{
						return Build(opt(clause.body + loop(clause.delimiter + clause.body)));
					}

					template<typename C>
					StatePair Build(const Opt<C>& clause)
					{
						StatePair pair;
						pair.begin = CreateState();
						pair.end = CreateState();
						startPoses.Add(pair.begin, clauseDisplayText.Length());

						clauseDisplayText += L"[ ";
						auto bodyPair = Build(clause.body);
						clauseDisplayText += L" ]";

						CreateEdge(pair.begin, bodyPair.begin);
						CreateEdge(bodyPair.end, pair.end);
						CreateEdge(pair.begin, pair.end);

						endPoses.Add(pair.end, clauseDisplayText.Length());
						return pair;
					}

					template<typename C1, typename C2>
					StatePair Build(const Seq<C1, C2>& clause)
					{
						auto firstPair = Build(clause.first);
						clauseDisplayText += L" ";
						auto secondPair = Build(clause.second);
						CreateEdge(firstPair.end, secondPair.begin);
						return { firstPair.begin,secondPair.end };
					}

					template<typename C>
					StatePair Build(const With<C>& clause)
					{
						auto pair = Build(clause.body);
						CHECK_ERROR(pair.end->InEdges().Count() == 1, L"Internal error!");
						auto edge = pair.end->InEdges()[0];
						edge->insBeforeInput.Add({ AstInsType::EnumItem,clause.enumItem });
						edge->insBeforeInput.Add({ AstInsType::Field,clause.field });
						return pair;
					}

					template<typename C>
					StatePair Build(const Create<C>& clause)
					{
						StatePair pair;
						pair.begin = CreateState();
						pair.end = CreateState();
						startPoses.Add(pair.begin, clauseDisplayText.Length());

						clauseDisplayText += L"< ";
						auto bodyPair = Build(clause.body);
						clauseDisplayText += L" >";
						{
							auto edge = CreateEdge(pair.begin, bodyPair.begin);
							edge->insBeforeInput.Add({ AstInsType::BeginObject,clause.type });
						}
						{
							auto edge = CreateEdge(bodyPair.end, pair.end);
							edge->insBeforeInput.Add({ AstInsType::EndObject });
						}
						endPoses.Add(pair.end, clauseDisplayText.Length());
						return pair;
					}

					template<typename C>
					StatePair Build(const Reuse<C>& clause)
					{
						StatePair pair;
						pair.begin = CreateState();
						pair.end = CreateState();
						startPoses.Add(pair.begin, clauseDisplayText.Length());

						clauseDisplayText += L"<< ";
						auto bodyPair = Build(clause.body);
						clauseDisplayText += L" >>";
						CreateEdge(pair.begin, bodyPair.begin);
						{
							auto edge = CreateEdge(bodyPair.end, pair.end);
							edge->insBeforeInput.Add({ AstInsType::EndObject });
						}
						endPoses.Add(pair.end, clauseDisplayText.Length());
						return pair;
					}

					template<typename C>
					void Assign(const C& clause)
					{
						clauseDisplayText = L"";
						startPoses.Clear();
						endPoses.Clear();

						auto pair = Build(clause);
						ruleSymbol->startStates.Add(pair.begin);
						pair.end->endingState = true;

						vint l = clauseDisplayText.Length();
						for (auto [state, pos] : startPoses)
						{
							state->label = clauseDisplayText.Left(pos) + L"@ " + clauseDisplayText.Right(l - pos);
						}
						for (auto [state, pos] : endPoses)
						{
							state->label = clauseDisplayText.Left(pos) + L" @" + clauseDisplayText.Right(l - pos);
						}
					}
				public:
					Clause(RuleSymbol* _ruleSymbol) : ruleSymbol(_ruleSymbol) {}

					template<typename C>
					Clause& operator=(const Create<C>& clause)
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
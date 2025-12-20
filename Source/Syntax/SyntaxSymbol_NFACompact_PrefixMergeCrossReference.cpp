#include "SyntaxSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;

/***********************************************************************
BitSet
***********************************************************************/

			struct BitSet
			{
			public:
				static const BitSet		Zero;

			private:
				vint					wordCount = 0;
				vuint64_t*				words = nullptr;

				static bool AllZero(vuint64_t* words, vint wordCount)
				{
					for (vint i = 0; i < wordCount; i++)
					{
						if (words[i] != 0) return false;
					}
					return true;
				}
			public:
				BitSet& operator=(const BitSet& bs) noexcept
				{
					if (this != &bs)
					{
						wordCount = bs.wordCount;
						if (words)
						{
							delete[] words;
							words = nullptr;
						}
						if (bs.words)
						{
							words = new vuint64_t[wordCount];
							memcpy(words, bs.words, sizeof(vuint64_t) * wordCount);
						}
					}
					return *this;
				}

				BitSet& operator=(BitSet&& bs) noexcept
				{
					if (this != &bs)
					{
						wordCount = bs.wordCount;
						if (words)
						{
							delete[] words;
						}
						words = bs.words;
						bs.wordCount = 0;
						bs.words = nullptr;
					}
					return *this;
				}

				BitSet() = default;
				BitSet(const BitSet& bs) noexcept { *this = bs; }
				BitSet(BitSet&& bs) noexcept { *this = std::move(bs); }
				~BitSet() { if (words) delete[] words; }

				auto operator<=>(const BitSet& bs) const
				{
					if (this == &bs) return std::strong_ordering::equal;
					vint minWordCount = wordCount < bs.wordCount ? wordCount : bs.wordCount;
					if (minWordCount > 0)
					{
						auto result = memcmp(words, bs.words, sizeof(vuint64_t) * minWordCount) <=> 0;
						if (result != 0) return result;
					}

					auto result = wordCount <=> bs.wordCount;
					if (result < 0)
					{
						return AllZero(bs.words + minWordCount, bs.wordCount - minWordCount) ? std::strong_ordering::equal : std::strong_ordering::less;
					}
					else if (result > 0)
					{
						return AllZero(words + minWordCount, wordCount - minWordCount) ? std::strong_ordering::equal : std::strong_ordering::greater;
					}
					else
					{
						return result;
					}
				}
				bool operator==(const BitSet& bs) const
				{
					return (*this <=> bs) == 0;
				}

				bool operator[](vint index) const
				{
					vint wordIndex = index / 64;
					vint bitIndex = index % 64;
					if (wordIndex >= wordCount)
					{
						return false;
					}
					return (words[wordIndex] & (vuint64_t(1) << bitIndex)) != 0;
				}

				void Set(vint index)
				{
					vint wordIndex = index / 64;
					vint bitIndex = index % 64;
					if (wordIndex >= wordCount)
					{
						vint oldWordCount = wordCount;
						wordCount = wordIndex + 1;
						vuint64_t* newWords = new vuint64_t[wordCount];
						memset(newWords, 0, sizeof(vuint64_t) * wordCount);
						if (words)
						{
							memcpy(newWords, words, sizeof(vuint64_t) * oldWordCount);
							delete[] words;
						}
						words = newWords;
					}
					words[wordIndex] |= (vuint64_t(1) << bitIndex);
				}

				operator bool() const
				{
					return !AllZero(words, wordCount);
				}

				BitSet operator|(const BitSet& bs) const
				{
					if (this == &bs) return *this;
					BitSet result;
					vint maxWordCount = wordCount > bs.wordCount ? wordCount : bs.wordCount;
					if (maxWordCount > 0)
					{
						result.wordCount = maxWordCount;
						result.words = new vuint64_t[maxWordCount];
						for (vint i = 0; i < maxWordCount; i++)
						{
							vuint64_t w1 = i < wordCount ? words[i] : 0;
							vuint64_t w2 = i < bs.wordCount ? bs.words[i] : 0;
							result.words[i] = w1 | w2;
						}
					}
					return result;
				}

				BitSet& operator|=(const BitSet& bs)
				{
					if (this == &bs) return *this;
					*this = *this | bs;
					return *this;
				}

				BitSet operator&(const BitSet& bs) const
				{
					if (this == &bs) return *this;
					BitSet result;
					vint minWordCount = wordCount < bs.wordCount ? wordCount : bs.wordCount;
					if (minWordCount > 0)
					{
						result.wordCount = minWordCount;
						result.words = new vuint64_t[minWordCount];
						for (vint i = 0; i < minWordCount; i++)
						{
							result.words[i] = words[i] & bs.words[i];
						}
					}
					return result;
				}

				BitSet& operator&=(const BitSet& bs)
				{
					if (this == &bs) return *this;
					*this = *this & bs;
					return *this;
				}

				BitSet& operator-=(const BitSet& bs)
				{
					if (this == &bs) 
					{
						*this = Zero;
					}
					else
					{
						vint minWordCount = wordCount < bs.wordCount ? wordCount : bs.wordCount;
						for (vint i = 0; i < minWordCount; i++)
						{
							words[i] &= ~bs.words[i];
						}
					}
					return *this;
				}
			};

			const BitSet BitSet::Zero;

/***********************************************************************
SyntaxSymbolManager::CreatePrefixMerge
***********************************************************************/

			struct PrefixMergeCache
			{
				// prepared by CreatePrefixMergeCache
				Array<RuleSymbol*>		rules;
				Array<RuleSymbol*>		rulesByDeps; // if a begins with b, a is before b
				Array<BitSet>			directStartSetTokens, startSetTokens;
				Array<BitSet>			directStartSetRules, startSetRules, reverseStartSetRules;
			};

			Ptr<PrefixMergeCache> SyntaxSymbolManager::CreatePrefixMergeCache()
			{
				auto cache = Ptr(new PrefixMergeCache);
				CopyFrom(cache->rules, rules.map.Values());
				const vint ruleCount = cache->rules.Count();

				cache->rulesByDeps.Resize(ruleCount);
				cache->directStartSetTokens.Resize(ruleCount);
				cache->startSetTokens.Resize(ruleCount);
				cache->directStartSetRules.Resize(ruleCount);
				cache->startSetRules.Resize(ruleCount);
				cache->reverseStartSetRules.Resize(ruleCount);
				for (auto [ruleSymbol, index] : indexed(cache->rules))
				{
					ruleSymbol->pmRuleIndex = index;
				}

				PartialOrderingProcessor pop;
				{
					Group<RuleSymbol*, RuleSymbol*> deps;
					for (auto ruleSymbol : cache->rules)
					{
						auto startState = ruleSymbol->startStates[0];
						BitSet directTokens, directRules;
						for (auto edge : startState->OutEdges())
						{
							switch (edge->input.type)
							{
							case EdgeInputType::Token:
								directTokens.Set(edge->input.token);
								break;
							case EdgeInputType::Rule:
								directRules.Set(edge->input.rule->pmRuleIndex);
								deps.Add(ruleSymbol, edge->input.rule);
								break;
							default:;
							}
						}
						cache->directStartSetTokens[ruleSymbol->pmRuleIndex] = directTokens;
						cache->directStartSetRules[ruleSymbol->pmRuleIndex] = directRules;
					}
					pop.InitWithGroup(cache->rules, deps);
					pop.Sort();
				}

				for (auto component : pop.components)
				{
					if (component.nodeCount > 1)
					{
						AddError(
							ParserErrorType::RuleIsIndirectlyLeftRecursive,
							{},
							From(Range<vint>(0, component.nodeCount))
							.Select([&](vint index) { return cache->rules[component.firstNode[index]]->Name(); })
							.OrderBySelf()
							.Aggregate([](auto&& a, auto&& b) { return a + L", " + b; })
						);
					}
				}
				if (global.Errors().Count() > 0) return nullptr;

				for (auto [component, index] : indexed(pop.components))
				{
					auto ruleSymbol = cache->rules[*component.firstNode];
					auto startSetTokens = cache->directStartSetTokens[ruleSymbol->pmRuleIndex];
					auto startSetRules = cache->directStartSetRules[ruleSymbol->pmRuleIndex];
					cache->rulesByDeps[ruleCount - 1 - index] = ruleSymbol;

					auto startState = ruleSymbol->startStates[0];
					for (auto edge : startState->OutEdges())
					{
						if (edge->input.type != EdgeInputType::Rule) continue;
						startSetTokens |= cache->startSetTokens[edge->input.rule->pmRuleIndex];
						startSetRules |= cache->startSetRules[edge->input.rule->pmRuleIndex];
					}
					cache->startSetTokens[ruleSymbol->pmRuleIndex] = startSetTokens;
					cache->startSetRules[ruleSymbol->pmRuleIndex] = startSetRules;
				}

				for (vint i = 0; i < ruleCount; i++)
				{
					for (vint j = 0; j < ruleCount; j++)
					{
						if (cache->startSetRules[i][j])
						{
							cache->reverseStartSetRules[j].Set(i);
						}
					}
				}

				return cache;
			}

#if defined VCZH_MSVC && defined _DEBUG
#define LOG_DECISION_MAKING
#define LOG console::Console::Write
#define LOGL console::Console::WriteLine
#endif

/***********************************************************************
SyntaxSymbolManager::PrefixMergeCrossReference_SolveInState
***********************************************************************/

			void SyntaxSymbolManager::PrefixMergeCrossReference_SolveInState(
				PrefixMergeCache * cache,
				RuleSymbol * rule,
				StateSymbol * currentState,
				Ptr<PrefixMergeSolutionApplication> application)
			{
				/*
				* To find out the minimum start set of rules to inject
				* 
				* Stores startSetRules[edge->input.rule] to startSetEdges
				* Try each rule in cache->rulesByDeps
				* If startSetRules[rule] satisfies both conditions on each edge to merge
				*   startSetEdges[edge] & startSetRules[rule] == startSetRules[rule]
				*   startSetEdges[edge] & startSetRules[rule] == BitSet::Zero
				* Then this rule is one in the minimum start set, update startSetEdges by:
				*   extract startSetRules[rule]
				*   extract any parent rule where startSetRules[parent rule] has rule
				* Repeat until the test results in all BitSet::Zero (or when all startSetEdges are BitSet::Zero)
				* 
				* We can see indirectStartSetRules as the relationship in a partial ordered graph starting from a rule
				* When a rule is picked up as a prefix rule
				*   it removes the rule and all its descendants from the graph
				*   and we will have to break all node that depends on this prefix rule into multiple prefix rules
				*   so those parents will never be prefix rules
				*   after parent nodes are removed, the graph breaks into multiple sub graphs, maybe disconnected
				* The goal is to pick up all smaller graphs that:
				*   they are big enough
				*   their leaf nodes do no intersect with each other
				*   leaf nodes of the current graph is covered by them
				*/

#define ERROR_MESSAGE_PREFIX L"vl::glr::parsergen::SyntaxSymbolManager::PrefixMergeCrossReference_SolveInState(PrefixMergeCache*, RuleSymbol*, StateSymbol*, Array<EdgeSymbol*>&, List<RuleSymbol*>&, PrefixMergeSolutionMap&)#"

#ifdef LOG_DECISION_MAKING
				LOG(L"  [GROUPED] :");
				for (auto [edge, index] : indexed(application->edgesToMerge))
				{
					LOG(L" " + edge->input.rule->Name());
				}
				LOGL(L"");
#endif
				// prepare startSetEdges to be startSetRules of each edge's input rule
				Array<BitSet> startSetEdges(application->edgesToMerge.Count());
				for (auto [edge, index] : indexed(application->edgesToMerge))
				{
					startSetEdges[index] = cache->startSetRules[edge->input.rule->pmRuleIndex];
					startSetEdges[index].Set(edge->input.rule->pmRuleIndex);
				}

				// at this moment, application->prefixRules are part of the solution that has already been picked u
				if (application->prefixRules.Count() > 0)
				{
					for (auto prefixRule : application->prefixRules)
					{
						// take away all rules that depends or reversed depends on the prefix rule
						auto startSetRule = cache->startSetRules[prefixRule->pmRuleIndex];
						startSetRule.Set(prefixRule->pmRuleIndex);
						auto reverseStartSetRule = cache->reverseStartSetRules[prefixRule->pmRuleIndex];
						auto startSetToExtract = startSetRule | reverseStartSetRule;

						for (vint edgeIndex = 0; edgeIndex < application->edgesToMerge.Count(); edgeIndex++)
						{
							startSetEdges[edgeIndex] -= startSetToExtract;
						}
					}
#ifdef LOG_DECISION_MAKING
					LOGL(L"    [APPLIED REUSED]");
					for (auto [edge, index] : indexed(application->edgesToMerge))
					{
						LOG(L"      " + edge->input.rule->Name() + L" :");
						auto&& startSetEdge = startSetEdges[index];
						for (auto rule : cache->rulesByDeps)
						{
							if (startSetEdge[rule->pmRuleIndex]) LOG(L" " + rule->Name());
						}
						LOGL(L"");
					}
#endif
					// if startSetEdges become all empty, no more prefix rule will be found
					bool allZero = true;
					for (vint edgeIndex = 0; edgeIndex < application->edgesToMerge.Count(); edgeIndex++)
					{
						if (startSetEdges[edgeIndex] != BitSet::Zero)
						{
							allZero = false;
							break;
						}
					}
					if (allZero) return;
				}

				{
				FOUND_ONE_SOLUTION:
#ifdef LOG_DECISION_MAKING
					LOGL(L"    [ITERATION]");
					for (auto [edge, index] : indexed(application->edgesToMerge))
					{
						LOG(L"      " + edge->input.rule->Name() + L" :");
						auto&& startSetEdge = startSetEdges[index];
						for (auto rule : cache->rulesByDeps)
						{
							if (startSetEdge[rule->pmRuleIndex]) LOG(L" " + rule->Name());
						}
						LOGL(L"");
					}
#endif

					bool matchedOthers = false;

					// try each rule in dependency order
					for (auto prefixRule : cache->rulesByDeps)
					{
						vint containedCount = 0;
						vint exclusiveCount = 0;
						vint otherCount = 0;
						auto startSetRule = cache->startSetRules[prefixRule->pmRuleIndex];
						startSetRule.Set(prefixRule->pmRuleIndex);

						// compare the start set of this rule to remaining start set subset of each edge
						// count each situation
						for (vint edgeIndex = 0; edgeIndex < application->edgesToMerge.Count(); edgeIndex++)
						{
							auto& startSetEdge = startSetEdges[edgeIndex];
							auto matchResult = startSetEdge & startSetRule;
							if (matchResult == startSetRule)
							{
								// startSetEdge contains startSetRule
								containedCount++;
							}
							else if (matchResult == BitSet::Zero)
							{
								// startSetEdge does no intercept with startSetRule
								exclusiveCount++;
							}
							else
							{
								// others
								otherCount++;
								break;
							}
						}
						if (otherCount > 0) matchedOthers = true;

						// if some startSetEdge contains start set of the candidate rule
						// and others are empty intersection
						// we found a prefix rule
						if (containedCount > 0 && otherCount == 0)
						{
							// take away all rules that depends or reversed depends on the prefix rule
							auto reverseStartSetRule = cache->reverseStartSetRules[prefixRule->pmRuleIndex];
							auto startSetToExtract = startSetRule | reverseStartSetRule;

							for (vint edgeIndex = 0; edgeIndex < application->edgesToMerge.Count(); edgeIndex++)
							{
								startSetEdges[edgeIndex] -= startSetToExtract;
							}

							application->prefixRules.Add(prefixRule);
#ifdef LOG_DECISION_MAKING
							LOG(L"    [FOUND " + prefixRule->Name() + L"] :");
							for (auto rule : cache->rulesByDeps)
							{
								if (startSetRule[rule->pmRuleIndex]) LOG(L" " + rule->Name());
							}
							LOGL(L"");
#endif
							goto FOUND_ONE_SOLUTION;
						}
					}

					CHECK_ERROR(!matchedOthers, ERROR_MESSAGE_PREFIX L"Internal error: Unable to find a proper prefix merge solution.");
				}

#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
SyntaxSymbolManager::PrefixMergeCrossReference_Solve
***********************************************************************/

			void SyntaxSymbolManager::PrefixMergeCrossReference_Solve(
				PrefixMergeCache* cache,
				bool forStartState,
				RuleSymbol* rule,
				StateSymbol* startState,
				PrefixMergeSolutionMap& prefixMergeSolutions)
			{
				auto logCache = [&]()
				{
#ifdef LOG_DECISION_MAKING
					if (prefixMergeSolutions.Count() == 0)
					{
						LOGL(L"[CACHE]");
						LOGL(L"  [RULES]");
						for (auto rule : cache->rulesByDeps)
						{
							LOG(L"    " + rule->Name() + L" :");
							auto&& startSetRule = cache->directStartSetRules[rule->pmRuleIndex];
							for (auto rule : cache->rulesByDeps)
							{
								if (startSetRule[rule->pmRuleIndex]) LOG(L" " + rule->Name());
							}
							LOGL(L"");
						}
						LOGL(L"  [START SET]");
						for (auto rule : cache->rulesByDeps)
						{
							LOG(L"    " + rule->Name() + L" :");
							auto&& startSetRule = cache->startSetRules[rule->pmRuleIndex];
							for (auto rule : cache->rulesByDeps)
							{
								if (startSetRule[rule->pmRuleIndex]) LOG(L" " + rule->Name());
							}
							LOGL(L"");
						}
						LOGL(L"  [REVERSED]");
						for (auto rule : cache->rulesByDeps)
						{
							LOG(L"    " + rule->Name() + L" :");
							auto&& startSetRule = cache->reverseStartSetRules[rule->pmRuleIndex];
							for (auto rule : cache->rulesByDeps)
							{
								if (startSetRule[rule->pmRuleIndex]) LOG(L" " + rule->Name());
							}
							LOGL(L"");
						}
						LOGL(L"");
					}
#endif
				};

				/*
				* If forStartState is true, only process the start state, otherwise process all reachable states except the start state
				* forStartState == true will be applied all rules in start set dependency order first
				* therefore whenever we see a rule input edge, its (rule, startState) should already have a solution
				* 
				* When making a solution, if any Rule input edge already have a solution, reuse it
				* Find edges whose start set have intersection, process each group
				* Merge all solutions into one
				* 
				* There are multiple applications in a solution
				* Each application records what prefix rules will be injected into what edges
				* For group that has only one edge, there will be no application
				* 
				* For any start state, a solution will be created when there are groups with multiple edges, or any edge whose input rule has a solution on its start state
				* For any other state, a solution will be created only when there are groups with multiple edges
				*/

#define ERROR_MESSAGE_PREFIX L"vl::glr::parsergen::SyntaxSymbolManager::PrefixMergeCrossReference_Solve(PrefixMergeCache*, RuleSymbol*, StateSymbol*, PrefixMergeSolutionMap&)#"
				SortedList<StateSymbol*> visitedStates;
				List<StateSymbol*> workingStates;
				workingStates.Add(startState);

				// Traverse all states
				for (vint i = 0; i < workingStates.Count(); i++)
				{
					auto currentState = workingStates[i];

					if (forStartState == (currentState == startState))
					{
						Group<EdgeSymbol*, EdgeSymbol*> biDeps;

						// Group all edges that have intersection in start set
						for (vint j = 0; j < currentState->OutEdges().Count() - 1; j++)
						{
							auto edge1 = currentState->OutEdges()[j];
							if (edge1->input.type != EdgeInputType::Rule) continue;
							auto tokens1 = cache->startSetTokens[edge1->input.rule->pmRuleIndex];
							auto rules1 = cache->startSetRules[edge1->input.rule->pmRuleIndex];
							rules1.Set(edge1->input.rule->pmRuleIndex);

							for (vint k = j + 1; k < currentState->OutEdges().Count(); k++)
							{
								auto edge2 = currentState->OutEdges()[k];
								if (edge2->input.type != EdgeInputType::Rule) continue;
								auto tokens2 = cache->startSetTokens[edge2->input.rule->pmRuleIndex];
								auto rules2 = cache->startSetRules[edge2->input.rule->pmRuleIndex];
								rules2.Set(edge2->input.rule->pmRuleIndex);

								CHECK_ERROR(edge1->input.rule != edge2->input.rule, ERROR_MESSAGE_PREFIX L"Internal error: Two edges from the same state should not consume the same rule, this should have been eliminated by MergeEdgesWithSameRuleUsingLeftrec.");
								if (!(tokens1 & tokens2)) continue;
								if (!(rules1 & rules2)) continue;

								biDeps.Add(edge1, edge2);
								biDeps.Add(edge2, edge1);
							}
						}

						PartialOrderingProcessor pop;
						pop.InitWithGroup(currentState->OutEdges(), biDeps);
						pop.Sort();

						// Find if a solution needs to be created for the current state
						bool hasSolution = false;
						for (auto component : pop.components)
						{
							if (component.nodeCount > 1)
							{
								// if there is a group with multiple edges, yes
								hasSolution = true;
								break;
							}
							else if (currentState == startState)
							{
								// if the input rule has a solution on its start state, and the current state is the start state, yes
								auto edge = currentState->OutEdges()[*component.firstNode];
								if (edge->input.type == EdgeInputType::Rule)
								{
									auto prefixRule = edge->input.rule;
									if (prefixMergeSolutions.Keys().Contains({ prefixRule, prefixRule->startStates[0] }))
									{
										hasSolution = true;
										break;
									}
								}
							}
						}

						// skip the current state if no solution is needed
						// edges are grouped by start set intersections
						// so whenever a rule input has a solution on its start state or not
						// the rule or its solution will never conflict with other groups
						// since the solution will be a subset of union of all start sets for the group
						if (hasSolution)
						{
							logCache();
#ifdef LOG_DECISION_MAKING
							LOGL(L"[PMCR] " + rule->Name() + L" @ " + currentState->label);
#endif
							// Check all groups with single edge
							auto solution = Ptr(new PrefixMergeSolutionValue);
							for (auto component : pop.components)
							{
								if (component.nodeCount == 1)
								{
									auto edge = currentState->OutEdges()[*component.firstNode];
									if (edge->input.type == EdgeInputType::Rule)
									{
										auto prefixRule = edge->input.rule;
										vint solutionIndex = prefixMergeSolutions.Keys().IndexOf({ prefixRule, prefixRule->startStates[0] });
										if (solutionIndex == -1)
										{
#ifdef LOG_DECISION_MAKING
											LOGL(L"  [SINGLE] " + prefixRule->Name());
#endif
											// if the input rule has no solution on its start state
											// but the current state needs a solution
											// then the input rule is a prefix
											solution->prefixRules.Add(prefixRule);
										}
										else
										{
#ifdef LOG_DECISION_MAKING
											LOGL(L"  [SINGLE USED] " + prefixRule->Name());
#endif
											// otherwise, copy all prefix rules
											CopyFrom(solution->prefixRules, prefixMergeSolutions.Values()[solutionIndex]->prefixRules, true);
										}
									}
								}
							}

							// Check all groups with multiple edges, applications will be created on these groups
							for (auto component : pop.components)
							{
								if (component.nodeCount > 1)
								{
									// collect all existing solutions for each edge
									auto application = Ptr(new PrefixMergeSolutionApplication);
									application->edgesToMerge.Resize(component.nodeCount);
									for (vint j = 0; j < component.nodeCount; j++)
									{
										application->edgesToMerge[j] = currentState->OutEdges()[component.firstNode[j]];
									}
									for (auto edge : application->edgesToMerge)
									{
										auto prefixRule = edge->input.rule;
										vint solutionIndex = prefixMergeSolutions.Keys().IndexOf({ prefixRule, prefixRule->startStates[0] });
										if (solutionIndex != -1)
										{
#ifdef LOG_DECISION_MAKING
											LOGL(L"  [SINGLE USED] " + prefixRule->Name());
#endif
											// copy all prefix rules, ensure no duplication
											for (auto prefixRule : prefixMergeSolutions.Values()[solutionIndex]->prefixRules)
											{
												if (!application->prefixRules.Contains(prefixRule))
												{
													application->prefixRules.Add(prefixRule);
												}
											}
										}
									}

									// solve for the current group
									PrefixMergeCrossReference_SolveInState(cache, rule, currentState, application);

									// copy all prefix rules, ensure no duplication
									solution->applications.Add(application);
									for (auto prefixRule : application->prefixRules)
									{
										if (!solution->prefixRules.Contains(prefixRule))
										{
											solution->prefixRules.Add(prefixRule);
										}
									}
								}
							}
							prefixMergeSolutions.Add({ rule, currentState }, solution);

#ifdef LOG_DECISION_MAKING
							LOG(L"  [MATCHED] :");
							for (auto rule : solution->prefixRules)
							{
								LOG(L" " + rule->Name());
							}
							LOGL(L"");
							LOGL(L"");
#endif
						}
					}

					// continue if required
					if (!forStartState)
					{
						for (auto edge : currentState->OutEdges())
						{
							if (!visitedStates.Contains(edge->To()))
							{
								visitedStates.Add(edge->To());
								workingStates.Add(edge->To());
							}
						}
					}
				}
#undef ERROR_MESSAGE_PREFIX
			}

			void SyntaxSymbolManager::PrefixMergeCrossReference_Solve(PrefixMergeCache* cache, PrefixMergeSolutionMap& prefixMergeSolutions)
			{
				for (auto rule : From(cache->rulesByDeps).Reverse())
				{
					PrefixMergeCrossReference_Solve(cache, true, rule, rule->startStates[0], prefixMergeSolutions);
				}
				for (auto rule : From(cache->rulesByDeps).Reverse())
				{
					PrefixMergeCrossReference_Solve(cache, false, rule, rule->startStates[0], prefixMergeSolutions);
				}
			}

/***********************************************************************
SyntaxSymbolManager::PrefixMergeCrossReference_AccumulatedEdges
***********************************************************************/

			struct PrefixMergeApplicationItems
			{
				using EdgeList = List<EdgeSymbol*>;
				using EdgeListPtr = Ptr<EdgeList>;
				using RuleToEdgesMap = Group<RuleSymbol*, EdgeListPtr>;
				using TokenToEdgesMap = Group<Pair<vint32_t, Nullable<WString>>, EdgeListPtr>;

				BitSet				coveredRules;
				RuleToEdgesMap		ruleToEdges;
				TokenToEdgesMap		tokenToEdges;
			};

			EdgeSymbol* SyntaxSymbolManager::PrefixMergeCrossReference_AccumulatedEdges(StateSymbol* fromState, const WString& pmLabel, const collections::List<PrefixMergeApplicationItems::EdgeListPtr>& accumulatedEdgesList, IncrementalChange& ic)
			{
				auto IsSingleReuseEdge = [](EdgeSymbol* edge)
				{
					if (edge->input.type != EdgeInputType::Rule) return false;
					if (edge->input.ruleType != automaton::ReturnRuleType::Reuse) return false;
					if (edge->competitions.Count() != 0) return false;

					vint edgeIns = edge->insAfterInput.Count();
					if (edgeIns >= 2) return false;
					if (edgeIns == 1 && edge->insAfterInput[0].type != AstInsType::StackBegin) return false;

					auto targetState = edge->To();
					List<EdgeSymbol*> endingEdges;
					CopyFrom(endingEdges, From(targetState->OutEdges()).Where([](EdgeSymbol* e)
					{
						return e->input.type == EdgeInputType::Ending;
					}));
					if (endingEdges.Count() != 1) return false;

					auto endingEdge = endingEdges[0];
					if (endingEdge->input.type != EdgeInputType::Ending) return false;

					if (edgeIns == 0)
					{
						if (endingEdge->insAfterInput.Count() != 2) return false;
						if (endingEdge->insAfterInput[0].type != AstInsType::StackBegin) return false;
						if (endingEdge->insAfterInput[1].type != AstInsType::StackEnd) return false;
					}
					else
					{
						if (endingEdge->insAfterInput.Count() != 1) return false;
						if (endingEdge->insAfterInput[0].type != AstInsType::StackEnd) return false;
					}
					return true;
				};

				Array<vint> accumulatedSizes(accumulatedEdgesList.Count());
				for (auto [edges, index] : indexed(accumulatedEdgesList))
				{
					auto&& edgeList = *edges.Obj();
					accumulatedSizes[index] = 0;
					for (vint i = edgeList.Count(); i > 0; i--)
					{
						auto edge = edgeList[i - 1];
						if (!IsSingleReuseEdge(edge))
						{
							accumulatedSizes[index] = i;
							break;
						}
					}
				}

				// newState labeling [pm-cr] is made if there are multiple choices
				auto newState = Ptr(new StateSymbol(fromState->Rule()));
				ic.createdStates.Add(newState);
				newState->label = fromState->label + pmLabel;

				// newEdge to connect fromState to newState
				auto newEdge = Ptr(new EdgeSymbol(fromState, newState.Obj()));
				ic.createdEdges.Add(newEdge);

				SortedList<WString> contStateLabels;
				Group<Pair<StateSymbol*, StateSymbol*>, EdgeSymbol*> createdContEdges;
				for (auto [edges, index] : indexed(accumulatedEdgesList))
				{
#ifdef LOG_DECISION_MAKING
					LOG(L"    ");
					for (auto [edge, edgeIndex] : indexed(*edges.Obj()))
					{
						if (edgeIndex > 0) LOG(L" -> ");
						if (edgeIndex == accumulatedSizes[index])
						{
							LOG(L"| ");
						}
						if (edge->input.type == EdgeInputType::Rule)
						{
							LOG(edge->input.rule->Name());
						}
						else
						{
							LOG(L"[TOKEN] " + (edge->input.condition ? edge->input.condition.Value() : itow(edge->input.token)));
						}
					}
					LOGL(L"");
#endif
					auto&& edgeList = *edges.Obj();
					vint accSize = accumulatedSizes[index];
					for (vint i = edgeList.Count(); i >= accSize && i > 0; i--)
					{
						auto lastEdge = edgeList[i - 1];

						Ptr<StateSymbol> contState;
						auto lastState = lastEdge->To();
						for (auto contEdge : lastState->OutEdges())
						{
							if (contEdge->input.type == EdgeInputType::PrefixMergeDiscardedRule)
							{
								continue;
							}

							if (contEdge->input.type == EdgeInputType::LeftRec)
							{
								if (contEdge->returnEdges.Count() == 0);
								else continue;
							}

							if (contEdge->input.type == EdgeInputType::Ending)
							{
								if (accSize == 0 && i == 1);
								else if (i == accSize);
								else continue;
							}

							auto AddNewContEdge = [&](StateSymbol* contFromState, bool useLastEdgeContent, bool useReturnEdges)
							{
								auto newContEdge = Ptr(new EdgeSymbol(contFromState, contEdge->To()));

								newContEdge->input = contEdge->input;
								if (useLastEdgeContent)
								{
									CopyFrom(newContEdge->competitions, lastEdge->competitions, true);
									CopyFrom(newContEdge->insAfterInput, lastEdge->insAfterInput, true);
								}
								CopyFrom(newContEdge->competitions, contEdge->competitions, true);
								CopyFrom(newContEdge->insAfterInput, contEdge->insAfterInput, true);

								if (useReturnEdges)
								{
									for (vint j = 0; j < i - 1; j++)
									{
										newContEdge->returnEdges.Add(edgeList[j]);
									}
								}

								auto key = Pair(newContEdge->From(), newContEdge->To());
								vint index = createdContEdges.Keys().IndexOf(key);
								if (index != -1)
								{
									for (auto created : createdContEdges.GetByIndex(index))
									{
										if (created->input != newContEdge->input) continue;
										if (CompareEnumerable(created->competitions, newContEdge->competitions) != 0) continue;
										if (CompareEnumerable(created->insAfterInput, newContEdge->insAfterInput) != 0) continue;
										if (CompareEnumerable(created->returnEdges, newContEdge->returnEdges) != 0) continue;

										newContEdge->From()->outEdges.Remove(newContEdge.Obj());
										newContEdge->To()->inEdges.Remove(newContEdge.Obj());
										return;
									}
								}

								createdContEdges.Add(key, newContEdge.Obj());
								ic.createdEdges.Add(newContEdge);
							};

							if (i == 1 && contEdge->input.type != EdgeInputType::Rule)
							{
								AddNewContEdge(newState.Obj(), true, false);
								continue;
							}

							if (contEdge->input.type == EdgeInputType::LeftRec)
							{
								AddNewContEdge(newState.Obj(), true, true);
								continue;
							}

							if (!contState)
							{
								auto label = fromState->label + pmLabel;
								for (vint j = 0; j < i; j++)
								{
									auto&& input = edgeList[j]->input;
									if (input.type == EdgeInputType::Rule)
									{
										label += L" " + input.rule->Name();
									}
									else
									{
										label += L" [TOKEN] " + (input.condition ? input.condition.Value() : itow(input.token));
									}
								}
								if (contStateLabels.Contains(label)) continue;
								contStateLabels.Add(label);

								contState = Ptr(new StateSymbol(fromState->Rule()));
								ic.createdStates.Add(contState);
								contState->label = label;

								auto lrEdge = Ptr(new EdgeSymbol(newState.Obj(), contState.Obj()));
								ic.createdEdges.Add(lrEdge);

								lrEdge->input.type = EdgeInputType::LeftRec;
								CopyFrom(lrEdge->competitions, lastEdge->competitions, true);
								for (vint j = 0; j < i - 1; j++)
								{
									lrEdge->returnEdges.Add(edgeList[j]);
								}
								CopyFrom(lrEdge->insAfterInput, lastEdge->insAfterInput, true);
							}

							{
								AddNewContEdge(contState.Obj(), false, false);
							}
						}
					}
				}

				return newEdge.Obj();
			}

/***********************************************************************
SyntaxSymbolManager::PrefixMergeCrossReference_Apply
***********************************************************************/

			void SyntaxSymbolManager::PrefixMergeCrossReference_Apply(PrefixMergeCache* cache, collections::List<EdgeSymbol*>& accumulatedEdges, PrefixMergeApplicationItems& pmai)
			{
				auto lastEdge = accumulatedEdges[accumulatedEdges.Count() - 1];
				auto lastRule = lastEdge->input.rule;

				if (pmai.coveredRules[lastRule->pmRuleIndex])
				{
					auto edges = Ptr(new List<EdgeSymbol*>);
					CopyFrom(*edges.Obj(), accumulatedEdges);
					pmai.ruleToEdges.Add(lastRule, edges);
				}
				else
				{
					for (auto edge : lastRule->startStates[0]->OutEdges())
					{
						switch (edge->input.type)
						{
						case EdgeInputType::Rule:
							// RuleIsIndirectlyLeftRecursive has been checked so there will be no deadloop 
							accumulatedEdges.Add(edge);
							PrefixMergeCrossReference_Apply(cache, accumulatedEdges, pmai);
							accumulatedEdges.RemoveAt(accumulatedEdges.Count() - 1);
							break;
						case EdgeInputType::Token:
							{
								auto edges = Ptr(new List<EdgeSymbol*>);
								CopyFrom(*edges.Obj(), accumulatedEdges);
								edges->Add(edge);
								pmai.tokenToEdges.Add({ edge->input.token, edge->input.condition }, edges);
							}
							break;
						default:;
						}
					}
				}

			}

			void SyntaxSymbolManager::PrefixMergeCrossReference_Apply(PrefixMergeCache* cache, RuleSymbol* rule, StateSymbol* currentState, Ptr<PrefixMergeSolutionValue> solution, IncrementalChange& ic)
			{
				/*
				* For any state A whose prefix calls look like:
				*   A -> r1 -> ... rn -> (u1, v1, ...)
				*   A -> s1 -> ... sn -> (u1, v1, ...)
				* The same (u1, v1, ...) will be merged.
				* Especially when A indirectly reuses (u1, v1, ...), this approach removes duplicated ambiguous results.
				*   by not doing this, it produces two duplicated A -> (different paths) -> (u1, v1, ...)
				*   and parsing efforts are wasted if (u1, v1, ...) costs a large number of transitions
				* 
				* Because a prefix will be produced and then passed into its parent rule like this:
				*   {CreateObject(u1)} ... {StackSlot(0), CreateObject(r1)} {StackSlot(0), CreateObject(A)}
				*   we can parse (u1, v1, ...) first, and then patch multiple EndingInput transitions directly into (rn, sn)
				*   such EndingInput transition will have non-empty returnEdges
				* 
				* (u1, v1, ...) are not required to be reuse clauses which only contain one rule
				*   as even if they are, they may be followed by LeftRec transitions, it doesn't make the work simpler, but even more checking
				*   recursive cross-reference merges will not be implemented at the moment
				*   treat it as a special case which only merge prefixes in the size of one rule
				* 
				* PrefixMergeCrossReference_Solve worked out how many transitions need to be reworked
				*   e.g. (a start set graph, assuming left nodes are selected prefix rules)
				*              +-> r2 ..
				*              |       +-> r4 ..
				*     A -> r1 -+-> r3 -+-> r5 ..
				*              |
				*              +-> r6..
				*   when r4 merges, (r2, r5, r6) will need similar edges to start from A:
				*     A -+- r4 (injects into r3)
				*        +- r5 (injects into r3)
				*        +- r2 (injects into r1)
				*        +- r6 (injects into r1)
				*/

				for (auto application : solution->applications)
				{
					PrefixMergeApplicationItems pmai;
					for (auto prefixRule : application->prefixRules)
					{
						pmai.coveredRules.Set(prefixRule->pmRuleIndex);
					}

					List<EdgeSymbol*> accumulatedEdges;
					for (auto edge : application->edgesToMerge)
					{
						accumulatedEdges.Add(edge);
						PrefixMergeCrossReference_Apply(cache, accumulatedEdges, pmai);
						accumulatedEdges.RemoveAt(accumulatedEdges.Count() - 1);
					}

#ifdef LOG_DECISION_MAKING
					LOGL(L"[PMAI] " + rule->Name() + L" @ " + currentState->label);
#endif
					for (vint i = 0; i < pmai.ruleToEdges.Count(); i++)
					{
						auto pmRule = pmai.ruleToEdges.Keys()[i];
#ifdef LOG_DECISION_MAKING
						LOGL(L"  [RULE] " + pmRule->Name() + L" :");
#endif
						auto&& accumulatedEdgesList = pmai.ruleToEdges.GetByIndex(i);
						auto newEdge = PrefixMergeCrossReference_AccumulatedEdges(currentState, (L"[pm-cr-rule: " + pmRule->Name() + L"]"), accumulatedEdgesList, ic);
						newEdge->input.type = EdgeInputType::PrefixMergeRule;
						newEdge->input.rule = pmRule;
					}
					for (vint i = 0; i < pmai.tokenToEdges.Count(); i++)
					{
						auto [pmToken, condition] = pmai.tokenToEdges.Keys()[i];
#ifdef LOG_DECISION_MAKING
						LOGL(L"  [TOKEN] " + (condition ? condition.Value() : itow(pmToken)));
#endif
						auto&& accumulatedEdgesList = pmai.tokenToEdges.GetByIndex(i);
						auto newEdge = PrefixMergeCrossReference_AccumulatedEdges(currentState, (L"[pm-cr-token: " + (condition ? condition.Value() : itow(pmToken)) + L"]"), accumulatedEdgesList, ic);
						newEdge->input.type = EdgeInputType::Token;
						newEdge->input.token = pmToken;
						newEdge->input.condition = condition;
					}
#ifdef LOG_DECISION_MAKING
					LOGL(L"");
#endif
				}
			}

#ifdef LOG_DECISION_MAKING
#undef LOG_DECISION_MAKING
#undef LOG
#undef LOGL
#endif
		}
	}
}
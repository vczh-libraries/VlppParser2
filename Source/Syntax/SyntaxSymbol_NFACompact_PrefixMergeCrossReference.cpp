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
				bool operator==(const BitSet& bs) const = default;

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

/***********************************************************************
SyntaxSymbolManager::PrefixMergeCrossReference_SolveInState
***********************************************************************/

			void SyntaxSymbolManager::PrefixMergeCrossReference_SolveInState(PrefixMergeCache * cache, RuleSymbol * rule, StateSymbol * currentState, collections::Array<EdgeSymbol*>& edgesToMerge, PrefixMergeSolutionMap& prefixMergeSolutions)
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
				*/
				console::Console::WriteLine(rule->Name() + L": " + currentState->label);
				for (auto edge : edgesToMerge)
				{
					console::Console::WriteLine(L"  " + edge->input.rule->Name());
				}
			}

/***********************************************************************
SyntaxSymbolManager::PrefixMergeCrossReference_Solve
***********************************************************************/

			void SyntaxSymbolManager::PrefixMergeCrossReference_Solve(PrefixMergeCache* cache, RuleSymbol* rule, StateSymbol* startState, PrefixMergeSolutionMap& prefixMergeSolutions)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::parsergen::SyntaxSymbolManager::PrefixMergeCrossReference_Solve(PrefixMergeCache*, RuleSymbol*, StateSymbol*)#"
				SortedList<StateSymbol*> visitedStates;
				List<StateSymbol*> workingStates;
				workingStates.Add(startState);

				for (vint i = 0; i < workingStates.Count(); i++)
				{
					auto currentState = workingStates[i];
					Group<EdgeSymbol*, EdgeSymbol*> biDeps;

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

					for (auto component : pop.components)
					{
						if (component.nodeCount > 1)
						{
							Array<EdgeSymbol*> edgesToMerge(component.nodeCount);
							for (vint j = 0; j < component.nodeCount; j++)
							{
								edgesToMerge[j] = currentState->OutEdges()[component.firstNode[j]];
							}
							PrefixMergeCrossReference_SolveInState(cache, rule, currentState, edgesToMerge, prefixMergeSolutions);
						}
					}

					for (auto edge : currentState->OutEdges())
					{
						if (!visitedStates.Contains(edge->To()))
						{
							visitedStates.Add(edge->To());
							workingStates.Add(edge->To());
						}
					}
				}
#undef ERROR_MESSAGE_PREFIX
			}

/***********************************************************************
SyntaxSymbolManager::PrefixMergeCrossReference_Apply
***********************************************************************/

			void SyntaxSymbolManager::PrefixMergeCrossReference_Apply(PrefixMergeCache* cache, RuleSymbol* rule, StateSymbol* startState, StateList& newStates, EdgeList& newEdges)
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
				* Pay attention to complex situation when prefix calls trees and only sub trees merge
				* 
				* This function checks all states with multiple outgoing Rule transitions
				*   if any mergable transitions are found, all outgoing Rule transitions will have to be reworked
				*   because if a sub tree merges with another
				*   all other roots of sub trees in all ancestors will have to be connected from that state
				*   e.g.
				*              +- r2 ..
				*              |      +- r4 ..
				*     A -> r1 -+- r3 -+  r5 ..
				*              |
				*              +- r6..
				*   when r4 merges, (r2, r5, r6) will need similar edges to start from A:
				*     A -+- r4 (injects into r3)
				*        +- r5 (injects into r3)
				*        +- r2 (injects into r1)
				*        +- r6 (injects into r1)
				*   only first level transitions are unaffected, all extra injections include Token from (r1, r3)
				* 
				* Do not merge edges when it is equivalent to just call those rules.
				*   It means merging should happen between different first level sub trees.
				*   Be careful when merging destroy the prefix structure, do we need to keep a copy of unmerged rule?
				*/
			}
		}
	}
}
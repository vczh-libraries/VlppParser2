#include "SyntaxSymbol.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;

/***********************************************************************
SyntaxSymbolManager::MergeEdgesWithSameInputCrossReference
***********************************************************************/

			void SyntaxSymbolManager::MergeEdgesWithSameInputCrossReference(RuleSymbol* rule, StateSymbol* startState, StateList& newStates, EdgeList& newEdges)
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
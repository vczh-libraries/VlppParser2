#include "SyntaxSymbolWriter.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{

/***********************************************************************
AutomatonBuilder
***********************************************************************/

			AutomatonBuilder::AutomatonBuilder(RuleSymbol* _ruleSymbol)
				: ruleSymbol(_ruleSymbol)
			{
			}

/***********************************************************************
AutomatonBuilder (Syntax)
***********************************************************************/

			AutomatonBuilder::StatePair AutomatonBuilder::BuildTokenSyntax(vint32_t tokenId, const WString& displayText, vint32_t field)
			{
				StatePair pair;
				pair.begin = CreateState();
				pair.end = CreateState();
				startPoses.Add(pair.begin, clauseDisplayText.Length());

				{
					auto edge = CreateEdge(pair.begin, pair.end);
					edge->input.type = EdgeInputType::Token;
					edge->input.token = tokenId;
					if (field != -1)
					{
						edge->insAfterInput.Add({ AstInsType::Token });
						edge->insAfterInput.Add({ AstInsType::Field,field });
					}
				}

				clauseDisplayText += displayText;
				endPoses.Add(pair.end, clauseDisplayText.Length());
				return pair;
			}

			AutomatonBuilder::StatePair AutomatonBuilder::BuildRuleSyntax(RuleSymbol* rule, vint32_t field)
			{
				StatePair pair;
				pair.begin = CreateState();
				pair.end = CreateState();
				startPoses.Add(pair.begin, clauseDisplayText.Length());

				{
					auto edge = CreateEdge(pair.begin, pair.end);
					edge->input.type = EdgeInputType::Rule;
					edge->input.rule = rule;
					if (field != -1)
					{
						edge->insAfterInput.Add({ AstInsType::Field,field });
					}
					else if (!rule->isPartial)
					{
						edge->insAfterInput.Add({ AstInsType::DiscardValue });
					}
				}

				clauseDisplayText += rule->Name();
				endPoses.Add(pair.end, clauseDisplayText.Length());
				return pair;
			}

			AutomatonBuilder::StatePair AutomatonBuilder::BuildUseSyntax(RuleSymbol* rule)
			{
				StatePair pair;
				pair.begin = CreateState();
				pair.end = CreateState();
				startPoses.Add(pair.begin, clauseDisplayText.Length());

				{
					auto edge = CreateEdge(pair.begin, pair.end);
					edge->input.type = EdgeInputType::Rule;
					edge->input.rule = rule;
					edge->insAfterInput.Add({ AstInsType::ReopenObject });
				}

				clauseDisplayText += L"!" + rule->Name();
				endPoses.Add(pair.end, clauseDisplayText.Length());
				return pair;
			}

			AutomatonBuilder::StatePair AutomatonBuilder::BuildLoopSyntax(const StateBuilder& loopBody, const StateBuilder& loopDelimiter, bool hasDelimiter)
			{
				StatePair pair, bodyPair, delimiterPair;
				pair.begin = CreateState();
				pair.end = CreateState();
				startPoses.Add(pair.begin, clauseDisplayText.Length());

				clauseDisplayText += L"{ ";
				bodyPair = loopBody();
				if (hasDelimiter)
				{
					clauseDisplayText += L" ; ";
					delimiterPair = loopDelimiter();
				}
				clauseDisplayText += L" }";

				CreateEdge(pair.begin, bodyPair.begin);
				CreateEdge(bodyPair.end, pair.end);
				CreateEdge(pair.begin, pair.end);
				if (hasDelimiter)
				{
					CreateEdge(bodyPair.end, delimiterPair.begin);
					CreateEdge(delimiterPair.end, bodyPair.begin);
				}
				else
				{
					CreateEdge(bodyPair.end, bodyPair.begin);
				}

				endPoses.Add(pair.end, clauseDisplayText.Length());
				return pair;
			}

			AutomatonBuilder::StatePair AutomatonBuilder::BuildOptionalSyntax(bool preferTake, bool preferSkip, const StateBuilder& optionalBody)
			{
				StatePair pair;
				pair.begin = CreateState();
				pair.end = CreateState();
				startPoses.Add(pair.begin, clauseDisplayText.Length());

				if (!preferTake && !preferSkip)
				{
					clauseDisplayText += L"[ ";
				}
				else if (preferTake)
				{
					clauseDisplayText += L"+[ ";
				}
				else if (preferSkip)
				{
					clauseDisplayText += L"-[ ";
				}
				auto bodyPair = optionalBody();
				clauseDisplayText += L" ]";

				auto takeEdge = CreateEdge(pair.begin, bodyPair.begin);
				CreateEdge(bodyPair.end, pair.end);
				auto skipEdge = CreateEdge(pair.begin, pair.end);

				if (preferTake)
				{
					takeEdge->important = true;
				}
				if (preferSkip)
				{
					skipEdge->important = true;
				}

				endPoses.Add(pair.end, clauseDisplayText.Length());
				return pair;
			}

			AutomatonBuilder::StatePair AutomatonBuilder::BuildSequenceSyntax(const StateBuilder& firstSequence, const StateBuilder& secondSequence)
			{
				auto firstPair = firstSequence();
				clauseDisplayText += L" ";
				auto secondPair = secondSequence();
				CreateEdge(firstPair.end, secondPair.begin);
				return { firstPair.begin,secondPair.end };
			}

			AutomatonBuilder::StatePair AutomatonBuilder::BuildAlternativeSyntax(const StateBuilder& firstBranch, const StateBuilder& secondBranch)
			{
				StatePair pair;
				pair.begin = CreateState();
				pair.end = CreateState();
				startPoses.Add(pair.begin, clauseDisplayText.Length());

				clauseDisplayText += L"( ";
				auto firstPair = firstBranch();
				clauseDisplayText += L" | ";
				auto secondPair = secondBranch();
				clauseDisplayText += L" )";

				CreateEdge(pair.begin, firstPair.begin);
				CreateEdge(firstPair.end, pair.end);
				CreateEdge(pair.begin, secondPair.begin);
				CreateEdge(secondPair.end, pair.end);

				endPoses.Add(pair.end, clauseDisplayText.Length());
				return pair;
			}

/***********************************************************************
AutomatonBuilder (Clause)
***********************************************************************/

			AutomatonBuilder::StatePair AutomatonBuilder::BuildClause(const StateBuilder& compileSyntax)
			{
				ruleSymbol->NewClause();
				clauseDisplayText = L"";
				startPoses.Clear();
				endPoses.Clear();

				auto pair = compileSyntax();

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

				return pair;
			}

			AutomatonBuilder::StatePair AutomatonBuilder::BuildAssignment(StatePair pair, vint32_t enumItem, vint32_t field)
			{
				auto withState = CreateState();
				auto edge = CreateEdge(pair.end, withState);
				edge->insBeforeInput.Add({ AstInsType::EnumItem,enumItem });
				edge->insBeforeInput.Add({ AstInsType::Field,field });
				endPoses.Add(withState, clauseDisplayText.Length());
				return { pair.begin,withState };
			}

			AutomatonBuilder::StatePair AutomatonBuilder::BuildCreateClause(vint32_t classId, const StateBuilder& compileSyntax)
			{
				StatePair pair;
				pair.begin = CreateState();
				pair.end = CreateState();
				startPoses.Add(pair.begin, clauseDisplayText.Length());

				clauseDisplayText += L"< ";
				auto bodyPair = compileSyntax();
				clauseDisplayText += L" >";
				{
					auto edge = CreateEdge(pair.begin, bodyPair.begin);
					edge->insBeforeInput.Add({ AstInsType::BeginObject,classId });
				}
				{
					auto edge = CreateEdge(bodyPair.end, pair.end);
					edge->insBeforeInput.Add({ AstInsType::EndObject });
				}
				endPoses.Add(pair.end, clauseDisplayText.Length());
				return pair;
			}

			AutomatonBuilder::StatePair AutomatonBuilder::BuildPartialClause(const StateBuilder& compileSyntax)
			{
				return compileSyntax();
			}

			AutomatonBuilder::StatePair AutomatonBuilder::BuildReuseClause(const StateBuilder& compileSyntax)
			{
				StatePair pair;
				pair.begin = CreateState();
				pair.end = CreateState();
				startPoses.Add(pair.begin, clauseDisplayText.Length());

				clauseDisplayText += L"<< ";
				auto bodyPair = compileSyntax();
				clauseDisplayText += L" >>";
				{
					auto edge = CreateEdge(pair.begin, bodyPair.begin);
					edge->insBeforeInput.Add({ AstInsType::DelayFieldAssignment });
				}
				{
					auto edge = CreateEdge(bodyPair.end, pair.end);
					edge->insBeforeInput.Add({ AstInsType::EndObject });
				}
				endPoses.Add(pair.end, clauseDisplayText.Length());
				return pair;
			}
		}
	}
}
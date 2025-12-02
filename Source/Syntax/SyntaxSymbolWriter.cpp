#include "SyntaxSymbolWriter.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;

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

			AutomatonBuilder::StatePair AutomatonBuilder::BuildTokenSyntax(vint32_t tokenId, const WString& displayText, Nullable<WString> condition, vint32_t field)
			{
				StatePair pair;
				pair.begin = CreateState();
				pair.end = CreateState();
				startPoses.Add(pair.begin, clauseDisplayText.Length());

				{
					auto edge = CreateEdge(pair.begin, pair.end);
					edge->input.type = EdgeInputType::Token;
					edge->input.token = tokenId;
					edge->input.condition = condition;

					vint count = usedFieldIds++;
					edge->insAfterInput.Add({ AstInsType::Token,-1,count });
					if (field != -1)
					{
						fieldIns.Add({ AstInsType::Field,field,count });
					}
				}

				clauseDisplayText += displayText;
				endPoses.Add(pair.end, clauseDisplayText.Length());
				return pair;
			}

			AutomatonBuilder::StatePair AutomatonBuilder::BuildRuleSyntaxInternal(RuleSymbol* rule, vint32_t field, automaton::ReturnRuleType ruleType)
			{
#define ERROR_MESSAGE_PREFIX L"vl::glr::parsergen::AutomatonBuilder::BuildRuleSyntaxInternal(RuleSymbol*, vint32_t, ReturnRuleType)#"
				StatePair pair;
				pair.begin = CreateState();
				pair.end = CreateState();
				startPoses.Add(pair.begin, clauseDisplayText.Length());

				{
					auto edge = CreateEdge(pair.begin, pair.end);
					edge->input.type = EdgeInputType::Rule;
					edge->input.rule = rule;
					edge->input.ruleType = ruleType;

					vint count = usedFieldIds++;
					switch (ruleType)
					{
					case automaton::ReturnRuleType::Field:
						CHECK_ERROR(field != -1, ERROR_MESSAGE_PREFIX L"Field must set for ReturnRuleType::Field.");
						edge->insAfterInput.Add({ AstInsType::StackSlot,-1,count });
						fieldIns.Add({ AstInsType::Field,field,count });
						break;
					case automaton::ReturnRuleType::Partial:
						CHECK_ERROR(field == -1, ERROR_MESSAGE_PREFIX L"Field must not set for ReturnRuleType::Partial.");
						break;
					case automaton::ReturnRuleType::Discard:
						CHECK_ERROR(field == -1, ERROR_MESSAGE_PREFIX L"Field must not set for ReturnRuleType::Discard.");
						edge->insAfterInput.Add({ AstInsType::StackSlot,-1,count });
						break;
					case automaton::ReturnRuleType::Reuse:
						CHECK_ERROR(field == -1, ERROR_MESSAGE_PREFIX L"Field must not set for ReturnRuleType::Reuse.");
						break;
					}
				}

				switch (ruleType)
				{
				case automaton::ReturnRuleType::Reuse:
					clauseDisplayText += L"!" + rule->Name();
					break;
				default:
					clauseDisplayText += rule->Name();
					break;
				}
				endPoses.Add(pair.end, clauseDisplayText.Length());
				return pair;
#undef ERROR_MESSAGE_PREFIX
			}

			AutomatonBuilder::StatePair AutomatonBuilder::BuildFieldRuleSyntax(RuleSymbol* rule, vint32_t field)
			{
				return BuildRuleSyntaxInternal(rule, field, automaton::ReturnRuleType::Field);
			}

			AutomatonBuilder::StatePair AutomatonBuilder::BuildDiscardRuleSyntax(RuleSymbol* rule)
			{
				return BuildRuleSyntaxInternal(rule, -1, automaton::ReturnRuleType::Discard);
			}

			AutomatonBuilder::StatePair AutomatonBuilder::BuildUseSyntax(RuleSymbol* rule)
			{
				return BuildRuleSyntaxInternal(rule, -1, automaton::ReturnRuleType::Reuse);
			}

			AutomatonBuilder::StatePair AutomatonBuilder::BuildLoopSyntax(const StateBuilder& loopBody, const StateBuilder& loopDelimiter, bool hasDelimiter)
			{
				/*
				*     +--------------------+
				*     |                    V
				* S --+--+--(loopBody)--+--+--> E
				*        ^              |
				*        +--------------+
				*/

				/*
				*     +-------------------------+
				*     |                         V
				* S --+--+--(  loopBody   )--+--+--> E
				*        ^                   |
				*        +--(loopDelimiter)--+
				*/

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
				/*
				*     +------------------+
				*     |                  V
				* S --+--(optionalBody)--+--> E
				*/

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

				if (preferTake || preferSkip)
				{
					vint32_t id = ++ruleSymbol->Owner()->usedCompetitionIds;
					takeEdge->competitions.Add({ id,preferTake });
					skipEdge->competitions.Add({ id,preferSkip });
				}

				endPoses.Add(pair.end, clauseDisplayText.Length());
				return pair;
			}

			AutomatonBuilder::StatePair AutomatonBuilder::BuildSequenceSyntax(collections::List<StateBuilder>& elements)
			{
				/*
				* S --(a)--> ? --(b)--> E
				*/
				CHECK_ERROR(elements.Count() > 0, L"vl::glr::parsergen::AutomatonBuilder::BuildSequenceSyntax(List<StateBuilder>&)#Elements must not be empty.");
				auto pair = elements[0]();
				// TODO: (enumerable) Linq:Skip
				for (vint i = 1; i < elements.Count(); i++)
				{
					clauseDisplayText += L" ";
					auto nextPair = elements[i]();
					CreateEdge(pair.end, nextPair.begin);
					pair.end = nextPair.end;
				}
				return pair;
			}

			AutomatonBuilder::StatePair AutomatonBuilder::BuildAlternativeSyntax(collections::List<StateBuilder>& elements)
			{
				/*
				*     +--(a)--+
				*     |       V
				* S --+--(b)--+--> E
				*/

				CHECK_ERROR(elements.Count() > 0, L"vl::glr::parsergen::AutomatonBuilder::BuildAlternativeSyntax(List<StateBuilder>&)#Elements must not be empty.");
				StatePair pair;
				pair.begin = CreateState();
				pair.end = CreateState();
				startPoses.Add(pair.begin, clauseDisplayText.Length());

				clauseDisplayText += L"( ";
				// TODO: (enumerable) foreach:indexed
				for (vint i = 0; i < elements.Count(); i++)
				{
					if (i > 0) clauseDisplayText += L" | ";
					auto branchPair = elements[i]();
					CreateEdge(pair.begin, branchPair.begin);
					CreateEdge(branchPair.end, pair.end);
				}
				clauseDisplayText += L" )";

				endPoses.Add(pair.end, clauseDisplayText.Length());
				return pair;
			}

/***********************************************************************
AutomatonBuilder (Clause)
***********************************************************************/

			AutomatonBuilder::StatePair AutomatonBuilder::BuildClause(const StateBuilder& compileSyntax)
			{
				usedFieldIds = 0;
				fieldIns.Clear();
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

			AutomatonBuilder::StatePair AutomatonBuilder::BuildAssignment(StatePair pair, vint32_t enumItem, vint32_t field, bool weakAssignment)
			{
				auto withState = CreateState();
				auto edge = CreateEdge(pair.end, withState);
				vint count = usedFieldIds++;
				edge->insAfterInput.Add({ AstInsType::EnumItem,enumItem,count });
				fieldIns.Add({ (weakAssignment ? AstInsType::FieldIfUnassigned : AstInsType::Field),field,count});
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
					edge->insAfterInput.Add({ AstInsType::StackBegin });
				}
				{
					auto edge = CreateEdge(bodyPair.end, pair.end);
					edge->insAfterInput.Add({ AstInsType::CreateObject,classId });
					CopyFrom(edge->insAfterInput, fieldIns, true);
					edge->insAfterInput.Add({ AstInsType::StackEnd });
				}
				endPoses.Add(pair.end, clauseDisplayText.Length());
				return pair;
			}

			AutomatonBuilder::StatePair AutomatonBuilder::BuildReuseClause(const StateBuilder& compileSyntax)
			{
				StatePair pair;
				pair.begin = CreateState();
				pair.end = CreateState();
				startPoses.Add(pair.begin, clauseDisplayText.Length());

				clauseDisplayText += L"<! ";
				auto bodyPair = compileSyntax();
				clauseDisplayText += L" !>";
				{
					auto edge = CreateEdge(pair.begin, bodyPair.begin);
					edge->insAfterInput.Add({ AstInsType::StackBegin });
				}
				{
					auto edge = CreateEdge(bodyPair.end, pair.end);
					CopyFrom(edge->insAfterInput, fieldIns, true);
					edge->insAfterInput.Add({ AstInsType::StackEnd });
				}
				endPoses.Add(pair.end, clauseDisplayText.Length());
				return pair;
			}
		}
	}
}
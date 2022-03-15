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

					switch (ruleType)
					{
					case automaton::ReturnRuleType::Field:
						CHECK_ERROR(field != -1, ERROR_MESSAGE_PREFIX L"Field must set for ReturnRuleType::Field.");
						edge->insAfterInput.Add({ AstInsType::Field,field });
						break;
					case automaton::ReturnRuleType::Partial:
						CHECK_ERROR(field == -1, ERROR_MESSAGE_PREFIX L"Field must not set for ReturnRuleType::Partial.");
						break;
					case automaton::ReturnRuleType::Discard:
						CHECK_ERROR(field == -1, ERROR_MESSAGE_PREFIX L"Field must not set for ReturnRuleType::Discard.");
						edge->insAfterInput.Add({ AstInsType::DiscardValue });
						break;
					case automaton::ReturnRuleType::Reuse:
						CHECK_ERROR(field == -1, ERROR_MESSAGE_PREFIX L"Field must not set for ReturnRuleType::Reuse.");
						edge->insAfterInput.Add({ AstInsType::ReopenObject });
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

			AutomatonBuilder::StatePair AutomatonBuilder::BuildPartialRuleSyntax(RuleSymbol* rule)
			{
				return BuildRuleSyntaxInternal(rule, -1, automaton::ReturnRuleType::Partial);
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

			AutomatonBuilder::StatePair AutomatonBuilder::BuildSequenceSyntax(collections::List<StateBuilder>& elements)
			{
				CHECK_ERROR(elements.Count() > 0, L"vl::glr::parsergen::AutomatonBuilder::BuildSequenceSyntax(List<StateBuilder>&)#Elements must not be empty.");
				auto pair = elements[0]();
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
				CHECK_ERROR(elements.Count() > 0, L"vl::glr::parsergen::AutomatonBuilder::BuildAlternativeSyntax(List<StateBuilder>&)#Elements must not be empty.");
				StatePair pair;
				pair.begin = CreateState();
				pair.end = CreateState();
				startPoses.Add(pair.begin, clauseDisplayText.Length());

				clauseDisplayText += L"( ";
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

			AutomatonBuilder::StatePair AutomatonBuilder::BuildPushConditionSyntax(collections::Dictionary<vint32_t, bool>& switches, const StateBuilder& pushBody)
			{
				StatePair pair;
				pair.begin = CreateState();
				pair.end = CreateState();

				clauseDisplayText += L"!(";
				for (auto [pair, index] : indexed(switches))
				{
					if (index > 0) clauseDisplayText += L",";
					if (!pair.value) clauseDisplayText += L"!";
					clauseDisplayText += itow(pair.key);
				}

				clauseDisplayText += L";";
				auto bodyPair = pushBody();
				clauseDisplayText += L")";

				auto enterEdge = CreateEdge(pair.begin, bodyPair.begin);
				auto exitEdge = CreateEdge(bodyPair.end, pair.end);

				enterEdge->insSwitch.Add({ automaton::SwitchInsType::SwitchPushFrame });
				for (auto [key, value] : switches)
				{
					if (value)
					{
						enterEdge->insSwitch.Add({ automaton::SwitchInsType::SwitchWriteTrue,key });
					}
					else
					{
						enterEdge->insSwitch.Add({ automaton::SwitchInsType::SwitchWriteFalse,key });
					}
				}
				exitEdge->insSwitch.Add({ automaton::SwitchInsType::SwitchPopFrame });

				return pair;
			}

			void AutomatonBuilder::PrintCondition(collections::List<automaton::SwitchIns>& insSwitch)
			{
				for (auto ins : insSwitch)
				{
					switch (ins.type)
					{
					case automaton::SwitchInsType::ConditionRead:
						clauseDisplayText += L"[" + itow(ins.param) + L"]";
						break;
					case automaton::SwitchInsType::ConditionNot:
						clauseDisplayText += L"!";
						break;
					case automaton::SwitchInsType::ConditionAnd:
						clauseDisplayText += L"&";
						break;
					case automaton::SwitchInsType::ConditionOr:
						clauseDisplayText += L"|";
						break;
					case automaton::SwitchInsType::ConditionTest:
						clauseDisplayText += L"?";
						break;
					}
				}
			}

			AutomatonBuilder::StatePair AutomatonBuilder::BuildTestConditionBranch(collections::List<automaton::SwitchIns>& insSwitch)
			{
				StatePair pair;
				pair.begin = CreateState();
				pair.end = CreateState();

				clauseDisplayText += L"(";
				PrintCondition(insSwitch);
				clauseDisplayText += L" )";

				auto edge = CreateEdge(pair.begin, pair.end);
				CopyFrom(edge->insSwitch, insSwitch);
				return pair;
			}

			AutomatonBuilder::StatePair AutomatonBuilder::BuildTestConditionBranch(collections::List<automaton::SwitchIns>& insSwitch, const StateBuilder& branchBody)
			{
				auto state = CreateState();

				clauseDisplayText += L"(";
				PrintCondition(insSwitch);
				clauseDisplayText += L" ";
				auto pair = branchBody();
				clauseDisplayText += L")";

				auto edge = CreateEdge(state, pair.begin);
				CopyFrom(edge->insSwitch, insSwitch);
				return { state,pair.end };
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

			AutomatonBuilder::StatePair AutomatonBuilder::BuildAssignment(StatePair pair, vint32_t enumItem, vint32_t field, bool weakAssignment)
			{
				auto withState = CreateState();
				auto edge = CreateEdge(pair.end, withState);
				edge->insBeforeInput.Add({ AstInsType::EnumItem,enumItem });
				edge->insBeforeInput.Add({ (weakAssignment ? AstInsType::FieldIfUnassigned : AstInsType::Field),field});
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

			AutomatonBuilder::StatePair AutomatonBuilder::BuildLrpClause(collections::List<vint32_t>& flags)
			{
				CHECK_FAIL(L"Not Implemented!");
			}

			AutomatonBuilder::StatePair AutomatonBuilder::BuildLriClause(RuleSymbol* rule, collections::List<RuleSymbol*>& targetRules)
			{
				CHECK_FAIL(L"Not Implemented!");
			}
		}
	}
}
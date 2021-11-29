#include "Compiler.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;

			using GlrRuleMap = Dictionary<RuleSymbol*, GlrRule*>;
			using LiteralTokenMap = Dictionary<GlrLiteralSyntax*, vint>;
			using RuleReuseDependencies = Group<RuleSymbol*, RuleSymbol*>;
			using RuleKnownTypes = Group<RuleSymbol*, AstClassSymbol*>;
			using ClauseReuseDependencies = Group<GlrClause*, RuleSymbol*>;
			using ClauseTypeMap = Dictionary<GlrClause*, AstClassSymbol*>;

			struct VisitorContext
			{
				ParserSymbolManager&		global;
				AstSymbolManager&			astManager;
				LexerSymbolManager&			lexerManager;
				SyntaxSymbolManager&		syntaxManager;
				Ptr<CppParserGenOutput>		output;

				GlrRuleMap					astRules;
				LiteralTokenMap				literalTokens;
				RuleReuseDependencies		ruleReuseDependencies;
				RuleKnownTypes				ruleKnownTypes;
				ClauseReuseDependencies		clauseReuseDependencies;
				ClauseTypeMap				clauseTypes;

				VisitorContext(
					AstSymbolManager& _astManager,
					LexerSymbolManager& _lexerManager,
					SyntaxSymbolManager& _syntaxManager,
					Ptr<CppParserGenOutput> _output
				)
					: global(_syntaxManager.Global())
					, astManager(_astManager)
					, lexerManager(_lexerManager)
					, syntaxManager(_syntaxManager)
					, output(_output)
				{
				}
			};

/***********************************************************************
ResolveNameVisitor
***********************************************************************/

			class ResolveNameVisitor
				: public Object
				, public virtual GlrSyntax::IVisitor
				, public virtual GlrClause::IVisitor
			{
			protected:
				VisitorContext&				context;
				RuleSymbol*					ruleSymbol;
				GlrClause*					clause = nullptr;

				AstClassSymbol* GetRuleClass(const WString& typeName)
				{
					vint index = context.astManager.Symbols().Keys().IndexOf(typeName);
					if (index == -1)
					{
						context.global.AddError(ParserErrorType::TypeNotExistsInRule, ruleSymbol->Name(), typeName);
						return nullptr;
					}

					auto classSymbol = dynamic_cast<AstClassSymbol*>(context.astManager.Symbols().Values()[index]);
					if (!classSymbol)
					{
						context.global.AddError(ParserErrorType::TypeNotClassInRule, ruleSymbol->Name(), typeName);
					}
					return classSymbol;
				}
			public:
				vint						createCount = 0;
				vint						partialCount = 0;
				vint						reuseCount = 0;

				ResolveNameVisitor(
					VisitorContext& _context,
					RuleSymbol* _ruleSymbol
				)
					: context(_context)
					, ruleSymbol(_ruleSymbol)
				{
				}

				void Visit(GlrRefSyntax* node) override
				{
					vint tokenIndex = context.lexerManager.TokenOrder().IndexOf(node->name.value);
					vint ruleIndex = context.syntaxManager.Rules().Keys().IndexOf(node->name.value);
					if (tokenIndex == -1 && ruleIndex == -1)
					{
						context.global.AddError(ParserErrorType::TokenOrRuleNotExistsInRule, ruleSymbol->Name(), node->name.value);
					}
				}

				void Visit(GlrLiteralSyntax* node) override
				{
					if (node->value.value.Length() > 2)
					{
						Array<wchar_t> buffer(node->value.value.Length());
						wchar_t* writing = &buffer[0];

						for (vint i = 1; i < node->value.value.Length() - 1; i++)
						{
							wchar_t c = node->value.value[i];
							*writing++ = c;
							if (c == L'\"')
							{
								i++;
							}
						}
						*writing = 0;

						auto literalValue = WString::Unmanaged(&buffer[0]);
						for (auto&& [tokenName, tokenIndex] : indexed(context.lexerManager.TokenOrder()))
						{
							auto tokenSymbol = context.lexerManager.Tokens()[tokenName];
							if (tokenSymbol->displayText==literalValue)
							{
								context.literalTokens.Add(node, tokenIndex);
								return;
							}
						}
					}
					context.global.AddError(ParserErrorType::TokenOrRuleNotExistsInRule, ruleSymbol->Name(), node->value.value);
				}

				void Visit(GlrUseSyntax* node) override
				{
					vint ruleIndex = context.syntaxManager.Rules().Keys().IndexOf(node->name.value);
					if (ruleIndex == -1)
					{
						context.global.AddError(ParserErrorType::TokenOrRuleNotExistsInRule, ruleSymbol->Name(), node->name.value);
					}
					else if (clause)
					{
						auto usedRuleSymbol = context.syntaxManager.Rules().Values()[ruleIndex];
						if (!context.ruleReuseDependencies.Contains(ruleSymbol, usedRuleSymbol))
						{
							context.ruleReuseDependencies.Add(ruleSymbol, usedRuleSymbol);
						}
						if (!context.clauseReuseDependencies.Contains(clause, usedRuleSymbol))
						{
							context.clauseReuseDependencies.Add(clause, usedRuleSymbol);
						}
					}
				}

				void Visit(GlrLoopSyntax* node) override
				{
					node->syntax->Accept(this);
					if (node->delimiter)
					{
						node->delimiter->Accept(this);
					}
				}

				void Visit(GlrOptionalSyntax* node) override
				{
					node->syntax->Accept(this);
				}

				void Visit(GlrSequenceSyntax* node) override
				{
					node->first->Accept(this);
					node->second->Accept(this);
				}

				void Visit(GlrAlternativeSyntax* node) override
				{
					node->first->Accept(this);
					node->second->Accept(this);
				}

				void Visit(GlrCreateClause* node) override
				{
					createCount++;
					if (auto classSymbol = GetRuleClass(node->type.value))
					{
						context.ruleKnownTypes.Add(ruleSymbol, classSymbol);
						context.clauseTypes.Add(node, classSymbol);
					}
					node->syntax->Accept(this);
				}

				void Visit(GlrPartialClause* node) override
				{
					partialCount++;
					if (auto classSymbol = GetRuleClass(node->type.value))
					{
						context.ruleKnownTypes.Add(ruleSymbol, classSymbol);
						context.clauseTypes.Add(node, classSymbol);
					}
					node->syntax->Accept(this);
				}

				void Visit(Glr_ReuseClause* node) override
				{
					reuseCount++;
					clause = node;
					node->syntax->Accept(this);
				}
			};

/***********************************************************************
CompileSyntaxVisitor
***********************************************************************/

			class CompileSyntaxVisitor
				: public Object
				, public virtual GlrSyntax::IVisitor
				, public virtual GlrClause::IVisitor
			{
			protected:
				struct StatePair
				{
					StateSymbol* begin;
					StateSymbol* end;
				};

				using StatePosMap = Dictionary<StateSymbol*, vint>;
			protected:
				VisitorContext&				context;
				RuleSymbol*					ruleSymbol;
				AstClassSymbol*				clauseType;

				WString						clauseDisplayText;
				StatePosMap					startPoses;
				StatePosMap					endPoses;
				StatePair					result;

				StateSymbol* CreateState()
				{
					return ruleSymbol->Owner()->CreateState(ruleSymbol);
				}

				EdgeSymbol* CreateEdge(StateSymbol* from, StateSymbol* to)
				{
					return ruleSymbol->Owner()->CreateEdge(from, to);
				}

				StatePair Build(const Ptr<GlrSyntax>& node)
				{
					node->Accept(this);
					return result;
				}

				StatePair Build(const Ptr<GlrClause>& node)
				{
					node->Accept(this);
					return result;
				}
			public:
				CompileSyntaxVisitor(
					VisitorContext& _context,
					RuleSymbol* _ruleSymbol
				)
					: context(_context)
					, ruleSymbol(_ruleSymbol)
				{
				}

				void AssignClause(const Ptr<GlrClause>& node)
				{
					clauseDisplayText = L"";
					startPoses.Clear();
					endPoses.Clear();

					auto pair = Build(node);
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

				void Visit(GlrRefSyntax* node) override
				{
					{
						vint index = context.lexerManager.TokenOrder().IndexOf(node->name.value);
						if (index != -1)
						{
							StatePair pair;
							pair.begin = CreateState();
							pair.end = CreateState();
							startPoses.Add(pair.begin, clauseDisplayText.Length());

							{
								auto edge = CreateEdge(pair.begin, pair.end);
								edge->input.type = EdgeInputType::Token;
								edge->input.token = index;
								if (node->field)
								{
									auto propSymbol = clauseType->Props()[node->field.value];
									edge->insAfterInput.Add({ AstInsType::Token });
									edge->insAfterInput.Add({ AstInsType::Field,context.output->fieldIds[propSymbol] });
								}
							}

							clauseDisplayText += context.lexerManager.Tokens()[node->name.value]->displayText;
							endPoses.Add(pair.end, clauseDisplayText.Length());
							result = pair;
							return;
						}
					}
					{
						vint index = context.syntaxManager.Rules().Keys().IndexOf(node->name.value);
						if (index != -1)
						{
							auto rule = context.syntaxManager.Rules().Values()[index];
							StatePair pair;
							pair.begin = CreateState();
							pair.end = CreateState();
							startPoses.Add(pair.begin, clauseDisplayText.Length());

							{
								auto edge = CreateEdge(pair.begin, pair.end);
								edge->input.type = EdgeInputType::Rule;
								edge->input.rule = rule;
								if (node->field)
								{
									auto propSymbol = clauseType->Props()[node->field.value];
									edge->insAfterInput.Add({ AstInsType::Field,context.output->fieldIds[propSymbol] });
								}
								else if (!rule->isPartial)
								{
									edge->insAfterInput.Add({ AstInsType::DiscardValue });
								}
							}

							clauseDisplayText += rule->Name();
							endPoses.Add(pair.end, clauseDisplayText.Length());
							result = pair;
							return;
						}
					}
					CHECK_FAIL(L"Should not reach here!");
				}

				void Visit(GlrLiteralSyntax* node) override
				{
					vint index = context.literalTokens[node];
					StatePair pair;
					pair.begin = CreateState();
					pair.end = CreateState();
					startPoses.Add(pair.begin, clauseDisplayText.Length());

					{
						auto edge = CreateEdge(pair.begin, pair.end);
						edge->input.type = EdgeInputType::Token;
						edge->input.token = context.literalTokens[node];
					}

					clauseDisplayText += context.lexerManager.Tokens()[context.lexerManager.TokenOrder()[index]]->displayText;
					endPoses.Add(pair.end, clauseDisplayText.Length());
					result = pair;
				}

				void Visit(GlrUseSyntax* node) override
				{
					vint index = context.syntaxManager.Rules().Keys().IndexOf(node->name.value);
					if (index == -1)
					{
						context.global.AddError(ParserErrorType::TokenOrRuleNotExistsInRule, node->name.value);
						return;
					}

					auto rule = context.syntaxManager.Rules().Values()[index];
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
					result = pair;
				}

				void Visit(GlrLoopSyntax* node) override
				{
					StatePair pair, bodyPair, delimiterPair;
					pair.begin = CreateState();
					pair.end = CreateState();
					startPoses.Add(pair.begin, clauseDisplayText.Length());

					clauseDisplayText += L"{ ";
					bodyPair = Build(node->syntax);
					if (node->delimiter)
					{
						clauseDisplayText += L" ; ";
						delimiterPair = Build(node->delimiter);
					}
					clauseDisplayText += L" }";

					CreateEdge(pair.begin, bodyPair.begin);
					CreateEdge(bodyPair.end, pair.end);
					CreateEdge(pair.begin, pair.end);
					if (node->delimiter)
					{
						CreateEdge(bodyPair.end, delimiterPair.begin);
						CreateEdge(delimiterPair.end, bodyPair.begin);
					}
					else
					{
						CreateEdge(bodyPair.end, bodyPair.begin);
					}

					endPoses.Add(pair.end, clauseDisplayText.Length());
					result = pair;
				}

				void Visit(GlrOptionalSyntax* node) override
				{
					StatePair pair;
					pair.begin = CreateState();
					pair.end = CreateState();
					startPoses.Add(pair.begin, clauseDisplayText.Length());

					clauseDisplayText += L"[ ";
					auto bodyPair = Build(node->syntax);
					clauseDisplayText += L" ]";

					CreateEdge(pair.begin, bodyPair.begin);
					CreateEdge(bodyPair.end, pair.end);
					CreateEdge(pair.begin, pair.end);

					endPoses.Add(pair.end, clauseDisplayText.Length());
					result = pair;
				}

				void Visit(GlrSequenceSyntax* node) override
				{
					auto firstPair = Build(node->first);
					clauseDisplayText += L" ";
					auto secondPair = Build(node->second);
					CreateEdge(firstPair.end, secondPair.begin);
					result = { firstPair.begin,secondPair.end };
				}

				void Visit(GlrAlternativeSyntax* node) override
				{
					StatePair pair;
					pair.begin = CreateState();
					pair.end = CreateState();
					startPoses.Add(pair.begin, clauseDisplayText.Length());

					clauseDisplayText += L"( ";
					auto firstPair = Build(node->first);
					clauseDisplayText += L" | ";
					auto secondPair = Build(node->second);
					clauseDisplayText += L" )";

					CreateEdge(pair.begin, firstPair.begin);
					CreateEdge(firstPair.end, pair.end);
					CreateEdge(pair.begin, secondPair.begin);
					CreateEdge(secondPair.end, pair.end);

					endPoses.Add(pair.end, clauseDisplayText.Length());
					result = pair;
				}

				StatePair Visit(StatePair pair, GlrAssignment* node)
				{
					auto withState = CreateState();
					auto edge = CreateEdge(pair.end, withState);

					auto propSymbol = clauseType->Props()[node->field.value];
					auto enumSymbol = dynamic_cast<AstEnumSymbol*>(propSymbol->propSymbol);
					edge->insBeforeInput.Add({ AstInsType::EnumItem,enumSymbol->ItemOrder().IndexOf(node->value.value) });
					edge->insBeforeInput.Add({ AstInsType::Field,context.output->fieldIds[propSymbol] });

					endPoses.Add(withState, clauseDisplayText.Length());
					return { pair.begin,withState };
				}

				void Visit(GlrCreateClause* node) override
				{
					clauseType = context.clauseTypes[node];
					StatePair pair;
					pair.begin = CreateState();
					pair.end = CreateState();
					startPoses.Add(pair.begin, clauseDisplayText.Length());

					clauseDisplayText += L"< ";
					auto bodyPair = Build(node->syntax);
					for (auto assignment : node->assignments)
					{
						bodyPair = Visit(bodyPair, assignment.Obj());
					}
					clauseDisplayText += L" >";
					{
						auto classSymbol = dynamic_cast<AstClassSymbol*>(context.astManager.Symbols()[node->type.value]);
						auto edge = CreateEdge(pair.begin, bodyPair.begin);
						edge->insBeforeInput.Add({ AstInsType::BeginObject,context.output->classIds[classSymbol] });
					}
					{
						auto edge = CreateEdge(bodyPair.end, pair.end);
						edge->insBeforeInput.Add({ AstInsType::EndObject });
					}
					endPoses.Add(pair.end, clauseDisplayText.Length());
					result = pair;
				}

				void Visit(GlrPartialClause* node) override
				{
					clauseType = context.clauseTypes[node];
					auto bodyPair = Build(node->syntax);
					for (auto assignment : node->assignments)
					{
						bodyPair = Visit(bodyPair, assignment.Obj());
					}
				}

				void Visit(Glr_ReuseClause* node) override
				{
					clauseType = context.clauseTypes[node];
					StatePair pair;
					pair.begin = CreateState();
					pair.end = CreateState();
					startPoses.Add(pair.begin, clauseDisplayText.Length());

					clauseDisplayText += L"<< ";
					auto bodyPair = Build(node->syntax);
					for (auto assignment : node->assignments)
					{
						bodyPair = Visit(bodyPair, assignment.Obj());
					}
					clauseDisplayText += L" >>";
					CreateEdge(pair.begin, bodyPair.begin);
					{
						auto edge = CreateEdge(bodyPair.end, pair.end);
						edge->insBeforeInput.Add({ AstInsType::EndObject });
					}
					endPoses.Add(pair.end, clauseDisplayText.Length());
					result = pair;
				}
			};

/***********************************************************************
CalculateRuleAndClauseTypes
***********************************************************************/

			AstClassSymbol* MergeClassSymbol(AstClassSymbol* c1, AstClassSymbol* c2)
			{
				if (c1 == c2) return c1;
				if (!c1) return c2;
				if (!c2) return c1;

				// find common base classes
				vint n1 = 0, n2 = 0;
				{
					auto c = c1;
					while (c)
					{
						n1++;
						c = c->baseClass;
					}
				}
				{
					auto c = c2;
					while (c)
					{
						n2++;
						c = c->baseClass;
					}
				}

				while (n1 > n2)
				{
					n1--;
					c1 = c1->baseClass;
				}
				while (n2 > n1)
				{
					n2--;
					c2 = c2->baseClass;
				}

				while (c1 && c2)
				{
					if (c1 == c2) return c1;
					c1 = c1->baseClass;
					c2 = c2->baseClass;
				}
				return nullptr;
			}

			void CalculateRuleAndClauseTypes(VisitorContext& context)
			{
				// find cyclic dependencies in "Rule ::= !Rule"
				auto&& rules = context.syntaxManager.Rules().Values();
				PartialOrderingProcessor pop;
				pop.InitWithGroup(rules, context.ruleReuseDependencies);
				pop.Sort();

				// remove cyclic dependended rules from ruleReuseDependencies
				for (auto&& component : pop.components)
				{
					for (vint i = 0; i < component.nodeCount - 1; i++)
					{
						for (vint j = i + 1; j < component.nodeCount; j++)
						{
							auto r1 = rules[component.firstNode[i]];
							auto r2 = rules[component.firstNode[j]];
							context.ruleReuseDependencies.Remove(r1, r2);
							context.ruleReuseDependencies.Remove(r2, r1);
						}
					}
				}

				// calculate types for rules from clauses with known types
				for (auto rule : rules)
				{
					for (auto clause : context.astRules[rule]->clauses)
					{
						vint index = context.clauseTypes.Keys().IndexOf(clause.Obj());
						if (index != -1)
						{
							rule->ruleType = MergeClassSymbol(rule->ruleType, context.clauseTypes.Values()[index]);
						}
					}
				}

				// calculate types for rules that contain reuse dependency
				for (auto&& component : pop.components)
				{
					for (vint i = 0; i < component.nodeCount; i++)
					{
						auto rule = rules[component.firstNode[i]];
						vint index = context.ruleReuseDependencies.Keys().IndexOf(rule);
						if (index != -1)
						{
							for (auto dep : context.ruleReuseDependencies.GetByIndex(index))
							{
								rule->ruleType = MergeClassSymbol(rule->ruleType, dep->ruleType);
							}
						}
					}
				}

				// prompt errors
				for (auto rule : rules)
				{
					if (!rule->ruleType)
					{
						context.global.AddError(ParserErrorType::RuleCannotResolveToDeterministicType, rule->Name());
					}
				}

				// calculate types for reuse clauses
				for (auto&& [clause, index] : indexed(context.clauseReuseDependencies.Keys()))
				{
					AstClassSymbol* type = nullptr;
					for (auto dep : context.clauseReuseDependencies.GetByIndex(index))
					{
						type = MergeClassSymbol(type, dep->ruleType);
					}

					if (type)
					{
						context.clauseTypes.Add(clause, type);
					}
				}
			}

/***********************************************************************
CompileSyntax
***********************************************************************/

			void CompileSyntax(AstSymbolManager& astManager, LexerSymbolManager& lexerManager, SyntaxSymbolManager& syntaxManager, Ptr<CppParserGenOutput> output, collections::List<Ptr<GlrSyntaxFile>>& files)
			{
				VisitorContext context(astManager, lexerManager, syntaxManager, output);

				// check rule names
				for (auto file : files)
				{
					for (auto rule : file->rules)
					{
						if (lexerManager.Tokens().Keys().Contains(rule->name.value))
						{
							syntaxManager.Global().AddError(ParserErrorType::RuleNameConflictedWithToken, rule->name.value);
						}
						else
						{
							auto ruleSymbol = syntaxManager.CreateRule(rule->name.value);
							context.astRules.Add(ruleSymbol, rule.Obj());
						}
					}
				}
				if (syntaxManager.Global().Errors().Count() > 0) return;

				// resolve tokens, rules, types
				for (auto file : files)
				{
					for (auto rule : file->rules)
					{
						auto ruleSymbol = syntaxManager.Rules()[rule->name.value];
						ResolveNameVisitor visitor(context, ruleSymbol);
						for (auto clause : rule->clauses)
						{
							clause->Accept(&visitor);
						}

						if (visitor.partialCount > 0 && (visitor.createCount > 0 || visitor.reuseCount > 0))
						{
							syntaxManager.Global().AddError(ParserErrorType::RuleCannotResolveToDeterministicType, ruleSymbol->Name());
						}
					}
				}
				if (syntaxManager.Global().Errors().Count() > 0) return;

				// resolve types for rules and clauses
				CalculateRuleAndClauseTypes(context);
				if (syntaxManager.Global().Errors().Count() > 0) return;

				// check syntax structures

				// build eNFA
				for (auto file : files)
				{
					for (auto rule : file->rules)
					{
						auto ruleSymbol = syntaxManager.Rules()[rule->name.value];
						CompileSyntaxVisitor visitor(context, ruleSymbol);
						for (auto clause : rule->clauses)
						{
							visitor.AssignClause(clause);
						}
					}
				}
			}
		}
	}
}
#include "Compiler.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;

/***********************************************************************
ResolveNameVisitor
***********************************************************************/

			class ResolveNameVisitor
				: public Object
				, public virtual GlrSyntax::IVisitor
				, public virtual GlrClause::IVisitor
			{
				using LiteralTokenMap = Dictionary<GlrLiteralSyntax*, vint>;
			protected:
				ParserSymbolManager&		global;
				AstSymbolManager&			astManager;
				LexerSymbolManager&			lexerManager;
				SyntaxSymbolManager&		syntaxManager;

				LiteralTokenMap&			literalTokens;
				RuleSymbol*					ruleSymbol;

				vint						createCount = 0;
				vint						partialCount = 0;
				vint						reuseCount = 0;

				AstClassSymbol* GetRuleClass(const WString& typeName)
				{
					vint index = astManager.Symbols().Keys().IndexOf(typeName);
					if (index == -1)
					{
						global.AddError(ParserErrorType::TypeNotExistsInRule, ruleSymbol->Name(), typeName);
						return nullptr;
					}

					auto classSymbol = dynamic_cast<AstClassSymbol*>(astManager.Symbols().Values()[index]);
					if (!classSymbol)
					{
						global.AddError(ParserErrorType::TypeNotClassInRule, ruleSymbol->Name(), typeName);
					}
					return classSymbol;
				}
			public:
				ResolveNameVisitor(
					AstSymbolManager& _astManager,
					LexerSymbolManager& _lexerManager,
					SyntaxSymbolManager& _syntaxManager,
					LiteralTokenMap& _literalTokens,
					RuleSymbol* _ruleSymbol
				)
					: global(_syntaxManager.Global())
					, astManager(_astManager)
					, lexerManager(_lexerManager)
					, syntaxManager(_syntaxManager)
					, literalTokens(_literalTokens)
					, ruleSymbol(_ruleSymbol)
				{
				}

				void Visit(GlrRefSyntax* node) override
				{
					vint tokenIndex = lexerManager.TokenOrder().IndexOf(node->name.value);
					vint ruleIndex = syntaxManager.Rules().Keys().IndexOf(node->name.value);
					if (tokenIndex == -1 && ruleIndex == -1)
					{
						global.AddError(ParserErrorType::TokenOrRuleNotExistsInRule, ruleSymbol->Name(), node->name.value);
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
						for (auto&& [tokenName, tokenIndex] : indexed(lexerManager.TokenOrder()))
						{
							auto tokenSymbol = lexerManager.Tokens()[tokenName];
							if (tokenSymbol->displayText==literalValue)
							{
								literalTokens.Add(node, tokenIndex);
								return;
							}
						}
					}
					global.AddError(ParserErrorType::TokenOrRuleNotExistsInRule, ruleSymbol->Name(), node->value.value);
				}

				void Visit(GlrUseSyntax* node) override
				{
					vint ruleIndex = syntaxManager.Rules().Keys().IndexOf(node->name.value);
					if (ruleIndex == -1)
					{
						global.AddError(ParserErrorType::TokenOrRuleNotExistsInRule, ruleSymbol->Name(), node->name.value);
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
					GetRuleClass(node->type.value);
					node->syntax->Accept(this);
				}

				void Visit(GlrPartialClause* node) override
				{
					partialCount++;
					GetRuleClass(node->type.value);
					node->syntax->Accept(this);
				}

				void Visit(Glr_ReuseClause* node) override
				{
					reuseCount++;
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
				using LiteralTokenMap = Dictionary<GlrLiteralSyntax*, vint>;
			protected:
				ParserSymbolManager&		global;
				AstSymbolManager&			astManager;
				LexerSymbolManager&			lexerManager;
				SyntaxSymbolManager&		syntaxManager;

				LiteralTokenMap&			literalTokens;
				Ptr<CppParserGenOutput>		output;
				RuleSymbol*					ruleSymbol;

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
					AstSymbolManager& _astManager,
					LexerSymbolManager& _lexerManager,
					SyntaxSymbolManager& _syntaxManager,
					LiteralTokenMap& _literalTokens,
					Ptr<CppParserGenOutput> _output,
					RuleSymbol* _ruleSymbol
				)
					: global(_syntaxManager.Global())
					, astManager(_astManager)
					, lexerManager(_lexerManager)
					, syntaxManager(_syntaxManager)
					, literalTokens(_literalTokens)
					, output(_output)
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
						vint index = lexerManager.TokenOrder().IndexOf(node->name.value);
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
									auto propSymbol = ruleSymbol->ruleType->Props()[node->field.value];
									edge->insAfterInput.Add({ AstInsType::Token });
									edge->insAfterInput.Add({ AstInsType::Field,output->fieldIds[propSymbol] });
								}
							}

							clauseDisplayText += lexerManager.Tokens()[node->name.value]->displayText;
							endPoses.Add(pair.end, clauseDisplayText.Length());
							result = pair;
							return;
						}
					}
					{
						vint index = syntaxManager.Rules().Keys().IndexOf(node->name.value);
						if (index != -1)
						{
							auto rule = syntaxManager.Rules().Values()[index];
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
									auto propSymbol = ruleSymbol->ruleType->Props()[node->field.value];
									edge->insAfterInput.Add({ AstInsType::Field,output->fieldIds[propSymbol] });
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
					vint index = literalTokens[node];
					StatePair pair;
					pair.begin = CreateState();
					pair.end = CreateState();
					startPoses.Add(pair.begin, clauseDisplayText.Length());

					{
						auto edge = CreateEdge(pair.begin, pair.end);
						edge->input.type = EdgeInputType::Token;
						edge->input.token = literalTokens[node];
					}

					clauseDisplayText += lexerManager.Tokens()[lexerManager.TokenOrder()[index]]->displayText;
					endPoses.Add(pair.end, clauseDisplayText.Length());
					result = pair;
				}

				void Visit(GlrUseSyntax* node) override
				{
					vint index = syntaxManager.Rules().Keys().IndexOf(node->name.value);
					if (index == -1)
					{
						global.AddError(ParserErrorType::TokenOrRuleNotExistsInRule, node->name.value);
						return;
					}

					auto rule = syntaxManager.Rules().Values()[index];
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

				void Visit(GlrCreateClause* node) override
				{
					StatePair pair;
					pair.begin = CreateState();
					pair.end = CreateState();
					startPoses.Add(pair.begin, clauseDisplayText.Length());

					clauseDisplayText += L"< ";
					auto bodyPair = Build(node->syntax);
					clauseDisplayText += L" >";
					{
						auto classSymbol = dynamic_cast<AstClassSymbol*>(astManager.Symbols()[node->type.value]);
						auto edge = CreateEdge(pair.begin, bodyPair.begin);
						edge->insBeforeInput.Add({ AstInsType::BeginObject,output->classIds[classSymbol] });
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
					Build(node->syntax);
				}

				void Visit(Glr_ReuseClause* node) override
				{
					StatePair pair;
					pair.begin = CreateState();
					pair.end = CreateState();
					startPoses.Add(pair.begin, clauseDisplayText.Length());

					clauseDisplayText += L"<< ";
					auto bodyPair = Build(node->syntax);
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
CompileSyntax
***********************************************************************/

			void CompileSyntax(AstSymbolManager& astManager, LexerSymbolManager& lexerManager, SyntaxSymbolManager& syntaxManager, Ptr<CppParserGenOutput> output, collections::List<Ptr<GlrSyntaxFile>>& files)
			{
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
							syntaxManager.CreateRule(rule->name.value);
						}
					}
				}
				if (syntaxManager.Global().Errors().Count() > 0) return;

				Dictionary<GlrLiteralSyntax*, vint> literalTokens;
				for (auto file : files)
				{
					for (auto rule : file->rules)
					{
						auto ruleSymbol = syntaxManager.Rules()[rule->name.value];
						ResolveNameVisitor visitor(astManager, lexerManager, syntaxManager, literalTokens, ruleSymbol);
						for (auto clause : rule->clauses)
						{
							clause->Accept(&visitor);
						}
					}
				}
				if (syntaxManager.Global().Errors().Count() > 0) return;

				for (auto file : files)
				{
					for (auto rule : file->rules)
					{
						auto ruleSymbol = syntaxManager.Rules()[rule->name.value];
						CompileSyntaxVisitor visitor(astManager, lexerManager, syntaxManager, literalTokens, output, ruleSymbol);
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
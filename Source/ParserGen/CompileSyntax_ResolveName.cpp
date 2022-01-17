#include "Compiler.h"

namespace vl
{
	namespace glr
	{
		namespace parsergen
		{
			using namespace collections;
			using namespace compile_syntax;

/***********************************************************************
ResolveNameVisitor
***********************************************************************/

			class ResolveNameVisitor
				: public Object
				, protected virtual GlrSyntax::IVisitor
				, protected virtual GlrClause::IVisitor
			{
			protected:
				VisitorContext&				context;
				RuleSymbol*					ruleSymbol;
				GlrClause*					clause = nullptr;

				AstClassSymbol* GetRuleClass(ParsingToken& typeName)
				{
					vint index = context.astManager.Symbols().Keys().IndexOf(typeName.value);
					if (index == -1)
					{
						context.syntaxManager.AddError(
							ParserErrorType::TypeNotExistsInRule,
							typeName.codeRange,
							ruleSymbol->Name(),
							typeName.value
							);
						return nullptr;
					}

					auto classSymbol = dynamic_cast<AstClassSymbol*>(context.astManager.Symbols().Values()[index]);
					if (!classSymbol)
					{
						context.syntaxManager.AddError(
							ParserErrorType::TypeNotClassInRule,
							typeName.codeRange,
							ruleSymbol->Name(),
							typeName.value
							);
					}
					return classSymbol;
				}
			public:
				ResolveNameVisitor(
					VisitorContext& _context,
					RuleSymbol* _ruleSymbol
				)
					: context(_context)
					, ruleSymbol(_ruleSymbol)
				{
				}

				void ResolveClause(Ptr<GlrClause> clause)
				{
					clause->Accept(this);
				}

			protected:

				void Visit(GlrRefSyntax* node) override
				{
					switch (node->refType)
					{
					case GlrRefType::Id:
						{
							vint tokenIndex = context.lexerManager.TokenOrder().IndexOf(node->literal.value);
							vint ruleIndex = context.syntaxManager.Rules().Keys().IndexOf(node->literal.value);
							if (tokenIndex == -1 && ruleIndex == -1)
							{
								context.syntaxManager.AddError(
									ParserErrorType::TokenOrRuleNotExistsInRule,
									node->codeRange,
									ruleSymbol->Name(),
									node->literal.value
									);
							}
						}
						break;
					case GlrRefType::Literal:
						{
							if (node->literal.value.Length() > 2)
							{
								auto literalValue = UnescapeLiteral(node->literal.value, L'\"');
								for (auto&& [tokenName, tokenIndex] : indexed(context.lexerManager.TokenOrder()))
								{
									auto tokenSymbol = context.lexerManager.Tokens()[tokenName];
									if (tokenSymbol->displayText == literalValue)
									{
										if (tokenSymbol->discarded)
										{
											context.syntaxManager.AddError(
												ParserErrorType::LiteralIsDiscardedToken,
												node->codeRange,
												ruleSymbol->Name(),
												node->literal.value
												);
										}
										else
										{
											context.literalTokens.Add(node, (vint32_t)tokenIndex);
										}
										return;
									}
								}
							}
							context.syntaxManager.AddError(
								ParserErrorType::LiteralNotValidToken,
								node->codeRange,
								ruleSymbol->Name(),
								node->literal.value
								);
						}
						break;
					case GlrRefType::ConditionalLiteral:
						{
							if (node->literal.value.Length() > 2)
							{
								auto literalValue = UnescapeLiteral(node->literal.value, L'\'');
								auto&& lexer = context.GetCachedLexer();
								List<regex::RegexToken> tokens;
								lexer.Parse(literalValue).ReadToEnd(tokens);
								if (tokens.Count() == 1 && tokens[0].token != -1 && tokens[0].completeToken)
								{
									auto tokenSymbol = context.lexerManager.Tokens()[context.lexerManager.TokenOrder()[tokens[0].token]];
									if (tokenSymbol->displayText != L"")
									{
										context.syntaxManager.AddError(
											ParserErrorType::ConditionalLiteralIsDisplayText,
											node->codeRange,
											ruleSymbol->Name(),
											node->literal.value
											);
									}
									if (tokenSymbol->discarded)
									{
										context.syntaxManager.AddError(
											ParserErrorType::ConditionalLiteralIsDiscardedToken,
											node->codeRange,
											ruleSymbol->Name(),
											node->literal.value
											);
									}
									context.literalTokens.Add(node, (vint32_t)tokens[0].token);
									return;
								}
							}
							context.syntaxManager.AddError(
								ParserErrorType::ConditionalLiteralNotValidToken,
								node->codeRange,
								ruleSymbol->Name(),
								node->literal.value
								);
						}
						break;
					default:;
					}
				}

				void Visit(GlrUseSyntax* node) override
				{
					vint ruleIndex = context.syntaxManager.Rules().Keys().IndexOf(node->name.value);
					if (ruleIndex == -1)
					{
						context.syntaxManager.AddError(
							ParserErrorType::TokenOrRuleNotExistsInRule,
							node->codeRange,
							ruleSymbol->Name(),
							node->name.value
							);
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
					if (auto classSymbol = GetRuleClass(node->type))
					{
						context.ruleKnownTypes.Add(ruleSymbol, classSymbol);
						context.clauseTypes.Add(node, classSymbol);
					}
					node->syntax->Accept(this);
				}

				void Visit(GlrPartialClause* node) override
				{
					if (auto classSymbol = GetRuleClass(node->type))
					{
						context.ruleKnownTypes.Add(ruleSymbol, classSymbol);
						context.clauseTypes.Add(node, classSymbol);
					}
					node->syntax->Accept(this);
				}

				void Visit(GlrReuseClause* node) override
				{
					clause = node;
					node->syntax->Accept(this);
				}
			};

/***********************************************************************
ResolveName
***********************************************************************/

			void ResolveName(VisitorContext& context, List<Ptr<GlrSyntaxFile>>& files)
			{
				for (auto file : files)
				{
					for (auto rule : file->rules)
					{
						auto ruleSymbol = context.syntaxManager.Rules()[rule->name.value];
						ResolveNameVisitor visitor(context, ruleSymbol);
						for (auto clause : rule->clauses)
						{
							visitor.ResolveClause(clause);
						}
					}
				}
			}
		}
	}
}
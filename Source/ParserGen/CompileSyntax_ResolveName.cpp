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

				AstClassSymbol* GetRuleClass(const WString& typeName)
				{
					vint index = context.astManager.Symbols().Keys().IndexOf(typeName);
					if (index == -1)
					{
						context.global.AddError(
							ParserErrorType::TypeNotExistsInRule,
							ruleSymbol->Name(),
							typeName
							);
						return nullptr;
					}

					auto classSymbol = dynamic_cast<AstClassSymbol*>(context.astManager.Symbols().Values()[index]);
					if (!classSymbol)
					{
						context.global.AddError(
							ParserErrorType::TypeNotClassInRule,
							ruleSymbol->Name(),
							typeName
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
					vint tokenIndex = context.lexerManager.TokenOrder().IndexOf(node->name.value);
					vint ruleIndex = context.syntaxManager.Rules().Keys().IndexOf(node->name.value);
					if (tokenIndex == -1 && ruleIndex == -1)
					{
						context.global.AddError(
							ParserErrorType::TokenOrRuleNotExistsInRule,
							ruleSymbol->Name(),
							node->name.value
							);
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
								context.literalTokens.Add(node, (vint32_t)tokenIndex);
								return;
							}
						}
					}
					context.global.AddError(
						ParserErrorType::TokenOrRuleNotExistsInRule,
						ruleSymbol->Name(),
						node->value.value
						);
				}

				void Visit(GlrUseSyntax* node) override
				{
					vint ruleIndex = context.syntaxManager.Rules().Keys().IndexOf(node->name.value);
					if (ruleIndex == -1)
					{
						context.global.AddError(
							ParserErrorType::TokenOrRuleNotExistsInRule,
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
					if (auto classSymbol = GetRuleClass(node->type.value))
					{
						context.ruleKnownTypes.Add(ruleSymbol, classSymbol);
						context.clauseTypes.Add(node, classSymbol);
					}
					node->syntax->Accept(this);
				}

				void Visit(GlrPartialClause* node) override
				{
					if (auto classSymbol = GetRuleClass(node->type.value))
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
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
ValidateTypesVisitor
***********************************************************************/

			class ValidateTypesVisitor
				: public Object
				, protected virtual GlrSyntax::IVisitor
				, protected virtual GlrClause::IVisitor
			{
			protected:
				VisitorContext&				context;
				RuleSymbol*					ruleSymbol;
				GlrClause*					clause = nullptr;
				
				AstClassPropSymbol* FindField(AstClassSymbol*& clauseType, ParsingToken& name)
				{
					clauseType = context.clauseTypes[clause];
					if (auto prop = FindPropSymbol(clauseType, name.value))
					{
						if (prop->propType != AstPropType::Array)
						{
							ruleSymbol->assignedNonArrayField = true;
						}
						return prop;
					}
					else
					{
						context.syntaxManager.AddError(
							ParserErrorType::FieldNotExistsInClause,
							name.codeRange,
							ruleSymbol->Name(),
							clauseType->Name(),
							name.value
							);
						return nullptr;
					}
				}

				bool ConvertibleTo(AstClassSymbol* from, AstClassSymbol* to)
				{
					while (from)
					{
						if (from == to) return true;
						from = from->baseClass;
					}
					return false;
				}

			public:
				ValidateTypesVisitor(
					VisitorContext& _context,
					RuleSymbol* _ruleSymbol
				)
					: context(_context)
					, ruleSymbol(_ruleSymbol)
				{
				}

				void ValidateClause(Ptr<GlrClause> clause)
				{
					clause->Accept(this);
				}

			protected:

				////////////////////////////////////////////////////////////////////////
				// GlrSyntax::IVisitor
				////////////////////////////////////////////////////////////////////////

				void Visit(GlrRefSyntax* node) override
				{
					vint ruleIndex = node->refType==GlrRefType::Id ? context.syntaxManager.Rules().Keys().IndexOf(node->literal.value) : -1;
					auto clauseType = context.clauseTypes[clause];
					auto fieldRule = ruleIndex == -1 ? nullptr : context.syntaxManager.Rules().Values()[ruleIndex];

					if (fieldRule && fieldRule->isPartial)
					{
						if (!ConvertibleTo(clauseType, fieldRule->ruleType))
						{
							context.syntaxManager.AddError(
								ParserErrorType::ClauseTypeMismatchedToPartialRule,
								node->codeRange,
								ruleSymbol->Name(),
								clauseType->Name(),
								fieldRule->Name(),
								fieldRule->ruleType->Name()
								);
						}
					}

					if (node->field)
					{
						AstClassSymbol* clauseType = nullptr;
						if (auto prop = FindField(clauseType, node->field))
						{
							if (fieldRule)
							{
								if (fieldRule->isPartial)
								{
									context.syntaxManager.AddError(
										ParserErrorType::PartialRuleUsedOnField,
										node->codeRange,
										ruleSymbol->Name(),
										clauseType->Name(),
										fieldRule->Name(),
										node->field.value
										);
								}

								if (auto propClassSymbol = dynamic_cast<AstClassSymbol*>(prop->propSymbol))
								{
									if (ConvertibleTo(fieldRule->ruleType, propClassSymbol))
									{
										goto PASS_FIELD_TYPE;
									}
								}
								context.syntaxManager.AddError(
									ParserErrorType::RuleTypeMismatchedToField,
									node->codeRange,
									ruleSymbol->Name(),
									clauseType->Name(),
									node->field.value,
									fieldRule->ruleType->Name()
									);
							PASS_FIELD_TYPE:;
							}
							else
							{
								if (prop->propType != AstPropType::Token)
								{
									context.syntaxManager.AddError(
										ParserErrorType::RuleTypeMismatchedToField,
										node->codeRange,
										ruleSymbol->Name(),
										clauseType->Name(),
										node->field.value,
										L"token"
										);
								}
							}
						}
					}
				}

				void Visit(GlrUseSyntax* node) override
				{
					auto fieldRule = context.syntaxManager.Rules()[node->name.value];
					if (fieldRule->isPartial)
					{
						context.syntaxManager.AddError(
							ParserErrorType::UseRuleWithPartialRule,
							node->codeRange,
							ruleSymbol->Name(),
							node->name.value
							);
					}
					if (!dynamic_cast<GlrReuseClause*>(clause))
					{
						context.syntaxManager.AddError(
							ParserErrorType::UseRuleInNonReuseClause,
							node->codeRange,
							ruleSymbol->Name(),
							node->name.value
							);
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

				void Visit(GlrPushConditionSyntax* node) override
				{
					node->syntax->Accept(this);
				}

				void Visit(GlrTestConditionSyntax* node) override
				{
					for (auto&& branch : node->branches)
					{
						if (branch->syntax)
						{
							branch->syntax->Accept(this);
						}
					}
				}

				////////////////////////////////////////////////////////////////////////
				// GlrClause::IVisitor
				////////////////////////////////////////////////////////////////////////

				void Visit(GlrAssignment* node)
				{
					AstClassSymbol* clauseType = nullptr;
					if (auto prop = FindField(clauseType, node->field))
					{
						if (auto enumPropSymbol = dynamic_cast<AstEnumSymbol*>(prop->propSymbol))
						{
							if (!enumPropSymbol->Items().Keys().Contains(node->value.value))
							{
								context.syntaxManager.AddError(
									ParserErrorType::EnumItemMismatchedToField,
									node->codeRange,
									ruleSymbol->Name(),
									clauseType->Name(),
									node->field.value,
									node->value.value
									);
							}
						}
						else
						{
							context.syntaxManager.AddError(
								ParserErrorType::AssignmentToNonEnumField,
								node->codeRange,
								ruleSymbol->Name(),
								clauseType->Name(),
								node->field.value
								);
						}
					}
				}

				void Visit(GlrCreateClause* node) override
				{
					clause = node;
					node->syntax->Accept(this);
					for (auto assignment : node->assignments)
					{
						Visit(assignment.Obj());
					}
				}

				void Visit(GlrPartialClause* node) override
				{
					clause = node;
					node->syntax->Accept(this);
					for (auto assignment : node->assignments)
					{
						Visit(assignment.Obj());
					}
				}

				void Visit(GlrReuseClause* node) override
				{
					clause = node;
					node->syntax->Accept(this);
					for (auto assignment : node->assignments)
					{
						Visit(assignment.Obj());
					}
				}

				void Visit(GlrLeftRecursionPlaceholderClause* node) override
				{
					CHECK_FAIL(L"Not Implemented!");
				}

				void Visit(GlrLeftRecursionInjectClause* node) override
				{
					CHECK_FAIL(L"Not Implemented!");
				}
			};

/***********************************************************************
ValidateTypes
***********************************************************************/

			void ValidateTypes(VisitorContext& context, List<Ptr<GlrSyntaxFile>>& files)
			{
				for (auto file : files)
				{
					for (auto rule : file->rules)
					{
						auto ruleSymbol = context.syntaxManager.Rules()[rule->name.value];
						ValidateTypesVisitor visitor(context, ruleSymbol);
						for (auto clause : rule->clauses)
						{
							visitor.ValidateClause(clause);
						}
					}
				}
			}
		}
	}
}
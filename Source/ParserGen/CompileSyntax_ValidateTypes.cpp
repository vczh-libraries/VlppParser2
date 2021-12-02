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
				, public virtual GlrSyntax::IVisitor
				, public virtual GlrClause::IVisitor
			{
			protected:
				VisitorContext&				context;
				RuleSymbol*					ruleSymbol;
				GlrClause*					clause = nullptr;

			public:
				ValidateTypesVisitor(
					VisitorContext& _context,
					RuleSymbol* _ruleSymbol
				)
					: context(_context)
					, ruleSymbol(_ruleSymbol)
				{
				}

				AstClassPropSymbol* FindField(AstClassSymbol*& clauseType, const WString& name)
				{
					clauseType = context.clauseTypes[clause];
					auto currentType = clauseType;
					while (currentType)
					{
						vint index = currentType->Props().Keys().IndexOf(name);
						if (index != -1)
						{
							auto prop = currentType->Props().Values()[index];
							if (prop->propType != AstPropType::Array)
							{
								ruleSymbol->assignedNonArrayField = true;
							}
							return prop;
						}
						currentType = currentType->baseClass;
					}

					context.global.AddError(
						ParserErrorType::FieldNotExistsInClause,
						ruleSymbol->Name(),
						clauseType->Name(),
						name
						);
					return nullptr;
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

				void Visit(GlrRefSyntax* node) override
				{
					vint ruleIndex = context.syntaxManager.Rules().Keys().IndexOf(node->name.value);
					auto clauseType = context.clauseTypes[clause];
					auto fieldRule = ruleIndex == -1 ? nullptr : context.syntaxManager.Rules().Values()[ruleIndex];

					if (fieldRule && fieldRule->isPartial)
					{
						if (!ConvertibleTo(clauseType, fieldRule->ruleType))
						{
							context.global.AddError(
								ParserErrorType::ClauseTypeMismatchedToPartialRule,
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
						if (auto prop = FindField(clauseType, node->field.value))
						{
							if (fieldRule)
							{
								if (fieldRule->isPartial)
								{
									context.global.AddError(
										ParserErrorType::PartialRuleUsedOnField,
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
								context.global.AddError(
									ParserErrorType::RuleTypeMismatchedToField,
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
									context.global.AddError(
										ParserErrorType::RuleTypeMismatchedToField,
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

				void Visit(GlrLiteralSyntax* node) override
				{
				}

				void Visit(GlrUseSyntax* node) override
				{
					auto fieldRule = context.syntaxManager.Rules()[node->name.value];
					if (fieldRule->isPartial)
					{
						context.global.AddError(
							ParserErrorType::UseRuleWithPartialRule,
							ruleSymbol->Name(),
							node->name.value
							);
					}
					if (!dynamic_cast<GlrReuseClause*>(clause))
					{
						context.global.AddError(
							ParserErrorType::UseRuleInNonReuseClause,
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

				void Visit(GlrAssignment* node)
				{
					AstClassSymbol* clauseType = nullptr;
					if (auto prop = FindField(clauseType, node->field.value))
					{
						if (auto enumPropSymbol = dynamic_cast<AstEnumSymbol*>(prop->propSymbol))
						{
							if (!enumPropSymbol->Items().Keys().Contains(node->value.value))
							{
								context.global.AddError(
									ParserErrorType::EnumItemMismatchedToField,
									ruleSymbol->Name(),
									clauseType->Name(),
									node->field.value,
									node->value.value
									);
							}
						}
						else
						{
							context.global.AddError(
								ParserErrorType::AssignmentToNonEnumField,
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
							clause->Accept(&visitor);
						}
					}
				}
			}
		}
	}
}
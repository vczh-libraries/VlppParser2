/***********************************************************************
This file is generated by: Vczh Parser Generator
From parser definition:Ast
Licensed under https://github.com/vczh-libraries/License
***********************************************************************/

#include "WorkflowAst_Empty.h"

namespace vl
{
	namespace glr
	{
		namespace workflow
		{
			namespace empty_visitor
			{

/***********************************************************************
TypeVisitor
***********************************************************************/

				// Visitor Members -----------------------------------

				void TypeVisitor::Visit(WorkflowPredefinedType* node)
				{
				}

				void TypeVisitor::Visit(WorkflowTopQualifiedType* node)
				{
				}

				void TypeVisitor::Visit(WorkflowReferenceType* node)
				{
				}

				void TypeVisitor::Visit(WorkflowRawPointerType* node)
				{
				}

				void TypeVisitor::Visit(WorkflowSharedPointerType* node)
				{
				}

				void TypeVisitor::Visit(WorkflowNullableType* node)
				{
				}

				void TypeVisitor::Visit(WorkflowEnumerableType* node)
				{
				}

				void TypeVisitor::Visit(WorkflowMapType* node)
				{
				}

				void TypeVisitor::Visit(WorkflowObservableListType* node)
				{
				}

				void TypeVisitor::Visit(WorkflowFunctionType* node)
				{
				}

				void TypeVisitor::Visit(WorkflowChildType* node)
				{
				}

/***********************************************************************
ExpressionVisitor
***********************************************************************/

				// Visitor Members -----------------------------------

				void ExpressionVisitor::Visit(WorkflowThisExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowTopQualifiedExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowReferenceExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowOrderedNameExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowOrderedLambdaExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowMemberExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowChildExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowLiteralExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowFloatingExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowIntegerExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowStringExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowUnaryExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowBinaryExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowLetExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowIfExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowRangeExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowSetTestingExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowConstructorExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowInferExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowTypeCastingExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowTypeTestingExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowTypeOfTypeExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowTypeOfExpressionExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowAttachEventExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowDetachEventExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowObserveExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowCallExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowFunctionExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowNewClassExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowNewInterfaceExpression* node)
				{
				}

				void ExpressionVisitor::Visit(WorkflowVirtualCfeExpression* node)
				{
					Dispatch(node);
				}

				void ExpressionVisitor::Visit(WorkflowVirtualCseExpression* node)
				{
					Dispatch(node);
				}

/***********************************************************************
StatementVisitor
***********************************************************************/

				// Visitor Members -----------------------------------

				void StatementVisitor::Visit(WorkflowBreakStatement* node)
				{
				}

				void StatementVisitor::Visit(WorkflowContinueStatement* node)
				{
				}

				void StatementVisitor::Visit(WorkflowReturnStatement* node)
				{
				}

				void StatementVisitor::Visit(WorkflowDeleteStatement* node)
				{
				}

				void StatementVisitor::Visit(WorkflowRaiseExceptionStatement* node)
				{
				}

				void StatementVisitor::Visit(WorkflowIfStatement* node)
				{
				}

				void StatementVisitor::Visit(WorkflowWhileStatement* node)
				{
				}

				void StatementVisitor::Visit(WorkflowTryStatement* node)
				{
				}

				void StatementVisitor::Visit(WorkflowBlockStatement* node)
				{
				}

				void StatementVisitor::Visit(WorkflowGotoStatement* node)
				{
				}

				void StatementVisitor::Visit(WorkflowVariableStatement* node)
				{
				}

				void StatementVisitor::Visit(WorkflowExpressionStatement* node)
				{
				}

				void StatementVisitor::Visit(WorkflowVirtualCseStatement* node)
				{
					Dispatch(node);
				}

				void StatementVisitor::Visit(WorkflowCoroutineStatement* node)
				{
					Dispatch(node);
				}

				void StatementVisitor::Visit(WorkflowStateMachineStatement* node)
				{
					Dispatch(node);
				}

/***********************************************************************
DeclarationVisitor
***********************************************************************/

				// Visitor Members -----------------------------------

				void DeclarationVisitor::Visit(WorkflowNamespaceDeclaration* node)
				{
				}

				void DeclarationVisitor::Visit(WorkflowFunctionDeclaration* node)
				{
				}

				void DeclarationVisitor::Visit(WorkflowVariableDeclaration* node)
				{
				}

				void DeclarationVisitor::Visit(WorkflowEventDeclaration* node)
				{
				}

				void DeclarationVisitor::Visit(WorkflowPropertyDeclaration* node)
				{
				}

				void DeclarationVisitor::Visit(WorkflowConstructorDeclaration* node)
				{
				}

				void DeclarationVisitor::Visit(WorkflowDestructorDeclaration* node)
				{
				}

				void DeclarationVisitor::Visit(WorkflowClassDeclaration* node)
				{
				}

				void DeclarationVisitor::Visit(WorkflowEnumDeclaration* node)
				{
				}

				void DeclarationVisitor::Visit(WorkflowStructDeclaration* node)
				{
				}

				void DeclarationVisitor::Visit(WorkflowVirtualCfeDeclaration* node)
				{
					Dispatch(node);
				}

				void DeclarationVisitor::Visit(WorkflowVirtualCseDeclaration* node)
				{
					Dispatch(node);
				}

/***********************************************************************
VirtualCfeDeclarationVisitor
***********************************************************************/

				// Visitor Members -----------------------------------

				void VirtualCfeDeclarationVisitor::Visit(WorkflowAutoPropertyDeclaration* node)
				{
				}

				void VirtualCfeDeclarationVisitor::Visit(WorkflowCastResultInterfaceDeclaration* node)
				{
				}

/***********************************************************************
VirtualCseDeclarationVisitor
***********************************************************************/

				// Visitor Members -----------------------------------

				void VirtualCseDeclarationVisitor::Visit(WorkflowStateMachineDeclaration* node)
				{
				}

/***********************************************************************
VirtualCseStatementVisitor
***********************************************************************/

				// Visitor Members -----------------------------------

				void VirtualCseStatementVisitor::Visit(WorkflowForEachStatement* node)
				{
				}

				void VirtualCseStatementVisitor::Visit(WorkflowSwitchStatement* node)
				{
				}

				void VirtualCseStatementVisitor::Visit(WorkflowCoProviderStatement* node)
				{
				}

/***********************************************************************
CoroutineStatementVisitor
***********************************************************************/

				// Visitor Members -----------------------------------

				void CoroutineStatementVisitor::Visit(WorkflowCoPauseStatement* node)
				{
				}

				void CoroutineStatementVisitor::Visit(WorkflowCoOperatorStatement* node)
				{
				}

/***********************************************************************
StateMachineStatementVisitor
***********************************************************************/

				// Visitor Members -----------------------------------

				void StateMachineStatementVisitor::Visit(WorkflowStateSwitchStatement* node)
				{
				}

				void StateMachineStatementVisitor::Visit(WorkflowStateInvokeStatement* node)
				{
				}

/***********************************************************************
VirtualCfeExpressionVisitor
***********************************************************************/

				// Visitor Members -----------------------------------

				void VirtualCfeExpressionVisitor::Visit(WorkflowFormatExpression* node)
				{
				}

/***********************************************************************
VirtualCseExpressionVisitor
***********************************************************************/

				// Visitor Members -----------------------------------

				void VirtualCseExpressionVisitor::Visit(WorkflowBindExpression* node)
				{
				}

				void VirtualCseExpressionVisitor::Visit(WorkflowNewCoroutineExpression* node)
				{
				}

				void VirtualCseExpressionVisitor::Visit(WorkflowMixinCastExpression* node)
				{
				}

				void VirtualCseExpressionVisitor::Visit(WorkflowExpectedTypeCastExpression* node)
				{
				}

				void VirtualCseExpressionVisitor::Visit(WorkflowCoOperatorExpression* node)
				{
				}

/***********************************************************************
ModuleUsingFragmentVisitor
***********************************************************************/

				// Visitor Members -----------------------------------

				void ModuleUsingFragmentVisitor::Visit(WorkflowModuleUsingNameFragment* node)
				{
				}

				void ModuleUsingFragmentVisitor::Visit(WorkflowModuleUsingWildCardFragment* node)
				{
				}
			}
		}
	}
}
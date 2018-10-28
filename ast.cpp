#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

#include "ast.h"
#include "common.h"
#include "parser.tab.h"

#define DEBUG_PRINT_TREE 0

node *ast = NULL;

//////////////////////////////////////////////////////////////////
//
// A Modern Object-Oriented Approach for AST
//
//////////////////////////////////////////////////////////////////
class ScopeNode;
class ExpressionNode;
class UnaryExpressionNode;
class BinaryExpressionNode;
class IntNode;
class FloatNode;
class IdentifierNode;
class VariableNode; // maybe change to refer to indexing vector
class FunctionNode;
class ConstructorNode;
class StatementNode;
class StatementsNode;
class IfStatementNode;
class WhileStatementNode;
class AssignmentNode;
class NestedScopeNode;
class DeclarationNode;
class DeclarationsNode;

class Visitor {
    public:
        void visit(ScopeNode *scopeNode);
        void visit(ExpressionNode *expressionNode);
        void visit(UnaryExpressionNode *unaryExpressionNode);
        void visit(BinaryExpressionNode *binaryExpressionNode);
        void visit(IntNode *intNode);
        void visit(FloatNode *floatNode);
        void visit(IdentifierNode *identifierNode);
        void visit(VariableNode *variableNode);
        void visit(FunctionNode *functionNode);
        void visit(ConstructorNode *constructorNode);
        void visit(StatementNode *statementNode);
        void visit(StatementsNode *statementsNode);
        void visit(IfStatementNode *ifStatementNode);
        void visit(WhileStatementNode *whileStatementNode);
        void visit(AssignmentNode *assignmentNode);
        void visit(NestedScopeNode *nestedScopeNode);
        void visit(DeclarationNode *declarationNode);
        void visit(DeclarationsNode *declarationsNode);
};

class PrintVisitor {
    public:
        void print(ScopeNode *scopeNode);
        void print(ExpressionNode *expressionNode);
        void print(UnaryExpressionNode *unaryExpressionNode);
        void print(BinaryExpressionNode *binaryExpressionNode);
        void print(IntNode *intNode);
        void print(FloatNode *floatNode);
        void print(IdentifierNode *identifierNode);
        void print(VariableNode *variableNode);
        void print(FunctionNode *functionNode);
        void print(ConstructorNode *constructorNode);
        void print(StatementNode *statementNode);
        void print(StatementsNode *statementsNode);
        void print(IfStatementNode *ifStatementNode);
        void print(WhileStatementNode *whileStatementNode);
        void print(AssignmentNode *assignmentNode);
        void print(NestedScopeNode *nestedScopeNode);
        void print(DeclarationNode *declarationNode);
        void print(DeclarationsNode *declarationsNode);
};

#define VISIT_THIS_NODE     virtual void visit(Visitor &visitor) {              \
                                visitor.visit(this);                            \
                            }

#define PRINT_THIS_NODE     virtual void print(PrintVisitor &printVisitor) {    \
                                printVisitor.print(this);                       \
                            }

class ScopeNode: public Node {
    private:
        DeclarationsNode *m_declarationsNode;
        StatementsNode *m_statementsNode;
    public:
        ScopeNode(DeclarationsNode *declarationsNode, StatementsNode *statementsNode):
            m_declarationsNode(declarationsNode), m_statementsNode(statementsNode) {}
    public:
        VISIT_THIS_NODE
        PRINT_THIS_NODE
};

//////////////////////////////////////////////////////////////////
//
// Interface Functions
//
//////////////////////////////////////////////////////////////////

node *ast_allocate(node_kind kind, ...) {
  va_list args;

  // make the node
  node *ast = (node *) malloc(sizeof(node));
  memset(ast, 0, sizeof *ast);
  ast->kind = kind;

  va_start(args, kind); 

  switch(kind) {
  
  // ...

  case BINARY_EXPRESSION_NODE:
    ast->binary_expr.op = va_arg(args, int);
    ast->binary_expr.left = va_arg(args, node *);
    ast->binary_expr.right = va_arg(args, node *);
    break;

  // ...

  default: break;
  }

  va_end(args);

  return ast;
}

void ast_free(node *ast) {

}

void ast_print(node * ast) {

}

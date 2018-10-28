#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>

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
#define ANY_TYPE -1

class Visitor;

class ExpressionNode;
class UnaryExpressionNode;
class BinaryExpressionNode;
class IntLiteralNode;
class FloatLiteralNode;
class BooleanLiteralNode;
class VariableNode;
class IdentifierNode;
class IndexingNode;
class FunctionNode;
class ConstructorNode;
class StatementNode;
class StatementsNode;
class DeclarationNode;
class DeclarationsNode;
class IfStatementNode;
class WhileStatementNode;
class AssignmentNode;
class StallStatementNode;
class NestedScopeNode;
class ScopeNode;

class Visitor {
    public:
        virtual void visit(ExpressionNode *expressionNode){}
        virtual void visit(UnaryExpressionNode *unaryExpressionNode){}
        virtual void visit(BinaryExpressionNode *binaryExpressionNode){}
        virtual void visit(IntLiteralNode *intLiteralNode){}
        virtual void visit(FloatLiteralNode *floatLiteralNode){}
        virtual void visit(BooleanLiteralNode *booleanLiteralNode){}
        virtual void visit(VariableNode *variableNode){}
        virtual void visit(IdentifierNode *identifierNode){}
        virtual void visit(IndexingNode *IndexingNode){}
        virtual void visit(FunctionNode *functionNode){}
        virtual void visit(ConstructorNode *constructorNode){}
        virtual void visit(StatementNode *statementNode){}
        virtual void visit(StatementsNode *statementsNode){}
        virtual void visit(DeclarationNode *declarationNode){}
        virtual void visit(DeclarationsNode *declarationsNode){}
        virtual void visit(IfStatementNode *ifStatementNode){}
        virtual void visit(WhileStatementNode *whileStatementNode){}
        virtual void visit(AssignmentNode *assignmentNode){}
        virtual void visit(StallStatementNode *stallStatementNode){}
        virtual void visit(NestedScopeNode *nestedScopeNode){}
        virtual void visit(ScopeNode *scopeNode){}
};

#define VISIT_THIS_NODE     virtual void visit(Visitor &visitor) {              \
                                visitor.visit(this);                            \
                            }

/*
 * Base class for AST Nodes
 */
class Node {
    public:
        virtual void visit(Visitor &vistor) = 0;
    protected:
        virtual ~Node() {}
    public:
        /* The only way to destruct any AST Node */
        static void destructNode(Node *node) { delete node; }
};

class ExpressionNode: public Node {
    /* Pure Virtual Intermediate Layer */
    public:
        virtual int getExpressionType() = 0;            // pure virtual
        virtual void setExpressionType(int type) {}     // provide default definition
    protected:
        virtual ~ExpressionNode() {}
};

class UnaryExpressionNode: public ExpressionNode {
    private:
        int m_type = ANY_TYPE;                          // types defined in parser.tab.h
        int m_op;                                       // unary operators defined in parser.tab.h
        ExpressionNode *m_expr;                         // sub-expression
    public:
        UnaryExpressionNode(int op, ExpressionNode *expr):
            m_op(op), m_expr(expr) {}
    public:
        virtual int getExpressionType() { return m_type; }
        virtual void setExpressionType(int type) { m_type = type; }
    protected:
        virtual ~UnaryExpressionNode() {
            Node::destructNode(m_expr);
        }
    
    VISIT_THIS_NODE
};

class BinaryExpressionNode: public ExpressionNode {
    private:
        int m_type = ANY_TYPE;                          // types defined in parser.tab.h
        int m_op;                                       // binary operators defined in parser.tab.h
        ExpressionNode *m_leftExpr;                     // left sub-expression
        ExpressionNode *m_rightExpr;                    // right sub-expression
    public:
        BinaryExpressionNode(int op, ExpressionNode *leftExpr, ExpressionNode *rightExpr):
            m_op(op), m_leftExpr(leftExpr), m_rightExpr(rightExpr) {}
    public:
        virtual int getExpressionType() { return m_type; }
        virtual void setExpressionType(int type) { m_type = type; }
    protected:
        virtual ~BinaryExpressionNode() {
            Node::destructNode(m_leftExpr);
            Node::destructNode(m_rightExpr);
        }
    
    VISIT_THIS_NODE
};

class IntLiteralNode: public ExpressionNode {
    private:
        int m_val;                                      // value of this int literal
    public:
        IntLiteralNode(int val):
            m_val(val) {}
    public:
        virtual int getExpressionType() { return INT_T; }
    protected:
        virtual ~IntLiteralNode() {}
    
    VISIT_THIS_NODE
};

class FloatLiteralNode: public ExpressionNode {
    private:
        float m_val;                                    // value of this float literal
    public:
        FloatLiteralNode(float val):
            m_val(val) {}
    public:
        virtual int getExpressionType() { return FLOAT_T; }
    protected:
        virtual ~FloatLiteralNode() {}

    VISIT_THIS_NODE
};

class BooleanLiteralNode: public ExpressionNode {
    private:
        bool m_val;                                     // value of this boolean literal
    public:
        BooleanLiteralNode(bool val):
            m_val(val) {}
    public:
        virtual int getExpressionType() { return BOOL_T; }
    protected:
        virtual ~BooleanLiteralNode() {}

    VISIT_THIS_NODE
};

class VariableNode: public ExpressionNode {
    /* Pure Virtual Intermediate Layer */
};

class IdentifierNode: public VariableNode {
    private:
        int m_type = ANY_TYPE;                          // types defined in parser.tab.h
        std::string m_id;                               // name of this identifier
    public:
        IdentifierNode(const std::string &id):
            m_id(id) {}
    public:
        virtual int getExpressionType() { return m_type; }
        virtual void setExpressionType(int type) { m_type = type; }
    protected:
        virtual ~IdentifierNode() {}

    VISIT_THIS_NODE
};

class IndexingNode: public VariableNode {
    private:
        int m_type = ANY_TYPE;                          // types defined in parser.tab.h
        IdentifierNode *m_identifier;                   // identifier node of the vector variable
        ExpressionNode *m_indexExpr;                    // index expression node
    public:
        IndexingNode(IdentifierNode *identifier, ExpressionNode *indexExpr):
            m_identifier(identifier), m_indexExpr(indexExpr) {}
    public:
        virtual int getExpressionType() { return m_type; }
        virtual void setExpressionType(int type) { m_type = type; }
    protected:
        virtual ~IndexingNode() {
            Node::destructNode(m_identifier);
            Node::destructNode(m_indexExpr);
        }

    VISIT_THIS_NODE
};

class FunctionNode: public ExpressionNode {
    private:
        int m_type = ANY_TYPE;                          // types defined in parser.tab.h
        std::string m_functionName;                     // name of this function
        std::vector<ExpressionNode *> m_arguments;      // argument expressions of this function
    public:
        FunctionNode(const std::string &functionName, const std::vector<ExpressionNode *> &arguments):
            m_functionName(functionName), m_arguments(arguments) {}
        FunctionNode(const std::string &functionName, std::vector<ExpressionNode *> &&arguments):
            m_functionName(functionName), m_arguments(arguments) {}
    public:
        virtual int getExpressionType() { return m_type; }
        virtual void setExpressionType(int type) { m_type = type; }
    protected:
        virtual ~FunctionNode() {
            for(ExpressionNode *node: m_arguments) {
                Node::destructNode(node);
            }
        }

    VISIT_THIS_NODE
};

class ConstructorNode: public ExpressionNode {
    private:
        int m_type = ANY_TYPE;                          // types defined in parser.tab.h
        std::vector<ExpressionNode *> m_arguments;      // argument expressions of this constructor
    public:
        ConstructorNode(int type, const std::vector<ExpressionNode *> &arguments):
            m_type(type), m_arguments(arguments) {}
        ConstructorNode(int type, std::vector<ExpressionNode *> &&arguments):
            m_type(type), m_arguments(arguments) {}
    public:
        virtual int getExpressionType() { return m_type; }
    protected:
        virtual ~ConstructorNode() {
            for(ExpressionNode *node: m_arguments) {
                Node::destructNode(node);
            }
        }

    VISIT_THIS_NODE
};

class StatementNode: public Node {
    /* Pure Virtual Intermediate Layer */
    protected:
        virtual ~StatementNode() {}
};

class StatementsNode: public Node {
    private:
        std::vector<StatementNode *> m_statements;      // a list of StatementNodes
    public:
        StatementsNode(const std::vector<StatementNode *> &statements):
            m_statements(statements) {}
        StatementsNode(std::vector<StatementNode *> &&statements):
            m_statements(statements) {}
    protected:
        virtual ~StatementsNode() {
            for(StatementNode *node: m_statements) {
                Node::destructNode(node);
            }
        }

    VISIT_THIS_NODE
};

class DeclarationNode: public Node {
    private:
        // not using IdentifierNode because the name itself is not yet an expression
        std::string m_variableName;                     // variable name
        int m_type;                                     // types defined in parser.tab.h
        ExpressionNode *m_initValExpr = nullptr;        // initial value expression (optional)
    public:
        DeclarationNode(const std::string &variableName, int type, ExpressionNode *initValExpr = nullptr):
            m_variableName(variableName), m_type(type), m_initValExpr(initValExpr) {}
    protected:
        virtual ~DeclarationNode() {
            if(m_initValExpr != nullptr) {
                Node::destructNode(m_initValExpr);
            }
        }
    
    VISIT_THIS_NODE
};

class DeclarationsNode: public Node {
    private:
        std::vector<DeclarationNode *> m_declarations;  // a list of DeclarationNodes
    public:
        DeclarationsNode(const std::vector<DeclarationNode *> &declarations):
            m_declarations(declarations) {}
        DeclarationsNode(std::vector<DeclarationNode *> &&declarations):
            m_declarations(declarations) {}
    protected:
        virtual ~DeclarationsNode() {
            for(DeclarationNode *node: m_declarations) {
                Node::destructNode(node);
            }
        }

    VISIT_THIS_NODE
};

class IfStatementNode: public StatementNode {
    private:
        ExpressionNode *m_condExpr;                     // condition expression
        StatementNode *m_thenStmt;                      // then statement
        StatementNode *m_elseStmt = nullptr;            // else statement (optional)
    public:
        IfStatementNode(ExpressionNode *condExpr, StatementNode *thenStmt, StatementNode *elseStmt = nullptr):
            m_condExpr(condExpr), m_thenStmt(thenStmt), m_elseStmt(elseStmt) {}
    protected:
        virtual ~IfStatementNode() {
            Node::destructNode(m_condExpr);
            Node::destructNode(m_thenStmt);
            if(m_elseStmt != nullptr) {
                Node::destructNode(m_elseStmt);
            }
        }
    
    VISIT_THIS_NODE
};

class WhileStatementNode: public StatementNode {
    /* No Support */
    private:
        ExpressionNode *m_condExpr;                     // condition expression
        StatementNode *m_bodyStmt;                      // loop body statement
    public:
        WhileStatementNode(ExpressionNode *condExpr, StatementNode *bodyStmt):
            m_condExpr(condExpr), m_bodyStmt(bodyStmt) {}
    protected:
        virtual ~WhileStatementNode() {
            Node::destructNode(m_condExpr);
            Node::destructNode(m_bodyStmt);
        }
    
    VISIT_THIS_NODE
};

class AssignmentNode: public StatementNode {
    private:
        int m_type = ANY_TYPE;                          // types defined in parser.tab.h
        VariableNode *m_var;                            // variable node
        ExpressionNode *m_newValExpr;                   // new value expression
    public:
        AssignmentNode(VariableNode *var, ExpressionNode *newValExpr):
            m_var(var), m_newValExpr(newValExpr) {}
    public:
        int getExpressionType() { return m_type; }
        void setExpressionType(int type) { m_type = type; }
    protected:
        virtual ~AssignmentNode() {
            Node::destructNode(m_var);
            Node::destructNode(m_newValExpr);
        }
    
    VISIT_THIS_NODE
};

/* In case of a single semicolon statement */
class StallStatementNode: public StatementNode {
    public:
        StallStatementNode() {}
    protected:
        virtual ~StallStatementNode() {}

    VISIT_THIS_NODE
};

/* Inner ScopeNode */
class NestedScopeNode: public StatementNode {
    private:
        DeclarationsNode *m_decls;
        StatementsNode *m_stmts;
    public:
        NestedScopeNode(DeclarationsNode *decls, StatementsNode *stmts):
            m_decls(decls), m_stmts(stmts) {}
    protected:
        virtual ~NestedScopeNode() {
            Node::destructNode(m_decls);
            Node::destructNode(m_stmts);
        }

    VISIT_THIS_NODE
};

/* Global ScopeNode */
class ScopeNode: public Node {
    private:
        DeclarationsNode *m_decls;
        StatementsNode *m_stmts;
    public:
        ScopeNode(DeclarationsNode *decls, StatementsNode *stmts):
            m_decls(decls), m_stmts(stmts) {}
    protected:
        virtual ~ScopeNode() {
            if(m_decls != nullptr) {
                Node::destructNode(m_decls);
            }
            if(m_stmts != nullptr) {
                Node::destructNode(m_stmts);
            }
        }
    
    public:
        static NestedScopeNode *convertToNestedScopeNode(ScopeNode *scopeNode) {
            NestedScopeNode *nestedScopeNode = new NestedScopeNode(scopeNode->m_decls, scopeNode->m_stmts);
            
            // Delete scopeNode
            scopeNode->m_decls = nullptr;
            scopeNode->m_stmts = nullptr;
            delete scopeNode;

            return nestedScopeNode;
        }

    VISIT_THIS_NODE
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

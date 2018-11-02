#include "semantic.h"

#include "ast.h"
#include "symbol.h"

#include "common.h"
#include "parser.tab.h"

#include <cassert>

namespace SEMA{ /* START NAMESPACE */

class SymbolDeclVisitor: public AST::Visitor {
    private:
        ST::SymbolTable &m_symbolTable;
    public:
        SymbolDeclVisitor(ST::SymbolTable &symbolTable):
            m_symbolTable(symbolTable) {}

    private:
        virtual void preNodeVisit(AST::IdentifierNode *identifierNode) {
            m_symbolTable.markSymbolRefPos(identifierNode);
        }

        virtual void postNodeVisit(AST::DeclarationNode *declarationNode) {
            // Traverse the possible evaluation on rhs before the declaration of symbol
            bool successful = m_symbolTable.declareSymbol(declarationNode);

            /// TODO: print error
            if(!successful) {
                printf("Error: Redeclaration for DeclarationNode(%p):\n");
                ast_print(declarationNode);
                printf("\n");

                return;
            }
        }

        virtual void preNodeVisit(AST::NestedScopeNode *nestedScopeNode) {
            m_symbolTable.enterScope();
        }
        virtual void postNodeVisit(AST::NestedScopeNode *nestedScopeNode) {
            m_symbolTable.exitScope();
        }

        virtual void preNodeVisit(AST::ScopeNode *scopeNode) {
            m_symbolTable.enterScope();
        }
        virtual void postNodeVisit(AST::ScopeNode *scopeNode) {
            m_symbolTable.exitScope();
        }
};

enum class DataTypeCategory {
    boolean,
    arithmetic
};

DataTypeCategory getDataTypeCategory(int dataType) {
    switch(dataType) {
        case BOOL_T:
        case BVEC2_T:
        case BVEC3_T:
        case BVEC4_T:
            return DataTypeCategory::boolean;
        case INT_T:
        case IVEC2_T:
        case IVEC3_T:
        case IVEC4_T:
        case FLOAT_T:
        case VEC2_T:
        case VEC3_T:
        case VEC4_T:
            return DataTypeCategory::arithmetic;
        default:
            assert(0);
    }
}

int getDataTypeOrder(int dataType) {
    switch(dataType) {
        case BOOL_T:
        case INT_T:
        case FLOAT_T:
            return 1;
        case BVEC2_T:
        case IVEC2_T:
        case VEC2_T:
            return 2;
        case BVEC3_T:
        case IVEC3_T:
        case VEC3_T:
            return 3;
        case BVEC4_T:
        case IVEC4_T:
        case VEC4_T:
            return 4;
        default:
            assert(0);
    }
}

int getDataTypeBaseType(int dataType) {
    switch(dataType) {
        case BOOL_T:
        case BVEC2_T:
        case BVEC3_T:
        case BVEC4_T:
            return BOOL_T;
        case INT_T:
        case IVEC2_T:
        case IVEC3_T:
        case IVEC4_T:
            return INT_T;
        case FLOAT_T:
        case VEC2_T:
        case VEC3_T:
        case VEC4_T:
            return FLOAT_T;
        default:
            assert(0);
    }
}

class TypeChecker: public AST::Visitor {
    private:
        ST::SymbolTable &m_symbolTable;
    public:
        TypeChecker(ST::SymbolTable &symbolTable):
            m_symbolTable(symbolTable) {}
    
    private:
        virtual void preNodeVisit(AST::IdentifierNode *identifierNode);

    private:
        virtual void postNodeVisit(AST::UnaryExpressionNode *unaryExpressionNode);
        virtual void postNodeVisit(AST::BinaryExpressionNode *binaryExpressionNode);
        virtual void postNodeVisit(AST::IndexingNode *indexingNode);
        virtual void postNodeVisit(AST::FunctionNode *functionNode);
        virtual void postNodeVisit(AST::ConstructorNode *constructorNode);
        virtual void postNodeVisit(AST::DeclarationNode *declarationNode);
        virtual void postNodeVisit(AST::IfStatementNode *ifStatementNode);
        virtual void postNodeVisit(AST::AssignmentNode *assignmentNode);

    private:
        int inferDataType(int op, int rhsDataType);
        int inferDataType(int op, int lhsDataType, int rhsDataType);
};

void TypeChecker::preNodeVisit(AST::IdentifierNode *identifierNode) {
    AST::DeclarationNode *decl = m_symbolTable.getSymbolDecl(identifierNode);

    /// TODO: print error
    if(decl == nullptr) {
        // Update info in identifierNode
        identifierNode->setExpressionType(ANY_TYPE);
        identifierNode->setConst(false);
        identifierNode->setDeclaration(nullptr);

        printf("Error: Missing Declaration for IdentifierNode(%p)\n", identifierNode);
        ast_print(identifierNode);
        printf("\n");

        return;
    }

    assert(decl->getName() == identifierNode->getName());

    // Update info in identifierNode
    identifierNode->setExpressionType(decl->getType());
    identifierNode->setConst(decl->isConst());
    identifierNode->setDeclaration(decl);
}

void TypeChecker::postNodeVisit(AST::UnaryExpressionNode *unaryExpressionNode) {
    const AST::ExpressionNode *rhsExpr = unaryExpressionNode->getExpression();
    int rhsDataType = rhsExpr->getExpressionType();
    bool rhsIsConst = rhsExpr->isConst();
    int op = unaryExpressionNode->getOperator();

    int resultDataType = inferDataType(op, rhsDataType);
    unaryExpressionNode->setExpressionType(resultDataType);
    unaryExpressionNode->setConst(rhsIsConst);
}

void TypeChecker::postNodeVisit(AST::BinaryExpressionNode *binaryExpressionNode) {
    const AST::ExpressionNode *lhsExpr = binaryExpressionNode->getLeftExpression();
    const AST::ExpressionNode *rhsExpr = binaryExpressionNode->getRightExpression();
    
    int lhsDataType = lhsExpr->getExpressionType();
    int rhsDataType = rhsExpr->getExpressionType();

    bool lhsIsConst = lhsExpr->isConst();
    bool rhsIsConst = rhsExpr->isConst();

    int op = binaryExpressionNode->getOperator();

    int resultDataType = inferDataType(op, lhsDataType, rhsDataType);
    binaryExpressionNode->setExpressionType(resultDataType);
    binaryExpressionNode->setConst(lhsIsConst && rhsIsConst);
}

void TypeChecker::postNodeVisit(AST::IndexingNode *indexingNode) {
    /*
        * - The index into a vector (e.g. 1 in foo[1]) must be in the range [0, i1] if the vector
        * has type veci. For example, the maximum index into a variable of type vec2 is 1.
        * - The result type of indexing into a vector is the base type associated with that vector’s
        * type. For example, if v has type bvec2 then v[0] has type bool.
        */
    int resultDataType = ANY_TYPE;

    const AST::IdentifierNode *identifier = indexingNode->getIdentifier();            
    int identifierDataType = identifier->getExpressionType();
    int identifierIsConst = identifier->isConst();
    // Identifier must be a vector
    int identifierTypeOrder = getDataTypeOrder(identifierDataType);
    if(identifierTypeOrder > 1) {
        const AST::IntLiteralNode *index = 
            dynamic_cast<AST::IntLiteralNode *>(indexingNode->getIndexExpression());
        int indexVal = index->getVal();
        // Index must be valid
        if(indexVal >= 0 && indexVal < identifierTypeOrder) {
            // The type of this indexing node is the base type for identifier node
            int identifierBaseType = getDataTypeBaseType(identifierDataType);
            resultDataType = identifierBaseType;
        }
    }

    indexingNode->setExpressionType(resultDataType);
    indexingNode->setConst(identifierIsConst);
}

void TypeChecker::postNodeVisit(AST::FunctionNode *functionNode) {
    int resultDataType = ANY_TYPE;

    const std::string &funcName = functionNode->getName();
    AST::ExpressionsNode *exprs = functionNode->getArgumentExpressions();
    const std::vector<AST::ExpressionNode *> &args = exprs->getExpressionList();
    if(funcName == "rsq") {
        if(args.size() == 1) {
            const AST::ExpressionNode *arg1 = args.front();
            int arg1Type = arg1->getExpressionType();
            if(arg1Type == FLOAT_T || arg1Type == INT_T) {
                // float rsq(float);
                // float rsq(int);
                resultDataType = FLOAT_T;
            }
        }
    } else if (funcName == "dp3") {
        if(args.size() == 2) {
            const AST::ExpressionNode *arg1 = args[0];
            int arg1Type = arg1->getExpressionType();
            const AST::ExpressionNode *arg2 = args[1];
            int arg2Type = arg2->getExpressionType();

            if(arg1Type == arg2Type) {
                if(arg1Type == VEC3_T || arg1Type == VEC4_T || arg1Type == IVEC3_T || arg1Type == IVEC4_T) {
                    // float dp3(vec4, vec4);
                    // float dp3(vec3, vec3);
                    // float dp3(ivec4, ivec4);
                    // float dp3(ivec3, ivec3);
                    resultDataType = FLOAT_T;
                }
            }
        }
    } else if (funcName == "lit") {
        if(args.size() == 1) {
            const AST::ExpressionNode *arg1 = args.front();
            int arg1Type = arg1->getExpressionType();
            if(arg1Type == VEC4_T) {
                // vec4 lit(vec4);
                resultDataType = VEC4_T;
            }
        }
    }

    functionNode->setExpressionType(resultDataType);
}

void TypeChecker::postNodeVisit(AST::ConstructorNode *constructorNode) {
    /*
    * Constructors for basic types (bool, int, float) must have one argument that exactly
    * matches that type. Constructors for vector types must have as many arguments as there
    * are elements in the vector, and the types of each argument must be exactly the type of the
    * basic type corresponding to the vector type. For example, bvec2(true,false) is valid ,
    * whereas bvec2(1,true) is invalid.
    */
    bool resultIsConst = false;
    int resultDataType = ANY_TYPE;

    int constructorType = constructorNode->getConstructorType();
    int constructorTypeBase = getDataTypeBaseType(constructorType);
    int constructorTypeOrder = getDataTypeOrder(constructorType);

    AST::ExpressionsNode *exprs = constructorNode->getArgumentExpressions();
    const std::vector<AST::ExpressionNode *> &args = exprs->getExpressionList();

    if(constructorTypeOrder == args.size()) {
        resultIsConst = true;
        bool argLegal = true;
        for(const AST::ExpressionNode *arg : args) {
            if(arg->getExpressionType() != constructorTypeBase) {
                argLegal = false;
            }
            resultIsConst &= arg->isConst();
        }

        if(argLegal) {
            resultDataType = constructorType;
        }
    }

    constructorNode->setExpressionType(resultDataType);
    constructorNode->setConst(resultIsConst);
}

void TypeChecker::postNodeVisit(AST::DeclarationNode *declarationNode) {}

void TypeChecker::postNodeVisit(AST::IfStatementNode *ifStatementNode) {
    /*
     * The expression that determines which branch of an if statement should be taken must
     * have the type bool (not bvec).
     */
    const AST::ExpressionNode *cond = ifStatementNode->getConditionExpression();

    /// TODO: print error
    if(cond->getExpressionType() != BOOL_T) {
        printf("Error: If-statement condition must be of boolean type.\n");
    }
}

void TypeChecker::postNodeVisit(AST::AssignmentNode *assignmentNode) {
    /*
     * The value assigned to a variable (this includes variables declared with an initial value)
     * must have the same type as that variable, or a type that can be widened to the correct type.
     */
    const AST::ExpressionNode *rhsExpr = assignmentNode->getExpression();
    const AST::VariableNode *lhsVar = assignmentNode->getVariable();
    int rhsDataType = rhsExpr->getExpressionType();
    int lhsDataType = lhsVar->getExpressionType();

    int resultDataType = ANY_TYPE;
    bool assignmentLegal = true;

    if(lhsDataType == ANY_TYPE || rhsDataType == ANY_TYPE) {
        assignmentLegal = false;
        
        /// TODO: print error
        printf("Error: Assignment has undetermined type inside.\n");
    } else {
        assert(rhsDataType != ANY_TYPE);
        assert(lhsDataType != ANY_TYPE);

        if(lhsDataType != rhsDataType) {
            assignmentLegal = false;

            /// TODO: print error
            printf("Error: Assignment has non-compatible types inside.\n");
        }
    }

    if(lhsVar->isConst()) {
        assignmentLegal = false;

        /// TODO: print error
        printf("Error: Cannot reassign value to const variable.\n");
    }

    if(assignmentLegal) {
        assert(rhsDataType != ANY_TYPE);
        assert(lhsDataType != ANY_TYPE);
        assert(lhsDataType == rhsDataType);

        resultDataType = lhsDataType;
    }

    assignmentNode->setExpressionType(resultDataType);
}

int TypeChecker::inferDataType(int op, int rhsDataType) {
    /* Unary Operator Type Inference */
    /*
     * - s, v Arithmetic
     * ! s, v Logical
     */

    if(rhsDataType == ANY_TYPE) {
        return ANY_TYPE;
    }

    DataTypeCategory typeCateg = getDataTypeCategory(rhsDataType);
    switch(op) {
        case MINUS: {
            if(typeCateg == DataTypeCategory::arithmetic) {
                return rhsDataType;
            }
            
            break;
        }
        case NOT: {
            if(typeCateg == DataTypeCategory::boolean) {
                return rhsDataType;
            }

            break;
        }
        default:
            assert(0);
    }

    return ANY_TYPE;
}

int TypeChecker::inferDataType(int op, int lhsDataType, int rhsDataType) {
    /* Binary Operator Type Inference */
    /*
     * +, - ss, vv Arithmetic
     * * ss, vv, sv, vs Arithmetic
     * /, ˆ ss Arithmetic
     * &&, || ss, vv Logical
     * <, <=, >, >= ss Comparison
     * ==, != ss, vv Comparison
     */

    if(lhsDataType == ANY_TYPE || rhsDataType == ANY_TYPE) {
        return ANY_TYPE;
    }

    if(getDataTypeBaseType(lhsDataType) != getDataTypeBaseType(rhsDataType)) {
        /*
         * Both operands of a binary operator must have exactly the same base type (e.g. it
         * is valid to multiple an int and an ivec3)
         */
        return ANY_TYPE;
    }

    int lhsTypeOrder = getDataTypeOrder(lhsDataType);
    int rhsTypeOrder = getDataTypeOrder(rhsDataType);
    if(lhsTypeOrder > 1 && rhsTypeOrder > 1) {
       if(lhsTypeOrder != rhsTypeOrder) {
            /*
            * If both arguments to a binary operator are vectors, then they must be vectors of the
            * same order. For example, it is not valid to add an ivec2 with an ivec3
            */
           return ANY_TYPE;
       }
    }

    DataTypeCategory lhsTypeCateg = getDataTypeCategory(lhsDataType);
    DataTypeCategory rhsTypeCateg = getDataTypeCategory(rhsDataType);
    switch(op) {
        case PLUS:
        case MINUS: {
            if(lhsTypeCateg == DataTypeCategory::arithmetic && rhsTypeCateg == DataTypeCategory::arithmetic) {
                if(lhsTypeOrder == rhsTypeOrder) {
                    // Same base type
                    // Both arithmetic type
                    // Same type order
                    assert(lhsDataType == rhsDataType);
                    return lhsDataType;
                }
            }
            break;
        }

        case TIMES: {
            if(lhsTypeCateg == DataTypeCategory::arithmetic && rhsTypeCateg == DataTypeCategory::arithmetic) {
                if(lhsTypeOrder > rhsTypeOrder) {
                    // Same base type
                    // Both arithmetic type
                    // If both vector, same type order; otherwise, any order combination eg. ss and sv
                    // Return the larger order one
                    return lhsDataType;
                } else {
                    return rhsDataType;
                }
            }
            break;
        }

        case SLASH:
        case EXP: {
            if(lhsTypeCateg == DataTypeCategory::arithmetic && rhsTypeCateg == DataTypeCategory::arithmetic) {
                if(lhsTypeOrder == 1 && rhsTypeOrder == 1) {
                    // Same base type
                    // Both arithmetic type
                    // Both scalar type
                    assert(lhsDataType == rhsDataType);
                    return lhsDataType;
                }
            }
            break;
        }

        case AND:
        case OR: {
            if(lhsTypeCateg == DataTypeCategory::boolean && rhsTypeCateg == DataTypeCategory::boolean) {
                if(lhsTypeOrder == rhsTypeOrder) {
                    // Same base type
                    // Both boolean type
                    // Same type order
                    assert(lhsDataType == rhsDataType);
                    return lhsDataType;
                }
            }
            break;
        }

        case LSS:
        case LEQ:
        case GTR:
        case GEQ: {
            if(lhsTypeCateg == DataTypeCategory::arithmetic && rhsTypeCateg == DataTypeCategory::arithmetic) {
                if(lhsTypeOrder == 1 && rhsTypeOrder == 1) {
                    // Same base type
                    // Both arithmetic type
                    // Both scalar type
                    assert(lhsDataType == rhsDataType);
                    return lhsDataType;
                }
            }
            break;
        }

        case EQL:
        case NEQ: {
            if(lhsTypeCateg == DataTypeCategory::arithmetic && rhsTypeCateg == DataTypeCategory::arithmetic) {
                if(lhsTypeOrder == rhsTypeOrder) {
                    // Same base type
                    // Both arithmetic type
                    // Same type order
                    assert(lhsDataType == rhsDataType);
                    return lhsDataType;
                }
            }
            break;
        }

        default:
            assert(0);
    }

    return ANY_TYPE;
}

} /* END NAMESPACE */

int semantic_check(node * ast) {
    /*
     * TODO
     * 
     * Better structure to wrap around errors with context info.
     * E.g. Line, Col, Meaningful Error Messages.
     * 
     */

    ST::SymbolTable symbolTable;

    /* Construct Symbol Tree */
    SEMA::SymbolDeclVisitor symbolDeclVisitor(symbolTable);
    static_cast<AST::ASTNode *>(ast)->visit(symbolDeclVisitor);
    // symbolTable.printScopeLeaves();

    /* Type Checker */
    SEMA::TypeChecker typeChecker(symbolTable);
    static_cast<AST::ASTNode *>(ast)->visit(typeChecker);
    // symbolTable.printSymbolReference();
    ast_print(ast);

    return 1;
}
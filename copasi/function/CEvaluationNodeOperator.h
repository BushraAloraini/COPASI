/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/function/CEvaluationNodeOperator.h,v $
   $Revision: 1.13 $
   $Name:  $
   $Author: nsimus $ 
   $Date: 2005/11/28 14:03:06 $
   End CVS Header */

#ifndef COPASI_CEvaluationNodeOperator
#define COPASI_CEvaluationNodeOperator

#include "mathematics.h"

#include "utilities/CCopasiMessage.h"

class ASTNode;

/**
 * This is the class for nodes presenting opertors used in an evaluation trees.
 */
class CEvaluationNodeOperator : public CEvaluationNode
  {
  public:
    /**
     * Enumeration of possible node types.
     */
    enum SubType
    {
      INVALID = 0x00FFFFFF,
      POWER = 0x00000000,
      MULTIPLY = 0x00000001,
      DIVIDE = 0x00000002,
      MODULUS = 0x00000003,
      PLUS = 0x00000004,
      MINUS = 0x00000005
    };

    // Operations
  private:
    /**
     * Default constructor
     */
    CEvaluationNodeOperator();

  public:
    /**
     * Default constructor
     * @param const SubType & subType
     * @param const Data & data
     */
    CEvaluationNodeOperator(const SubType & subType,
                            const Data & data);

    /**
     * Copy constructor
     * @param const CEvaluationNodeOperator & src
     */
    CEvaluationNodeOperator(const CEvaluationNodeOperator & src);

    /**
     * Destructor
     */
    virtual ~CEvaluationNodeOperator();

    /**
     * Retrieve the value of the node
     * @return const C_FLOAT64 & value
     */
    virtual inline const C_FLOAT64 & value() const
      {
        C_FLOAT64 &Value = *const_cast<C_FLOAT64 *>(&mValue);
        switch (mType & 0x00FFFFFF)
          {
          case POWER:
            Value = pow(mpLeft->value(), mpRight->value());
            break;

          case MULTIPLY:
            Value = mpLeft->value() * mpRight->value();
            break;

          case DIVIDE:
            Value = mpLeft->value() / mpRight->value();
            break;

          case MODULUS:
            Value = (C_FLOAT64) (((C_INT32) mpLeft->value()) * ((C_INT32) mpRight->value()));
            break;

          case PLUS:
            Value = mpLeft->value() + mpRight->value();
            break;

          case MINUS:
            Value = mpLeft->value() - mpRight->value();
            break;

          default:
            break;
          }

        return Value;
      }

    /**
     * Compile a node;
     * @param const CEvaluationTree * pTree
     * @return bool success;
     */
    virtual bool compile(const CEvaluationTree * pTree);

    /**
     * Retrieve the infix value of the node and its eventual child nodes.
     * @return const Data & value
     */
    virtual std::string getInfix() const;

    /**
     * Retrieve the display string of the node and its eventual child nodes.
     * @return const Data & value
     */
    virtual std::string getDisplayString(const CEvaluationTree * pTree) const;

    /**
     * Retrieve the display string of the node and its eventual child nodes in C.
     * @return const Data & value
     */
    virtual std::string getDisplay_C_String(const CEvaluationTree * pTree) const;

    /**
     * Create a new operator node from an ASTNode tree.
     * @param const ASTNode* node
     * @return CEvaluationNode* return a pointer to the newly created node;
     */
    static CEvaluationNode* createNodeFromASTTree(const ASTNode& node);

    /**
     * Create a new ASTNode corresponding to this OperatorNode.
     * @return ASTNode* return a pointer to the newly created node;
     */
    virtual ASTNode* toAST() const;

    /**
     * Create a simplified node for an operatorNode with children child1 and child2 (if not exist, = NULL),
     * and assign new children
     * @return CEvaluationNode* return a pointer to the simplified node;
     */
    virtual CEvaluationNode* simplifyNode(CEvaluationNode *child1, CEvaluationNode *child2) const;

    /**
     * Convert our modulo to something SBML understands
     * @param const CEvaluationNodeOperator* pNode the modulo
     *  operator node to be converted.
     * @param const ASTNode* pASTNode the root node for the SBML math expression
     * @return bool which is true on sucessfull conversion.
     */

    bool createModuloTree(const CEvaluationNodeOperator* pNode, ASTNode* pASTNode) const;

    // Attributes
  private:
    CEvaluationNode * mpLeft;

    CEvaluationNode * mpRight;
  };

#endif // COPASI_CEvaluationNodeOperator

/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/function/CEvaluationNodeChoice.cpp,v $
   $Revision: 1.10 $
   $Name:  $
   $Author: nsimus $ 
   $Date: 2006/03/07 12:37:08 $
   End CVS Header */

#include "copasi.h"

#include "mathematics.h"
#include "CEvaluationNode.h"
#include "CEvaluationTree.h"

#include "sbml/math/ASTNode.h"

CEvaluationNodeChoice::CEvaluationNodeChoice():
    CEvaluationNode(CEvaluationNode::INVALID, "")
{mPrecedence = PRECEDENCE_NUMBER;}

CEvaluationNodeChoice::CEvaluationNodeChoice(const SubType & subType,
    const Data & data):
    CEvaluationNode((Type) (CEvaluationNode::FUNCTION | subType), data),
    mpIf(NULL),
    mpTrue(NULL),
    mpFalse(NULL)
{
  switch (subType)
    {
    case IF:
      break;

    default:
      fatalError();
      break;
    }

  mPrecedence = PRECEDENCE_FUNCTION;
}

CEvaluationNodeChoice::CEvaluationNodeChoice(const CEvaluationNodeChoice & src):
    CEvaluationNode(src),
    mpIf(src.mpIf),
    mpTrue(src.mpTrue),
    mpFalse(src.mpFalse)
{}

CEvaluationNodeChoice::~CEvaluationNodeChoice() {}

const C_FLOAT64 & CEvaluationNodeChoice::value() const
  {
    C_FLOAT64 &Value = *const_cast<C_FLOAT64 *>(&mValue);
    if (mpIf->value() != 0.0) return Value = mpTrue->value();
    else return Value = mpFalse->value();
  }

bool CEvaluationNodeChoice::compile(const CEvaluationTree * /* pTree */)
{
  mpIf = static_cast<CEvaluationNode *>(getChild());
  if (mpIf == NULL) return false;

  mpTrue = static_cast<CEvaluationNode *>(mpIf->getSibling());
  if (mpTrue == NULL) return false;

  mpFalse = static_cast<CEvaluationNode *>(mpTrue->getSibling());
  if (mpFalse == NULL) return false;

  return (mpFalse->getSibling() == NULL); // We must have exactly three children
}

std::string CEvaluationNodeChoice::getInfix() const
  {
    if (const_cast<CEvaluationNodeChoice *>(this)->compile(NULL))
      return mData + "(" + mpIf->getInfix() + "," + mpTrue->getInfix() + "," + mpFalse->getInfix() + ")";
    else
      return "@";
  }

std::string CEvaluationNodeChoice::getDisplayString(const CEvaluationTree * pTree) const
  {
    if (const_cast<CEvaluationNodeChoice *>(this)->compile(NULL))
      return mData + "(" + mpIf->getDisplayString(pTree) + "," + mpTrue->getDisplayString(pTree) + "," + mpFalse->getDisplayString(pTree) + ")";
    else
      return "@";
  }

std::string CEvaluationNodeChoice::getDisplay_C_String(const CEvaluationTree * pTree) const
  {
    if (const_cast<CEvaluationNodeChoice *>(this)->compile(NULL))
      return "(" + mpIf->getDisplay_C_String(pTree) + " ? " + mpTrue->getDisplay_C_String(pTree) + " : " + mpFalse->getDisplay_C_String(pTree) + ")";
    else
      return "@";
  }

std::string CEvaluationNodeChoice::getDisplay_MMD_String(const CEvaluationTree * pTree) const
  {
    if (const_cast<CEvaluationNodeChoice *>(this)->compile(NULL))
      return "(" "if " + mpIf->getDisplay_MMD_String(pTree) + " then " + mpTrue->getDisplay_MMD_String(pTree) + " else " + mpFalse->getDisplay_MMD_String(pTree) + ")";
    else
      return "@";
  }

CEvaluationNode* CEvaluationNodeChoice::createNodeFromASTTree(const ASTNode& node)
{
  SubType subType;
  std::string data = "";
  switch (node.getType())
    {
    case AST_FUNCTION_PIECEWISE:
      subType = IF;
      data = "if";
      break;
    default:
      subType = INVALID;
      break;
    }

  CEvaluationNodeChoice* convertedNode = new CEvaluationNodeChoice(subType, data);
  // convert the two children
  if (subType != INVALID)
    {
      convertedNode->addChild(CEvaluationTree::convertASTNode(*node.getLeftChild()));
      convertedNode->addChild(CEvaluationTree::convertASTNode(*node.getRightChild()));
    }
  return convertedNode;
}

ASTNode* CEvaluationNodeChoice::toAST() const
  {
    ASTNode* node = new ASTNode(AST_FUNCTION_PIECEWISE);
    const CEvaluationNode* child1 = dynamic_cast<const CEvaluationNode*>(this->getChild());
    const CEvaluationNode* child2 = dynamic_cast<const CEvaluationNode*>(child1->getSibling());
    node->addChild(child1->toAST());
    node->addChild(child2->toAST());
    return node;
  }

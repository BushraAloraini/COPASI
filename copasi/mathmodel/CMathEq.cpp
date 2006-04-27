/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/mathmodel/Attic/CMathEq.cpp,v $
   $Revision: 1.5 $
   $Name:  $
   $Author: shoops $
   $Date: 2006/04/27 01:28:50 $
   End CVS Header */

// Copyright � 2005 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/**
 * CMathEq class.
 * The class CMathEq is describes the right hand side of an equation describing
 * a CMathVariable.
 *
 * Created for Copasi by Stefan Hoops 2003
 */

#include "copasi.h"
#include "CMathEq.h"

CMathEq::CMathEq():
    mpVariable(NULL),
    mLeft(),
    mRight()
{}

CMathEq::CMathEq(const CMathEq & src):
    mpVariable(src.mpVariable),
    mLeft(src.mLeft),
    mRight(src.mRight)
{}

CMathEq::~CMathEq() {}

bool CMathEq::setVariable(const CMathVariable * pVariable)
{
  mpVariable = pVariable;
  return true;
}

const CMathVariable * CMathEq::getVariable() const {return mpVariable;}

CCopasiTree< CMathNode > & CMathEq::getLeft() {return mLeft;}

CCopasiTree< CMathNode > & CMathEq::getRight() {return mRight;}

std::string CMathEq::getLeftText() const
  {
    CMathNode * pChild =
      (CMathNode *) const_cast< CMathEq * >(this)->mLeft.begin()->getChild();

    if (pChild)
      return pChild->getData();
    else
      return "Not defined";
  }

std::string CMathEq::getRightText() const
  {
    CMathNode * pChild =
      (CMathNode *) const_cast< CMathEq * >(this)->mRight.begin()->getChild();

    if (pChild)
      return pChild->getData();
    else
      return "Not defined";
  }

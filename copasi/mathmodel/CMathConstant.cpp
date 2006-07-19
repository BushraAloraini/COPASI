/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/mathmodel/Attic/CMathConstant.cpp,v $
   $Revision: 1.18 $
   $Name:  $
   $Author: shoops $
   $Date: 2006/07/19 20:56:50 $
   End CVS Header */

// Copyright � 2005 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/**
 *  CMathConstant class.
 *  The class CMathConstant associates a symbol with a CCopasiObject with
 *  that represents a fixed metabolite.
 *
 *  Created for Copasi by Stefan Hoops 2003
 */

#include "copasi.h"
#include "CMathConstant.h"
#include "CMathVariable.h"

#include "model/CModel.h"
#include "model/CMetab.h"
#include "model/CReaction.h"
#include "model/CCompartment.h"
#include "report/CCopasiObjectReference.h"

CMathConstant::CMathConstant(const CMathConstant & src):
    CMathSymbol(src)
{}

CMathConstant::CMathConstant(const CCopasiObject * pObject):
    CMathSymbol(pObject)
{}

CMathConstant::~CMathConstant() {}

/* Metabolites */
CMathConstantMetab::CMathConstantMetab(const CMathConstantMetab & src):
    CMathConstant(src)
{}

CMathConstantMetab::CMathConstantMetab(const CMetab & metabolite):
    CMathConstant(& metabolite)
{}

CMathConstantMetab::~CMathConstantMetab() {}

bool CMathConstantMetab::setValue(const C_FLOAT64 & value)
{return setConcentration(value);}

const C_FLOAT64 & CMathConstantMetab::getValue() const
  {return getConcentration();}

bool CMathConstantMetab::setConcentration(const C_FLOAT64 & concentration)
{
  ((CMetab *) mpObject)->setInitialConcentration(concentration);
  return true;
}

const C_FLOAT64 & CMathConstantMetab::getConcentration() const
  {return ((CMetab *) mpObject)->getInitialConcentration();}

bool CMathConstantMetab::setParticleNumber(const C_FLOAT64 & particleNumber)
{
  ((CMetab *) mpObject)->setInitialValue(particleNumber);
  return true;
}

C_FLOAT64 CMathConstantMetab::getParticleNumber() const
  {return ((CMetab *) mpObject)->getInitialValue();}

bool CMathConstantMetab::compile()
{
  mpCompartment = (CMathConstantCompartment *)(CMathConstant *)
                  CMathSymbol::find(((CMetab *) mpObject)->getCompartment());

  if (mpCompartment) return true;
  else return false;
}

CMathConstantCompartment & CMathConstantMetab::getCompartment() const
{return *mpCompartment;}

/* Reference */
CMathConstantReference::CMathConstantReference(const CMathConstantReference & src):
    CMathConstant(src)
{}

CMathConstantReference::CMathConstantReference(const CCopasiObjectReference< C_FLOAT64 > & reference):
    CMathConstant(&reference)
{assert(reference.isReference());}

CMathConstantReference::~CMathConstantReference() {}

bool CMathConstantReference::setValue(const C_FLOAT64 & value)
{
  *(C_FLOAT64 *)mpObject->getValuePointer() = value;
  return true;
}

const C_FLOAT64 & CMathConstantReference::getValue() const
  {return *(C_FLOAT64 *)mpObject->getValuePointer();}

/* Parameter */
std::map< std::string, CCopasiObject * > CMathConstantParameter::mSelection;

CMathConstantParameter::CMathConstantParameter(const CMathConstantParameter & src):
    CMathConstant(src)
{}

CMathConstantParameter::CMathConstantParameter(const CCopasiParameter & parameter, const std::string & reaction):
    CMathConstant(& parameter),
    mReaction(reaction)
{
  if (parameter.getObjectName() != getName())
    {
      std::string Name(parameter.getObjectName() + "{" + mReaction + "}");
      setName(Name);
    }
}

CMathConstantParameter::~CMathConstantParameter() {}

bool CMathConstantParameter::setValue(const C_FLOAT64 & value)
{
  ((CCopasiParameter *) mpObject)->setValue(value);
  return true;
}

const C_FLOAT64 & CMathConstantParameter::getValue() const
  {
    return
    * static_cast<CCopasiParameter *>(mpObject)->getValue().pDOUBLE;
  }

const std::string & CMathConstantParameter::getReaction() const
  {return mReaction;}

const std::map< std::string, CCopasiObject * > &
CMathConstantParameter::getSelection()
{return mSelection;}

bool CMathConstantParameter::buildSelection(const CModel * pModel)
{
  const CCopasiVector< CReaction > & Reactions = pModel->getReactions();
  unsigned C_INT32 i, imax = Reactions.size();
  unsigned C_INT32 j, jmax;

  CReaction * pReaction;
  const CCopasiParameterGroup * pParameters;
  std::string Name;

  mSelection.clear();

  for (i = 0; i < imax; i++)
    {
      pReaction = Reactions[i];
      Name = pReaction->getObjectName();
      pParameters = & pReaction->getParameters();

      jmax = pParameters->size();

      for (j = 0; j < jmax; j++)
        mSelection[pParameters->getName(j) + "(" + Name + ")"] =
          const_cast< CCopasiParameterGroup * >(pParameters)->getParameter(j);
    }

  return true;
}

/* Compartment */
CMathConstantCompartment::CMathConstantCompartment(const CMathConstantCompartment & src):
    CMathConstant(src),
    mMetaboliteList(src.mMetaboliteList)
{}

CMathConstantCompartment::CMathConstantCompartment(const CCompartment & compartment):
    CMathConstant(& compartment)
{compile();}

CMathConstantCompartment::~CMathConstantCompartment() {}

bool CMathConstantCompartment::setValue(const C_FLOAT64 & value)
{
  ((CCompartment *) mpObject)->setInitialValue(value);
  return true;
}

const C_FLOAT64 & CMathConstantCompartment::getValue() const
  {return ((CCompartment *) mpObject)->getInitialValue();}

const C_FLOAT64 & CMathConstantCompartment::getTransientValue() const
  {return ((CCompartment *) mpObject)->getValue();}

const std::vector< CMathSymbol * > &
CMathConstantCompartment::getMetaboliteList() const
  {return mMetaboliteList;}

bool CMathConstantCompartment::compile()
{
  CCopasiVectorN< CMetab > & mList =
    ((CCompartment *) mpObject)->getMetabolites();
  unsigned C_INT32 i, imax = mList.size();
  CMathSymbol * m;
  mMetaboliteList.clear();
  bool Success = true;

  for (i = 0; i < imax; i++)
    {
      m = CMathSymbol::find(mList[i]);

      if (m) mMetaboliteList.push_back(m);
      else Success = false;
    }

  return Success;
}

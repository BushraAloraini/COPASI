// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/model/CMoiety.cpp,v $
//   $Revision: 1.50 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2009/05/19 16:11:34 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#include <stdio.h>
#include <float.h>
#include "mathematics.h"

#include "copasi.h"
#include "CMoiety.h"
#include "CCompartment.h"
#include "CMetabNameInterface.h"

#include "report/CCopasiObjectReference.h"
#include "report/CKeyFactory.h"//By G
#include "function/CExpression.h"
#include "utilities/CCopasiMessage.h"
#include "utilities/CReadConfig.h"
#include "utilities/CCopasiVector.h"
#include "utilities/utility.h"
#include "copasi/report/CCopasiRootContainer.h"

CMoiety::CMoiety(const std::string & name,
                 const CCopasiContainer * pParent):
    CCopasiContainer(name, pParent, "Moiety"),
    mKey(CCopasiRootContainer::getKeyFactory()->add("Moiety", this)), //By G
    mNumber(0),
    mINumber(0),
    mEquation()
{
  initObjects();
  CONSTRUCTOR_TRACE;
}

CMoiety::CMoiety(const CMoiety & src,
                 const CCopasiContainer * pParent):
    CCopasiContainer(src, pParent),
    mKey(CCopasiRootContainer::getKeyFactory()->add("Moiety", this)), //By G
    mNumber(src.mNumber),
    mINumber(src.mINumber),
    mEquation(src.mEquation)
{
  initObjects();
  CONSTRUCTOR_TRACE;
}

CMoiety::~CMoiety()
{
  CCopasiRootContainer::getKeyFactory()->remove(mKey);
  DESTRUCTOR_TRACE;
}

void CMoiety::initObjects()
{
  setRefresh(this, &CMoiety::refreshDependentNumber);

  mpINumberReference =
    static_cast<CCopasiObjectReference< C_FLOAT64 > *>(addObjectReference("Value", mINumber, CCopasiObject::ValueDbl));
  mpINumberReference->setRefresh(this, &CMoiety::refreshInitialValue);

  mpDNumberReference =
    static_cast<CCopasiObjectReference< C_FLOAT64 > *>(addObjectReference("DependentValue", mNumber, CCopasiObject::ValueDbl));
  mpDNumberReference->addDirectDependency(this);

  return;
}

void CMoiety::add(C_FLOAT64 value, CMetab * pMetabolite)
{
  if (!mEquation.size())
    pMetabolite->setDependentOn(this);
  else
    addDirectDependency(pMetabolite->mpValueReference);

  mpINumberReference->addDirectDependency(pMetabolite->mpIValueReference);

  std::pair<C_FLOAT64, CMetab *> element;

  element.first = value;
  element.second = pMetabolite;

  mEquation.push_back(element);
}

void CMoiety::cleanup()
{
  clearDirectDependencies();
  mEquation.clear();
  mpINumberReference->setDirectDependencies(getDirectDependencies());
}

void CMoiety::refreshDependentNumber()
{
  mNumber = mINumber;

  std::vector< std::pair< C_FLOAT64, CMetab * > >::iterator it = mEquation.begin() + 1;
  std::vector< std::pair< C_FLOAT64, CMetab * > >::iterator end = mEquation.end();

  for (; it != end; ++it)
    mNumber -= it->first * it->second->getValue();

  return;
}

const C_FLOAT64 & CMoiety::dependentNumber()
{
  refreshDependentNumber();
  return mNumber;
}

const C_FLOAT64 & CMoiety::getDependentNumber() const
{return mNumber;}

const std::string & CMoiety::getKey() const {return mKey;} //By G

std::string CMoiety::getDescription(const CModel * model) const
{
  std::string Description;

  std::vector< std::pair< C_FLOAT64, CMetab * > >::const_iterator it = mEquation.begin();
  std::vector< std::pair< C_FLOAT64, CMetab * > >::const_iterator end = mEquation.end();

  for (; it != end; ++it)
    {
      if (it != mEquation.begin())
        {
          if (it->first < 0.0)
            Description += " - ";
          else
            Description += " + ";
        }

      if (fabs(it->first) > 1.0 + 100 * DBL_EPSILON ||
          fabs(it->first) < 1.0 - 100 * DBL_EPSILON)
        Description += StringPrint("%g * ", fabs(it->first));

      Description += CMetabNameInterface::getDisplayName(model, *it->second);
    }

  return Description;
}

void CMoiety::refreshInitialValue()
{
  mINumber = 0;

  std::vector< std::pair< C_FLOAT64, CMetab * > >::iterator it = mEquation.begin();
  std::vector< std::pair< C_FLOAT64, CMetab * > >::iterator end = mEquation.end();

  for (; it != end; ++it)
    mINumber += it->first * it->second->getInitialValue();

  return;
}

CCopasiObject * CMoiety::getInitialValueReference() const
{return mpINumberReference;}

C_FLOAT64 CMoiety::getNumber() const
{
  return mINumber;
}

std::string CMoiety::getExpression() const
{
  std::string Infix;

  std::vector< std::pair< C_FLOAT64, CMetab * > >::const_iterator it = mEquation.begin();
  std::vector< std::pair< C_FLOAT64, CMetab * > >::const_iterator end = mEquation.end();

  for (; it != end; ++it)
    {
      if (it != mEquation.begin())
        {
          if (it->first < 0.0)
            Infix += "-";
          else
            Infix += "+";
        }

      if (fabs(it->first) > 1.0 + 100 * DBL_EPSILON ||
          fabs(it->first) < 1.0 - 100 * DBL_EPSILON)
        Infix += StringPrint("%g*", fabs(it->first));

      Infix += "<" + it->second->getInitialValueReference()->getCN() + ">";
    }

  return Infix;
}

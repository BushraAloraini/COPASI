// Copyright (C) 2010 - 2015 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 - 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2005 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/*!
    \file CModelValue.cpp
    \brief Implementation file of class CModelEntity and CModelValue
 */

#include <iostream>
#include <string>
#include <vector>
#include <limits>

#include "copasi.h"

#include "CModel.h"
#include "CModelValue.h"

#include "CopasiDataModel/CCopasiDataModel.h"
#include "function/CExpression.h"
#include "report/CCopasiObjectReference.h"
#include "report/CKeyFactory.h"
#include "utilities/utility.h"
#include "report/CCopasiRootContainer.h"

//static
const std::string CModelEntity::StatusName[] =
{
  "fixed",
  "assignment",
  "reactions",
  "ode",
  "time",
  ""
};

//static
const char * CModelEntity::XMLStatus[] =
{
  "fixed",
  "assignment",
  "reactions",
  "ode",
  "time",
  NULL
};

// the "variable" keyword is used for compatibility reasons. It actually means "this metab is part
// of the reaction network, copasi needs to figure out if it is independent, dependent (moieties) or unused."

CModelEntity::CModelEntity(const std::string & name,
                           const CCopasiContainer * pParent,
                           const std::string & type,
                           const unsigned C_INT32 & flag):
  CCopasiContainer(name, pParent, type, (flag | CCopasiObject::Container | CCopasiObject::ValueDbl | CCopasiObject::ModelEntity)),
  CAnnotation(),
  mValue(std::numeric_limits<C_FLOAT64>::quiet_NaN()),
  mIValue(1.0),
  mRate(0.0),
  mpExpression(NULL),
  mpInitialExpression(NULL),
  mStatus(FIXED),
  mUsed(false),
  mpModel(NULL),
  mpUnit(NULL)
{
  mKey = CCopasiRootContainer::getKeyFactory()->add(getObjectType(), this);

  initObjects();

  CONSTRUCTOR_TRACE;
}

CModelEntity::CModelEntity(const CModelEntity & src,
                           const CCopasiContainer * pParent):
  CCopasiContainer(src, pParent),
  CAnnotation(src),
  mValue(src.mValue),
  mIValue(src.mIValue),
  mRate(src.mRate),
  mpExpression(src.mpExpression != NULL ? new CExpression(*src.mpExpression, this) : NULL),
  mpInitialExpression(src.mpInitialExpression != NULL ? new CExpression(*src.mpInitialExpression, this) : NULL),
  mStatus(FIXED),
  mUsed(false),
  mpModel(NULL),
  mpUnit(NULL)
{
  mKey = CCopasiRootContainer::getKeyFactory()->add(getObjectType(), this);

  initObjects();

  if (src.mpUnit != NULL)
    {
      if (mpModel == NULL)
        {
          new CUnit(*src.mpUnit, CUnit::Avogadro, this);
        }
      else
        {
          new CUnit(*src.mpUnit, mpModel->getAvogadro(), this);
        }
    }

  setStatus(src.mStatus);
  setMiriamAnnotation(src.getMiriamAnnotation(), mKey, src.mKey);
}

CModelEntity::~CModelEntity()
{
  if (mpModel)
    mpModel->removeModelEntity(this);

  // After the above call we definitely own the data and
  // therefore must destroy them.

  pdelete(mpUnit);
  // since the expressions now have the model entity as parent, they should
  // automatically be destroyed be the destructor of CCopasiContainer
  //pdelete(mpExpression);
  //pdelete(mpInitialExpression);

  DESTRUCTOR_TRACE;
}

// virtual
const std::string & CModelEntity::getKey() const
{
  return CAnnotation::getKey();
}

const C_FLOAT64 & CModelEntity::getValue() const {return mValue;}

const C_FLOAT64 & CModelEntity::getInitialValue() const
{return mIValue;}

const CModelEntity::Status & CModelEntity::getStatus() const {return mStatus;}

bool CModelEntity::compile()
{
  bool success = true;

  std::set< const CCopasiObject * > NoDependencies;
  CObjectInterface::ContainerList listOfContainer;
  listOfContainer.push_back(mpModel);
  CCopasiDataModel* pDataModel = NULL;

  switch (mStatus)
    {
      case ASSIGNMENT:
        success &= mpExpression->compile(listOfContainer);
        mpValueReference->setDirectDependencies(mpExpression->getDirectDependencies());

        pdelete(mpInitialExpression);
        pDataModel = getObjectDataModel();
        assert(pDataModel != NULL);
        mpInitialExpression = CExpression::createInitialExpression(*mpExpression, pDataModel);
        mpInitialExpression->setObjectName("InitialExpression");
        add(mpInitialExpression, true);
        break;

      case ODE:
        mpValueReference->addDirectDependency(this);

        success &= mpExpression->compile(listOfContainer);
        mpRateReference->setDirectDependencies(mpExpression->getDirectDependencies());
        break;

      default:
        break;
    }

  // Here we handle initial expressions for all types.
  if (mpInitialExpression != NULL &&
      mpInitialExpression->getInfix() != "")
    {
      success &= mpInitialExpression->compile(listOfContainer);
      mpIValueReference->setDirectDependencies(mpInitialExpression->getDirectDependencies());

      // If we have a valid initial expression, we update the initial value.
      // In case the expression is constant this suffices other are updated lated again.
      if (mpInitialExpression->isUsable())
        mIValue = mpInitialExpression->calcValue();
    }
  else
    {
      mpIValueReference->setDirectDependencies(NoDependencies);
    }

  return success;
}

void CModelEntity::calculate()
{
  switch (mStatus)
    {
      case ASSIGNMENT:
        mValue = mpExpression->calcValue();
        break;

      case ODE:
        mRate = mpExpression->calcValue();
        break;

      default:
        break;
    }
}

void CModelEntity::refreshInitialValue()
{
  if (mpInitialExpression != NULL &&
      mpInitialExpression->getInfix() != "")
    mIValue = mpInitialExpression->calcValue();
}

bool CModelEntity::setExpression(const std::string & expression)
{
  if (isFixed()) return false;

  if (mpExpression == NULL)
    mpExpression = new CExpression("Expression", this);

  if (mpModel)
    mpModel->setCompileFlag(true);

  if (!mpExpression->setInfix(expression)) return false;

  return compile();
}

std::string CModelEntity::getExpression() const
{
  if (isFixed() || mpExpression == NULL)
    return "";

  mpExpression->updateInfix();
  return mpExpression->getInfix();
}

CExpression* CModelEntity::getExpressionPtr()
{
  if (mpExpression != NULL) mpExpression->updateInfix();

  return mpExpression;
}

const CExpression* CModelEntity::getExpressionPtr() const
{
  if (mpExpression != NULL) mpExpression->updateInfix();

  return mpExpression;
}

bool CModelEntity::setExpressionPtr(CExpression* pExpression)
{
  if (isFixed()) return false;

  if (pExpression == mpExpression) return true;

  if (pExpression == NULL) return false;

  if (mpModel != NULL)
    {
      mpModel->setCompileFlag(true);
    }

  CExpression * pOld = mpExpression;
  mpExpression = pExpression;

  mpExpression->setObjectName("Expression");
  add(mpExpression, true);

  if (compile())
    {
      pdelete(pOld);
      return true;
    }

  // If compile fails we do not take ownership
  // and we remove the object from the container
  remove(mpExpression);
  mpExpression->setObjectParent(NULL);
  mpExpression = pOld;

  return false;
}

CExpression* CModelEntity::getInitialExpressionPtr()
{
  if (mpInitialExpression != NULL) mpInitialExpression->updateInfix();

  return mpInitialExpression;
}

const CExpression* CModelEntity::getInitialExpressionPtr() const
{
  if (mpInitialExpression != NULL) mpInitialExpression->updateInfix();

  return mpInitialExpression;
}

bool CModelEntity::setInitialExpressionPtr(CExpression* pExpression)
{
  if (mStatus == ASSIGNMENT) return false;

  if (pExpression == mpInitialExpression) return true;

  if (pExpression == NULL) return false;

  if (mpModel != NULL)
    {
      mpModel->setCompileFlag(true);
    }

  CExpression * pOld = mpInitialExpression;
  mpInitialExpression = pExpression;

  mpInitialExpression->setObjectName("InitialExpression");
  add(mpInitialExpression, true);

  if (compile())
    {
      pdelete(pOld);
      return true;
    }

  // If compile fails we do not take ownership
  // and we remove the object from the container
  remove(mpInitialExpression);
  mpInitialExpression->setObjectParent(NULL);
  mpInitialExpression = pOld;

  return false;
}

bool CModelEntity::setInitialExpression(const std::string & expression)
{
  if (mStatus == ASSIGNMENT) return false;

  if (mpModel)
    mpModel->setCompileFlag(true);

  if (mpInitialExpression == NULL)
    {
      mpInitialExpression = new CExpression("InitialExpression", this);
    }

  if (!mpInitialExpression->setInfix(expression)) return false;

  return compile();
}

std::string CModelEntity::getInitialExpression() const
{
  if (mStatus == ASSIGNMENT || mpInitialExpression == NULL)
    return "";

  mpInitialExpression->updateInfix();
  return mpInitialExpression->getInfix();
}

// virtual
bool CModelEntity::setUnitDefinition(const std::string & unitDefinition)
{
  return false;
}

// virtual
bool CModelEntity::setUnit(const CUnit & unit)
{
  return false;
}

// virtual
const CUnit & CModelEntity::getUnit() const
{
  if (mpUnit == NULL)
    {
      return CUnit::EmptyUnit;
    }

  return *mpUnit;
}

/**
 * Return rate of production of this entity
 */
const C_FLOAT64 & CModelEntity::getRate() const
{
  return mRate;
}

CCopasiObject * CModelEntity::getInitialValueReference() const
{return mpIValueReference;}

CCopasiObject * CModelEntity::getValueReference() const
{return mpValueReference;}

CCopasiObject * CModelEntity::getRateReference() const
{return mpRateReference;}

//***********

void CModelEntity::setValue(const C_FLOAT64 & value)
{
  mValue = value;
}

void CModelEntity::setInitialValue(const C_FLOAT64 & initialValue)
{
  mIValue = initialValue;
}

void CModelEntity::setRate(const C_FLOAT64 & rate)
{
  mRate = rate;
}

//  ******************

void CModelEntity::setStatus(const CModelEntity::Status & status)
{
  if (mStatus != status)
    {
      if (mpModel != NULL)
        mpModel->setCompileFlag(true);

      // An assignment may not have an initial expression.
      // However, internally we always create one, which need
      // to be deleted.
      if (mStatus == ASSIGNMENT)
        pdelete(mpInitialExpression);

      mStatus = status;

      if (mpModel != NULL)
        mpModel->setCompileFlag(true);

      std::set< const CCopasiObject * > NoDependencies;

      setDirectDependencies(NoDependencies);

      mpIValueReference->setDirectDependencies(NoDependencies);

      mpValueReference->setDirectDependencies(NoDependencies);

      mpRateReference->setDirectDependencies(NoDependencies);
      CCopasiDataModel* pDataModel = NULL;

      switch (mStatus)
        {
          case ASSIGNMENT:

            if (mpExpression == NULL)
              mpExpression = new CExpression("Expression", this);

            pdelete(mpInitialExpression);
            pDataModel = getObjectDataModel();
            mpInitialExpression = CExpression::createInitialExpression(*mpExpression, pDataModel);
            mpInitialExpression->setObjectName("InitialExpression");
            add(mpInitialExpression, true);

            mpValueReference->setDirectDependencies(mpExpression->getDirectDependencies());

            mRate = std::numeric_limits<C_FLOAT64>::quiet_NaN();

            mUsed = true;
            break;

          case ODE:

            if (mpExpression == NULL)
              mpExpression = new CExpression("Expression", this);

            mpRateReference->setDirectDependencies(mpExpression->getDirectDependencies());

            mUsed = true;
            break;

          case REACTIONS:
            pdelete(mpExpression);

            mUsed = true;
            break;

          case TIME:
            pdelete(mpExpression);

            mUsed = true;
            break;

          case FIXED:
            pdelete(mpExpression);

            mRate = 0.0;

            mUsed = false;
            break;
        }
    }
}

// virtual
const CCopasiObject * CModelEntity::getValueObject() const
{
  return mpValueReference;
}

// virtual
void * CModelEntity::getValuePointer() const
{
  return const_cast<C_FLOAT64 *>(&mValue);
}

void CModelEntity::initObjects()
{
  C_FLOAT64 Dummy;

  mpValueReference =
    static_cast<CCopasiObjectReference<C_FLOAT64> *>(addObjectReference("Value",
        mValue,
        CCopasiObject::ValueDbl));

  mpIValueReference =
    static_cast<CCopasiObjectReference<C_FLOAT64> *>(addObjectReference("InitialValue",
        mIValue,
        CCopasiObject::ValueDbl));

  mpRateReference =
    static_cast<CCopasiObjectReference<C_FLOAT64> *>(addObjectReference("Rate", mRate, CCopasiObject::ValueDbl));

  addObjectReference("SBMLId", mSBMLId, CCopasiObject::ValueString);

  mpModel = static_cast<CModel *>(getObjectAncestor("Model"));

  if (mpModel)
    {
      mpModel->addModelEntity(this);
    }
}

bool CModelEntity::setObjectParent(const CCopasiContainer * pParent)
{
  CCopasiContainer::setObjectParent(pParent);
  CModel * pNewModel = static_cast<CModel *>(getObjectAncestor("Model"));

  if (mpModel == pNewModel) return true;

  if (mpModel)
    {
      // This allocates new objects for mpIValue and mpValueData
      mpModel->removeModelEntity(this);
    }

  // We can safely remove the currently allocated objects as they
  // are not part of an CStateTemplate
  if (pNewModel)
    {
      pNewModel->addModelEntity(this);
    }

  mpModel = pNewModel;

  return true;
}

std::set< const CCopasiObject * > CModelEntity::getDeletedObjects() const
{
  std::set< const CCopasiObject * > Deleted;

  Deleted.insert(this);
  Deleted.insert(mpIValueReference);
  Deleted.insert(mpValueReference);
  Deleted.insert(mpRateReference);

  return Deleted;
}

// virtual
bool CModelEntity::mustBeDeleted(const CCopasiObject::DataObjectSet & deletedObjects) const
{
  bool MustBeDeleted = false;

  DataObjectSet ChildObjects = getDeletedObjects();

  DataObjectSet::const_iterator it = ChildObjects.begin();
  DataObjectSet::const_iterator end = ChildObjects.end();

  for (; it != end; ++it)
    {
      if (*it == this)
        {
          if ((*it)->CCopasiObject::mustBeDeleted(deletedObjects))
            {
              MustBeDeleted = true;
              break;
            }

          continue;
        }

      if ((*it)->mustBeDeleted(deletedObjects))
        {
          MustBeDeleted = true;
          break;
        }
    }

  return MustBeDeleted;
}

void CModelEntity::setSBMLId(const std::string& id)
{
  this->mSBMLId = id;
}

const std::string& CModelEntity::getSBMLId() const
{
  return this->mSBMLId;
}

void CModelEntity::setUsed(const bool & used)
{mUsed = used;}

const bool & CModelEntity::isUsed() const
{return mUsed;}

//********************************************************************+

CModelValue::CModelValue(const std::string & name,
                         const CCopasiContainer * pParent):
  CModelEntity(name, pParent, "ModelValue")
{
  initObjects();

  CONSTRUCTOR_TRACE;
}

CModelValue::CModelValue(const CModelValue & src,
                         const CCopasiContainer * pParent):
  CModelEntity(src, pParent)
{
  initObjects();
  CONSTRUCTOR_TRACE;
}

CModelValue::~CModelValue()
{
  DESTRUCTOR_TRACE;
}

void CModelValue::initObjects()
{}

// virtual
bool CModelValue::setUnitDefinition(const std::string & unitDefinition)
{
  if (mpUnit == NULL)
    {
      mpUnit = new CUnit();
    }

  C_FLOAT64 Avogadro = CUnit::Avogadro;

  const CModel * pModel = static_cast<const CModel *>(getObjectAncestor("Model"));

  if (pModel != NULL)
    {
      Avogadro = pModel->getAvogadro();
    }

  return mpUnit->setInfix(unitDefinition, Avogadro);
}

// virtual
bool CModelValue::setUnit(const CUnit & unit)
{
  if (mpUnit == NULL)
    {
      mpUnit = new CUnit(unit);
    }
  else
    {
      *mpUnit = unit;
    }

  return true;
}

std::ostream & operator<<(std::ostream &os, const CModelValue & d)
{
  os << "    ++++CModelValue: " << d.getObjectName() << std::endl;
  os << "        mValue " << d.mValue << " mIValue " << d.mIValue << std::endl;
  os << "        mRate " << d.mRate << " mStatus " << d.getStatus() << std::endl;
  os << "    ----CModelValue " << std::endl;

  return os;
}

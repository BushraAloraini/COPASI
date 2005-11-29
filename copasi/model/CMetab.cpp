/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/model/CMetab.cpp,v $
   $Revision: 1.89 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2005/11/29 17:28:12 $
   End CVS Header */

#include <iostream>
#include <string>
#include <vector>

#include "copasi.h"
#include "CopasiDataModel/CCopasiDataModel.h"
#include "utilities/utility.h"
#include "report/CCopasiObjectReference.h"
#include "report/CKeyFactory.h"
#include "CCompartment.h"
#include "CModel.h"
#include "CMetab.h"
#include "CMetabNameInterface.h"

//static
const CCompartment * CMetab::mpParentCompartment = NULL;

//static
void CMetab::setParentCompartment(const CCompartment * parentCompartment)
{mpParentCompartment = parentCompartment;}

//static
C_FLOAT64 CMetab::convertToNumber(const C_FLOAT64 & concentration,
                                  const CCompartment & compartment,
                                  const CModel & model)
{return concentration * compartment.getVolume() * model.getQuantity2NumberFactor();}

//static
C_FLOAT64 CMetab::convertToConcentration(const C_FLOAT64 & number,
    const CCompartment & compartment,
    const CModel & model)
{return number * compartment.getVolumeInv() * model.getNumber2QuantityFactor();}

CMetab::CMetab(const std::string & name,
               const CCopasiContainer * pParent):
    CModelEntity(name, pParent, "Metabolite",
                 CCopasiObject::NonUniqueName),
    //mKey(GlobalKeys.add("Metabolite", this)),
    mConc(-1.0),
    mIConc(-1.0),
    //mNumber(-1.0),
    //mINumber(-1.0),
    //mRate(0.0),
    mTT(0.0),
    //mStatus(REACTIONS),
    mpCompartment(NULL),
    mpModel(NULL)
{
  mKey = GlobalKeys.add("Metabolite", this);
  mStatus = REACTIONS;
  if (getObjectParent())
    {
      initModel();
      initCompartment(NULL);

      setInitialConcentration(1.0);
      setConcentration(1.0);
    }

  initObjects();
  CONSTRUCTOR_TRACE;
}

CMetab::CMetab(const CMetab & src,
               const CCopasiContainer * pParent):
    CModelEntity(src, pParent),
    //mKey(GlobalKeys.add("Metabolite", this)),
    mConc(src.mConc),
    mIConc(src.mIConc),
    //mNumber(src.mNumber),
    //mINumber(src.mINumber),
    //mRate(src.mRate),
    mTT(src.mTT),
    //mStatus(src.mStatus),
    mpCompartment(NULL),
    mpModel(NULL)
{
  mKey = GlobalKeys.add("Metabolite", this);
  initModel();
  initCompartment(src.mpCompartment);
  initObjects();
  CONSTRUCTOR_TRACE;
}

CMetab &CMetab::operator=(const CMetabOld &RHS)
{
  setObjectName(RHS.getObjectName());

  setInitialConcentration(RHS.mIConc);
  setConcentration(RHS.mIConc);

  mRate = 0.0;
  mTT = 0.0;
  mStatus = RHS.mStatus;

  return *this;  // Assignment operator returns left side.
}

CMetab::~CMetab()
{
  GlobalKeys.remove(mKey);
  DESTRUCTOR_TRACE;
}

void CMetab::cleanup() {}

void CMetab::initModel()
{
  mpModel = dynamic_cast< CModel * >(getObjectAncestor("Model"));
  if (!mpModel && CCopasiDataModel::Global) mpModel = CCopasiDataModel::Global->getModel();
}

void CMetab::initCompartment(const CCompartment * pCompartment)
{
  mpCompartment = (const CCompartment *) getObjectAncestor("Compartment");
  if (!mpCompartment) mpCompartment = pCompartment;
  if (!mpCompartment) mpCompartment = mpParentCompartment;
}

const C_FLOAT64 & CMetab::getConcentration() const {return mConc;}

//const C_FLOAT64 & CMetab::getNumber() const {return mNumber;}

const C_FLOAT64 & CMetab::getInitialConcentration() const {return mIConc;}

//const C_FLOAT64 & CMetab::getInitialNumber() const {return mINumber;}

//const CMetab::Status & CMetab::getStatus() const {return mStatus;}

const CCompartment * CMetab::getCompartment() const {return mpCompartment;}

const CModel * CMetab::getModel() const {return mpModel;}

void CMetab::setTransitionTime(const C_FLOAT64 & transitionTime)
{mTT = transitionTime;}

const C_FLOAT64 & CMetab::getTransitionTime() const {return mTT;}

bool CMetab::setObjectParent(const CCopasiContainer * pParent)
{
  CCopasiObject::setObjectParent(pParent);

  initCompartment(NULL);
  initModel();

  return true;
}

// ***** set quantities ********

void CMetab::setConcentration(const C_FLOAT64 concentration)
{
  mConc = concentration;
  mValue = concentration * mpCompartment->getVolume()
           * mpModel->getQuantity2NumberFactor();

#ifdef COPASI_DEBUG
  //if (mStatus == METAB_FIXED)
  //std::cout << "warning: set the transient concentration on a fixed metab" << std::endl;
#endif
}

bool CMetab::setInitialConcentration(const C_FLOAT64 & initialConcentration)
{
  //if (mIConc == initialConcentration) return true;

  mIConc = initialConcentration;
  mIValue = initialConcentration * mpCompartment->getVolume()
            * mpModel->getQuantity2NumberFactor();

  if (mStatus == FIXED)
    setConcentration(initialConcentration);

  if (mpModel)
    const_cast<CModel*>(mpModel)->updateMoietyValues();

  return true;
}

void CMetab::setValue(const C_FLOAT64 number)
{
  mConc = number * mpCompartment->getVolumeInv()
          * mpModel->getNumber2QuantityFactor();
  mValue = number;

#ifdef COPASI_DEBUG
  //  if (mStatus == METAB_FIXED)
  //    std::cout << "warning: set the transient particle number on a fixed metab" << std::endl;
#endif
}

bool CMetab::setInitialValue(const C_FLOAT64 & initialNumber)
{
  //if (mINumber == initialNumber) return true;

  mIConc = initialNumber * mpCompartment->getVolumeInv()
           * mpModel->getNumber2QuantityFactor();
  mIValue = initialNumber;

  if (mStatus == FIXED)
    setNumber(initialNumber);

  if (mpModel)
    const_cast<CModel*>(mpModel)->updateMoietyValues();

  return true;
}

//  ******************

void CMetab::setStatus(const CMetab::Status & status)
{
  mStatus = status;
  if (mStatus == FIXED)
    {
      if (mpCompartment && mpModel)
        setNumber(getInitialNumber());
    }
}

//void CMetab::setCompartment(const CCompartment * compartment)
//{mpCompartment = compartment;}

//void CMetab::setModel(CModel * model) {mpModel = model;}

void CMetab::initObjects()
{
  CCopasiObject * pObject;

  addObjectReference("Concentration", mConc, CCopasiObject::ValueDbl);
  pObject = addObjectReference("InitialConcentration", mIConc, CCopasiObject::ValueDbl);
  pObject->setUpdateMethod(this, &CMetab::setInitialConcentration);
  addObjectReference("ParticleNumber", mValue, CCopasiObject::ValueDbl);
  pObject = addObjectReference("InitialParticleNumber", mIValue, CCopasiObject::ValueDbl);
  pObject->setUpdateMethod(this, &CMetab::setInitialNumber);
  addObjectReference("TransitionTime", mTT, CCopasiObject::ValueDbl);
}

// non-member
/*bool operator<(const CMetab &lhs, const CMetab &rhs)
{
  // Do the comparison based on the name
  if (lhs.getObjectName() < rhs.getObjectName())
    {
      return true;
    }
  else
    {
      return false;
    }
}*/

/**
 * Return rate of production of this metaboLite
 */ 
/*const C_FLOAT64 & CMetab::getNumberRate() const
  {
    return mRate;
  }*/

C_FLOAT64 CMetab::getConcentrationRate() const
  {
    return mRate * getCompartment()->getVolumeInv()
    * mpModel->getNumber2QuantityFactor();
  }

/*void CMetab::setNumberRate(const C_FLOAT64 & rate)
{
  //converts particles/time to concentration/time
  mRate = rate;
}*/

//void CMetab::setConcentrationRate(const C_FLOAT64 & rate)
//{mRate = rate * ...;}

void * CMetab::getReference() const
  {
    //std::cout << "CMetab::getReference();" << std::endl;
    return const_cast<C_FLOAT64 *>(&mConc);
  }

std::ostream & operator<<(std::ostream &os, const CMetab & d)
{
  os << "    ++++CMetab: " << d.getObjectName() << std::endl;
  os << "        mConc " << d.mConc << " mIConc " << d.mIConc << std::endl;
  os << "        mValue (particle number) " << d.mValue << " mIValue " << d.mIValue << std::endl;
  os << "        mRate " << d.mRate << " mTT " << d.mTT << " mStatus " << d.mStatus << std::endl;

  if (d.mpCompartment)
    os << "        mpCompartment == " << d.mpCompartment << std::endl;
  else
    os << "        mpCompartment == 0 " << std::endl;

  if (d.mpModel)
    os << "        mpModel == " << d.mpModel << std::endl;
  else
    os << "        mpModel == 0 " << std::endl;

  os << "    ----CMetab " << std::endl;

  return os;
}

C_INT32 CMetab::load(CReadConfig &configbuffer)
{
  C_INT32 Fail = 0;

  std::string tmp;
  Fail = configbuffer.getVariable("Metabolite", "string",
                                  (void *) & tmp,
                                  CReadConfig::SEARCH);

  if (Fail)
    return Fail;
  setObjectName(tmp);

  Fail = configbuffer.getVariable("InitialConcentration", "C_FLOAT64",
                                  (void *) & mIConc);

  setInitialConcentration(mIConc);
  setConcentration(mIConc);

  Fail = configbuffer.getVariable("Type", "C_INT16",
                                  (void *) & mStatus);

  if (Fail)
    return Fail;

  // sanity check
  if ((mStatus < 0) || (mStatus > 7))
    {
      CCopasiMessage(CCopasiMessage::WARNING,
                     "The file specifies a non-existing type "
                     "for '%s'.\nReset to internal metabolite.",
                     getObjectName().c_str());
      mStatus = CMetab::REACTIONS;
    }

  // sanity check
  if ((mStatus != METAB_MOIETY) && (mIConc < 0.0))
    {
      CCopasiMessage(CCopasiMessage::WARNING,
                     "The file specifies a negative concentration "
                     "for '%s'.\nReset to default.",
                     getObjectName().c_str());
      mIConc = 1.0;
    }

  return Fail;
}

std::string CMetab::getObjectDisplayName(bool regular, bool richtext) const
  {
    CModel* tmp = dynamic_cast<CModel*>(this->getObjectAncestor("Model"));
    if (tmp)
      {
        return CMetabNameInterface::getDisplayName(tmp, *this);
      }

    return CCopasiObject::getObjectDisplayName(regular, richtext);
  }

//******************* CMetabOld ***************************************************

CMetabOld::CMetabOld(const std::string & name,
                     const CCopasiContainer * pParent):
    CCopasiContainer(name, pParent, "Old Metabolite"),
    mIConc(1.0),
    mStatus(CModelEntity::REACTIONS),
    mCompartment()
{CONSTRUCTOR_TRACE;}

CMetabOld::CMetabOld(const CMetabOld & src,
                     const CCopasiContainer * pParent):
    CCopasiContainer(src, pParent),
    mIConc(src.mIConc),
    mStatus(src.mStatus),
    mCompartment(src.mCompartment)
{CONSTRUCTOR_TRACE;}

CMetabOld::~CMetabOld() {DESTRUCTOR_TRACE;}

void CMetabOld::cleanup(){}

C_INT32 CMetabOld::load(CReadConfig &configbuffer)
{
  C_INT32 Fail = 0;
  std::string tmp;
  Fail = configbuffer.getVariable("Metabolite", "string",
                                  (void *) & tmp,
                                  CReadConfig::SEARCH);

  if (Fail)
    return Fail;
  setObjectName(tmp);

  Fail = configbuffer.getVariable("Concentration", "C_FLOAT64",
                                  (void *) & mIConc);

  if (Fail)
    return Fail;

  Fail = configbuffer.getVariable("Compartment", "C_INT32",
                                  (void *) & mCompartment);

  if (Fail)
    return Fail;

  C_INT32 Status;

  Fail = configbuffer.getVariable("Type", "C_INT32",
                                  (void *) & Status);

  mStatus = (CMetab::Status) Status;

  // sanity check
  if ((mStatus < 0) || (mStatus > 7))
    {
      CCopasiMessage(CCopasiMessage::WARNING,
                     "The file specifies a non-existing type "
                     "for '%s'.\nReset to internal metabolite.",
                     getObjectName().c_str());
      mStatus = CModelEntity::REACTIONS;
    }

  // sanity check
  if ((mStatus != METAB_MOIETY) && (mIConc < 0.0))
    {
      CCopasiMessage(CCopasiMessage::WARNING,
                     "The file specifies a negative concentration "
                     "for '%s'.\nReset to default.",
                     getObjectName().c_str());
      mIConc = 1.0;
    }

  return Fail;
}

C_INT32 CMetabOld::getIndex() const {return mCompartment;}

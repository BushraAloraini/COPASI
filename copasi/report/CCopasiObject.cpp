/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/report/CCopasiObject.cpp,v $
   $Revision: 1.52 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2006/03/28 23:13:49 $
   End CVS Header */

/**
 * Class CCopasiObject
 *
 * This class is the base class for all global accessible objects in copasi.
 *
 * Copyright Stefan Hoops 2002
 */

#include <sstream>

#include "copasi.h"
#include "CCopasiObjectName.h"
#include "CCopasiObject.h"
#include "CCopasiContainer.h"
#include "CRenameHandler.h"
#include "utilities/CCopasiVector.h"

//static
const C_FLOAT64 CCopasiObject::DummyValue = 0.0;

//static
const CRenameHandler * CCopasiObject::smpRenameHandler = NULL;

//static
UpdateMethod CCopasiObject::mDefaultUpdateMethod;

CCopasiObject::CCopasiObject():
    mObjectName("No Name"),
    mObjectType("Unknown Type"),
    mpObjectParent(NULL),
    mObjectFlag(0),
    mpUpdateMethod(&this->mDefaultUpdateMethod),
    mpRefresh(NULL)
{}

CCopasiObject::CCopasiObject(const std::string & name,
                             const CCopasiContainer * pParent,
                             const std::string & type,
                             const unsigned C_INT32 & flag):
    mObjectName((name == "") ? "No Name" : name),
    mObjectType(type),
    mpObjectParent(const_cast<CCopasiContainer *>(pParent)),
    mObjectFlag(flag),
    mpUpdateMethod(&this->mDefaultUpdateMethod),
    mpRefresh(NULL)
{
  if (mpObjectParent)
    if (mpObjectParent->isContainer()) mpObjectParent->add(this);
}

CCopasiObject::CCopasiObject(const CCopasiObject & src,
                             const CCopasiContainer * pParent):
mObjectName(src.mObjectName),
mObjectType(src.mObjectType),
mpObjectParent(const_cast<CCopasiContainer *>(pParent)),
mObjectFlag(src.mObjectFlag),
mpUpdateMethod(&this->mDefaultUpdateMethod),
mpRefresh(NULL)
{if (mpObjectParent) mpObjectParent->add(this);}

CCopasiObject::~CCopasiObject()
{
  if (mpObjectParent)
    mpObjectParent->remove(this);

  if (mpUpdateMethod != &mDefaultUpdateMethod)
    pdelete(mpUpdateMethod);

  pdelete(mpRefresh);
}

void CCopasiObject::print(std::ostream * ostream) const {(*ostream) << (*this);}

CCopasiObjectName CCopasiObject::getCN() const
  {
    CCopasiObjectName CN;

    if (mpObjectParent)
      {
        std::stringstream tmp;
        tmp << mpObjectParent->getCN();

        if (mpObjectParent->isNameVector())
          tmp << "[" << CCopasiObjectName::escape(mObjectName) << "]";
        else if (mpObjectParent->isVector())
          tmp << "[" << static_cast<const CCopasiVector< CCopasiObject > *>(mpObjectParent)->getIndex(this) << "]";
        else
          tmp << "," << CCopasiObjectName::escape(mObjectType)
          << "=" << CCopasiObjectName::escape(mObjectName);

        CN = tmp.str();
      }
    else
      {
        CN = CCopasiObjectName::escape(mObjectType)
             + "=" + CCopasiObjectName::escape(mObjectName);
      }

    return CN;
  }

const CCopasiObject *
CCopasiObject::getObject(const CCopasiObjectName & C_UNUSED(cn)) const
  {return NULL;}

const std::string
CCopasiObject::getObjectUniqueNameEx(const bool & isParent) const
  {
    if (isParent && isVector())
      return mpObjectParent->getObjectUniqueNameEx();

    if (mpObjectParent && 0 < (mObjectFlag & NonUniqueName))
      return mObjectName + "{" + mpObjectParent->getObjectUniqueNameEx() + "}";

    return mObjectName;
  }

const std::string CCopasiObject::getObjectUniqueName() const
{return getObjectUniqueNameEx(false);}

bool CCopasiObject::setObjectName(const std::string & name)
{
  if (name == mObjectName) return true;

  std::string Name = (name == "") ? "No Name" : name;

  if (mpObjectParent &&
      mpObjectParent->isNameVector() &&
      mpObjectParent->getObject("[" + Name + "]"))
    return false;

  if (smpRenameHandler && mpObjectParent)
    {
      std::string oldCN = this->getCN();
      mObjectName = Name;
      std::string newCN = this->getCN();
      smpRenameHandler->handle(oldCN, newCN);

      //TODO performance considerations.
      //Right now after every rename the CNs are checked. In some cases
      //we may know that this is not necessary
    }
  else
  {mObjectName = Name;}

  if (mpObjectParent)
    {
      mpObjectParent->CCopasiContainer::remove(this);
      mpObjectParent->CCopasiContainer::add(this, false);
    }

  return true;
}

/*virtual*/
std::string CCopasiObject::getObjectDisplayName(bool regular /*=true*/, bool richtext /*=false*/) const
  {
    std::string ret = "";

    if (mpObjectParent)
      {
        ret = mpObjectParent->getObjectDisplayName(regular, richtext);
        if (ret == "(CN)Root") ret = "";
        if (ret.substr(0, 7) == "(Model)") ret = "M";
      }

    if (ret.length() >= 2)
      if ((ret.substr(ret.length() - 2) == "[]") && (!isReference()))
        {
          ret.insert(ret.length() - 1, getObjectName());
          if (isNameVector() || isVector() || getObjectType() == "ParameterGroup")
            ret += "[]";
          return ret;
        }

    if (ret.length() != 0)
      ret += ".";

    if (isNameVector() || isVector() || getObjectType() == "ParameterGroup")
      ret += getObjectName() + "[]";
    else if (isReference()
             || getObjectType() == "Parameter"
             || getObjectType() == getObjectName())
      ret += getObjectName();
    else
      ret += "(" + getObjectType() + ")" + getObjectName();

    return ret;
  }

const std::string & CCopasiObject::getObjectName() const {return mObjectName;}

const std::string & CCopasiObject::getObjectType() const {return mObjectType;}

bool CCopasiObject::setObjectParent(const CCopasiContainer * pParent)
{
  mpObjectParent = const_cast<CCopasiContainer *>(pParent);

  return true;
}

CCopasiContainer * CCopasiObject::getObjectParent() const {return mpObjectParent;}

CCopasiContainer *
CCopasiObject::getObjectAncestor(const std::string & type) const
  {
    CCopasiContainer * p = getObjectParent();

    while (p)
      {
        if (p->getObjectType() == type) return p;
        p = p->getObjectParent();
      }

    return NULL;
  }

const std::set< CCopasiObject * > & CCopasiObject::getDirectDependencies() const
{return mDependencies;}

void CCopasiObject::getAllDependencies(std::set< CCopasiObject * > & dependencies) const
  {
    std::set< CCopasiObject * >::const_iterator it = mDependencies.begin();
    std::set< CCopasiObject * >::const_iterator end = mDependencies.end();

    std::pair<std::set< CCopasiObject * >::iterator, bool> Inserted;

    for (it; it != end; ++it)
      {
        // Dual purpose insert
        Inserted = dependencies.insert(*it);

        // The direct dependency *it was among the dependencies
        // we assume also its dependencies have been added already.
        if (!Inserted.second) continue;

        // Add all the dependencies of the direct dependency *it.
        (*it)->getAllDependencies(dependencies);
      }
  }

bool CCopasiObject::hasCircularDependencies(std::set<const CCopasiObject * > & candidates) const
  {
    std::set< CCopasiObject * >::const_iterator it = mDependencies.begin();
    std::set< CCopasiObject * >::const_iterator end = mDependencies.end();

    std::pair<std::set< const CCopasiObject * >::iterator, bool> Inserted;

    // Dual purpose insert
    Inserted = candidates.insert(this);

    // Check whether the insert was successfull, if not
    // the object this was among the candidates. Thus we have a dedected
    // a circular dependency
    if (!Inserted.second) return true;

    for (it; it != end; ++it)
      // Check whether the dependency *it depends on the candidates or this
      if ((*it)->hasCircularDependencies(candidates)) return true;

    // Remove the inserted object this from the candidates.
    candidates.erase(Inserted.first);

    return false;
  }

bool CCopasiObject::operator < (const CCopasiObject * rhs) const
  {
    if (this != rhs)
      {
        std::set< const CCopasiObject * > Candidates;
        Candidates.insert(this);

        return rhs->hasCircularDependencies(Candidates);
      }

    return false;
  }

void * CCopasiObject::getReference() const
  {return const_cast<CCopasiObject *>(this);}

bool CCopasiObject::isContainer() const
  {return (0 < (mObjectFlag & Container));}

bool CCopasiObject::isVector() const
  {return (0 < (mObjectFlag & Vector));}

bool CCopasiObject::isMatrix() const
  {return (0 < (mObjectFlag & Matrix));}

bool CCopasiObject::isNameVector() const
  {return (0 < (mObjectFlag & NameVector));}

bool CCopasiObject::isReference() const
  {return (0 < (mObjectFlag & Reference));}

bool CCopasiObject::isValueBool() const
  {return (0 < (mObjectFlag & ValueBool));}

bool CCopasiObject::isValueInt() const
  {return (0 < (mObjectFlag & ValueInt));}

bool CCopasiObject::isValueDbl() const
  {return (0 < (mObjectFlag & ValueDbl));}

bool CCopasiObject::isNonUniqueName() const
  {return (0 < (mObjectFlag & NonUniqueName));}

bool CCopasiObject::isStaticString() const
  {return (0 < (mObjectFlag & StaticString));}

bool CCopasiObject::isValueString() const
  {return (0 < (mObjectFlag & ValueString));}

bool CCopasiObject::isSeparator() const
  {return (0 < (mObjectFlag & Separator));}

const std::string & CCopasiObject::getKey() const
  {
    static std::string DefaultKey("");
    //std::cout << "*********** CCopasiObject::getKey() should never be called! *********" << std::endl;
    return DefaultKey;
  }

void CCopasiObject::setObjectValue(const C_FLOAT64 & value)
{(*mpUpdateMethod)(value);}

void CCopasiObject::setObjectValue(const C_INT32 & value)
{(*mpUpdateMethod)(value);}

void CCopasiObject::setObjectValue(const bool & value)
{(*mpUpdateMethod)(value);}

UpdateMethod * CCopasiObject::getUpdateMethod() const
  {return mpUpdateMethod;}

Refresh * CCopasiObject::getRefresh() const
  {return mpRefresh;}

std::ostream &operator<<(std::ostream &os, const CCopasiObject & o)
{
  os << "Name:      " << o.getObjectName() << std::endl;
  os << "Type:      " << o.getObjectType() << std::endl;
  os << "Container: " << o.isContainer() << std::endl;
  os << "Vector:    " << o.isVector() << std::endl;
  os << "VectorN:   " << o.isNameVector() << std::endl;
  os << "Matrix:    " << o.isMatrix() << std::endl;
  os << "Reference: " << o.isReference() << std::endl;
  os << "Bool:      " << o.isValueBool() << std::endl;
  os << "Int:       " << o.isValueInt() << std::endl;
  os << "Dbl:       " << o.isValueDbl() << std::endl;

  return os;
}

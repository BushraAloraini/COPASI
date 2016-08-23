// Copyright (C) 2010 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 - 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2003 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/**
 *  CCopasiParameterGroup class.
 *  This class is used to describe parameters. This class is intended
 *  to be used with integration or optimization methods or reactions.
 *
 *  Created for COPASI by Stefan Hoops 2002
 */

#include <sstream>

#include "copasi.h"

#include "CCopasiParameterGroup.h"
#include "CCopasiMessage.h"

#include "utilities/utility.h"

#define  END_NAME (std::map< std::string, std::set< CCopasiObject * > >::iterator(NULL))
#define  END_OBJECT (std::set< CCopasiObject * >::iterator(NULL))
#define  END_PARAMETER (std::vector< CCopasiParameter * >::iterator(NULL))

CCopasiParameterGroup::name_iterator::name_iterator():
  mpGroup(NULL),
  mName(END_NAME),
  mObject(END_OBJECT),
  mParameter(END_PARAMETER)
{}

CCopasiParameterGroup::name_iterator::name_iterator(const CCopasiParameterGroup & group,
    const bool & begin):
  mpGroup(&group),
  mName(END_NAME),
  mObject(END_OBJECT),
  mParameter(END_PARAMETER)
{
  if (mpGroup != NULL &&
      mpGroup->mObjects.begin() != mpGroup->mObjects.end())
    {
      if (begin)
        {
          mName = reinterpret_cast< objectMap::data * >(const_cast< objectMap * >(&mpGroup->mObjects))->begin();

          if (!mName->second.empty())
            {
              mObject = mName->second.begin();

              if (mName->second.size() > 1)
                {
                  mParameter = mpGroup->beginIndex();

                  while (mParameter != mpGroup->endIndex() &&
                         (*mParameter)->getObjectName() != mName->first)
                    ++mParameter;

                  if (mParameter == mpGroup->endIndex())
                    mParameter = END_PARAMETER;
                }
            }
        }
    }
}

CCopasiParameterGroup::name_iterator::name_iterator(const CCopasiParameterGroup::name_iterator & src):
  mpGroup(src.mpGroup),
  mName(src.mName),
  mObject(src.mObject),
  mParameter(src.mParameter)
{}

CCopasiParameterGroup::name_iterator::~name_iterator()
{}

CCopasiObject * CCopasiParameterGroup::name_iterator::operator*() const
{
  if (mParameter != END_PARAMETER)
    return *mParameter;

  return *mObject;
}

CCopasiObject * CCopasiParameterGroup::name_iterator::operator->() const
{
  if (mParameter != END_PARAMETER)
    return *mParameter;

  return *mObject;
}

CCopasiParameterGroup::name_iterator & CCopasiParameterGroup::name_iterator::operator++()
{
  // We first advance through all parameters
  if (mParameter != END_PARAMETER)
    {
      ++mParameter;

      // We skip parameters which name is not matching
      while (mParameter != mpGroup->endIndex() &&
             (*mParameter)->getObjectName() != mName->first)
        ++mParameter;

      if (mParameter != mpGroup->endIndex())
        return *this;

      mParameter = END_PARAMETER;
    }
  else if (mObject != END_OBJECT)
    {
      ++mObject;

      if (mObject == mName->second.end())
        mObject = END_OBJECT;
    }

  if (mObject != END_OBJECT)
    {
      // We need to skip the parameters which have been handled above;
      while (mObject != mName->second.end() &&
             dynamic_cast< CCopasiParameter * >(*mObject) != NULL)
        ++mObject;

      if (mObject != mName->second.end())
        return *this;

      mObject = END_OBJECT;
    }

  if (mName != END_NAME)
    {
      ++mName;

      if (mName != reinterpret_cast< objectMap::data * >(const_cast< objectMap * >(&mpGroup->mObjects))->end())
        {
          mObject = mName->second.begin();

          if (mName->second.size() > 1)
            {
              mParameter = mpGroup->beginIndex();

              while (mParameter != mpGroup->endIndex() &&
                     (*mParameter)->getObjectName() == mName->first)
                ++mParameter;

              if (mParameter == mpGroup->endIndex())
                mParameter = END_PARAMETER;
            }
        }
      else
        {
          mName = END_NAME;
        }
    }

  return *this;
}

CCopasiParameterGroup::name_iterator CCopasiParameterGroup::name_iterator::operator++(int)
{
  name_iterator Current(*this);
  operator++();

  return Current;
}

bool CCopasiParameterGroup::name_iterator::operator != (const CCopasiParameterGroup::name_iterator & rhs) const
{
  return (mpGroup != rhs.mpGroup ||
          mName != rhs.mName ||
          mObject != rhs.mObject ||
          mParameter != rhs.mParameter);
}

CCopasiParameterGroup::CCopasiParameterGroup():
  CCopasiParameter("NoName", GROUP),
  mpElementTemplates(NULL)
{}

CCopasiParameterGroup::CCopasiParameterGroup(const CCopasiParameterGroup & src,
    const CCopasiContainer * pParent):
  CCopasiParameter(src, pParent),
  mpElementTemplates(src.mpElementTemplates != NULL ? new CCopasiParameterGroup(*src.mpElementTemplates, this) : NULL)
{
  operator=(src);
}

CCopasiParameterGroup::CCopasiParameterGroup(const std::string & name,
    const CCopasiContainer * pParent,
    const std::string & objectType):
  CCopasiParameter(name, CCopasiParameter::GROUP, NULL, pParent, objectType),
  mpElementTemplates(NULL)
{}

CCopasiParameterGroup::~CCopasiParameterGroup()
{
  clear();
}

// virtual
const CObjectInterface * CCopasiParameterGroup::getObject(const CCopasiObjectName & cn) const
{
  const CObjectInterface * pObjectInterface = CCopasiContainer::getObject(cn);

  if (pObjectInterface != NULL)
    {
      return pObjectInterface;
    }

  std::string UniqueName = cn.getObjectName();

  std::string::size_type pos = UniqueName.find_last_of('[');
  std::string Name = UniqueName.substr(0, pos);
  size_t Index = strToUnsignedInt(UniqueName.substr(pos + 1).c_str());
  size_t counter = C_INVALID_INDEX;

  index_iterator it = beginIndex();
  index_iterator end = endIndex();

  for (; it != end; ++it)
    {
      if ((*it)->getObjectName() == Name)
        {
          counter++;

          if (counter == Index)
            {
              return (*it)->getObject(cn.getRemainder());
            }
        }
    }

  return NULL;
}

bool CCopasiParameterGroup::elevateChildren() {return true;}

CCopasiParameterGroup & CCopasiParameterGroup::operator = (const CCopasiParameterGroup & rhs)
{
  if (getObjectName() != rhs.getObjectName())
    setObjectName(rhs.getObjectName());

  name_iterator itRHS(rhs, true);
  name_iterator endRHS(rhs, false);

  name_iterator itLHS(*this, true);
  name_iterator endLHS(*this, false);

  std::vector< CCopasiParameter * > ToBeRemoved;
  std::vector< CCopasiParameter * > ToBeAdded;

  CCopasiParameter * pLHS;
  CCopasiParameter * pRHS;

  while (itRHS != endRHS && itLHS != endLHS)
    {
      // We only assign parameters
      if ((pRHS = dynamic_cast< CCopasiParameter * >(*itRHS)) == NULL)
        {
          ++itRHS;
          continue;
        }

      // We only assign parameters
      if ((pLHS = dynamic_cast< CCopasiParameter * >(*itLHS)) == NULL)
        {
          ++itLHS;
          continue;
        }

      const std::string & NameLHS = pLHS->getObjectName();
      const std::string & NameRHS = pRHS->getObjectName();

      // The LHS parameter is missing on the RHS thus we need to remove it
      if (NameLHS < NameRHS)
        {
          ToBeRemoved.push_back(pLHS);
          ++itLHS;
          continue;
        }

      // The RHS parameter is missing on the LHS thus we need to add it
      if (NameLHS > NameRHS)
        {
          ToBeAdded.push_back(pRHS);
          ++itRHS;
          continue;
        }

      // The names are equal it suffices to use the assignment operator of the parameter
      *pLHS = *pRHS;
      ++itLHS;
      ++itRHS;
    }

  // All remaining parameters of the LHS need to be removed
  while (itLHS != endLHS)
    {
      // We only assign parameters
      if ((pLHS = dynamic_cast< CCopasiParameter * >(*itLHS)) != NULL)
        ToBeRemoved.push_back(pLHS);

      ++itLHS;
    }

  // All remaining parameter of the RHS need to be added
  while (itRHS != endRHS)
    {
      // We only assign parameters
      if ((pRHS = dynamic_cast< CCopasiParameter * >(*itRHS)) != NULL)
        ToBeAdded.push_back(pRHS);

      ++itRHS;
    }

  // We remove the parameters
  std::vector< CCopasiParameter * >::const_iterator itToBeRemoved = ToBeRemoved.begin();
  std::vector< CCopasiParameter * >::const_iterator endToBeRemoved = ToBeRemoved.end();

  for (; itToBeRemoved != endToBeRemoved; ++itToBeRemoved)
    this->removeParameter(*itToBeRemoved);

  // We add the missing parameters
  CCopasiParameter * pParameter;
  std::vector< CCopasiParameter * >::const_iterator itToBeAdded = ToBeAdded.begin();
  std::vector< CCopasiParameter * >::const_iterator endToBeAdded = ToBeAdded.end();

  for (; itToBeAdded != endToBeAdded; ++itToBeAdded)
    {
      if ((*itToBeAdded)->getType() == GROUP)
        pParameter = new CCopasiParameterGroup(* static_cast< CCopasiParameterGroup * >(*itToBeAdded), NO_PARENT);
      else
        pParameter = new CCopasiParameter(**itToBeAdded, NO_PARENT);

      addParameter(pParameter);
    }

  return *this;
}

void CCopasiParameterGroup::print(std::ostream * ostream) const
{*ostream << *this;}

std::ostream &operator<<(std::ostream &os, const CCopasiParameterGroup & o)
{
  os << "<<< Parameter Group: " << o.getObjectName() << std::endl;

  CCopasiParameterGroup::elements::const_iterator it = o.beginIndex();
  CCopasiParameterGroup::elements::const_iterator end = o.endIndex();

  for (; it != end; ++it)
    {
      (*it)->print(&os);
      os << std::endl;
    }

  os << ">>> Parameter Group: " << o.getObjectName() << std::endl;
  return os;
}

bool operator==(const CCopasiParameterGroup & lhs,
                const CCopasiParameterGroup & rhs)
{
  if (lhs.getObjectName() != rhs.getObjectName()) return false;

  if (lhs.size() != rhs.size()) return false;

  CCopasiParameterGroup::elements::const_iterator itLhs = lhs.beginIndex();
  CCopasiParameterGroup::elements::const_iterator endLhs = lhs.endIndex();
  CCopasiParameterGroup::elements::const_iterator itRhs = rhs.beginIndex();

  for (; itLhs != endLhs; ++itLhs, ++itRhs)
    if (!(**itLhs == **itRhs)) return false;

  return true;
}

bool CCopasiParameterGroup::addParameter(const CCopasiParameter & parameter)
{
  if (parameter.getType() == CCopasiParameter::GROUP)
    {
      CCopasiParameterGroup * pGroup =
        new CCopasiParameterGroup(*dynamic_cast<const CCopasiParameterGroup *>(&parameter), NO_PARENT);
      addParameter(pGroup);
    }
  else
    {
      CCopasiParameter * pParameter = new CCopasiParameter(parameter, NO_PARENT);
      addParameter(pParameter);
    }

  return true;
}

void CCopasiParameterGroup::addParameter(CCopasiParameter * pParameter)
{
  if (pParameter == NULL) return;

  CCopasiContainer::add(pParameter, true);
  static_cast< elements * >(mpValue)->push_back(pParameter);
}

CCopasiParameterGroup & CCopasiParameterGroup::getElementTemplates()
{
  if (mpElementTemplates == NULL)
    {
      mpElementTemplates = new CCopasiParameterGroup("Element Templates", this);
    }

  return *mpElementTemplates;
}

const CCopasiParameterGroup & CCopasiParameterGroup::getElementTemplates() const
{
  if (mpElementTemplates == NULL)
    {
      mpElementTemplates = new CCopasiParameterGroup("Element Templates", this);
    }

  return *mpElementTemplates;
}

CCopasiParameterGroup::index_iterator CCopasiParameterGroup::beginIndex() const
{return static_cast< elements * >(mpValue)->begin();}

CCopasiParameterGroup::index_iterator CCopasiParameterGroup::endIndex() const
{return static_cast< elements * >(mpValue)->end();}

bool CCopasiParameterGroup::addParameter(const std::string & name,
    const CCopasiParameter::Type type)
{
  CCopasiParameter * pParameter;

  if (type == GROUP)
    pParameter = new CCopasiParameterGroup(name);
  else
    pParameter = new CCopasiParameter(name, type);

  addParameter(pParameter);

  return true;
}

bool CCopasiParameterGroup::addGroup(const std::string & name)
{
  addParameter(new CCopasiParameterGroup(name));
  return true;
}

CCopasiParameterGroup * CCopasiParameterGroup::assertGroup(const std::string & name)
{
  CCopasiParameterGroup * pGrp = getGroup(name);

  if (pGrp) return pGrp;

  removeParameter(name);

  addGroup(name);
  return getGroup(name);
}

bool CCopasiParameterGroup::removeParameter(const std::string & name)
{
  size_t index = getIndex(name);

  if (index != C_INVALID_INDEX)
    {
      index_iterator it = static_cast< elements * >(mpValue)->begin() + index;

      pdelete(*it);
      static_cast< elements * >(mpValue)->erase(it, it + 1);

      return true;
    }

  return false;
}

bool CCopasiParameterGroup::removeParameter(const size_t & index)
{
  if (index < size())
    {
      index_iterator it = static_cast< elements * >(mpValue)->begin() + index;

      pdelete(*it);
      static_cast< elements * >(mpValue)->erase(it, it + 1);

      return true;
    }

  return false;
}

bool CCopasiParameterGroup::removeParameter(CCopasiParameter * pParameter)
{
  index_iterator it = static_cast< elements * >(mpValue)->begin();
  index_iterator end = static_cast< elements * >(mpValue)->begin();

  for (; it != end; ++it)
    if (*it == pParameter)
      {
        pdelete(*it);
        static_cast< elements * >(mpValue)->erase(it, it + 1);

        return true;
      }

  return false;
}

CCopasiParameter * CCopasiParameterGroup::getParameter(const std::string & name)
{
  objectMap::range range = getObjects().equal_range(name);

  if (range.first == range.second) return NULL;

  return dynamic_cast<CCopasiParameter *>(const_cast< CCopasiObject * >(*range.first));
}

const CCopasiParameter * CCopasiParameterGroup::getParameter(const std::string & name) const
{
  objectMap::range range = getObjects().equal_range(name);

  if (range.first == range.second) return NULL;

  return dynamic_cast<CCopasiParameter *>(*range.first);
}

CCopasiParameter * CCopasiParameterGroup::getParameter(const size_t & index)
{
  if (index < size())
    return *(static_cast< elements * >(mpValue)->begin() + index);

  return NULL;
}

const CCopasiParameter * CCopasiParameterGroup::getParameter(const size_t & index) const
{
  if (index < size())
    return *(static_cast< elements * >(mpValue)->begin() + index);

  return NULL;
}

CCopasiParameterGroup * CCopasiParameterGroup::getGroup(const std::string & name)
{return dynamic_cast<CCopasiParameterGroup *>(getParameter(name));}

const CCopasiParameterGroup * CCopasiParameterGroup::getGroup(const std::string & name) const
{return dynamic_cast<const CCopasiParameterGroup *>(getParameter(name));}

CCopasiParameterGroup * CCopasiParameterGroup::getGroup(const size_t & index)
{return dynamic_cast<CCopasiParameterGroup *>(getParameter(index));}

const CCopasiParameterGroup * CCopasiParameterGroup::getGroup(const size_t & index) const
{return dynamic_cast<const CCopasiParameterGroup *>(getParameter(index));}

CCopasiParameter::Type CCopasiParameterGroup::getType(const std::string & name) const
{
  CCopasiParameter * pParameter =
    const_cast< CCopasiParameterGroup * >(this)->getParameter(name);

  if (pParameter) return pParameter->getType();

  return CCopasiParameter::INVALID;
}

CCopasiParameter::Type CCopasiParameterGroup::getType(const size_t & index) const
{
  CCopasiParameter * pParameter =
    const_cast< CCopasiParameterGroup * >(this)->getParameter(index);

  if (pParameter) return pParameter->getType();

  return CCopasiParameter::INVALID;
}

std::string CCopasiParameterGroup::getKey(const std::string & name) const
{
  CCopasiParameter * pParameter =
    const_cast< CCopasiParameterGroup * >(this)->getParameter(name);

  if (pParameter) return pParameter->getKey();

  return "Not Found";
}

std::string CCopasiParameterGroup::getKey(const size_t & index) const
{
  CCopasiParameter * pParameter =
    const_cast< CCopasiParameterGroup * >(this)->getParameter(index);

  if (pParameter) return pParameter->getKey();

  return "Not Found";
}

const std::string & CCopasiParameterGroup::getName(const size_t & index) const
{
  static std::string Invalid("Invalid Index");

  CCopasiParameter * pParameter =
    const_cast< CCopasiParameterGroup * >(this)->getParameter(index);

  if (pParameter) return pParameter->getObjectName();

  return Invalid;
}

bool CCopasiParameterGroup::swap(const size_t & iFrom,
                                 const size_t & iTo)
{
  index_iterator from = beginIndex() + iFrom;
  index_iterator to = beginIndex() + iTo;

  return swap(from, to);
}

bool CCopasiParameterGroup::swap(index_iterator & from,
                                 index_iterator & to)
{
  if (from < beginIndex() || endIndex() <= from ||
      to < beginIndex() || endIndex() <= to)
    return false;

  CCopasiParameter *tmp = *from;
  *from = *to;
  *to = tmp;

  return true;
}

size_t CCopasiParameterGroup::size() const
{return static_cast< elements * >(mpValue)->size();}

void CCopasiParameterGroup::clear()
{
  if (mpValue != NULL)
    {
      index_iterator it = static_cast< elements * >(mpValue)->begin();
      index_iterator end = static_cast< elements * >(mpValue)->end();

      for (; it != end; ++it) pdelete(*it);

      static_cast< elements * >(mpValue)->clear();
    }
}

size_t CCopasiParameterGroup::getIndex(const std::string & name) const
{
  index_iterator it = static_cast< elements * >(mpValue)->begin();
  index_iterator end = static_cast< elements * >(mpValue)->end();

  for (size_t i = 0; it != end; ++it, ++i)
    if (name == (*it)->getObjectName()) return i;;

  return C_INVALID_INDEX;
}

std::string CCopasiParameterGroup::getUniqueParameterName(const CCopasiParameter * pParameter) const
{
  size_t counter = C_INVALID_INDEX;
  size_t Index = C_INVALID_INDEX;

  std::string Name = pParameter->getObjectName();

  index_iterator it = static_cast< elements * >(mpValue)->begin();
  index_iterator end = static_cast< elements * >(mpValue)->end();

  for (; it != end; ++it)
    {
      if ((*it)->getObjectName() == Name)
        {
          counter++;

          if (*it == pParameter)
            {
              Index = counter;
            }
        }
    }

  if (counter == 0 || Index == C_INVALID_INDEX)
    {
      return Name;
    }

  std::stringstream UniqueName;
  UniqueName << Name << "[" << Index << "]";

  return UniqueName.str();
}

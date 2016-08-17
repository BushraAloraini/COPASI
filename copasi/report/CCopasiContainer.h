// Copyright (C) 2010 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 - 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2002 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/*!
    \file CCopasiContainer.h
    \brief Header file of class CCopasiContainer
 */

/**
 * Class CCopasiContainer
 *
 * This class is the is used to group CCopasiObjects logically. It inself is
 * an object. Contained objects are still globally accessible.
 *
 * Copyright Stefan Hoops 2002
 */

#ifndef COPASI_CCopasiContainer
#define COPASI_CCopasiContainer

#include <map>
#include <vector>

#include "copasi/report/CCopasiObject.h"

class CUnit;

class CCopasiContainer: public CCopasiObject
{
  //Attributes
public:

  static const CObjectInterface::ContainerList EmptyList;

  typedef std::multimap<std::string, CCopasiObject * > objectMap;
  typedef std::pair<objectMap::const_iterator, objectMap::const_iterator> range;

protected:
  objectMap mObjects;

private:
  // Operations
  CCopasiContainer();

protected:
  CCopasiContainer(const CCopasiContainer & src);

public:
  CCopasiContainer(const std::string & name,
                   const CCopasiContainer * pParent = NO_PARENT,
                   const std::string & type = "CN",
                   const unsigned C_INT32 & flag = CCopasiObject::Container);

  CCopasiContainer(const CCopasiContainer & src,
                   const CCopasiContainer * pParent);

  virtual ~CCopasiContainer();

  virtual const CObjectInterface * getObject(const CCopasiObjectName & cn) const;

  virtual const CCopasiObject * getValueObject() const;

  virtual const objectMap & getObjects() const;

  virtual bool add(CCopasiObject * pObject, const bool & adopt = true);

  virtual bool remove(CCopasiObject * pObject);

  /**
   * Retrieve the units of the object.
   * @return std::string units
   */
  virtual const std::string getUnits() const;

  /**
   * Retrieve the units of the child object.
   * @return std::string units
   */
  virtual std::string getChildObjectUnits(const CCopasiObject * pObject) const;

protected:
  void initObjects();

  template <class CType> CCopasiObject * addObjectReference(const std::string & name,
      CType & reference,
      const unsigned C_INT32 & flag = 0)
  {return createReference(name, this, reference, flag);}

  template <class CType> CCopasiObject * addVectorReference(const std::string & name,
      CType & reference,
      const unsigned C_INT32 & flag = 0)
  {return createVectorReference(name, this, reference, flag);}

  template <class CType> CCopasiObject * addMatrixReference(const std::string & name,
      CType & reference,
      const unsigned C_INT32 & flag = 0)
  {return createMatrixReference(name, this, reference, flag);}
};

#endif // COPASI_CCopasiContainer

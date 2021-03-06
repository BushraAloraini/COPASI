// Copyright (C) 2017 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and University of
// of Connecticut School of Medicine.
// All rights reserved.

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

/**
 * CFunctionParameter
 *
 * Created for COPASI by Stefan Hoops
 * (C) Stefan Hoops 2001
 */

#include "copasi/copasi.h"

#include "CFunctionParameter.h"

#include "copasi/report/CKeyFactory.h"
#include "copasi/core/CRootContainer.h"
#include "copasi/undo/CData.h"

//static
const std::string CFunctionParameter::DataTypeName[] =
{"Integer", "Double", "Vector of Integer", "Vector of Double", ""};

//static
const std::string CFunctionParameter::RoleNameXML[] =
{"substrate", "product", "modifier", "constant", "volume", "time", "variable", ""};

//static
const std::string CFunctionParameter::RoleNameDisplay[] =
{"Substrate", "Product", "Modifier", "Parameter", "Volume", "Time", "Variable", ""};

//static
CFunctionParameter::Role CFunctionParameter::xmlRole2Enum(const std::string & xmlrole)
{
  C_INT32 i;

  for (i = 0; (RoleNameXML[i] != "") && (RoleNameXML[i] != xmlrole); ++i) {};

  if (RoleNameXML[i] == "")
    return VARIABLE; //default for invalid XML string
  else
    return (Role)i;
}

// static
CFunctionParameter * CFunctionParameter::fromData(const CData & data)
{
  return new CFunctionParameter(data.getProperty(CData::OBJECT_NAME).toString(),
                                NO_PARENT);
}

// virtual
CData CFunctionParameter::toData() const
{
  CData Data;

  // TODO CRITICAL Implement me!
  fatalError();

  return Data;
}

// virtual
bool CFunctionParameter::applyData(const CData & data)
{
  bool success = true;

  // TODO CRITICAL Implement me!
  fatalError();

  return success;
}

CFunctionParameter::CFunctionParameter(const std::string & name,
                                       const CDataContainer * pParent):
  CDataContainer(name, pParent, "Variable"),
  mKey(CRootContainer::getKeyFactory()->add("FunctionParameter", this)),
  mType((CFunctionParameter::DataType) - 1),
  mUsage(VARIABLE),
  mIsUsed(true)
{CONSTRUCTOR_TRACE;}

CFunctionParameter::CFunctionParameter(const CFunctionParameter & src,
                                       const CDataContainer * pParent):
  CDataContainer(src, pParent),
  mKey(CRootContainer::getKeyFactory()->add("FunctionParameter", this)),
  mType(src.mType),
  mUsage(src.mUsage),
  mIsUsed(src.mIsUsed)
{CONSTRUCTOR_TRACE;}

CFunctionParameter::CFunctionParameter(const std::string &name,
                                       const enum CFunctionParameter::DataType &type,
                                       Role usage,
                                       const CDataContainer * pParent) :
  CDataContainer(name, pParent, "Variable"),
  mKey(CRootContainer::getKeyFactory()->add("FunctionParameter", this)),
  mType(type),
  mUsage(usage),
  mIsUsed(true)
{CONSTRUCTOR_TRACE;}

CFunctionParameter::~CFunctionParameter()
{
  CRootContainer::getKeyFactory()->remove(mKey);
  DESTRUCTOR_TRACE;
}

void CFunctionParameter::cleanup() {}

void CFunctionParameter::load(CReadConfig & configbuffer,
                              CReadConfig::Mode mode)
{
  std::string tmp;
  configbuffer.getVariable("FunctionParameter", "string", &tmp, mode);
  setObjectName(tmp);
  configbuffer.getVariable("DataType", "C_INT32", &mType);
  configbuffer.getVariable("Usage", "string", &mUsage);
}

const std::string & CFunctionParameter::getKey() const {return mKey;}

void CFunctionParameter::setUsage(Role usage) {mUsage = usage;}

CFunctionParameter::Role CFunctionParameter::getUsage() const {return mUsage;}

void CFunctionParameter::setType(const CFunctionParameter::DataType & type)
{mType = type;}

const CFunctionParameter::DataType &

CFunctionParameter::getType() const
{
  return mType;
}

void CFunctionParameter::setIsUsed(const bool & isUsed)
{mIsUsed = isUsed;}

/**
 * Retrieve whether the parameter is used within a function
 * @return const bool & isUsed
 */
const bool & CFunctionParameter::isUsed() const
{return mIsUsed;}

std::ostream& operator<<(std::ostream &os, const CFunctionParameter & d)
{
  //os << "CFunctionParameter: "
  os << d.getObjectName();

  if (d.mType != 1) os << " mType " << d.mType;

  os << " [" << CFunctionParameter::RoleNameDisplay[d.mUsage] << "]";

  return os;
}

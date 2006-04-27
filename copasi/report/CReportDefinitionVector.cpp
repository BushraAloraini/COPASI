/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/report/CReportDefinitionVector.cpp,v $
   $Revision: 1.19 $
   $Name:  $
   $Author: shoops $
   $Date: 2006/04/27 01:31:10 $
   End CVS Header */

// Copyright � 2005 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

// ReportDefinitionVector.cpp: implementation of the CReportDefinitionVector class.
//
//////////////////////////////////////////////////////////////////////

#include "copasi.h"
#include "CReportDefinitionVector.h"
#include "CKeyFactory.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CReportDefinitionVector::CReportDefinitionVector(const std::string & name,
    const CCopasiContainer * pParent):
    CCopasiVectorN< CReportDefinition >(name, pParent),
    mKey(GlobalKeys.add("CReportDefinitionVector", this))
{}

CReportDefinitionVector::~CReportDefinitionVector()
{
  cleanup();
}

void CReportDefinitionVector::cleanup()
{
  GlobalKeys.remove(mKey);
}

const std::string& CReportDefinitionVector::getKey()
{
  return mKey;
}

CReportDefinition* CReportDefinitionVector::createReportDefinition(const std::string & name, const std::string & comment)
{
  unsigned C_INT32 i;
  for (i = 0; i < size(); i++)
    if ((*this)[i]->getObjectName() == name)
      return NULL; // duplicate name

  CReportDefinition* pNewReportDef = new CReportDefinition(name, this);
  pNewReportDef->setComment(comment);
  pNewReportDef->setObjectName(name);

  add(pNewReportDef);
  return pNewReportDef;
}

bool CReportDefinitionVector::removeReportDefinition(const std::string & key)
{
  CReportDefinition* pRep =
    dynamic_cast<CReportDefinition *>(GlobalKeys.get(key));
  unsigned C_INT32 index = this->CCopasiVector<CReportDefinition>::getIndex(pRep);
  if (index == C_INVALID_INDEX)
    return false;

  this->CCopasiVector<CReportDefinition>::remove(index);

  //pdelete(pRep);  //TODO: propably a memory leak, may be not

  return true;
}

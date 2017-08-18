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

// Copyright (C) 2003 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/**
 *  CReportDefinition class.
 *  This class describes the Report Definition
 *
 *  Created for COPASI by Mudita Singhal
 */

#include "copasi.h"

#include "CKeyFactory.h"
#include "CReportDefinition.h"
#include "CReport.h"

#include "utilities/CCopasiMessage.h"
#include "copasi/report/CCopasiRootContainer.h"

//////////////////////////////////////////////////
//
//class CReportDefinition
//
//////////////////////////////////////////////////
CReportDefinition::CReportDefinition(const std::string & name,
                                     const CCopasiContainer * pParent):
  CCopasiObject(name, pParent, "ReportDefinition"),
  mKey(CCopasiRootContainer::getKeyFactory()->add("Report", this)),
  mComment(""),
  mTaskType(CTaskEnum::timeCourse),
  mSeparator("\t"),
  mTable(true),
  mbTitle(true),
  mPrecision(6)
{}

CReportDefinition::CReportDefinition(const CReportDefinition & src,
                                     const CCopasiContainer * pParent):
  CCopasiObject(src, pParent),
  mKey(CCopasiRootContainer::getKeyFactory()->add("Report", this)),
  mComment(src.mComment),
  mTaskType(src.mTaskType),
  mSeparator(src.mSeparator, NO_PARENT),
  mTable(src.mTable),
  mbTitle(src.mbTitle),
  mPrecision(src.mPrecision),
  mHeaderVector(src.mHeaderVector),
  mBodyVector(src.mBodyVector),
  mFooterVector(src.mFooterVector),
  mTableVector(src.mTableVector)
{}

CReportDefinition::~CReportDefinition()
{cleanup();}

void CReportDefinition::cleanup()
{
  CCopasiRootContainer::getKeyFactory()->remove(mKey);
  mHeaderVector.clear();
  mBodyVector.clear();
  mFooterVector.clear();
  mTableVector.clear();
}

bool CReportDefinition::preCompileTable(const CObjectInterface::ContainerList & listOfContainer)
{
  bool success = true;

  mHeaderVector.clear();
  mBodyVector.clear();
  mFooterVector.clear();

  std::vector<CRegisteredObjectName>::const_iterator it = mTableVector.begin();
  std::vector<CRegisteredObjectName>::const_iterator end = mTableVector.end();

  for (; it != end; ++it)
    {
      const CCopasiObject * pObject = CObjectInterface::DataObject(CObjectInterface::GetObjectFromCN(listOfContainer, *it));

      if (pObject != NULL)
        {
          addTableElement(pObject);
        }
      else
        {
          CCopasiMessage(CCopasiMessage::WARNING, MCCopasiTask + 6, it->c_str());
        }
    }

  return success;
}

std::vector<CRegisteredObjectName>* CReportDefinition::getBodyAddr()
{return &mBodyVector;}

const std::vector<CRegisteredObjectName>* CReportDefinition::getBodyAddr() const
{return &mBodyVector;}

std::vector<CRegisteredObjectName>* CReportDefinition::getHeaderAddr()
{return &mHeaderVector;}

const std::vector<CRegisteredObjectName>* CReportDefinition::getHeaderAddr() const
{return &mHeaderVector;}

std::vector<CRegisteredObjectName>* CReportDefinition::getFooterAddr()
{return &mFooterVector;}

const std::vector<CRegisteredObjectName>* CReportDefinition::getFooterAddr() const
{return &mFooterVector;}

std::vector<CRegisteredObjectName>* CReportDefinition::getTableAddr()
{return &mTableVector;}

const std::vector<CRegisteredObjectName>* CReportDefinition::getTableAddr() const
{return &mTableVector;}

bool CReportDefinition::setTaskType(const CTaskEnum::Task & taskType)
{mTaskType = taskType; return true;}

const CTaskEnum::Task & CReportDefinition::getTaskType() const
{return mTaskType;}

void CReportDefinition::setSeparator(const std::string & separator)
{mSeparator = separator;}

const CCopasiReportSeparator & CReportDefinition::getSeparator() const
{return mSeparator;}

bool CReportDefinition::getTitle() const
{return mbTitle;}

void CReportDefinition::setTitle(bool title)
{mbTitle = title;}

bool CReportDefinition::isTable() const
{return mTable;}

void CReportDefinition::setIsTable(bool table)
{mTable = table;}

void CReportDefinition::setPrecision(const unsigned C_INT32 & precision)
{mPrecision = precision;}

const unsigned C_INT32 & CReportDefinition::getPrecision() const
{return mPrecision;}

const std::string & CReportDefinition::getKey() const
{return mKey;}

void CReportDefinition::addTableElement(const CCopasiObject * pObject)
{
  bool isFirst = false;

  if ((mHeaderVector.size() == 0) && (mBodyVector.size() == 0))
    isFirst = true;

  CCopasiObjectName SeparatorCN(mSeparator.getCN());
  CCopasiObjectName Title;

  if (!pObject) return;

  // Add separator
  if (!isFirst)
    {
      if (mbTitle)
        mHeaderVector.push_back(SeparatorCN);

      mBodyVector.push_back(SeparatorCN);
    }

  // Determine column title
  if (pObject->getObjectParent())
    {
      if (pObject->getObjectType() == "Separator")
        {
          Title = "Separator=" + pObject->getCN().getObjectName();
        }
      else
        {
          Title = pObject->getCN() + ",Property=DisplayName";
        }
    }
  else
    Title =
      CCopasiStaticString(pObject->getObjectName()).getCN();

  if (mbTitle)
    mHeaderVector.push_back(Title);

  if (pObject->getObjectType() == "Separator")
    {
      mBodyVector.push_back("Separator=" + pObject->getCN().getObjectName());
    }
  else
    {
      mBodyVector.push_back(pObject->getCN());
    }

  return;
}

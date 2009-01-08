// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/Attic/TableDefinition.cpp,v $
//   $Revision: 1.60 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2009/01/08 16:07:44 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#include "TableDefinition.h"

#include <qlayout.h>
#include <qwidget.h>
#include <qfont.h>
#include <qpushbutton.h>
#include <qaction.h>

//#include "MyTable.h"
#include "listviews.h"
#include "CQMessageBox.h"
#include "qtUtilities.h"
#include "model/CModel.h"
#include "CopasiDataModel/CCopasiDataModel.h"
#include "report/CKeyFactory.h"
#include "report/CReportDefinitionVector.h"
#include "report/CCopasiStaticString.h"

#include "trajectory/CTrajectoryTask.h"
#include "steadystate/CSteadyStateTask.h"
#include "steadystate/CMCATask.h"
#include "scan/CScanTask.h"

std::vector<const CCopasiObject*> TableDefinition::getObjects() const
  {
    CCopasiVector< CReportDefinition >* tmp =
      CCopasiDataModel::Global->getReportDefinitionList();

    std::vector<const CCopasiObject*> ret;

    C_INT32 i, imax = tmp->size();
    for (i = 0; i < imax; ++i)
      ret.push_back((*tmp)[i]);

    return ret;
  }

void TableDefinition::init()
{
  mOT = ListViews::REPORT;
  numCols = 2;
  table->setNumCols(numCols);

  //Setting table headers
  Q3Header *tableHeader = table->horizontalHeader();
  tableHeader->setLabel(0, "Status");
  tableHeader->setLabel(1, "Name");
  //  tableHeader->setLabel(2, "Comment");
}

void TableDefinition::tableLineFromObject(const CCopasiObject* obj, unsigned C_INT32 row)
{
  if (!obj) return;
  const CReportDefinition* pRep = (const CReportDefinition*)obj;
  table->setText(row, 1, FROM_UTF8(pRep->getObjectName()));
  //  table->setText(row, 2, FROM_UTF8(pRep->getComment()));
}

void TableDefinition::tableLineToObject(unsigned C_INT32 /*row*/, CCopasiObject* obj)
{
  if (!obj) return;
  //CReportDefinition* pRep = (CReportDefinition*)obj;
  //  pRep->setComment(TO_UTF8(table->text(row, 2)));
}

void TableDefinition::defaultTableLineContent(unsigned C_INT32 /*row*/, unsigned C_INT32 /*exc*/)
{
  //  if (exc != 2)
  //    table->setText(row, 2, "");
}

QString TableDefinition::defaultObjectName() const
  {
    return "report";
  }

CCopasiObject* TableDefinition::createNewObject(const std::string & name)
{
  std::string nname = name;
  int i = 0;
  CReportDefinition* pRep;
  while (!(pRep = CCopasiDataModel::Global->getReportDefinitionList()->createReportDefinition(nname, "")))
    {
      i++;
      nname = name;
      nname += TO_UTF8(QString::number(i));
    }

  return pRep;
}

void TableDefinition::deleteObjects(const std::vector<std::string> & keys)
{
  if (!CCopasiDataModel::Global->getModel())
    return;

  if (keys.size() == 0)
    return;

  unsigned C_INT32 i, imax = keys.size();

  std::set< std::string > TaskKeys;
  std::set< std::string >::const_iterator it;
  std::set< std::string >::const_iterator end;

  for (i = 0; i < imax; i++)
    {
      //check where the reports are used...
      std::set< std::string > Keys =
        CCopasiDataModel::Global->listTaskDependentOnReport(keys[i]);

      for (it = Keys.begin(), end = Keys.end(); it != end; ++it)
        TaskKeys.insert(*it);
    }

  if (TaskKeys.size() > 0)
    {
      std::set< std::string >::const_iterator it = TaskKeys.begin();
      std::set< std::string >::const_iterator end = TaskKeys.end();

      CCopasiTask * pTask;
      QString msg = "The following tasks are effected:\n";

      for (it = TaskKeys.begin(), end = TaskKeys.end(); it != end; ++it)
        if ((pTask = dynamic_cast< CCopasiTask * >(GlobalKeys.get(*it))))
          msg += FROM_UTF8(pTask->getObjectName()) + ", ";

      msg = msg.remove(msg.length() - 2, 2);

      if (CQMessageBox::question(this,
                                 "CONFIRM DELETE",
                                 msg,
                                 QMessageBox::Ok,
                                 QMessageBox::Cancel | QMessageBox::Default | QMessageBox::Escape,
                                 QMessageBox::NoButton) == QMessageBox::Cancel)
        return;

      for (it = TaskKeys.begin(); it != end; ++it)
        if ((pTask = dynamic_cast< CCopasiTask * >(GlobalKeys.get(*it))))
          pTask->getReport().setReportDefinition(NULL);
    }

  //remove the reports
  for (i = 0; i < imax; i++)
    {
      CCopasiDataModel::Global->getReportDefinitionList()->removeReportDefinition(keys[i]);
      protectedNotify(ListViews::REPORT, ListViews::DELETE, keys[i]);
    }

  mChanged = true;
}

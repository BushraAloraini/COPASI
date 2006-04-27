/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/CopasiUI/Attic/MoietyWidget.cpp,v $
   $Revision: 1.71 $
   $Name:  $
   $Author: shoops $
   $Date: 2006/04/27 01:27:44 $
   End CVS Header */

// Copyright � 2005 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#include <qlayout.h>
#include <qwidget.h>
#include <qmessagebox.h>
#include <qfont.h>
#include <qpushbutton.h>
#include <qaction.h>
#include <qmessagebox.h>
#include <qapplication.h>

#include "MoietyWidget.h"
#include "listviews.h"
#include "DataModelGUI.h"
#include "qtUtilities.h"
#include "copasiui3window.h"

//#include "MyTable.h"
#include "model/CModel.h"
#include "model/CMoiety.h"
#include "CopasiDataModel/CCopasiDataModel.h"
#include "report/CKeyFactory.h"

std::vector<const CCopasiObject*> MoietyWidget::getObjects() const
  {
    const CCopasiVector<CMoiety>& tmp = CCopasiDataModel::Global->getModel()->getMoieties();
    std::vector<const CCopasiObject*> ret;

    C_INT32 i, imax = tmp.size();
    for (i = 0; i < imax; ++i)
      ret.push_back(tmp[i]);

    return ret;
  }

void MoietyWidget::init()
{
  mExtraLayout->addStretch();
  btnRun = new QPushButton("&Recalculate", this);
  mExtraLayout->addWidget(btnRun);
  connect(btnRun, SIGNAL(clicked ()), this,
          SLOT(slotBtnRunClicked()));

  numCols = 4;
  table->setNumCols(numCols);
  //table->QTable::setNumRows(1);

  //Setting table headers
  QHeader *tableHeader = table->horizontalHeader();
  tableHeader->setLabel(0, "Status");
  table->hideColumn(0);
  tableHeader->setLabel(1, "Name");
  tableHeader->setLabel(2, "Equation");
  tableHeader->setLabel(3, "Number");
}

void MoietyWidget::tableLineFromObject(const CCopasiObject* obj, unsigned C_INT32 row)
{
  if (!obj) return;
  const CMoiety* pMoi = (const CMoiety*)obj;
  table->setText(row, 1, FROM_UTF8(pMoi->getObjectName()));
  table->setText(row, 2, FROM_UTF8(pMoi->getDescription(CCopasiDataModel::Global->getModel())));
  table->setText(row, 3, QString::number(pMoi->getNumber()));
}

void MoietyWidget::tableLineToObject(unsigned C_INT32 C_UNUSED(row), CCopasiObject* C_UNUSED(obj))
{}

void MoietyWidget::defaultTableLineContent(unsigned C_INT32 C_UNUSED(row), unsigned C_INT32 C_UNUSED(exc))
{}

QString MoietyWidget::defaultObjectName() const
  {return "";}

CCopasiObject* MoietyWidget::createNewObject(const std::string & C_UNUSED(name))
{return NULL;}

void MoietyWidget::deleteObjects(const std::vector<std::string> & C_UNUSED(keys))
{}

void MoietyWidget::slotBtnRunClicked()
{
  static_cast<CopasiUI3Window *>(qApp->mainWidget())->autoSave();
  static_cast<CopasiUI3Window *>(qApp->mainWidget())->suspendAutoSave(true);

  CCopasiDataModel::Global->getModel()->compileIfNecessary();
  fillTable();

  mIgnoreUpdates = true; //to avoid recursive calls
  ListViews::notify(ListViews::MODEL, ListViews::CHANGE);
  mIgnoreUpdates = false; //to avoid recursive calls

  static_cast<CopasiUI3Window *>(qApp->mainWidget())->suspendAutoSave(false);
}

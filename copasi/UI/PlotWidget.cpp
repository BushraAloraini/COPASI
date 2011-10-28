// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/Attic/PlotWidget.cpp,v $
//   $Revision: 1.30 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2009/02/19 19:54:03 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#include "PlotWidget.h"

#include <qlayout.h>
#include <qwidget.h>
#include <qmessagebox.h>
#include <qfont.h>
#include <qpushbutton.h>
#include <qaction.h>

//#include "MyTable.h"
#include "model/CModel.h"
#include "listviews.h"
#include "report/CKeyFactory.h"
//#include "report/CReportDefinition.h"
#include "report/CCopasiStaticString.h"
#include "qtUtilities.h"
#include "plot/CPlotSpecification.h"
#include "plot/COutputDefinitionVector.h"
#include "CopasiDataModel/CCopasiDataModel.h"
#include "report/CCopasiRootContainer.h"
#include "DataModelGUI.h"

std::vector<const CCopasiObject*> PlotWidget::getObjects() const
  {
    assert(CCopasiRootContainer::getDatamodelList()->size() > 0);
    CCopasiVector<CPlotSpecification> & tmp =
      * (*CCopasiRootContainer::getDatamodelList())[0]->getPlotDefinitionList();

    std::vector<const CCopasiObject*> ret;

    C_INT32 i, imax = tmp.size();
    for (i = 0; i < imax; ++i)
      ret.push_back(tmp[i]);

    return ret;
  }

void PlotWidget::init()
{
  mOT = ListViews::PLOT;
  numCols = 4;
  table->setNumCols(numCols);

  //Setting table headers
  Q3Header *tableHeader = table->horizontalHeader();
  tableHeader->setLabel(0, "Status");
  tableHeader->setLabel(1, "Name");
  tableHeader->setLabel(2, "Curves");
  tableHeader->setLabel(3, "active");

  //this restricts users from editing the number of curves
  table->setColumnReadOnly (2, true);
}

void PlotWidget::tableLineFromObject(const CCopasiObject* obj, unsigned C_INT32 row)
{
  if (!obj) return;
  //const CPlotSpecification* pPl = (const CPlotSpecification*)obj;
  const CPlotSpecification* pPl = dynamic_cast<const CPlotSpecification*>(obj);

  // 1: name
  table->setText(row, 1, FROM_UTF8(pPl->getObjectName()));

  // 2: NCurves
  table->setText(row, 2, QString::number(pPl->getItems().size()));

  // 3: active?
  Q3CheckTableItem * activeCB;
  activeCB = new Q3CheckTableItem(table, "");
  activeCB->setChecked(pPl->isActive());
  table->setItem(row, 3, activeCB);
}

void PlotWidget::tableLineToObject(unsigned C_INT32 row, CCopasiObject* obj)
{
  if (!obj) return;
  CPlotSpecification* pPl = (CPlotSpecification*)obj;

  // 3: active?
  bool active = ((Q3CheckTableItem*)(table->item(row, 3)))->isChecked();
  pPl->setActive(active);
}

void PlotWidget::defaultTableLineContent(unsigned C_INT32 row, unsigned C_INT32 exc)
{
  if (exc != 2)
    table->clearCell(row, 2);

  // 3: active?
  if (exc != 3)
    {
      Q3CheckTableItem * activeCB;
      activeCB = new Q3CheckTableItem(table, "");
      activeCB->setChecked(true);
      table->setItem(row, 3, activeCB);
    }
}

QString PlotWidget::defaultObjectName() const
  {
    return "plot";
  }

CCopasiObject* PlotWidget::createNewObject(const std::string & name)
{
  std::string nname = name;
  int i = 0;
  CPlotSpecification* pPl;
  assert(CCopasiRootContainer::getDatamodelList()->size() > 0);
  CCopasiDataModel* pDataModel = (*CCopasiRootContainer::getDatamodelList())[0];
  assert(pDataModel != NULL);
  while (!(pPl = pDataModel->getPlotDefinitionList()->createPlotSpec(nname, CPlotItem::plot2d)))
    {
      i++;
      nname = name;
      nname += TO_UTF8(QString::number(i));
    }

  return pPl;
}

void PlotWidget::deleteObjects(const std::vector<std::string> & keys)
{
  assert(CCopasiRootContainer::getDatamodelList()->size() > 0);
  CCopasiDataModel* pDataModel = (*CCopasiRootContainer::getDatamodelList())[0];
  assert(pDataModel != NULL);
  if (!pDataModel->getModel())
    return;

  if (keys.size() == 0)
    return;

  unsigned C_INT32 i, imax = keys.size();
  for (i = 0; i < imax; i++)
    {
      pDataModel->getPlotDefinitionList()->removePlotSpec(keys[i]);
      protectedNotify(ListViews::PLOT, ListViews::DELETE, keys[i]);
    }

  mChanged = true;
}
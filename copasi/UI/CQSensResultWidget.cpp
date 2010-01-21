// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/CQSensResultWidget.cpp,v $
//   $Revision: 1.14 $
//   $Name:  $
//   $Author: pwilly $
//   $Date: 2010/01/21 10:49:19 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

//#include <qpushbutton.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <q3table.h>
//Added by qt3to4:
#include <Q3GridLayout>

#include "copasi.h"

#include "CQSensResultWidget.h"
//#include "StateSubwidget.h"
//#include "CTimeSeriesTable.h"
#include "qtUtilities.h"
#include "sensitivities/CSensTask.h"
#include "sensitivities/CSensProblem.h"

#include "CopasiDataModel/CCopasiDataModel.h"
#include "report/CCopasiRootContainer.h"
#include "utilities/CCopasiVector.h"

#include <qtabwidget.h>

/*
 *  Constructs a CQSensResultWidget which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
CQSensResultWidget::CQSensResultWidget(QWidget* parent, const char* name, Qt::WFlags fl)
    : CopasiWidget(parent, name, fl),
    mpResult(NULL)
{
  if (!name)
    setName("CQSensResultWidget");

  setCaption("CQSensResultWidget");

  mWidgetLayout = new Q3GridLayout(this, 1, 1, 11, 6, "CQSensResultWidgetLayout");

  // **********  Label **************
  mLabelTitle = new QLabel(this, "SensLabel");
  mLabelTitle->setText("Sensitivities");
  mLabelTitle->setAlignment(int(Qt::AlignVCenter
                                | Qt::AlignLeft));
  mLabelTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  mWidgetLayout->addWidget(mLabelTitle, 0, 0);

  // tab widget
  mpTab = new QTabWidget(this, "TabWidget");
  mWidgetLayout->addMultiCellWidget(mpTab, 1, 2, 0, 2);

  // unscaled array
  mArrayWidget = new CQArrayAnnotationsWidget(mpTab, "ArrayWidget");
  //mArrayWidget->setColorCoding(new CColorScale1());
  //CColorScaleAverage * tcs = new CColorScaleAverage();
  CColorScaleBiLog * tcs = new CColorScaleBiLog();
  mArrayWidget->setColorCoding(tcs);
  //tcs->setMinMax(-1,1);
  //tcs->setSymmetric(true);
  //tcs->setFactor(3.0);
  mArrayWidget->setColorScalingAutomatic(true);
  mpTab->addTab(mArrayWidget, "unscaled");

  //scaled array
  mArrayWidgetScaled = new CQArrayAnnotationsWidget(mpTab, "ArrayWidget2");
  //mArrayWidgetScaled->setColorCoding(new CColorScale1());
  CColorScaleAverage* tcs2 = new CColorScaleAverage();
  mArrayWidgetScaled->setColorCoding(tcs2);
  //tcs2->setMinMax(-1,1);
  //tcs2->setSymmetric(true);
  tcs2->setFactor(3.0);
  mArrayWidgetScaled->setColorScalingAutomatic(true);
  mpTab->addTab(mArrayWidgetScaled, "scaled");

  //scaled array
  mArrayWidgetCollapsed = new CQArrayAnnotationsWidget(mpTab, "ArrayWidget3");
  //mArrayWidgetScaled->setColorCoding(new CColorScale1());
  CColorScaleAverage* tcs3 = new CColorScaleAverage();
  mArrayWidgetCollapsed->setColorCoding(tcs3);
  //tcs3->setMinMax(-1,1);
  //tcs3->setSymmetric(true);
  tcs3->setFactor(3.0);
  mArrayWidgetCollapsed->setColorScalingAutomatic(true);
  mpTab->addTab(mArrayWidgetCollapsed, "summarized");
}

/*
 *  Destroys the object and frees any allocated resources
 */
CQSensResultWidget::~CQSensResultWidget()
{}

void CQSensResultWidget::newResult()
{
  assert(CCopasiRootContainer::getDatamodelList()->size() > 0);

  // clear the old one
  clearArrays();

  CSensTask * pTask =
    dynamic_cast<CSensTask*>((*(*CCopasiRootContainer::getDatamodelList())[0]->getTaskList())["Sensitivities"]);

  if (!pTask)
    return;

  CSensProblem * pProblem = dynamic_cast< CSensProblem * >(pTask->getProblem());

  if (!pProblem)
    return;

  mpResult = pProblem->getResultAnnotated();
  mpScaledResult = pProblem->getScaledResultAnnotated();

  mArrayWidget->setArrayAnnotation(mpResult);
  mArrayWidgetScaled->setArrayAnnotation(mpScaledResult);

  //tab for collapsed result
  if (pProblem->collapsRequested())
    {
      mpCollapsedResult = pProblem->getCollapsedResultAnnotated();
      mpTab->setTabEnabled(mArrayWidgetCollapsed, true);
    }
  else
    {
      mpCollapsedResult = NULL;
      mpTab->setTabEnabled(mArrayWidgetCollapsed, false);
    }

  mArrayWidgetCollapsed->setArrayAnnotation(mpCollapsedResult);
}

void CQSensResultWidget::clearArrays()
{
  mArrayWidget->setArrayAnnotation(NULL);
  mArrayWidgetScaled->setArrayAnnotation(NULL);
  mArrayWidgetCollapsed->setArrayAnnotation(NULL);
}

//*************************************

bool CQSensResultWidget::update(ListViews::ObjectType C_UNUSED(objectType), ListViews::Action
                                C_UNUSED(action), const std::string & C_UNUSED(key))
{
  /*  if (this->isShown())
      return loadFromBackend();
    else
      return true;*/
  return true;
}

bool CQSensResultWidget::leave()
{
  return true;
}

bool CQSensResultWidget::enterProtected()
{
  //clear the widget if the pointer to the result has changed
  assert(CCopasiRootContainer::getDatamodelList()->size() > 0);
  CSensTask * pTask =
    dynamic_cast<CSensTask*>((*(*CCopasiRootContainer::getDatamodelList())[0]->getTaskList())["Sensitivities"]);

  if (!pTask)
    {
      clearArrays();
      return false;
    }

  CSensProblem * pProblem = dynamic_cast< CSensProblem * >(pTask->getProblem());

  if (!pProblem)
    {
      clearArrays();
      return false;
    }

  if (mpResult != pProblem->getResultAnnotated())
    {
      clearArrays();
      return false;
    }

  return true;
}

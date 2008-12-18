// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/Attic/OptimizationResultWidget.cpp,v $
//   $Revision: 1.9 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2008/12/18 19:58:12 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#include <qpushbutton.h>
#include <qlayout.h>
#include <q3textedit.h>
#include <qlabel.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>

#include "copasi.h"

#include "OptimizationResultWidget.h"
#include "StateSubwidget.h"
#include "CTimeSeriesTable.h"
#include "qtUtilities.h"

#include "CopasiDataModel/CCopasiDataModel.h"
#include "report/CKeyFactory.h"
#include "steadystate/CSteadyStateTask.h"
#include "model/CModel.h"
#include "trajectory/CTrajectoryTask.h"
#include "trajectory/CTimeSeries.h"

/*
 *  Constructs a OptimizationResultWidget which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
OptimizationResultWidget::OptimizationResultWidget(QWidget* parent, const char* name, Qt::WFlags fl, const int comingFrom)
    : CopasiWidget(parent, name, fl), objKey(""),
    mpTimeSeries(NULL)
{
  if (!name)
    setName("OptimizationResultWidget");
  setCaption(trUtf8("OptimizationResultWidget"));

  parentLayout = new Q3VBoxLayout(this, 0, 0, "parentLayout");
  stateLayout = new Q3HBoxLayout(0, 0 , 6, "StateLayout");
  mWidgetLayout = new Q3GridLayout(NULL, 1, 1, 0, -1, "Layout");

  if (comingFrom == 0)
    {
      mCentralWidgetTime = NULL;
      mCentralWidgetSteady = new StateSubwidget(this, "StateSubwidget");
      mWidgetLayout->addWidget(mCentralWidgetSteady, 0, 0);
      mCentralWidgetSteady->displayOptimizationTab(true);
    }
  else
    {
      mCentralWidgetSteady = NULL;
      mCentralWidgetTime = new TimeSeriesSubWidget(this, "TimeSeriesSubWidget", 0);
      mWidgetLayout->addWidget(mCentralWidgetTime, 0, 0);
      mCentralWidgetTime->displayOptimizationTab(true);
    }

  //parentLayout->addLayout(stateLayout);
  parentLayout->addLayout(mWidgetLayout);
}

/*
 *  Destroys the object and frees any allocated resources
 */
OptimizationResultWidget::~OptimizationResultWidget()
{
  pdelete(mpTimeSeries);
}

/* This function loads the optimization result widget when its name is
  clicked in the tree   */
bool OptimizationResultWidget::loadFromBackend()
{
  bool success = true;
  std::ostringstream os;

  CCopasiTask * pTask = (*CCopasiDataModel::Global->getTaskList())["Optimization"];
  if (!pTask) return false;

  COptProblem * pProblem = dynamic_cast< COptProblem * >(pTask->getProblem());
  if (!pProblem) return false;

  // We need to use the solution and run Steady-State or Time-Course.
  const CVector< C_FLOAT64 > & Solution = pProblem->getSolutionVariables();

  success &= (pProblem->getSolutionValue() < DBL_MAX);

  if (success)
    pProblem->printResult(&os);
  else
    os << "<h2>No result available, please execute the optimization task.</h2>";

  if (mCentralWidgetTime != NULL)
    {
      mCentralWidgetTime->table()->setTimeSeries(dynamic_cast<CTrajectoryTask *>((*CCopasiDataModel::Global->getTaskList())["Time-Course"])->getTimeSeries());
      mCentralWidgetTime->optimizationResultText->setText(FROM_UTF8(os.str()));
    }

  if (mCentralWidgetSteady != NULL)
    {
      CSteadyStateTask * pSteadyStateTask =
        dynamic_cast<CSteadyStateTask *>((*CCopasiDataModel::Global->getTaskList())["Steady-State"]);

      mCentralWidgetSteady->optimizationResultText->setText(FROM_UTF8(os.str()));

      if (!pSteadyStateTask || !success || !pSteadyStateTask->getState())
        {
          mCentralWidgetSteady->clear();
          success = false;
        }
      else
        success = mCentralWidgetSteady->loadAll(pSteadyStateTask);
    }

  try
    {
      if (Solution.size()) pProblem->restore(pTask->isUpdateModel());
    }

catch (...) {}

  return success;
}

bool OptimizationResultWidget::saveToBackend()
{
  return true;
}

bool OptimizationResultWidget::update(ListViews::ObjectType objectType,
                                      ListViews::Action action,
                                      const std::string & C_UNUSED(key))
{
  if (objectType == ListViews::MODEL &&
      action == ListViews::ADD)
    {
      if (mCentralWidgetTime != NULL)
        {
          mCentralWidgetTime->table()->setTimeSeries(CTimeSeries());
          mCentralWidgetTime->optimizationResultText->setText("<h2>No result available, please execute the optimization task.</h2>");
        }

      if (mCentralWidgetSteady != NULL)
        mCentralWidgetSteady->optimizationResultText->setText("<h2>No result available, please execute the optimization task.</h2>");
    }

  return true;
}

bool OptimizationResultWidget::leave()
{return true;}

bool OptimizationResultWidget::enter(const std::string & C_UNUSED(key))
{return true;}

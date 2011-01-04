// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/SteadyStateWidget.cpp,v $
//   $Revision: 1.126.2.1 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2011/01/04 13:57:48 $
// End CVS Header

// Copyright (C) 2010 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#include <q3filedialog.h>

#include <qvariant.h>
#include <qcheckbox.h>
//#include <q3frame.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <q3whatsthis.h>
#include <qmessagebox.h>
//Added by qt3to4:
/*
#include <Q3GridLayout>
#include <Q3VBoxLayout>
 */
#include <QHBoxLayout>

#include "DataModelGUI.h"

#include "SteadyStateWidget.h"
#include "listviews.h"
#include "CQTaskBtnWidget.h"
#include "CQTaskHeaderWidget.h"
#include "CQTaskMethodWidget.h"
#include "CProgressBar.h"
#include "CQSteadyStateResult.h"

#include "copasi.h"
#include "qtUtilities.h"

#include "CopasiDataModel/CCopasiDataModel.h"
#include "report/CCopasiRootContainer.h"
#include "steadystate/CSteadyStateTask.h"
#include "steadystate/CSteadyStateProblem.h"
#include "model/CModel.h"
#include "utilities/CCopasiException.h"
#include "report/CKeyFactory.h"

/**
 *  Constructs a SteadyStateWidget which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
SteadyStateWidget::SteadyStateWidget(QWidget* parent, const char* name, Qt::WFlags fl)
    : TaskWidget(parent, name, fl)
{
  setupUi(this);

  init();
  retranslateUi(this);
}

/*
 *  Destroys the object and frees any allocated resources
 */
SteadyStateWidget::~SteadyStateWidget()
{}

void SteadyStateWidget::languageChange()
{
  retranslateUi(this);
}

void SteadyStateWidget::init()
{
  mpHeaderWidget->setTaskName("Steady State");

  SteadyStateWidgetLayout->insertWidget(0, mpHeaderWidget); // header
  // SteadyStateWidgetLayout->insertSpacing(1, 14);      // space between header and body

  mpMethodWidget->enableMethodParameter(true);
  SteadyStateWidgetLayout->addWidget(mpMethodWidget);

  SteadyStateWidgetLayout->addWidget(mpBtnWidget);      // 'footer'
}

CCopasiMethod * SteadyStateWidget::createMethod(const CCopasiMethod::SubType & /*type*/)
{return NULL; /*CTSSMethod::createTSSMethod(type);*/}

bool SteadyStateWidget::runTask()
{
  bool success = true;

  if (!commonBeforeRunTask()) return false;

  if (!commonRunTask()) success = false;

  return success;
}

bool SteadyStateWidget::taskFinishedEvent()
{
  bool success = true;

  CQSteadyStateResult *pResult = dynamic_cast< CQSteadyStateResult * >(mpListView->findWidgetFromId(211));

  if (pResult) pResult->loadResult();

  if (success && isShown()) mpListView->switchToOtherWidget(211, ""); //change to the results window

  return success;
}

bool SteadyStateWidget::loadTask()
{
  loadCommon();
  loadMethod();

  CSteadyStateTask* mSteadyStateTask =
    dynamic_cast<CSteadyStateTask *>(CCopasiRootContainer::getKeyFactory()->get(mKey));

  if (mSteadyStateTask == NULL)
    return false;

  CSteadyStateProblem* steadystateproblem =
    dynamic_cast<CSteadyStateProblem *>(mSteadyStateTask->getProblem());

  if (steadystateproblem == NULL)
    return false;

  CSteadyStateMethod* steadystatemethod =
    dynamic_cast<CSteadyStateMethod *>(mSteadyStateTask->getMethod());

  if (steadystatemethod == NULL)
    return false;

  bool bJacobian = steadystateproblem->isJacobianRequested();
  bool bStatistics = steadystateproblem->isStabilityAnalysisRequested();
  taskJacobian->setChecked(bJacobian);

  if (bJacobian)
    {
      taskStability->setEnabled(true);
      taskStability->setChecked(bStatistics);
    }

  mChanged = false;

  return true;
}

bool SteadyStateWidget::saveTask()
{
  saveCommon();
  saveMethod();

  CSteadyStateTask* mSteadyStateTask =
    dynamic_cast<CSteadyStateTask *>(CCopasiRootContainer::getKeyFactory()->get(mKey));

  if (mSteadyStateTask == NULL)
    return false;

  CSteadyStateProblem* steadystateproblem =
    dynamic_cast<CSteadyStateProblem *>(mSteadyStateTask->getProblem());

  if (steadystateproblem == NULL)
    return false;

  CSteadyStateMethod* steadystatemethod =
    dynamic_cast<CSteadyStateMethod *>(mSteadyStateTask->getMethod());

  if (steadystatemethod == NULL)
    return false;

  bool bJacobian = taskJacobian->isChecked();
  bool bStatistics = taskStability->isChecked();

  steadystateproblem->setJacobianRequested(bJacobian);
  steadystateproblem->setStabilityAnalysisRequested(bStatistics);

  if (mChanged)
    {
      if (mpDataModel != NULL)
        {
          mpDataModel->changed();
        }

      mChanged = false;
    }

  return true;
}

void SteadyStateWidget::taskJacobianToggled()
{
  if (!taskJacobian->isChecked())
    {
      taskStability->setChecked(false);
      taskStability->setEnabled(false);
    }
  else taskStability->setEnabled(true);
}

// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/Attic/CQTrajectoryWidget.ui.h,v $
//   $Revision: 1.19 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2009/02/19 19:53:30 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/****************************************************************************
 ** ui.h extension file, included from the uic-generated form implementation.
 **
 ** If you want to add, delete, or rename functions or slots, use
 ** Qt Designer to update this file, preserving your code.
 **
 ** You should not define a constructor or destructor in this file.
 ** Instead, write your code in functions called init() and destroy().
 ** These will automatically be called by the form's constructor and
 ** destructor.
 *****************************************************************************/

#include "copasi.h"

#include "UI/CQTaskBtnWidget.h"
#include "UI/CQTaskHeaderWidget.h"
#include "UI/CProgressBar.h"
#include "UI/CQValidator.h"
#include "UI/CQMessageBox.h"
#include "UI/qtUtilities.h"
#include "TimeSeriesWidget.h"

#include "trajectory/CTrajectoryTask.h"
#include "trajectory/CTrajectoryProblem.h"
#include "model/CModel.h"
#include "report/CKeyFactory.h"
#include "utilities/CCopasiException.h"
#include "report/CCopasiRootContainer.h"

#define TSMAX 10000000

void CQTrajectoryWidget::init()
{
  mpTrajectoryProblem = NULL;

  mpHeaderWidget->setTaskName("Time Course");

  vboxLayout->insertWidget(0, mpHeaderWidget);
  vboxLayout->addWidget(mpBtnWidget);

  addMethodSelectionBox(CTrajectoryTask::ValidMethods);
  addMethodParameterTable(0);

  slotOutputDelay(false);

  mpValidatorDuration = new CQValidatorDouble(mpEditDuration);
  mpEditDuration->setValidator(mpValidatorDuration);

  mpValidatorIntervalSize = new CQValidatorDouble(mpEditIntervalSize);
  mpValidatorIntervalSize->setRange(0, DBL_MAX);
  mpEditIntervalSize->setValidator(mpValidatorIntervalSize);

  mpValidatorIntervals = new CQValidatorInt(mpEditIntervals);
  mpValidatorIntervals->setRange(0, LONG_MAX);
  mpEditIntervals->setValidator(mpValidatorIntervals);

  mpValidatorDelay = new CQValidatorDouble(mpEditDelay);
  mpEditDelay->setValidator(mpValidatorDelay);
}

void CQTrajectoryWidget::destroy()
{
  pdelete(mpTrajectoryProblem);
}

void CQTrajectoryWidget::slotDuration()
{
  if (!mpEditDuration->hasAcceptableInput())
    return;

  try
    {
      mpTrajectoryProblem->setDuration(mpEditDuration->text().toDouble());
    }
  catch (...)
    {
      CQMessageBox::information(this, QString("Information"),
                                FROM_UTF8(CCopasiMessage::getAllMessageText()),
                                QMessageBox::Ok, QMessageBox::Ok);
    }

  mpEditIntervalSize->setText(QString::number(mpTrajectoryProblem->getStepSize()));
  mpValidatorIntervalSize->revalidate();
  mpEditIntervals->setText(QString::number(mpTrajectoryProblem->getStepNumber()));
  mpValidatorIntervals->revalidate();

  checkTimeSeries();
  updateIntervals();
}

void CQTrajectoryWidget::slotIntervalSize()
{
  if (!mpEditIntervalSize->hasAcceptableInput())
    return;

  try
    {
      mpTrajectoryProblem->setStepSize(mpEditIntervalSize->text().toDouble());
    }

  catch (...)
    {
      CQMessageBox::information(this, QString("Information"),
                                FROM_UTF8(CCopasiMessage::getAllMessageText()),
                                QMessageBox::Ok, QMessageBox::Ok);
    }

  mpEditIntervalSize->setText(QString::number(mpTrajectoryProblem->getStepSize()));
  mpValidatorIntervalSize->revalidate();
  mpEditIntervals->setText(QString::number(mpTrajectoryProblem->getStepNumber()));
  mpValidatorIntervals->revalidate();

  checkTimeSeries();
  updateIntervals();
}

void CQTrajectoryWidget::slotIntervals()
{
  if (!mpEditIntervals->hasAcceptableInput())
    return;

  try
    {
      mpTrajectoryProblem->setStepNumber(mpEditIntervals->text().toULong());
    }
  catch (...)
    {
      CQMessageBox::information(this, QString("Information"),
                                FROM_UTF8(CCopasiMessage::getAllMessageText()),
                                QMessageBox::Ok, QMessageBox::Ok);
    }

  mpEditIntervalSize->setText(QString::number(mpTrajectoryProblem->getStepSize()));
  mpValidatorIntervalSize->revalidate();

  checkTimeSeries();
  updateIntervals();
}

void CQTrajectoryWidget::slotOutputDelay(bool checked)
{
  mpEditDelay->setEnabled(checked);
  updateIntervals();
}

bool CQTrajectoryWidget::saveTask()
{
  CTrajectoryTask * pTask =
    dynamic_cast< CTrajectoryTask * >(mpTask);
  if (!pTask) return false;

  saveCommon();
  saveMethod();

  CTrajectoryProblem* trajectoryproblem =
    dynamic_cast<CTrajectoryProblem *>(pTask->getProblem());
  assert(trajectoryproblem);

  //numbers
  if (mpEditIntervalSize->hasAcceptableInput() &&
      trajectoryproblem->getStepSize() != mpEditIntervalSize->text().toDouble())
    {
      trajectoryproblem->setStepSize(mpEditIntervalSize->text().toDouble());
      mChanged = true;
    }
  else if (mpEditIntervals->hasAcceptableInput() &&
           trajectoryproblem->getStepNumber() != mpEditIntervals->text().toULong())
    {
      trajectoryproblem->setStepNumber(mpEditIntervals->text().toLong());
      mChanged = true;
    }

  if (mpEditDuration->hasAcceptableInput() &&
      trajectoryproblem->getDuration() != mpEditDuration->text().toDouble())
    {
      trajectoryproblem->setDuration(mpEditDuration->text().toDouble());
      mChanged = true;
    }

  C_FLOAT64 StartTime = mpEditDelay->text().toDouble();

  if (mpCheckDelay->isChecked())
    {
      if (mpEditDelay->hasAcceptableInput() &&
          StartTime != trajectoryproblem->getOutputStartTime())
        {
          trajectoryproblem->setOutputStartTime(StartTime);
          mChanged = true;
        }
    }
  else
    {
      assert(CCopasiRootContainer::getDatamodelList()->size() > 0);
      C_FLOAT64 InitialTime =
        (*CCopasiRootContainer::getDatamodelList())[0]->getModel()->getInitialTime();
      if (trajectoryproblem->getStepSize() > 0.0)
        {
          if (StartTime > InitialTime)
            {
              trajectoryproblem->setOutputStartTime(InitialTime);
              mChanged = true;
            }
        }
      else
        {
          if (StartTime < InitialTime)
            {
              trajectoryproblem->setOutputStartTime(InitialTime);
              mChanged = true;
            }
        }
    }

  if (trajectoryproblem->timeSeriesRequested() != mpCheckSave->isChecked())
    {
      trajectoryproblem->setTimeSeriesRequested(mpCheckSave->isChecked());
      mChanged = true;
    }

  mpValidatorDuration->saved();
  mpValidatorIntervalSize->saved();
  mpValidatorIntervals->saved();
  mpValidatorDelay->saved();
  return true;
}

bool CQTrajectoryWidget::loadTask()
{
  CTrajectoryTask * pTask =
    dynamic_cast< CTrajectoryTask * >(mpTask);
  if (!pTask) return false;

  loadCommon();
  loadMethod();

  CTrajectoryProblem* trajectoryproblem =
    dynamic_cast<CTrajectoryProblem *>(pTask->getProblem());
  assert(trajectoryproblem);

  pdelete(mpTrajectoryProblem);
  mpTrajectoryProblem = new CTrajectoryProblem(*trajectoryproblem);

  //numbers
  mpEditIntervalSize->setText(QString::number(trajectoryproblem->getStepSize()));
  mpEditIntervals->setText(QString::number(trajectoryproblem->getStepNumber()));
  mpEditDuration->setText(QString::number(trajectoryproblem->getDuration()));

  assert(CCopasiRootContainer::getDatamodelList()->size() > 0);
  C_FLOAT64 InitialTime = (*CCopasiRootContainer::getDatamodelList())[0]->getModel()->getInitialTime();

  bool Delayed;
  if (trajectoryproblem->getStepSize() > 0.0)
    Delayed = (trajectoryproblem->getOutputStartTime() - InitialTime) > DBL_MIN;
  else
    Delayed = (InitialTime - trajectoryproblem->getOutputStartTime()) > DBL_MIN;

  mpCheckDelay->setChecked(Delayed);
  mpEditDelay->setEnabled(Delayed);

  mpEditDelay->setText(QString::number(trajectoryproblem->getOutputStartTime()));

  //store time series checkbox
  mpCheckSave->setChecked(trajectoryproblem->timeSeriesRequested());

  checkTimeSeries();

  updateIntervals();

  mpValidatorDuration->saved();
  mpValidatorIntervalSize->saved();
  mpValidatorIntervals->saved();
  mpValidatorDelay->saved();
  return true;
}

CCopasiMethod * CQTrajectoryWidget::createMethod(const CCopasiMethod::SubType & type)
{
  return CTrajectoryMethod::createTrajectoryMethod(type);
}

bool CQTrajectoryWidget::runTask()
{
  checkTimeSeries();
  if (!commonBeforeRunTask()) return false;

  bool success = true;
  if (!commonRunTask()) success = false;

  if (!commonAfterRunTask()) success = false;

  // We need to load the result here as this is the only place where
  // we know that it is correct.
  TimeSeriesWidget * pResult =
    dynamic_cast< TimeSeriesWidget * >(mpListView->findWidgetFromId(231));

  if (pResult == NULL)
    return false;

  success &= pResult->loadFromBackend();

  return success;
}

void CQTrajectoryWidget::checkTimeSeries()
{
  assert(CCopasiRootContainer::getDatamodelList()->size() > 0);
  if (mpEditIntervals->text().toLong() * (*CCopasiRootContainer::getDatamodelList())[0]->getModel()->getStateTemplate().getNumVariable() > TSMAX)
    {
      mpCheckSave->setChecked(false);
      mpCheckSave->setEnabled(false);
    }
  else
    {
      mpCheckSave->setEnabled(true);
    }
}

void CQTrajectoryWidget::updateIntervals()
{
  assert(CCopasiRootContainer::getDatamodelList()->size() > 0);
  C_FLOAT64 InitialTime = (*CCopasiRootContainer::getDatamodelList())[0]->getModel()->getInitialTime();
  C_FLOAT64 Duration = mpEditDuration->text().toDouble();
  C_FLOAT64 OutputStartTime = InitialTime;

  if (mpCheckDelay->isChecked())
    {
      if (!mpEditIntervalSize->hasAcceptableInput())
        return;

      OutputStartTime = mpEditDelay->text().toDouble();
    }

  mpEditIntegrationInterval->setText(QString::number(InitialTime) +
                                     " to " +
                                     QString::number(InitialTime + Duration));

  if (Duration > 0.0)
    {
      if (std::max(InitialTime, OutputStartTime) > InitialTime + Duration)
        mpEditOutputInterval->setText("empty");
      else if (InitialTime < OutputStartTime)
        {
          C_FLOAT64 StepSize = mpEditIntervalSize->text().toDouble();
          OutputStartTime = InitialTime + (ceil((OutputStartTime - InitialTime) / StepSize)) * StepSize;
          mpEditOutputInterval->setText(QString::number(OutputStartTime) +
                                        " to " +
                                        QString::number(InitialTime + Duration));
        }
      else
        {
          mpEditOutputInterval->setText(QString::number(InitialTime) +
                                        " to " +
                                        QString::number(InitialTime + Duration));
        }
    }
  else
    {
      if (std::min(InitialTime, OutputStartTime) < InitialTime + Duration)
        mpEditOutputInterval->setText("empty");
      else if (InitialTime > OutputStartTime)
        {
          C_FLOAT64 StepSize = mpEditIntervalSize->text().toDouble();
          OutputStartTime = InitialTime + (ceil((OutputStartTime - InitialTime) / StepSize)) * StepSize;
          mpEditOutputInterval->setText(QString::number(OutputStartTime) +
                                        " to " +
                                        QString::number(InitialTime + Duration));
        }
      else
        mpEditOutputInterval->setText(QString::number(InitialTime) +
                                      " to " +
                                      QString::number(InitialTime + Duration));
    }
}

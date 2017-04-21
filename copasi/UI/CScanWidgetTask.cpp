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

// Copyright (C) 2005 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#include <QValidator>

#include <iostream>

#include "CScanWidgetTask.h"
#include "CCopasiSelectionDialog.h"

#include "copasi.h"

#include "utilities/CCopasiParameterGroup.h"
#include "core/CRegisteredCommonName.h"
#include "scan/CScanProblem.h"

/*
 *  Constructs a CScanWidgetTask as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
CScanWidgetTask::CScanWidgetTask(QWidget* parent, const char* name, Qt::WindowFlags f):
  QWidget(parent, f),
  mpObject(NULL),
  mpModel(NULL)

{
  setObjectName(QString::fromUtf8(name));
  setupUi(this);
}

/*
 *  Destroys the object and frees any allocated resources
 */
CScanWidgetTask::~CScanWidgetTask()
{
  // no need to delete child widgets, Qt does it all for us
}

void CScanWidgetTask::init()
{}

// virtual
void CScanWidgetTask::load(const CScanProblem * pg)
{
  CTaskEnum::Task type = pg->getSubtask();
  int n;

  switch (type)
    {
      case CTaskEnum::steadyState:
        n = 0;
        break;

      case CTaskEnum::timeCourse:
        n = 1;
        break;

      case CTaskEnum::mca:
        n = 2;
        break;

      case CTaskEnum::lyap:
        n = 3;
        break;

      case CTaskEnum::optimization:
        n = 4;
        break;

      case CTaskEnum::parameterFitting:
        n = 5;
        break;

      case CTaskEnum::sens:
        n = 6;
        break;

      case CTaskEnum::lna:
        n = 7;
        break;

      case CTaskEnum::crosssection:
        n = 8;
        break;

      case CTaskEnum::tssAnalysis:
        n = 9;
        break;

      default:
        n = 0;
        break;
    }

  comboType->setCurrentIndex(n);

  mpCheckContinue->setChecked(pg->getContinueFromCurrentState());

  checkOutput->setChecked(pg->getOutputInSubtask());

  return;
}

// virtual
bool CScanWidgetTask::save(CScanProblem * pg) const
{
  bool changed = false;
  int type = comboType->currentIndex();

  CTaskEnum::Task Type;

  switch (type)
    {
      case 0:
        Type = CTaskEnum::steadyState;
        break;

      case 1:
        Type = CTaskEnum::timeCourse;
        break;

      case 2:
        Type = CTaskEnum::mca;
        break;

      case 3:
        Type = CTaskEnum::lyap;
        break;

      case 4:
        Type = CTaskEnum::optimization;
        break;

      case 5:
        Type = CTaskEnum::parameterFitting;
        break;

      case 6:
        Type = CTaskEnum::sens;
        break;

      case 7:
        Type = CTaskEnum::lna;
        break;

      case 8:
        Type = CTaskEnum::crosssection;
        break;

      case 9:
        Type = CTaskEnum::tssAnalysis;
        break;

      default :
        Type = CTaskEnum::steadyState;
        break;
    }

  if (Type != pg->getSubtask())
    {
      pg->setSubtask(Type);
      changed = true;
    }

  if (pg->getContinueFromCurrentState() != mpCheckContinue->isChecked())
    {
      pg->setContinueFromCurrentState(mpCheckContinue->isChecked());
      changed = true;
    }

  if (pg->getOutputInSubtask() != checkOutput->isChecked())
    {
      pg->setOutputInSubtask(checkOutput->isChecked());
      changed = true;
    }

  return changed;
}

void CScanWidgetTask::typeChanged(int n)
{
  switch (n)
    {
      case 1:
      case 8:
        checkOutput->setChecked(true);
        break;

      default:
        checkOutput->setChecked(false);
        break;
    }
}

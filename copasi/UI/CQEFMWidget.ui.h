// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/Attic/CQEFMWidget.ui.h,v $
//   $Revision: 1.8.6.1 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2008/11/11 17:19:46 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#include <qmessagebox.h>
#include <qcheckbox.h>
#include <qregexp.h>

#include "UI/CQTaskBtnWidget.h"
#include "UI/CQTaskHeaderWidget.h"
#include "UI/CProgressBar.h"
#include "UI/qtUtilities.h"
#include "UI/CopasiFileDialog.h"

#include "copasi.h"

#include "CopasiDataModel/CCopasiDataModel.h"
#include "model/CReaction.h"
#include "elementaryFluxModes/CEFMMethod.h"
#include "elementaryFluxModes/CEFMTask.h"
#include "elementaryFluxModes/CSSAMethod.h"
#include "report/CKeyFactory.h"
#include "utilities/CCopasiException.h"

void CQEFMWidget::init()
{
  mpHeaderWidget->setTaskName("Elementary Flux Modes");
  mpHeaderWidget->mpUpdateModel->hide();

#ifdef COPASI_SSA
  addMethodSelectionBox(CEFMTask::ValidMethods, 0);
#endif

  CQEFMWidgetLayout->insertWidget(0, mpHeaderWidget);
  CQEFMWidgetLayout->addWidget(mpBtnWidget);
}

void CQEFMWidget::destroy()
{}

void CQEFMWidget::loadFluxModes()
{
  mpListView->clear();

  QListViewItem* item;

  CEFMTask * pTask =
    dynamic_cast< CEFMTask * >(mpTask);

  if (pTask)
    {
      unsigned C_INT32 const noOfModesRows = pTask->getFluxModeSize();
      mpEditFluxModes->setText(QString::number(noOfModesRows));

      mpListView->setColumnText(0, "Reversibility");
#ifdef COPASI_SSA
      bool ssatask = pTask->getMethod()->getSubType() == CCopasiMethod::stoichiometricStabilityAnalysis;
      if (ssatask)
        mpListView->setColumnText(0, "Stability");
#endif
      unsigned C_INT32 j;
      for (j = 0; j < noOfModesRows; j++)
        {
#ifdef COPASI_SSA
          if (ssatask)
            {
              item = new QListViewItem(mpListView, "");

              std::string title;
              switch (dynamic_cast<CSSAMethod *>(pTask->getMethod())->isMixingStable(j))
                {
                case TriTrue:
                  title = "Mixing stable";
                  break;

                case TriFalse:
                  title = "Not mixing stable";
                  break;

                default:
                  title = "Unknown";
                }

              item->setText(0, title);
            }
          else
            {
#endif // COPASI_SSA
              if (pTask->isFluxModeReversible(j) == true)
                {
                  item = new QListViewItem(mpListView, "Reversible");
                }
              else
                {
                  item = new QListViewItem(mpListView, "Irreversible");
                }
#ifdef COPASI_SSA
            }
#endif // COPASI_SSA
          item->setMultiLinesEnabled(true);

          item->setText(1, FROM_UTF8(pTask->getFluxModeDescription(j)));
          std::string reactionEq = "";
          unsigned int x, xmax = pTask->getFluxModeSize(j);
          //const CFluxMode & mode = pTask->getFluxMode(j);
          for (x = 0; x < xmax; x++)
            {
              reactionEq += pTask->getReactionEquation(j, x);
              reactionEq += "\n";
            }
          item->setText(2, FROM_UTF8(reactionEq).stripWhiteSpace() + "\n");
        }
    }
}

bool CQEFMWidget::runTask()
{
  if (dynamic_cast< CEFMTask * >(mpTask) == NULL)
    return false;

  if (!commonBeforeRunTask()) return false;

  bool success = commonRunTask();

  commonAfterRunTask();

  if (success) loadFluxModes();

  return success;
}

bool CQEFMWidget::saveTask()
{
  CEFMTask * pTask =
    dynamic_cast< CEFMTask * >(mpTask);
  if (!pTask) return false;

  saveCommon();
  return true;
}

bool CQEFMWidget::loadTask()
{
  CEFMTask * pTask =
    dynamic_cast< CEFMTask * >(mpTask);
  if (!pTask) return false;

  loadCommon();

  return true;
}

CCopasiMethod * CQEFMWidget::createMethod(const CCopasiMethod::SubType & type)
{
  mpTask->setMethodType(type);
  return mpTask->getMethod();
}

void CQEFMWidget::slotSave()
{
  C_INT32 Answer = QMessageBox::No;
  QString fileName;

  while (Answer == QMessageBox::No)
    {
      fileName =
        CopasiFileDialog::getSaveFileName(this, "Save File Dialog",
                                          QString::null, "TEXT Files (*.txt);;All Files (*.*);;", "Save to");

      if (fileName.isEmpty()) return;

      if (!fileName.endsWith(".txt") &&
          !fileName.endsWith(".")) fileName += ".txt";

      fileName = fileName.remove(QRegExp("\\.$"));

      Answer = checkSelection(fileName);

      if (Answer == QMessageBox::Cancel) return;
    }

  std::ofstream file(utf8ToLocale((const char *) fileName.utf8()).c_str());

  if (file.fail())
    return;

  if (mpTask != NULL)
    file << mpTask->getResult();

  return;
}

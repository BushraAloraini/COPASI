// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/Attic/CQEFMWidget.ui.h,v $
//   $Revision: 1.14 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2009/09/01 15:57:28 $
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
#ifdef COPASI_SSA
#include "elementaryFluxModes/CSSAMethod.h"
#endif // COPASI_SSA
#include "report/CKeyFactory.h"
#include "utilities/CCopasiException.h"

void CQEFMWidget::init()
{
  mpHeaderWidget->setTaskName("Elementary Flux Modes");
  mpHeaderWidget->mpUpdateModel->hide();

#ifdef COPASI_SSA
  addMethodSelectionBox(CEFMTask::ValidMethods, 0);
#endif

  vboxLayout->insertWidget(0, mpHeaderWidget);
  vboxLayout->addWidget(mpBtnWidget);
}

void CQEFMWidget::destroy()
{}

void CQEFMWidget::loadFluxModes()
{
  Ui::CQEFMWidget::mpListView->clear();

  Q3ListViewItem* item;

  CEFMTask * pTask =
    dynamic_cast< CEFMTask * >(mpTask);

  if (pTask)
    {
      unsigned C_INT32 const noOfModesRows = pTask->getFluxModeSize();
      mpEditFluxModes->setText(QString::number(noOfModesRows));

      Ui::CQEFMWidget::mpListView->setColumnText(0, "Reversibility");
#ifdef COPASI_SSA
      bool ssatask = pTask->getMethod()->getSubType() == CCopasiMethod::stoichiometricStabilityAnalysis;

      if (ssatask)
        Ui::CQEFMWidget::mpListView->setColumnText(0, "Stability");

#endif
      unsigned C_INT32 j;

      for (j = 0; j < noOfModesRows; j++)
        {
#ifdef COPASI_SSA

          if (ssatask)
            {
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

              item = new Q3ListViewItem(Ui::CQEFMWidget::mpListView, "title");
            }
          else
            {
#endif // COPASI_SSA

              if (pTask->isFluxModeReversible(j) == true)
                {
                  item = new Q3ListViewItem(Ui::CQEFMWidget::mpListView, "Reversible");
                }
              else
                {
                  item = new Q3ListViewItem(Ui::CQEFMWidget::mpListView, "Irreversible");
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
  saveMethod();

  return true;
}

bool CQEFMWidget::loadTask()
{
  CEFMTask * pTask =
    dynamic_cast< CEFMTask * >(mpTask);

  if (!pTask) return false;

  loadCommon();
  loadMethod();

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
                                          "untitled.txt", "TEXT Files (*.txt)", "Save to");

      if (fileName.isEmpty()) return;

      // Checks whether the file exists
      Answer = checkSelection(fileName);

      if (Answer == QMessageBox::Cancel) return;
    }

  std::ofstream file(utf8ToLocale(TO_UTF8(fileName)).c_str());

  if (file.fail())
    return;

  if (mpTask != NULL)
    file << mpTask->getResult();

  return;
}

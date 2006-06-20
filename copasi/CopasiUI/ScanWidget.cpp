/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/CopasiUI/Attic/ScanWidget.cpp,v $
   $Revision: 1.199 $
   $Name:  $
   $Author: shoops $
   $Date: 2006/06/20 13:18:23 $
   End CVS Header */

// Copyright � 2005 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

//***  In this file I have put "//+++" in all places where something has to be added
//***  if a new scan item is introduced.

#include <qfiledialog.h>
#include <qvariant.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qapplication.h>

#include "copasi.h"

#include "ScanWidget.h"
#include "scan/CScanTask.h"
#include "scan/CScanProblem.h"
#include "scan/CScanMethod.h"
#include "model/CModel.h"
#include "listviews.h"
#include "CopasiDataModel/CCopasiDataModel.h"
#include "ScanItemWidget.h"
#include "ObjectBrowserDialog.h"
#include "ObjectBrowserItem.h"
#include "DefaultplotDialog.h"
#include "copasiui3window.h"
#include "CReportDefinitionSelect.h"

//#include "SteadyStateWidget.h"
//#include "TrajectoryWidget.h"
//#include "trajectory/CTrajectoryTask.h"
//#include "trajectory/CTrajectoryProblem.h"
//#include "steadystate/CSteadyStateTask.h"
//#include "steadystate/CSteadyStateProblem.h"

#include "report/CKeyFactory.h"
//#include "CReportDefinitionSelect.h"
#include "qtUtilities.h"
#include "CProgressBar.h"
#include "utilities/CCopasiException.h"
#include "CScanContainerWidget.h"
#include "utilities/CopasiTime.h"

//+++
//#include "CScanWidgetBreak.h"
#include "CScanWidgetRandom.h"
#include "CScanWidgetRepeat.h"
#include "CScanWidgetScan.h"
#include "CScanWidgetTask.h"

#include "./icons/scanwidgetbuttonicon.xpm"

ScanWidget::ScanWidget(QWidget* parent, const char* name, WFlags f)
    : CopasiWidget(parent, name, f),
    pParent(parent)
{
  if (!name)
    setName("ScanWidget");

  setCaption(trUtf8("ScanWidget"));
  ScanWidgetLayout = new QGridLayout(this, 1, 1, 11, 6, "ScanWidgetLayout");

  //*****************

  QHBoxLayout* tmpLayout = new QHBoxLayout();

  taskNameLabel = new QLabel(this, "taskNameLabel");
  //taskNameLabel->setText(trUtf8("Task Name"));
  taskNameLabel->setText(trUtf8("<h2>Parameter Scan</h2>"));
  taskNameLabel->setAlignment(int(QLabel::AlignVCenter
                                  | QLabel::AlignLeft));
  //SteadyStateWidgetLayout->addWidget(taskNameLabel, 0, 0);
  tmpLayout->addWidget(taskNameLabel);

  QSpacerItem* tmpSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  tmpLayout->addItem(tmpSpacer);

  bExecutable = new QCheckBox(this, "bExecutable");
  bExecutable->setText(trUtf8("executable"));
  // this is the child widget to edit an steadystatetask
  bExecutable->setChecked(parent == NULL);
  bExecutable->setEnabled(parent != NULL);
  //SteadyStateWidgetLayout->addWidget(bExecutable, 0, 2);
  tmpLayout->addWidget(bExecutable);

  ScanWidgetLayout->addMultiCellLayout(tmpLayout, 0, 0, 0, 2);

  //***********

  //TextLabel1 = new QLabel(this, "TextLabel1");
  //TextLabel1->setText("<h2>Parameter Scan</h2>");
  //ScanWidgetLayout->addWidget(TextLabel1, 0, 0);

  //**********************

  tmpLayout = new QHBoxLayout();

  tmpSpacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
  tmpLayout->addItem(tmpSpacer);

  QLabel* tmpLabel = new QLabel(this);
  tmpLabel->setText("New scan item: ");
  tmpLayout->addWidget(tmpLabel);

  comboType = new QComboBox(this);
  //+++
  comboType->insertItem("Parameter Scan");
  comboType->insertItem("Repeat");
  comboType->insertItem("Random distribution");
  //comboType->insertItem("Output separator");
  tmpLayout->addWidget(comboType);

  buttonNewItem = new QPushButton(this, "buttonNewItem");
  buttonNewItem->setText("... Create!");
  //ScanWidgetLayout->addWidget(buttonNewItem, 1, 2);
  tmpLayout->addWidget(buttonNewItem);

  ScanWidgetLayout->addMultiCellLayout(tmpLayout, 1, 1, 0, 2);

  //sExecutable = new QCheckBox(this, "sExecutable");
  //sExecutable->setText(trUtf8("Scan Executable"));

  //ScanWidgetLayout->addWidget(sExecutable, 0, 2);

  //**********************
  Layout2 = new QHBoxLayout(0, 0, 6, "Layout2");

  scanButton = new QPushButton(this, "scanButton");
  scanButton->setText(trUtf8("Run"));
  Layout2->addWidget(scanButton);

  cancelChange = new QPushButton(this, "cancelChange");
  cancelChange->setText(trUtf8("Revert"));
  Layout2->addWidget(cancelChange);

  reportDefinitionButton = new QPushButton(this, "ReportDefinition");
  reportDefinitionButton->setText(trUtf8("Report"));
  Layout2->addWidget(reportDefinitionButton);

  outputDefinitionButton = new QPushButton(this, "OutputDefinition");
  outputDefinitionButton->setText(trUtf8("Output Assistant"));
  Layout2->addWidget(outputDefinitionButton);

  ScanWidgetLayout->addMultiCellLayout(Layout2, 6, 6, 0, 2);

  //*****************************

  Layout24 = new QHBoxLayout(0, 0, 6, "Layout24");

  scrollview = new CScanContainerWidget(this);
  Layout24->addWidget(scrollview);

  ScanWidgetLayout->addMultiCellLayout(Layout24, 4, 4, 0, 2);

  // tab order
  /*setTabOrder(taskName, sExecutable);
  setTabOrder(sExecutable, steadyState);
  setTabOrder(steadyState, trajectory);
  setTabOrder(trajectory, addButton);
  setTabOrder(downButton, scrollview);
  setTabOrder(scrollview, ObjectListBox);
  setTabOrder(ObjectListBox, scanButton);
  setTabOrder(scanButton, cancelChange);
  setTabOrder(cancelChange, reportDefinitionButton);*/

  connect(scanButton, SIGNAL(clicked()), this, SLOT(runScanTask()));
  connect(cancelChange, SIGNAL(clicked()), this, SLOT(CancelChangeButton()));
  connect(reportDefinitionButton, SIGNAL(clicked()), this, SLOT(ReportDefinitionClicked()));
  connect(outputDefinitionButton, SIGNAL(clicked()), this, SLOT(outputDefinitionClicked()));
  connect(buttonNewItem, SIGNAL(clicked()), this, SLOT(slotAddItem()));

  //connect(sExecutable, SIGNAL(clicked()), this, SLOT(ScanCheckBoxClicked()));

  /*sExecutable->setEnabled(false);
  scanButton->setEnabled(true);*/

  //reportDefinitionButton->setEnabled(false);
}

ScanWidget::~ScanWidget()
{}

void ScanWidget::CancelChangeButton()
{
  loadScan();
}

void ScanWidget::ScanCheckBoxClicked()
{
  /*  CScanTask* scanTask =
      dynamic_cast< CScanTask * >(GlobalKeys.get(scanTaskKey));
    scanTask->setRequested(sExecutable->isChecked());
    scanButton->setEnabled(sExecutable->isChecked());*/
}

void ScanWidget::runScanTask()
{
  CScanTask* scanTask =
    dynamic_cast< CScanTask * >(GlobalKeys.get(scanTaskKey));
  if (!scanTask) return;

  //((CScanProblem*)(scanTask->getProblem()))->createDebugScan(dataModel->getModel());

  // save the state of the widget
  saveScan();

  static_cast<CopasiUI3Window *>(qApp->mainWidget())->autoSave();
  static_cast<CopasiUI3Window *>(qApp->mainWidget())->suspendAutoSave(true);

  scanTask->initialize(CCopasiTask::OUTPUT_COMPLETE, NULL);

  setCursor(Qt::WaitCursor);
  CProgressBar * tmpBar = new CProgressBar();
  scanTask->setCallBack(tmpBar);

  CCopasiTimeVariable time = CCopasiTimeVariable::getCurrentWallTime();

  try
    {
      scanTask->process(true);
    }
  catch (CCopasiException Exception)
    {
      //std::cout << std::endl << "exception in scan task" << std::endl;
      //TODO: message box
    }

  //std::cout << "*************** The scan took "
  //<< (CCopasiTimeVariable::getCurrentWallTime() - time).isoFormat()
  //<< " ***************" << std::endl;

  //should be renamed?
  scanTask->restore();

  tmpBar->finish(); pdelete(tmpBar);
  CCopasiDataModel::Global->finish();

  CCopasiMessage::clearDeque();
  CCopasiDataModel::Global->finish();
  protectedNotify(ListViews::STATE, ListViews::CHANGE,
                  CCopasiDataModel::Global->getModel()->getKey());

  unsetCursor();
  static_cast<CopasiUI3Window *>(qApp->mainWidget())->suspendAutoSave(false);

  return;
}

bool ScanWidget::loadScan()
{
  CScanTask* scanTask =
    dynamic_cast< CScanTask * >(GlobalKeys.get(scanTaskKey));
  if (!scanTask) return false;

  CScanProblem *scanProblem = dynamic_cast<CScanProblem *>(scanTask->getProblem());
  if (!scanProblem) return false;

  bExecutable->setChecked(scanTask->isScheduled());

  mModel = scanProblem->getModel();

  scrollview->clearWidgetList();

  //std::vector<QWidget*> & widgetList = scrollview->getWidgetList();

  //+++
  CScanWidgetScan* tmp1;
  CScanWidgetRepeat* tmp2;
  CScanWidgetRandom* tmp3;
  //CScanWidgetBreak* tmp4;

  // the scan items
  unsigned C_INT32 i, imax = scanProblem->getNumberOfScanItems();
  for (i = 0; i < imax; ++i)
    {
      void* pTmp;
      if (!(pTmp = scanProblem->getScanItem(i)->getValue("Type").pVOID)) return false;
      CScanProblem::Type type = *(CScanProblem::Type*)pTmp;

      switch (type)
        {
          //+++
        case CScanProblem::SCAN_LINEAR:
          tmp1 = new CScanWidgetScan(scrollview);
          tmp1->initFromScanItem(scanProblem->getScanItem(i), CCopasiDataModel::Global->getModel());
          scrollview->addWidget(tmp1);
          break;

        case CScanProblem::SCAN_REPEAT:
          tmp2 = new CScanWidgetRepeat(scrollview);
          tmp2->initFromScanItem(scanProblem->getScanItem(i));
          scrollview->addWidget(tmp2);
          break;

        case CScanProblem::SCAN_RANDOM:
          tmp3 = new CScanWidgetRandom(scrollview);
          tmp3->initFromScanItem(scanProblem->getScanItem(i), CCopasiDataModel::Global->getModel());
          scrollview->addWidget(tmp3);
          break;

          /*case CScanProblem::SCAN_BREAK:
            tmp4 = new CScanWidgetBreak(scrollview);
            tmp4->initFromScanItem(scanProblem->getScanItem(i));
            scrollview->addWidget(tmp4);
            break;*/

        default:
;
        }
    }

  // the widget for the subtask
  CScanWidgetTask* tmpT = new CScanWidgetTask(scrollview);
  tmpT->initFromScanProblem(scanProblem, CCopasiDataModel::Global->getModel());
  scrollview->addWidget(tmpT, false); //false: no control buttons (up/down/del)

  //scrollview->updateFromWidgetList();
  return true;
}

bool ScanWidget::slotAddItem()
{
  int totalRows = -1;
  //+++
  CScanWidgetScan* tmp1;
  CScanWidgetRepeat* tmp2;
  CScanWidgetRandom* tmp3;
  //CScanWidgetBreak* tmp4;

  int intType = comboType->currentItem();
  CScanProblem::Type type;
  switch (intType)
    {
      //+++
    case 0:
      type = CScanProblem::SCAN_LINEAR;
      break;
    case 1:
      type = CScanProblem::SCAN_REPEAT;
      break;
    case 2:
      type = CScanProblem::SCAN_RANDOM;
      break;
      /*case 3:
        type = CScanProblem::SCAN_BREAK;
        break;*/
    default:
      type = CScanProblem::SCAN_LINEAR;
    }

  //create item to get the default values
  CScanProblem* tmpProblem = new CScanProblem();
  CCopasiParameterGroup* tmpItem = tmpProblem->createScanItem(type, 10);

  switch (type)
    {
      //+++
    case CScanProblem::SCAN_LINEAR:
      tmp1 = new CScanWidgetScan(scrollview);
      tmp1->initFromScanItem(tmpItem, CCopasiDataModel::Global->getModel());
      scrollview->insertWidget(tmp1);
      totalRows = scrollview->numRows();
      scrollview->ensureCellVisible(totalRows - 1, 0);
      tmp1->lineEditMin->setFocus();
      break;

    case CScanProblem::SCAN_REPEAT:
      tmp2 = new CScanWidgetRepeat(scrollview);
      tmp2->initFromScanItem(tmpItem);
      scrollview->insertWidget(tmp2);
      totalRows = scrollview->numRows();
      scrollview->ensureCellVisible(totalRows - 1, 0);
      tmp2->lineEditNumber->setFocus();
      break;

    case CScanProblem::SCAN_RANDOM:
      tmp3 = new CScanWidgetRandom(scrollview);
      tmp3->initFromScanItem(tmpItem, CCopasiDataModel::Global->getModel());
      scrollview->insertWidget(tmp3);
      totalRows = scrollview->numRows();
      scrollview->ensureCellVisible(totalRows - 1, 0);
      tmp3->lineEditMin->setFocus();
      break;

      /*case CScanProblem::SCAN_BREAK:
        tmp4 = new CScanWidgetBreak(scrollview);
        tmp4->initFromScanItem(tmpItem);
        scrollview->insertWidget(tmp4);
        break;*/

    default:
;
    }

  if (tmpProblem) delete tmpProblem;

  return true;
}

bool ScanWidget::saveScan() const
  {
    CScanTask* scanTask =
      dynamic_cast< CScanTask * >(GlobalKeys.get(scanTaskKey));
    if (!scanTask) return false;

    CScanProblem *scanProblem = dynamic_cast<CScanProblem *>(scanTask->getProblem());
    if (!scanProblem) return false;

    //set the executable flag
    bool bScheduled = bExecutable->isChecked();
    scanTask->setScheduled(bScheduled);

    scanProblem->clearScanItems();

    const std::vector<QWidget*> & widgetList = scrollview->getWidgetList();

    unsigned C_INT32 i, imax = widgetList.size();
    for (i = 0; i < imax; ++i)
      {
        //+++

        // item: scan parameter
        const CScanWidgetScan* tmp1 = dynamic_cast<CScanWidgetScan*>(widgetList[i]);
        if (tmp1) {tmp1->saveToScanItem(scanProblem); continue;}

        // item: repeat
        const CScanWidgetRepeat* tmp2 = dynamic_cast<CScanWidgetRepeat*>(widgetList[i]);
        if (tmp2) {tmp2->saveToScanItem(scanProblem); continue;}

        // item: random
        const CScanWidgetRandom* tmp3 = dynamic_cast<CScanWidgetRandom*>(widgetList[i]);
        if (tmp3) {tmp3->saveToScanItem(scanProblem); continue;}

        // item: break
        /*const CScanWidgetBreak* tmp4 = dynamic_cast<CScanWidgetBreak*>(widgetList[i]);
        if (tmp4) {tmp4->saveToScanItem(scanProblem); continue;}*/

        // the subtask
        const CScanWidgetTask* tmpT = dynamic_cast<CScanWidgetTask*>(widgetList[i]);
        if (tmpT) {tmpT->saveToScanProblem(scanProblem); continue;}

        return false;
      }
    //scrollview->updateFromWidgetList();

    //set initial state
    CCopasiDataModel::Global->getModel()->compileIfNecessary();
    //scanProblem->setInitialState(CCopasiDataModel::Global->getModel()->getInitialState());

    // :TODO Bug 322: This should only be called when actual changes have been saved.
    CCopasiDataModel::Global->changed();

    return true;
  }

void ScanWidget::ReportDefinitionClicked()
{
  CScanTask* task =
    dynamic_cast< CScanTask * >(GlobalKeys.get(scanTaskKey));
  assert(task);

  CReportDefinitionSelect * pSelectDlg = new CReportDefinitionSelect(pParent);
  pSelectDlg->setReport(&task->getReport());
  pSelectDlg->loadReportDefinitionVector();
  pSelectDlg->exec();

  delete pSelectDlg;
}

void ScanWidget::outputDefinitionClicked()
{
  CCopasiTask* task =
    dynamic_cast< CCopasiTask * >(GlobalKeys.get(scanTaskKey));
  assert(task);

  DefaultPlotDialog * pDlg = new DefaultPlotDialog(this);
  pDlg->setTask(task);
  if (pDlg->exec() == QDialog::Accepted)
    {
      //std::cout << "plot created" << std::endl;
      protectedNotify(ListViews::PLOT, ListViews::ADD, "");
    }
  else
    {
      //std::cout << "no plot created" << std::endl;
    }

  if (pDlg)delete pDlg;
}

//************* CCopasiWidget interface *******************************

bool ScanWidget::enter(const std::string & key)
{
  CScanTask* scanTask =
    dynamic_cast< CScanTask * >(GlobalKeys.get(key));
  if (!scanTask) return false;

  scanTaskKey = key;

  //pSteadyStateWidget->enter(SteadyStateKey);
  //pTrajectoryWidget->enter(TrajectoryKey);

  // ((CScanProblem*)(scanTask->getProblem()))->createDebugScan(dataModel->getModel());

  loadScan();

  return true;
}

bool ScanWidget::leave()
{
  //CScanTask* scanTask =
  //  dynamic_cast< CScanTask * >(GlobalKeys.get(scanTaskKey));
  //if (!scanTask) return false;

  return saveScan();
}

bool ScanWidget::update(ListViews::ObjectType objectType, ListViews::Action C_UNUSED(action), const std::string & C_UNUSED(key))
{
  if (mIgnoreUpdates) return true;

  switch (objectType)
    {
    case ListViews::MODEL:
      // check if there is a list of Report Defs
      CReportDefinitionVector* pReportDefinitionVector;
      pReportDefinitionVector = CCopasiDataModel::Global->getReportDefinitionList();
      if (pReportDefinitionVector)
        reportDefinitionButton->setEnabled(true);
      break;
    default:
      break;
    }
  return true;
}

//***********************************************************

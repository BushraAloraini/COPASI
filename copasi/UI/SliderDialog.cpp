/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/SliderDialog.cpp,v $
   $Revision: 1.54 $
   $Name:  $
   $Author: ssahle $ 
   $Date: 2005/10/11 08:55:37 $
   End CVS Header */

#include <iostream>
#include <sstream>
#include <mathematics.h>

#include "qhbox.h"
#include "qvbox.h"
#include "qlistbox.h"
#include "qcheckbox.h"
#include "qpushbutton.h"
#include "qstring.h"
#include "qslider.h"
#include "qlayout.h"
#include "qlabel.h"
#include "qobjectlist.h"
#include "qtooltip.h"
#include "qpopupmenu.h"
#include "qlayout.h"
#include "qmessagebox.h"

#include "SliderDialog.h"
#include "copasiui3window.h"
#include "TrajectoryWidget.h"
#include "SteadyStateWidget.h"
#include "SliderSettingsDialog.h"
#include "xml/CCopasiXMLInterface.h"
#include "CopasiDataModel/CCopasiDataModel.h"
#include "CopasiSlider.h"
#include "mathematics.h"
#include "utilities/CCopasiTask.h"
#include "utilities/CCopasiProblem.h"
#include "report/CCopasiObjectName.h"
#include "qtUtilities.h"
#include "trajectory/CTrajectoryTask.h"
#include "steadystate/CSteadyStateTask.h"
#include "utilities/CSlider.h"
#include "model/CModel.h"

C_INT32 SliderDialog::numMappings = 4;
C_INT32 SliderDialog::folderMappings[][2] = {
      {21, 21}, {211, 21}, {23, 23}, {231, 23}
    };

C_INT32 SliderDialog::numKnownTasks = 2;
C_INT32 SliderDialog::knownTaskIDs[] = {21, 23};
char* SliderDialog::knownTaskNames[] = {"Steady State", "Time Course"};

SliderDialog::SliderDialog(QWidget* parent): QDialog(parent),
    runTaskButton(NULL),
    newSliderButton(NULL),
    autoRunCheckBox(NULL),
    mpAutoModifyRangesCheckBox(NULL),
    scrollView(NULL),
    sliderBox(NULL),
    contextMenu(NULL),
    currSlider(NULL),
    currentFolderId(0),
    mSliderValueChanged(false),
    mSliderPressed(false)
{
  this->setWFlags(this->getWFlags() | WStyle_StaysOnTop);
  QVBoxLayout* mainLayout = new QVBoxLayout(this);
  mainLayout->setMargin(5);
  QHBoxLayout* layout2 = new QHBoxLayout(0);
  layout2->setMargin(3);
  layout2->addStretch();
  this->newSliderButton = new QPushButton(this);
  this->newSliderButton->setText("new slider");
  this->newSliderButton->setEnabled(true);
  layout2->addWidget(this->newSliderButton);
  layout2->addStretch();
  mainLayout->addLayout(layout2);

  this->scrollView = new QScrollView(this);
  this->scrollView->setResizePolicy(QScrollView::AutoOneFit);
  this->sliderBox = new QVBox(0);
  this->sliderBox->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
  this->sliderBox->layout()->setAutoAdd(false);
  this->scrollView->addChild(this->sliderBox);
  mainLayout->addWidget(this->scrollView);

  QHBoxLayout* layout1 = new QHBoxLayout(0);
  layout1->addStretch();
  this->mpAutoModifyRangesCheckBox = new QCheckBox(this);
  this->mpAutoModifyRangesCheckBox->setChecked(true);
  this->mpAutoModifyRangesCheckBox->setText("update ranges");
  layout1->addWidget(this->mpAutoModifyRangesCheckBox);
  layout1->addStretch();
  mainLayout->addSpacing(10);
  mainLayout->addLayout(layout1);

  layout1 = new QHBoxLayout(0);
  layout1->addStretch();
  this->autoRunCheckBox = new QCheckBox(this);
  this->autoRunCheckBox->setChecked(true);
  this->autoRunCheckBox->setText("update automatically");
  layout1->addWidget(this->autoRunCheckBox);
  layout1->addStretch();
  mainLayout->addSpacing(10);
  mainLayout->addLayout(layout1);

  layout2 = new QHBoxLayout(0);
  layout2->addStretch();
  this->runTaskButton = new QPushButton(this);
  this->runTaskButton->setText("run task");
  this->runTaskButton->setEnabled(true);
  layout2->addWidget(this->runTaskButton);
  layout2->addStretch();
  mainLayout->addLayout(layout2);

  this->contextMenu = new QPopupMenu(this);
  this->contextMenu->insertItem("Add New Slider", this, SLOT(createNewSlider()));
  this->contextMenu->insertItem("Remove Slider", this, SLOT(removeSlider()));
  this->contextMenu->insertItem("Edit Slider", this, SLOT(editSlider()));
  this->contextMenu->insertItem("Reset Value", this, SLOT(resetValue()));
  this->contextMenu->insertItem("Set new default value", this, SLOT(setDefault()));

  this->sliderMap[ -1].push_back(new QLabel("<p>There are no sliders available for this task. If you select one of the tasks that supports sliders in the copasi object tree, this dialog will become active.</p>", this->sliderBox));

  this->taskMap[23] = &SliderDialog::runTimeCourse;
  this->taskMap[21] = &SliderDialog::runSteadyStateTask;

  connect(runTaskButton, SIGNAL(clicked()), this, SLOT(runTask()));
  connect(newSliderButton, SIGNAL(clicked()), this, SLOT(createNewSlider()));
  this->setCurrentFolderId(-1);
  this->init();
}

void SliderDialog::contextMenuEvent(QContextMenuEvent* e)
{
  CopasiSlider* pSlider = this->findCopasiSliderAtPosition(e->pos());
  if (pSlider)
    {
      this->contextMenu->setItemEnabled(this->contextMenu->idAt(0), false);
      this->contextMenu->setItemEnabled(this->contextMenu->idAt(1), true);
      this->contextMenu->setItemEnabled(this->contextMenu->idAt(2), true);
      this->setCurrentSlider(pSlider);
    }
  else
    {
      this->contextMenu->setItemEnabled(this->contextMenu->idAt(0), true);
      this->contextMenu->setItemEnabled(this->contextMenu->idAt(1), false);
      this->contextMenu->setItemEnabled(this->contextMenu->idAt(2), false);
    }
  this->contextMenu->popup(e->globalPos());
}

void SliderDialog::setCurrentSlider(CopasiSlider* pSlider)
{
  this->currSlider = pSlider;
  if (this->currSlider)
    {
      this->currSlider->setFocus();
    }
}

CopasiSlider* SliderDialog::findCopasiSliderAtPosition(const QPoint& p)
{
  QWidget* pWidget = this->childAt(p);
  CopasiSlider* pSlider = NULL;
  while (pWidget != this && !pSlider)
    {
      pSlider = dynamic_cast<CopasiSlider*>(pWidget);
      pWidget = (QWidget*)pWidget->parent();
    }
  return pSlider;
}

void SliderDialog::createNewSlider()
{
  SliderSettingsDialog* pSettingsDialog = new SliderSettingsDialog(this);
  pSettingsDialog->setModel(CCopasiDataModel::Global->getModel());
  // set the list of sliders that is already known
  std::vector<CSlider*>* pVector = this->getCSlidersForCurrentFolderId();
  pSettingsDialog->setDefinedSliders(*pVector);

  if (pSettingsDialog->exec() == QDialog::Accepted)
    {
      CSlider* pCSlider = pSettingsDialog->getSlider();
      if (pCSlider)
        {
          if (!this->equivalentSliderExists(pCSlider))
            {
              // add the new slider
              this->addSlider(pCSlider);
            }
          else
            {
              // show a message dialog
              if (QMessageBox::information(NULL, "Slider Exists", "A slider for this object already exists.\nDo you want to update this slider?", QMessageBox::Yes | QMessageBox::Default, QMessageBox::No | QMessageBox::Escape, QMessageBox::NoButton) == QMessageBox::Yes)
                {
                  // update the slider
                  this->currSlider->update();
                }
            }
        }
    }
  else
    {
      this->setCurrentSlider(NULL);
    }
  delete pSettingsDialog;
  delete pVector;
}

void SliderDialog::removeSlider()
{
  if (this->currSlider)
    {
      CCopasiVector<CSlider>* pSliderList = CCopasiDataModel::Global->getGUI()->pSliderList;
      unsigned int i, maxCount = pSliderList->size();
      for (i = 0; i < maxCount;++i)
        {
          CSlider* pTmpSlider = (*pSliderList)[i];
          if (pTmpSlider == this->currSlider->getCSlider())
            {
              pSliderList->remove(i);
              break;
            }
        }
      this->deleteSlider(this->currSlider);
      this->currSlider = NULL;
    }
}

void SliderDialog::deleteSlider(CopasiSlider* pSlider)
{
  if (pSlider)
    {
      std::vector<QWidget*>* v = &this->sliderMap[this->currentFolderId];
      std::vector<QWidget*>::iterator it = v->begin();
      std::vector<QWidget*>::iterator end = v->end();
      while (it != end)
        {
          if (*it == pSlider)
            {
              break;
            }
          ++it;
        }
      assert(it != end);
      v->erase(it);
      ((QVBoxLayout*)this->sliderBox->layout())->remove(pSlider);
      pdelete(pSlider);
    }
}

void SliderDialog::editSlider()
{
  SliderSettingsDialog* pSettingsDialog = new SliderSettingsDialog(this);
  pSettingsDialog->setModel(CCopasiDataModel::Global->getModel());
  // set the list of sliders that is already known
  CCopasiObject* object = (CCopasiObject*)this->getTaskForFolderId(this->currentFolderId);
  if (!object) return;
  std::vector<CSlider*>* pVector = this->getCSlidersForCurrentFolderId();
  pSettingsDialog->setDefinedSliders(*pVector);

  //pSettingsDialog->disableObjectChoosing(true);

  pSettingsDialog->setSlider(this->currSlider->getCSlider());
  if (pSettingsDialog->exec() == QDialog::Accepted)
    {
      this->addSlider(pSettingsDialog->getSlider());
      this->currSlider->updateSliderData();
      /*
      if ((!this->currSlider->isEnabled()) && this->currSlider->getCSlider()->compile())
        {
          this->currSlider->setEnabled(true);
        }
        */
    }
  delete pSettingsDialog;
  delete pVector;
}

SliderDialog::~SliderDialog()
{
  delete this->runTaskButton;
  delete this->autoRunCheckBox;
  delete this->mpAutoModifyRangesCheckBox;
  delete this->sliderBox;
  delete this->scrollView;
  unsigned int i, j, maxWidgets, maxVectors = this->sliderMap.size();
  for (i = 0; i < maxVectors;++i)
    {
      std::vector<QWidget*> v = this->sliderMap[i];
      maxWidgets = v.size();
      for (j = 0; j < maxWidgets;++j)
        {
          pdelete(v[j]);
        }
    }
}

void SliderDialog::init()
{}

void SliderDialog::addSlider(CSlider* pSlider)
{
  // check if there already is a slider for this  object
  SCopasiXMLGUI* pGUI = CCopasiDataModel::Global->getGUI();
  assert(pGUI);
  if (!this->equivalentSliderExists(pSlider))
    {
      pGUI->pSliderList->add(pSlider);
    }
  CopasiSlider* tmp = findCopasiSliderForCSlider(pSlider);
  if (!tmp)
    {
      this->setCurrentSlider(new CopasiSlider(pSlider, this->sliderBox));
      this->currSlider->installEventFilter(this);
      this->currSlider->setHidden(true);
      this->currSlider->updateSliderData();
      this->sliderMap[this->currentFolderId].push_back(this->currSlider);
      ((QVBoxLayout*)this->sliderBox->layout())->add(this->currSlider);
      connect(this->currSlider, SIGNAL(valueChanged(double)), this , SLOT(sliderValueChanged()));
      connect(this->currSlider, SIGNAL(sliderReleased()), this, SLOT(sliderReleased()));
      connect(this->currSlider, SIGNAL(sliderPressed()), this, SLOT(sliderPressed()));
      connect(this->currSlider, SIGNAL(closeClicked(CopasiSlider*)), this, SLOT(removeSlider(CopasiSlider*)));
      connect(this->currSlider, SIGNAL(editClicked(CopasiSlider*)), this, SLOT(editSlider(CopasiSlider*)));
      this->currSlider->setHidden(false);
    }
  else
    {
      this->currSlider = tmp;
    }
}

CSlider* SliderDialog::equivalentSliderExists(CSlider* pCSlider)
{
  CSlider* pResult = NULL;
  SCopasiXMLGUI* pGUI = CCopasiDataModel::Global->getGUI();
  assert(pGUI);
  unsigned i, maxCount = pGUI->pSliderList->size();
  for (i = 0; i < maxCount;++i)
    {
      CSlider* pTmpSlider = (*(pGUI->pSliderList))[i];
      if (pTmpSlider->getSliderObject() == pCSlider->getSliderObject() && pTmpSlider->getAssociatedEntityKey() == pCSlider->getAssociatedEntityKey())
        {
          pResult = pTmpSlider;
          break;
        }
    }
  return pResult;
}

CopasiSlider* SliderDialog::findCopasiSliderForCSlider(CSlider* pCSlider)
{
  CopasiSlider* pResult = NULL;
  std::vector<QWidget*> v = this->sliderMap[this->currentFolderId];
  unsigned int i, maxCount = v.size();
  CopasiSlider* pTmpSlider;
  for (i = 0; i < maxCount;++i)
    {
      pTmpSlider = dynamic_cast<CopasiSlider*>(v[i]);
      if (!pTmpSlider) break;
      if (pTmpSlider->getCSlider() == pCSlider)
        {
          pResult = pTmpSlider;
          break;
        }
    }
  return pResult;
}

void SliderDialog::setCurrentFolderId(C_INT32 id)
{
  id = this->mapFolderId2EntryId(id);
  if (id == this->currentFolderId) return;
  if (id == -1)
    {
      this->setEnabled(false);
    }
  else
    {
      this->setEnabled(true);
    }

  this->clearSliderBox();

  this->currentFolderId = id;

  this->fillSliderBox();
}

void SliderDialog::fillSliderBox()
{
  std::vector<QWidget*> v = this->sliderMap[this->currentFolderId];
  if (this->currentFolderId != -1)
    {
      std::vector<CSlider*>* pVector = this->getCSlidersForCurrentFolderId();
      // maybe other program parts have added or deleted some sliders
      assert(pVector);
      unsigned int i, j, maxSliders, maxWidgets;
      maxWidgets = v.size();
      maxSliders = pVector->size();
      SCopasiXMLGUI* pGUI = CCopasiDataModel::Global->getGUI();
      assert(pGUI);
      // add CopasiSlider for all CSliders that don't have one.
      bool issueWarning = false;
      for (i = 0; i < maxSliders;++i)
        {
          bool found = false;
          if (!(*pVector)[i]->compile())
            {
              issueWarning = true;
            }
          for (j = 0; j < maxWidgets; j++)
            {
              CopasiSlider* pTmpSlider = dynamic_cast<CopasiSlider*>(v[j]);
              if (!pTmpSlider) break;
              if (pTmpSlider->getCSlider() == (*pVector)[i])
                {
                  found = true;
                  break;
                }
            }
          if (!found)
            {
              this->setCurrentSlider(new CopasiSlider((*pVector)[i], this->sliderBox));
              connect(this->currSlider, SIGNAL(valueChanged(double)), this , SLOT(sliderValueChanged()));
              connect(this->currSlider, SIGNAL(sliderReleased()), this, SLOT(sliderReleased()));
              connect(this->currSlider, SIGNAL(sliderPressed()), this, SLOT(sliderPressed()));
              connect(this->currSlider, SIGNAL(closeClicked(CopasiSlider*)), this, SLOT(removeSlider(CopasiSlider*)));
              connect(this->currSlider, SIGNAL(editClicked(CopasiSlider*)), this, SLOT(editSlider(CopasiSlider*)));
              this->currSlider->installEventFilter(this);
              this->currSlider->setHidden(true);
              this->sliderMap[this->currentFolderId].push_back(this->currSlider);
            }
        }
      // delete CopasiSliders which have no correponding CSlider
      for (j = 0; j < maxWidgets;++j)
        {
          bool found = false;
          for (i = 0; i < maxSliders; i++)
            {
              CopasiSlider* pTmpSlider = dynamic_cast<CopasiSlider*>(v[j]);
              if (!pTmpSlider) break;
              if (pTmpSlider->getCSlider() == (*pVector)[i])
                {
                  found = true;
                  break;
                }
            }
          if (!found)
            {
              CopasiSlider* pTmpSlider = dynamic_cast<CopasiSlider*>(v[j]);
              assert(pTmpSlider);
              this->deleteSlider(pTmpSlider);
            }
        }
      if (issueWarning)
        {
          QMessageBox::warning(NULL, "Invalid Slider", "One or more sliders are invalid\n and have been disabled!", QMessageBox::Ok, QMessageBox::NoButton);
        }

      delete pVector;
    }
  v = this->sliderMap[this->currentFolderId];
  unsigned int i, maxCount = v.size();
  for (i = maxCount; i != 0;--i)
    {
      QWidget* widget = v[i - 1];
      widget->setHidden(true);
      ((QVBoxLayout*)this->sliderBox->layout())->insertWidget(0, widget);
      this->setCurrentSlider(dynamic_cast<CopasiSlider*>(widget));
      if (this->currSlider)
        {
          this->currSlider->updateSliderData();
        }
      widget->setHidden(false);
    }
}

C_INT32 SliderDialog::mapFolderId2EntryId(C_INT32 folderId) const
  {
    C_INT32 id = -1;
    int counter;
    for (counter = 0; counter < SliderDialog::numMappings;++counter)
      {
        if (SliderDialog::folderMappings[counter][0] == folderId)
          {
            id = SliderDialog::folderMappings[counter][1];
            break;
          }
      }
    return id;
  }

void SliderDialog::runTask()
{
  if (this->taskMap.find(this->currentFolderId) != this->taskMap.end())
    {
      this->setEnabled(false);
      this->updateAllSliders();
      ((this)->*(this->taskMap[this->currentFolderId]))();
      this->updateAllSliders();
      this->setEnabled(true);
    }
}

void SliderDialog::sliderValueChanged()
{
  this->mSliderValueChanged = true;
  if ((!this->mSliderPressed) && this->autoRunCheckBox->isChecked())
    {
      //this->runTask();
      this->mSliderValueChanged = false;
    }
}

void SliderDialog::sliderReleased()
{
  if (this->mSliderValueChanged && this->autoRunCheckBox->isChecked())
    {
      this->runTask();
      this->mSliderValueChanged = false;
    }
  this->mSliderPressed = false;
}

void SliderDialog::sliderPressed()
{
  this->mSliderPressed = true;
}

void SliderDialog::runTimeCourse()
{
  CopasiUI3Window* p = dynamic_cast<CopasiUI3Window*>(this->parent());
  if (p)
    {
      p->getTrajectoryWidget()->enter((*CCopasiDataModel::Global->getTaskList())["Time-Course"]->getKey());
      p->getTrajectoryWidget()->runTrajectoryTask();
    }
}

void SliderDialog::runSteadyStateTask()
{
  CopasiUI3Window* p = dynamic_cast<CopasiUI3Window*>(this->parent());
  if (p)
    {
      p->getSteadyStateWidget()->runTask();
    }
}
void SliderDialog::closeEvent(QCloseEvent* e)
{
  QDialog::closeEvent(e);
  CopasiUI3Window* p = dynamic_cast<CopasiUI3Window*>(this->parent());
  if (p)
    {
      p->slotToggleSliders();
    }
}

CCopasiTask* SliderDialog::getTaskForFolderId(C_INT32 folderId)
{
  folderId = mapFolderId2EntryId(folderId);
  CCopasiTask* task = NULL;
  switch (folderId)
    {
    case 21:
      task = dynamic_cast<CSteadyStateTask *>((*CCopasiDataModel::Global->getTaskList())["Steady-State"]);
      break;
    case 23:
      task = dynamic_cast<CTrajectoryTask *>((*CCopasiDataModel::Global->getTaskList())["Time-Course"]);
      break;
    default:
      task = NULL;
      break;
    }
  return task;
}

void SliderDialog::updateAllSliders()
{
  if (this->currentFolderId == -1) return;

  bool autoModify = this->mpAutoModifyRangesCheckBox->isChecked();
  std::vector<QWidget*> v = this->sliderMap[this->currentFolderId];
  unsigned int i, maxCount = v.size();
  for (i = 0; i < maxCount;++i)
    {
      CopasiSlider* pCopasiSlider = dynamic_cast<CopasiSlider*>(v[i]);
      if (pCopasiSlider)
        {
          pCopasiSlider->updateValue(autoModify);
        }
    }
}

void SliderDialog::removeSlider(CopasiSlider* slider)
{
  this->setCurrentSlider(slider);
  this->removeSlider();
}

void SliderDialog::editSlider(CopasiSlider* slider)
{
  this->setCurrentSlider(slider);
  this->editSlider();
}

std::vector<CSlider*>* SliderDialog::getCSlidersForObject(CCopasiObject* pObject, std::vector<CSlider*>* pVector) const
  {
    SCopasiXMLGUI* pGUI = CCopasiDataModel::Global->getGUI();
    assert(pGUI);
    CCopasiVector<CSlider>* pSliderList = pGUI->pSliderList;
    assert(pSliderList);
    unsigned int i, maxSliders = pSliderList->size();
    bool issueWarning = false;
    for (i = 0; i < maxSliders;++i)
      {
        CSlider* pSlider = (*pSliderList)[i];
        if (pSlider->getAssociatedEntityKey() == CCopasiDataModel::Global->getModel()->getKey() || pSlider->getAssociatedEntityKey() == pObject->getKey())
          {
            if (!pSlider->compile())
              {
                issueWarning = true;
              }
            pVector->push_back(pSlider);
          }
      }
    return pVector;
  }

void SliderDialog::clearSliderBox()
{
  std::vector<QWidget*> v = this->sliderMap[this->currentFolderId];
  unsigned int i, maxCount = v.size();
  for (i = 0; i < maxCount;++i)
    {
      QWidget* widget = v[i];
      widget->setHidden(true);
      ((QVBoxLayout*)this->sliderBox->layout())->remove(widget);
    }
}

std::vector<CSlider*>* SliderDialog::getCSlidersForCurrentFolderId()
{
  CCopasiObject* object = (CCopasiObject*)this->getTaskForFolderId(this->currentFolderId);
  if (!object) return NULL;
  std::vector<CSlider*>* pVector = new std::vector<CSlider*>();
  pVector = this->getCSlidersForObject(object, pVector);
  return pVector;
}

bool SliderDialog::eventFilter(QObject*, QEvent* event)
{
  QMouseEvent* pQME = dynamic_cast<QMouseEvent*>(event);
  if (pQME && pQME->type() == QEvent::MouseButtonPress && pQME->button() == Qt::LeftButton)
    {
      CopasiSlider* pSlider = this->findCopasiSliderAtPosition(this->mapFromGlobal(pQME->globalPos()));
      assert(pSlider);
      this->setCurrentSlider(pSlider);
    }
  return false;
}

void SliderDialog::resetValue()
{
  this->currSlider->resetValue();
}

void SliderDialog::setDefault()
{
  this->currSlider->setOriginalValue(this->currSlider->value());
}

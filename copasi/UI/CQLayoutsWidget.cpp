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

#include "CQLayoutsWidget.h"

#include <iostream>

#include "CQMessageBox.h"
#include "CQLayoutsDM.h"
#include "CQSortFilterProxyModel.h"
#include "CQPushButtonDelegate.h"

#include "listviews.h"
#include "qtUtilities.h"
#include "resourcesUI/CQIconResource.h"

#include "copasi/layout/CLayout.h"
#include "copasi/layout/CListOfLayouts.h"
#include "copasi/model/CModel.h"
#include "copasi/report/CKeyFactory.h"
#include "copasi/CopasiDataModel/CDataModel.h"
#include <copasi/commandline/CConfigurationFile.h>
#include "copasi/core/CRootContainer.h"

#include "copasi/layoutUI/CQNewMainWindow.h"

#ifndef DISABLE_QT_LAYOUT_RENDERING
# include <qlayout/CQAnimationWindow.h>
#endif //DISABLE_QT_LAYOUT_RENDERING

#include "copasi/layoutUI/CQAutolayoutWizard.h"

#include <copasi/UI/copasiui3window.h>

#include <sstream>

CQLayoutsWidget::CQLayoutsWidget(QWidget *parent)
  : CopasiWidget(parent)
{
  setupUi(this);
  // Create Source Data Model.
  mpLayoutsDM = new CQLayoutsDM(this);
  // Create the Proxy Model for sorting/filtering and set its properties.
  mpProxyModel = new CQSortFilterProxyModel();
  mpProxyModel->setSortCaseSensitivity(Qt::CaseInsensitive);
  mpProxyModel->setFilterKeyColumn(-1);
  mpProxyModel->setSourceModel(mpLayoutsDM);
#if QT_VERSION >= 0x050000
  mpTblLayouts->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#else
  mpTblLayouts->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#endif
  mpTblLayouts->verticalHeader()->hide();
  mpTblLayouts->sortByColumn(COL_ROW_NUMBER, Qt::AscendingOrder);
  mpTblLayouts->setModel(mpProxyModel);
  mpPushButtonDelegate = new CQPushButtonDelegate(CQIconResource::icon(CQIconResource::unknown), QString(), CQPushButtonDelegate::PushButton, this);
  mpTblLayouts->setItemDelegateForColumn(COL_SHOW, mpPushButtonDelegate);
  // Connect the table widget
  connect(mpLayoutsDM, SIGNAL(notifyGUI(ListViews::ObjectType, ListViews::Action, const std::string)),
          this, SLOT(protectedNotify(ListViews::ObjectType, ListViews::Action, const std::string)));
  connect(mpLayoutsDM, SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
          this, SLOT(dataChanged(const QModelIndex &, const QModelIndex &)));
  connect(mpLEFilter, SIGNAL(textChanged(const QString &)),
          this, SLOT(slotFilterChanged()));
  connect(mpPushButtonDelegate, SIGNAL(clicked(const QModelIndex &)), this, SLOT(slotShowLayout(const QModelIndex &)));
}

// virtual
CQLayoutsWidget::~CQLayoutsWidget()
{
  pdelete(mpProxyModel);
}

// virtual
bool CQLayoutsWidget::update(ListViews::ObjectType /* objectType */, ListViews::Action /* action */, const std::string & /* key */)
{
  if (!mIgnoreUpdates)
    {
      enterProtected();
    }

  return true;
}

// virtual
bool CQLayoutsWidget::leave()
{
  return true;
}

void CQLayoutsWidget::deleteLayoutWindows()
{
  LayoutWindowMap::iterator it = mLayoutWindowMap.begin(), endit = mLayoutWindowMap.end();

  while (it != endit)
    {
      delete it->second;
      ++it;
    }

  mLayoutWindowMap.clear();
}

void CQLayoutsWidget::deleteSelectedLayouts()
{
  const QItemSelectionModel *pSelectionModel = mpTblLayouts->selectionModel();
  QModelIndexList mappedSelRows;
  size_t i, imax = mpLayoutsDM->rowCount();

  for (i = 0; i < imax; i++)
    {
      if (pSelectionModel->isRowSelected((int) i, QModelIndex()))
        {
          mappedSelRows.append(mpProxyModel->mapToSource(mpProxyModel->index((int) i, 0)));
        }
    }

  if (mappedSelRows.empty()) return;

  // We need to make sure that we remove the window mapped for each layout
  QModelIndexList::const_iterator it = mappedSelRows.begin();
  QModelIndexList::const_iterator end = mappedSelRows.end();
  assert(mpDataModel != NULL);
  CListOfLayouts *pListOfLayouts = mpDataModel->getListOfLayouts();

  for (; it != end; ++it)
    {
      LayoutWindowMap::iterator itWindow = mLayoutWindowMap.find((*pListOfLayouts)[it->row()].getKey());

      if (itWindow != mLayoutWindowMap.end())
        {
          mLayoutWindowMap.erase(itWindow);
          delete itWindow->second;
        }
    }

  mpLayoutsDM->removeRows(mappedSelRows);
}

void CQLayoutsWidget::updateDeleteBtns()
{
  mpBtnDelete->setEnabled(mpTblLayouts->selectionModel()->selectedRows().size() > 0);
  mpBtnClear->setEnabled(mpProxyModel->rowCount() > 0);
}

// virtual
bool CQLayoutsWidget::enterProtected()
{
  if (mpTblLayouts->selectionModel() != NULL)
    {
      disconnect(mpTblLayouts->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
                 this, SLOT(slotSelectionChanged(const QItemSelection &, const QItemSelection &)));
    }

  assert(mpDataModel != NULL);
  CListOfLayouts *pListOfLayouts = mpDataModel->getListOfLayouts();
  mpLayoutsDM->setListOfLayouts(pListOfLayouts);

  // check if we have at least a compartment
  // that we can lay out.

  if (mpDataModel != NULL &&
      mpDataModel->getModel() != NULL &&
      mpDataModel->getModel()->getCompartments().size() > 0)
    {
      mpBtnNew->setEnabled(true);
    }
  else
    {
      mpBtnNew->setEnabled(false);
    }

  // We need to make sure that we have a window mapped for each layout
  CListOfLayouts::const_iterator it = pListOfLayouts->begin();
  CListOfLayouts::const_iterator end = pListOfLayouts->end();

  for (; it != end; ++it)
    {
      LayoutWindowMap::iterator pos = mLayoutWindowMap.find(it->getKey());

      // if this layout does not have an entry in the layout window map, add one
      if (pos == mLayoutWindowMap.end())
        {
          mLayoutWindowMap.insert(std::pair<std::string, LayoutWindow *>(it->getKey(), (LayoutWindow *)NULL));
        }
    }

  connect(mpTblLayouts->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
          this, SLOT(slotSelectionChanged(const QItemSelection &, const QItemSelection &)));
  dataChanged(QModelIndex(), QModelIndex());
  return true;
}

void CQLayoutsWidget::showButtons()
{
  int i, imax = mpLayoutsDM->rowCount();

  for (i = 0; i < imax; ++i)
    {
      this->mpTblLayouts->openPersistentEditor(mpProxyModel->index(i, COL_SHOW, QModelIndex()));
    }
}

bool hasLayout(const CListOfLayouts &layouts, const std::string &name)
{
  for (size_t i = 0; i < layouts.size(); ++i)
    {
      const CLayout *layout = &layouts[i];
      const std::string &current = layout->getObjectName();

      if (current == name)
        return true;
    }

  return false;
}

// virtual
void CQLayoutsWidget::slotBtnNewClicked()
{
  const CModel *pModel = mpDataModel->getModel();
  assert(pModel != NULL);
  std::string name = "COPASI autolayout";
  int ncount = 1;

  while (hasLayout(*(mpDataModel->getListOfLayouts()), name))
    {
      std::stringstream str;
      str << "COPASI autolayout " << ncount++;
      name = str.str();
    }

  CQAutolayoutWizard pWizard(*pModel);

  if (pWizard.exec() != QDialog::Accepted)
    return;

  // add the layout to the datamodel
  std::map<std::string, std::string> m;
  CListOfLayouts *pListOfLayouts = mpDataModel->getListOfLayouts();
  // create the random layout
  CCopasiSpringLayout::Parameters p;
  CLayout *pLayout = CCopasiSpringLayout::createLayout(
                       mpDataModel, pWizard.getSelectedCompartments(),
                       pWizard.getSelectedReactions(),
                       pWizard.getSelectedMetabolites(),
                       pWizard.getSideMetabolites(),
                       &p);
  pLayout->setObjectName(name);
  pListOfLayouts->addLayout(pLayout, m);
  // update the table
  mpLayoutsDM->insertRows(mpLayoutsDM->rowCount() - 1, 1, mpLayoutsDM->index(mpLayoutsDM->rowCount() - 1, 0));
  dataChanged(QModelIndex(), QModelIndex());
  LayoutWindow *window = createLayoutWindow(pListOfLayouts->size() - 1, pLayout);
  CQNewMainWindow *pWin = dynamic_cast<CQNewMainWindow *>(window);
#ifndef DISABLE_QT_LAYOUT_RENDERING
  CQAnimationWindow *pAnim = dynamic_cast<CQAnimationWindow *>(window);

  if (pAnim != NULL)
    {
      pAnim->show();
      // now we create the spring layout
      pAnim->slotAutoLayout();
    }
  else
#endif //DISABLE_QT_LAYOUT_RENDERING
    if (pWin != NULL)
      {
        pWin->updateRenderer();
        pWin->setMode();
        // show the new layout
        pWin->show();
        pWin->redrawNow();
        // now we create the spring layout
        pWin->slotRunSpringLayout();
      }
    else
      {
        delete pLayout;
      }
}

// virtual
void CQLayoutsWidget::slotBtnDeleteClicked()
{
  if (mpTblLayouts->hasFocus())
    {deleteSelectedLayouts();}
}

// virtual
void CQLayoutsWidget::slotBtnClearClicked()
{
  int ret = CQMessageBox::question(this, tr("Confirm Delete"), "Delete all Layouts?",
                                   QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

  if (ret == QMessageBox::Yes)
    {
      mpLayoutsDM->clear();
      deleteLayoutWindows();
    }
}

// virtual
void CQLayoutsWidget::slotSelectionChanged(const QItemSelection & /* selected */,
    const QItemSelection & /* deselected */)
{
  updateDeleteBtns();
}

// virtual
void CQLayoutsWidget::slotDoubleClicked(const QModelIndex proxyIndex)
{
  QModelIndex index = mpProxyModel->mapToSource(proxyIndex);
  int row = index.row();

  if (row >= mpLayoutsDM->rowCount() || row < 0) return;

  slotShowLayout(index);
}

// virtual
void CQLayoutsWidget::dataChanged(const QModelIndex & /* topLeft */,
                                  const QModelIndex & /* bottomRight */)
{
  mpTblLayouts->resizeColumnsToContents();
  updateDeleteBtns();
  showButtons();
}

// virtual
void CQLayoutsWidget::slotFilterChanged()
{
  QRegExp regExp(mpLEFilter->text(), Qt::CaseInsensitive, QRegExp::RegExp);
  mpProxyModel->setFilterRegExp(regExp);
}

/**
 * This creates a new layout window and return a pointer to it.
 * In case of an error, NULL is returned.
 */
CQLayoutsWidget::LayoutWindow *CQLayoutsWidget::createLayoutWindow(int row, CLayout *pLayout)
{
  if (pLayout == NULL || row < 0) return NULL;

  LayoutWindow *pWin = NULL;
#ifndef DISABLE_QT_LAYOUT_RENDERING

  if (CRootContainer::getConfiguration()->useOpenGL())
    {
      pWin = new CQNewMainWindow(mpDataModel);
      (static_cast<CQNewMainWindow *>(pWin))->slotLayoutChanged(row);
    }
  else
    {
      pWin = new CQAnimationWindow(pLayout, mpDataModel);
    }

#else
  pWin = new CQNewMainWindow(mpDataModel);
  (static_cast<CQNewMainWindow *>(pWin))->slotLayoutChanged(row);
#endif //DISABLE_QT_LAYOUT_RENDERING
  std::string title = "COPASI Diagram: "  + pLayout->getObjectName();
  pWin->setWindowTitle(title.c_str());
  pWin->addToMainWindow();
  pWin->resize(900, 600);
  mLayoutWindowMap[pLayout->getKey()] = pWin;
  return pWin;
}

void CQLayoutsWidget::slotShowLayout(const QModelIndex &index)
{
  int row = index.row();
  assert(mpDataModel != NULL);
  CListOfLayouts *pListOfLayouts = mpDataModel->getListOfLayouts();
  CLayout *pLayout = &pListOfLayouts->operator[](row);
  std::string Key = pLayout->getKey();

  if (pLayout != NULL)
    {
      // check if we already have a widget for the layout
      // if yes, open it, else create one and add it to the map
      LayoutWindow *pLayoutWindow = NULL;
      LayoutWindowMap::iterator pos = mLayoutWindowMap.find(Key);

      if (pos != mLayoutWindowMap.end())
        {
          pLayoutWindow = pos->second;
        }

      if (pLayoutWindow == NULL)
        {
          pLayoutWindow = createLayoutWindow(row, pLayout);
          // need to add it to the list, so the window can be deleted later
          mLayoutWindowMap[pLayout->getKey()] = pLayoutWindow;
        }

      if (pLayoutWindow != NULL)
        {
          CQNewMainWindow *cqWin = dynamic_cast<CQNewMainWindow *>(pLayoutWindow);

          if (cqWin != NULL)
            {
              cqWin ->slotLayoutChanged(row);
              cqWin ->setMode();
            }

          pLayoutWindow->show();
          pLayoutWindow->showNormal();
          pLayoutWindow->activateWindow();
        }
    }
  else
    {
      //std::cerr << "Could not find layout." << std::endl;
    }
}

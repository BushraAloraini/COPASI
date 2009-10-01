// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/CQEFMResultWidget.cpp,v $
//   $Revision: 1.3 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2009/10/01 19:59:21 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

#include "CQEFMResultWidget.h"
#include "CQEFMNetReactionDM.h"
#include "CQEFMReactionDM.h"
#include "CQEFMSpeciesDM.h"

#include "elementaryFluxModes/CEFMTask.h"
#include "elementaryFluxModes/CFluxMode.h"

CQEFMResultWidget::CQEFMResultWidget(QWidget* parent, const char* name) :
    CopasiWidget(parent, name),
    mpTask(NULL),
    mpProxyModelReactions(NULL),
    mpReactionDM(NULL),
    mpProxyModelSpecies(NULL),
    mpSpeciesDM(NULL),
    mpProxyModelNetReactions(NULL),
    mpNetReactionDM(NULL)
{
  setupUi(this);

  mpReactionMatrix->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  mpReactionMatrix->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  mpReactionMatrix->verticalHeader()->hide();
  mpReactionMatrix->setAlternatingRowColors(true);
  mpReactionMatrix->setSortingEnabled(true);
  mpReactionMatrix->sortByColumn(COL_ROW_NUMBER, Qt::AscendingOrder);

  //Create Source Data Model.
  mpReactionDM = new CQEFMReactionDM(this);

  //Create the Proxy Model for sorting/filtering and set its properties.
  mpProxyModelReactions = new CQSortFilterProxyModel();
  mpProxyModelReactions->setSortCaseSensitivity(Qt::CaseInsensitive);
  mpProxyModelReactions->setDynamicSortFilter(true);
  mpProxyModelReactions->setFilterKeyColumn(COL_REACTION_NAME);

  mpProxyModelReactions->setSourceModel(mpReactionDM);

  //Set Model for the TableView
  mpReactionMatrix->setModel(NULL);
  mpReactionMatrix->setModel(mpProxyModelReactions);
  mpReactionMatrix->resizeColumnsToContents();

  mpSpeciesMatrix->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  mpSpeciesMatrix->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  mpSpeciesMatrix->verticalHeader()->hide();
  mpSpeciesMatrix->setAlternatingRowColors(true);
  mpSpeciesMatrix->setSortingEnabled(true);
  mpSpeciesMatrix->sortByColumn(COL_ROW_NUMBER, Qt::AscendingOrder);

  //Create Source Data Model.
  mpSpeciesDM = new CQEFMSpeciesDM(this);

  //Create the Proxy Model for sorting/filtering and set its properties.
  mpProxyModelSpecies = new CQSortFilterProxyModel();
  mpProxyModelSpecies->setSortCaseSensitivity(Qt::CaseInsensitive);
  mpProxyModelSpecies->setDynamicSortFilter(true);
  mpProxyModelSpecies->setFilterKeyColumn(COL_REACTION_NAME);

  mpProxyModelSpecies->setSourceModel(mpSpeciesDM);

  //Set Model for the TableView
  mpSpeciesMatrix->setModel(NULL);
  mpSpeciesMatrix->setModel(mpProxyModelSpecies);
  mpSpeciesMatrix->resizeColumnsToContents();

  mpNetReactions->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  mpNetReactions->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  mpNetReactions->verticalHeader()->hide();
  mpNetReactions->setAlternatingRowColors(true);
  mpNetReactions->setSortingEnabled(true);
  mpNetReactions->sortByColumn(COL_ROW_NUMBER, Qt::AscendingOrder);

  //Create Source Data Model.
  mpNetReactionDM = new CQEFMNetReactionDM(this);

  //Create the Proxy Model for sorting/filtering and set its properties.
  mpProxyModelNetReactions = new CQSortFilterProxyModel();
  mpProxyModelNetReactions->setSortCaseSensitivity(Qt::CaseInsensitive);
  mpProxyModelNetReactions->setDynamicSortFilter(true);
  mpProxyModelNetReactions->setFilterKeyColumn(COL_REACTION_NAME);

  mpProxyModelNetReactions->setSourceModel(mpNetReactionDM);

  //Set Model for the TableView
  mpNetReactions->setModel(NULL);
  mpNetReactions->setModel(mpProxyModelNetReactions);
  mpNetReactions->resizeColumnsToContents();
}

CQEFMResultWidget::~CQEFMResultWidget()
{
}

void CQEFMResultWidget::languageChange()
{
  retranslateUi(this);
}

// virtual
bool CQEFMResultWidget::leave()
{
  return true;
}

// virtual
bool CQEFMResultWidget::update(ListViews::ObjectType /* objectType */,
                               ListViews::Action /* action */,
                               const std::string & /* key */)
{
  return true;
}

// virtual
bool CQEFMResultWidget::enterProtected()
{
  return true;
}

// virtual
bool CQEFMResultWidget::loadResult(const CCopasiTask * pTask)
{
  mpTask = dynamic_cast<const CEFMTask *>(pTask);

  if (mpTask != NULL)
    {
      mpEditFluxModes->setText(QString::number(mpTask->getFluxModes().size()));
    }
  else
    {
      mpEditFluxModes->setText(QString::number(0));
    }

  bool success = true;
  success &= mpEFMListWidget->loadResult(mpTask);

  mpReactionDM->setTask(mpTask);

  mpProxyModelReactions->setSourceModel(mpReactionDM);

  //Set Model for the TableView
  mpReactionMatrix->setModel(NULL);
  mpReactionMatrix->setModel(mpProxyModelReactions);
  mpReactionMatrix->resizeColumnsToContents();

  mpSpeciesDM->setTask(mpTask);

  mpProxyModelSpecies->setSourceModel(mpSpeciesDM);

  //Set Model for the TableView
  mpSpeciesMatrix->setModel(NULL);
  mpSpeciesMatrix->setModel(mpProxyModelSpecies);
  mpSpeciesMatrix->resizeColumnsToContents();

  mpNetReactionDM->setTask(mpTask);

  mpProxyModelNetReactions->setSourceModel(mpNetReactionDM);

  //Set Model for the TableView
  mpNetReactions->setModel(NULL);
  mpNetReactions->setModel(mpProxyModelNetReactions);
  mpNetReactions->resizeColumnsToContents();

  return success;
}

void CQEFMResultWidget::slotSave()
{
  return;
}

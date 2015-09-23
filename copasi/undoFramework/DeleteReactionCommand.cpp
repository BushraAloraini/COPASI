// Copyright (C) 2014 - 2015 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

/*
 * DeleteReactionCommand.cpp
 *
 *  Created on: 4 Jul 2014
 *      Author: dada
 */

#include "qtUtilities.h"

#include "report/CCopasiRootContainer.h"
#include "model/CReaction.h"
#include "model/CReactionInterface.h"
#include "model/CModel.h"
#include "UI/ReactionsWidget1.h"
#include "UndoReactionData.h"

#include "DeleteReactionCommand.h"

DeleteReactionCommand::DeleteReactionCommand(ReactionsWidget1 *pReactionWidget)
  : CCopasiUndoCommand("Reaction", REACTION_DELETE)
  , mReaObjectName()
  , mpRi(NULL)
  , mpReaction(NULL)
  , mpReactionData(new UndoReactionData())
  , mpReactionWidget(pReactionWidget)
{
  mpReaction = dynamic_cast< CReaction * >(CCopasiRootContainer::getKeyFactory()->get(mpReactionWidget->mKey));
  mReaObjectName = mpReaction->getObjectName();
  mpRi = new CReactionInterface((*CCopasiRootContainer::getDatamodelList())[0]->getModel());
  mpRi->initFromReaction(mpReaction->getKey());

  mpReactionData->setName(mReaObjectName);
  mpReactionData->setRi(mpRi);

  this->setText(deleteReactionText(mReaObjectName));
  setName(mReaObjectName);
}

DeleteReactionCommand::~DeleteReactionCommand()
{
  pdelete(mpReactionData);
  pdelete(mpRi);
}

void DeleteReactionCommand::redo()
{
  mpReactionWidget->deleteReaction();
  setUndoState(true);
  setAction("Delete");
}

void DeleteReactionCommand::undo()
{
  mpReactionWidget->addReaction(mReaObjectName, mpRi);
  setUndoState(false);
  setAction("Undelete");
}

QString DeleteReactionCommand::deleteReactionText(std::string &name) const
{
//  std::string myEntityName(": Delete Reaction " + name);
//  char* entityName = (char*)myEntityName.c_str();
//  return QObject::tr(entityName);
  return QString(": Deleted reaction %1").arg(name.c_str());
}

UndoData *DeleteReactionCommand::getUndoData() const
{
  return mpReactionData;
}

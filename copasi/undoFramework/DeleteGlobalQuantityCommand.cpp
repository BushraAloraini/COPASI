// Copyright (C) 2014 - 2015 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

/*
 * DeleteGlobalQuantityCommand.cpp
 *
 *  Created on: 11 Sep 2014
 *      Author: dada
 */

#include "report/CCopasiRootContainer.h"
#include "model/CModelValue.h"
#include "model/CReactionInterface.h"
#include "model/CModel.h"
#include "UI/CQModelValue.h"

#include "UndoGlobalQuantityData.h"
#include "UndoReactionData.h"

#include "DeleteGlobalQuantityCommand.h"

DeleteGlobalQuantityCommand::DeleteGlobalQuantityCommand(CQModelValue *pModelValue)
  : CCopasiUndoCommand("Global Quantity", GLOBALQUANTITYDELETE)
  , mFirstTime(true)
  , mpGlobalQuantityData(new UndoGlobalQuantityData())
  , mpModelValue(pModelValue)
{

  const std::string& sName = mpModelValue->mpModelValue->getObjectName();
  mpGlobalQuantityData->setName(sName);
  mpGlobalQuantityData->setStatus(mpModelValue->mpModelValue->getStatus());

  if (mpModelValue->mpModelValue->getStatus() != CModelEntity::ASSIGNMENT)
    {
      mpGlobalQuantityData->setInitialValue(mpModelValue->mpModelValue->getInitialValue());
    }

  if (mpModelValue->mpModelValue->getStatus() != CModelEntity::FIXED)
    {
      mpGlobalQuantityData->setExpression(mpModelValue->mpModelValue->getExpression());
    }

  // set initial expression
  if (mpModelValue->mpModelValue->getStatus() != CModelEntity::ASSIGNMENT)
    {
      mpGlobalQuantityData->setInitialExpression(mpModelValue->mpModelValue->getInitialExpression());
    }

  //store to be deleted data
  setDependentObjects(mpModelValue->mpModelValue->getDeletedObjects());
  mpGlobalQuantityData->setReactionDependencyObjects(getReactionData());
  mpGlobalQuantityData->setSpecieDependencyObjects(getSpecieData());
  mpGlobalQuantityData->setEventDependencyObjects(getEventData());

  this->setText(deleteGlobalQuantityText(sName));
  setName(sName);
}

void DeleteGlobalQuantityCommand::redo()
{
  if (mFirstTime)
    {
      mpModelValue->deleteGlobalQuantity();
      mFirstTime = false;
    }
  else
    {
      mpModelValue->deleteGlobalQuantity(mpGlobalQuantityData);
    }

  setUndoState(true);
  setAction("Delete");
}

void DeleteGlobalQuantityCommand::undo()
{
  mpModelValue->addGlobalQuantity(mpGlobalQuantityData);
  setUndoState(false);
  setAction("Undelete");
}

QString DeleteGlobalQuantityCommand::deleteGlobalQuantityText(const std::string &name) const
{
//  std::string myEntityName(": Delete Global Quantity " + name);
//  char* entityName = (char*)myEntityName.c_str();
//  return QObject::tr(entityName);
  return QString(": Deleted global quantity %1").arg(name.c_str());
}

UndoData *DeleteGlobalQuantityCommand::getUndoData() const
{
  return mpGlobalQuantityData;
}

DeleteGlobalQuantityCommand::~DeleteGlobalQuantityCommand()
{
  pdelete(mpGlobalQuantityData);
}

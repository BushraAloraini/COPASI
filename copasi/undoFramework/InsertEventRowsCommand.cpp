// Copyright (C) 2014 - 2015 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

/*
 * InsertEventRowsCommand.cpp
 *
 *  Created on: 14 Oct 2014
 *      Author: dada
 */

#include "CopasiDataModel/CCopasiDataModel.h"
#include "report/CCopasiRootContainer.h"
#include "model/CEvent.h"
#include "model/CModel.h"
#include "CQEventDM.h"

#include "UndoEventData.h"
#include "UndoEventAssignmentData.h"
#include "InsertEventRowsCommand.h"

InsertEventRowsCommand::InsertEventRowsCommand(int position, int rows, CQEventDM *pEventDM, const QModelIndex& index)
  : CCopasiUndoCommand("Event", EVENT_INSERT)
  , mpEventDM(pEventDM)
  , mRows(rows)
  , mPosition(position)
  , mIndex(index)
  , mpEventData(NULL)
{
  this->setText(QObject::tr(": Inserted new event"));
}

void InsertEventRowsCommand::redo()
{
  if (mpEventData == NULL)
    {
      mpEventDM->insertNewEventRow(mPosition, mRows);
      GET_MODEL_OR_RETURN(pModel);

      CEvent *pEvent = pModel->getEvents()[mPosition];
      mpEventData = new UndoEventData(pEvent);
    }
  else
    {
      mpEventDM->addEventRow(mpEventData);
    }

  setUndoState(true);
  setAction("Add to list");
  setName(mpEventData->getName());
}

void InsertEventRowsCommand::undo()
{
  mpEventDM->deleteEventRow(mpEventData);
  setUndoState(false);
  setAction("Remove from list");
}

InsertEventRowsCommand::~InsertEventRowsCommand()
{
  pdelete(mpEventData);
}

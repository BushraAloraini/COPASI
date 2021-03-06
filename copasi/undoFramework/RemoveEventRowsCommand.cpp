// Copyright (C) 2017 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and University of
// of Connecticut School of Medicine.
// All rights reserved.

// Copyright (C) 2014 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

/*
 * RemoveEventRowsCommand.cpp
 *
 *  Created on: 14 Oct 2014
 *      Author: dada
 */

#include "copasi/copasi.h"

#include "copasi/core/CRootContainer.h"
#include "copasi/model/CModel.h"
#include "copasi/UI/CQEventDM.h"
#include "copasi/model/CEvent.h"
#include "copasi/CopasiDataModel/CDataModel.h"

#include "UndoEventData.h"
#include "UndoEventAssignmentData.h"

#include "RemoveEventRowsCommand.h"

RemoveEventRowsCommand::RemoveEventRowsCommand(
  QModelIndexList rows, CQEventDM * pEventDM)
  : CCopasiUndoCommand("Event", EVENT_REMOVE)
  , mpEventDM(pEventDM)
  , mpEventData()
{
  CDataModel * pDataModel = pEventDM->getDataModel();
  assert(pDataModel != NULL);
  CModel * pModel = pDataModel->getModel();
  assert(pModel != NULL);

  QModelIndexList::const_iterator i;

  for (i = rows.begin(); i != rows.end(); ++i)
    {

      if (!pEventDM->isDefaultRow(*i) && &pModel->getEvents()[i->row()])
        {
          UndoEventData *data = new UndoEventData(&pModel->getEvents()[i->row()]);
          mpEventData.append(data);
        }
    }

  setText(QObject::tr(": Removed Event"));
}

void RemoveEventRowsCommand::redo()
{
  mpEventDM->deleteEventRows(mpEventData);

  setUndoState(true);
  setAction("Delete set");
}

void RemoveEventRowsCommand::undo()
{
  mpEventDM->insertEventRows(mpEventData);
  setUndoState(false);
  setAction("Undelete set");
}

RemoveEventRowsCommand::~RemoveEventRowsCommand()
{
  // freeing the memory allocated above
  foreach(UndoEventData * data, mpEventData)
  {
    pdelete(data);
  }

  mpEventData.clear();
}

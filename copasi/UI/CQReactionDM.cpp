// Copyright (C) 2010 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

#include <QtCore/QString>
#include <QtCore/QList>

#include "CopasiDataModel/CCopasiDataModel.h"
#include "report/CCopasiRootContainer.h"
#include "model/CChemEqInterface.h"
#include "model/CReaction.h"
#include "model/CReactionInterface.h"

#include "CQMessageBox.h"
#include "CQReactionDM.h"
#include "qtUtilities.h"

#include "undoFramework/InsertReactionRowsCommand.h"
#include "undoFramework/RemoveReactionRowsCommand.h"
#include "undoFramework/RemoveAllReactionRowsCommand.h"
#include "undoFramework/ReactionDataChangeCommand.h"
#include "undoFramework/ReactionChangeCommand.h"
#include "undoFramework/UndoReactionData.h"

CQReactionDM::CQReactionDM(QObject *parent)
  : CQBaseDataModel(parent)
  , mNewEquation()
  , mCreatedKeys()
{}

int CQReactionDM::rowCount(const QModelIndex&) const
{
  return (int)(*CCopasiRootContainer::getDatamodelList())[0]->getModel()->getReactions().size() + 1;
}
int CQReactionDM::columnCount(const QModelIndex&) const
{
  return TOTAL_COLS_REACTIONS;
}

Qt::ItemFlags CQReactionDM::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return Qt::ItemIsEnabled;

  if (index.column() == COL_NAME_REACTIONS || index.column() == COL_EQUATION)
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
  else
    return QAbstractItemModel::flags(index);
}

QVariant CQReactionDM::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (index.row() >= rowCount())
    return QVariant();

  if (index.column() > 0 && role == Qt::ForegroundRole && !(flags(index) & Qt::ItemIsEditable))
    return QColor(Qt::darkGray);

  if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
      if (isDefaultRow(index))
        {
          switch (index.column())
            {
              case COL_ROW_NUMBER:
                return QVariant(QString(""));

              case COL_NAME_REACTIONS:
                return QVariant(QString("New Reaction"));

              default:
                return QVariant(QString(""));
            }
        }
      else
        {
          CReaction *pRea = (*CCopasiRootContainer::getDatamodelList())[0]->getModel()->getReactions()[index.row()];

          switch (index.column())
            {
              case COL_ROW_NUMBER:
                return QVariant(index.row() + 1);

              case COL_NAME_REACTIONS:
                return QVariant(QString(FROM_UTF8(pRea->getObjectName())));

              case COL_EQUATION:

                if (mNewEquation.isEmpty())
                  {
                    return QVariant(QString(FROM_UTF8(CChemEqInterface::getChemEqString((*CCopasiRootContainer::getDatamodelList())[0]->getModel(), *pRea, false))));
                  }
                else
                  {
                    return QVariant(mNewEquation);
                  }

              case COL_RATE_LAW:

                if (pRea->getFunction())
                  return QVariant(QString(FROM_UTF8(pRea->getFunction()->getObjectName())));
                else
                  return QVariant();

              case COL_FLUX:
                return QVariant(pRea->getFlux());

              case COL_PARTICLE_FLUX:
                return QVariant(pRea->getParticleFlux());
            }
        }
    }

  return QVariant();
}

QVariant CQReactionDM::headerData(int section, Qt::Orientation orientation,
                                  int role) const
{
  if (role != Qt::DisplayRole)
    return QVariant();

  if (orientation == Qt::Horizontal)
    {
      switch (section)
        {
          case COL_ROW_NUMBER:
            return QVariant(QString("#"));

          case COL_NAME_REACTIONS:
            return QVariant(QString("Name"));

          case COL_EQUATION:
            return QVariant(QString("Reaction"));

          case COL_RATE_LAW:
            return QVariant(QString("Rate Law"));

          case COL_FLUX:
          {
            const CModel * pModel = (*CCopasiRootContainer::getDatamodelList())[0]->getModel();

            if (pModel == NULL) return QVariant();

            QString RateUnits;

            if (pModel)
              RateUnits = FROM_UTF8(pModel->getQuantityRateUnitsDisplayString());

            if (!RateUnits.isEmpty())
              RateUnits = "\n(" + RateUnits + ")";

            return QVariant("Flux" + RateUnits);
          }

          case COL_PARTICLE_FLUX:
          {
            const CModel * pModel = (*CCopasiRootContainer::getDatamodelList())[0]->getModel();

            if (pModel == NULL) return QVariant();

            QString FrequencyUnits;

            if (pModel)
              FrequencyUnits = FROM_UTF8(pModel->getFrequencyUnit().getSymbol());

            if (FrequencyUnits != "none")
              FrequencyUnits = "\n(" + FrequencyUnits + ")";

            return QVariant("Flux" + FrequencyUnits);
          }

          default:
            return QVariant();
        }
    }
  else
    return QString("%1").arg(section + 1);
}

bool CQReactionDM::setData(const QModelIndex &index, const QVariant &value,
                           int role)
{
  if (index.data() == value)
    return false;

  bool defaultRow = isDefaultRow(index);

  if (defaultRow)
    {
      mpUndoStack->push(new InsertReactionRowsCommand(rowCount(), 1, this, index, value));
    }
  else
    {
      mpUndoStack->push(new ReactionDataChangeCommand(index, value, role, this));
    }

  return true;
}

void CQReactionDM::setEquation(const CReaction *pRea, const QModelIndex& index, const QVariant &value)
{
  std::string objKey = pRea->getKey();
  mNewEquation = value.toString();
  mCreatedKeys.clear();

  GET_MODEL_OR_RETURN(pModel);

  // this loads the reaction into a CReactionInterface object.
  // the gui works on this object and later writes back the changes to ri;
  CReactionInterface ri(pModel);
  ri.initFromReaction(objKey);

  if (TO_UTF8(mNewEquation) != ri.getChemEqString())
    {
      //first check if the string is a valid equation
      if (!CChemEqInterface::isValidEq(TO_UTF8(mNewEquation)))
        {
          mNewEquation = "";
          return;
        }
      else
        {
          //tell the reaction interface
          ri.setChemEqString(TO_UTF8(mNewEquation), "");
        }
    }

  // Before we save any changes we must check whether any local reaction parameters,
  // which are used in any mathematical expression in the model are removed.
  // If that is the case the user must have option to cancel the changes or remove the
  // affected expressions.
  std::set< const CCopasiObject * > DeletedParameters = ri.getDeletedParameters();

  if (DeletedParameters.size() != 0)
    {
      QString ObjectType = "parameter(s) of reaction " + this->index(index.row(), COL_NAME_REACTIONS).data().toString();
      QString Objects;

      std::set< const CCopasiObject * >::const_iterator itParameter, endParameter = DeletedParameters.end();
      std::set< const CCopasiObject * > DeletedObjects;

      for (itParameter = DeletedParameters.begin(); itParameter != endParameter; ++itParameter) //all parameters
        {
          Objects.append(FROM_UTF8((*itParameter)->getObjectName()) + ", ");
          DeletedObjects.insert(static_cast< const CCopasiObject * >((*itParameter)->getObject(CCopasiObjectName("Reference=Value"))));
        }

      Objects.remove(Objects.length() - 2, 2);

      QMessageBox::StandardButton choice =
        CQMessageBox::confirmDelete(NULL, ObjectType,
                                    Objects, DeletedObjects);

      switch (choice)
        {
          case QMessageBox::Ok:

            for (itParameter = DeletedParameters.begin(); itParameter != endParameter; ++itParameter) //all parameters
              pModel->removeLocalReactionParameter((*itParameter)->getKey());

            break;

          default:
            mNewEquation = "";
            return;
            break;
        }
    }

  // We need to check whether the current reaction still exists, since it is possible that
  // removing a local reaction parameter triggers its deletion.
  CReaction * reac = dynamic_cast< CReaction * >(CCopasiRootContainer::getKeyFactory()->get(objKey));

  if (reac == NULL)
    {
      ri.setFunctionWithEmptyMapping("");
      emit notifyGUI(ListViews::REACTION, ListViews::DELETE, objKey);
      emit notifyGUI(ListViews::REACTION, ListViews::DELETE, ""); //Refresh all as there may be dependencies.
      mNewEquation = "";
      return;
    }

  //first check if new metabolites need to be created
  bool createdMetabs = ri.createMetabolites(mCreatedKeys);
  bool createdObjects = ri.createOtherObjects(mCreatedKeys);
  //this writes all changes to the reaction
  ri.writeBackToReaction(NULL);

  //(*CCopasiRootContainer::getDatamodelList())[0]->getModel()->compile();
  //this tells the gui what it needs to know.
  if (createdObjects ||
      DeletedParameters.size() != 0)
    {
      emit notifyGUI(ListViews::MODEL, ListViews::CHANGE, "");
    }
  else if (createdMetabs)
    {
      emit notifyGUI(ListViews::METABOLITE, ListViews::ADD, "");
    }

  mNewEquation = "";
}

bool CQReactionDM::insertRows(int position, int rows, const QModelIndex&)
{
  mpUndoStack->push(new InsertReactionRowsCommand(position, rows, this));

  return true;
}

bool CQReactionDM::removeRows(int position, int rows)
{
  if (rows <= 0)
    return true;

  beginRemoveRows(QModelIndex(), position, position + rows - 1);

  CModel * pModel = (*CCopasiRootContainer::getDatamodelList())[0]->getModel();

  std::vector< std::string > DeletedKeys;
  DeletedKeys.resize(rows);

  std::vector< std::string >::iterator itDeletedKey;
  std::vector< std::string >::iterator endDeletedKey = DeletedKeys.end();

  CCopasiVector< CReaction >::const_iterator itRow = pModel->getReactions().begin() + position;

  for (itDeletedKey = DeletedKeys.begin(); itDeletedKey != endDeletedKey; ++itDeletedKey, ++itRow)
    {
      *itDeletedKey = (*itRow)->getKey();
    }

  for (itDeletedKey = DeletedKeys.begin(); itDeletedKey != endDeletedKey; ++itDeletedKey)
    {
      pModel->removeReaction(*itDeletedKey);
      emit notifyGUI(ListViews::REACTION, ListViews::DELETE, *itDeletedKey);
      emit notifyGUI(ListViews::REACTION, ListViews::DELETE, "");//Refresh all as there may be dependencies.
    }

  endRemoveRows();

  return true;
}

bool CQReactionDM::removeRows(QModelIndexList rows, const QModelIndex&)
{
  mpUndoStack->push(new RemoveReactionRowsCommand(rows, this, QModelIndex()));
  return true;
}

bool CQReactionDM::reactionDataChange(const QModelIndex &index,
                                      const QVariant &value,
                                      int role,
                                      QString &funcName,
                                      std::vector<std::string>& createdObjects)
{
  switchToWidget(CCopasiUndoCommand::REACTIONS);

  if (!index.isValid() || role != Qt::EditRole)
    return true;

  GET_MODEL_OR(pModel, return false);

  bool defaultRow = isDefaultRow(index);

  if (defaultRow)
    {
      if (index.data() != value)
        insertRow(rowCount(), index);
      else
        return false;
    }

  // this loads the reaction into a CReactionInterface object.
  // the gui works on this object and later writes back the changes to ri;
  CReaction *pRea = pModel->getReactions()[index.row()];
  bool refreshAll = false;

  if (index.column() == COL_NAME_REACTIONS)
    {
      pRea->setObjectName(TO_UTF8(value.toString()));
      emit notifyGUI(ListViews::REACTION, ListViews::RENAME, pRea->getKey());
    }
  else if (index.column() == COL_EQUATION)
    {
      mCreatedKeys.clear();

      if (!createdObjects.empty())
        {
          pRea->cleanup();
          pRea->compile();

          ReactionChangeCommand::removeCreatedObjects(createdObjects,
              pModel, pRea);
          refreshAll = true;
        }

      setEquation(pRea, index, value);
      updateReactionWithFunctionName(pRea, funcName);
      createdObjects.assign(mCreatedKeys.begin(), mCreatedKeys.end());
    }

  if (defaultRow && this->index(index.row(), COL_NAME_REACTIONS).data().toString() == "reaction")
    pRea->setObjectName(TO_UTF8(createNewName("reaction", COL_NAME_REACTIONS)));

  emit dataChanged(index, index);
  emit notifyGUI(ListViews::REACTION, ListViews::CHANGE, pRea->getKey());
  emit notifyGUI(ListViews::REACTION, ListViews::CHANGE, "");

  return true;
}
bool CQReactionDM::updateReactionWithFunctionName(CReaction *pRea, QString &funcName)
{
  GET_MODEL_OR(pModel, return false);

  CReactionInterface ri(pModel);
  ri.initFromReaction(pRea->getKey());
  ri.setFunctionAndDoMapping(TO_UTF8(funcName));
  ri.writeBackToReaction(pRea);
  return false;
}

void CQReactionDM::insertNewReactionRow(InsertReactionRowsCommand* command)
{
  GET_MODEL_OR_RETURN(pModel);

  int position = command->position();
  int rows = command->rows();
  const QModelIndex& index = command->index();
  const QVariant& value = command->value();

  beginInsertRows(QModelIndex(), position, position + rows - 1);

  int column = index.column();

  for (int row = 0; row < rows; ++row)
    {
      mCreatedKeys.clear();
      QString name = createNewName(index.isValid() && column == COL_NAME_REACTIONS ? value.toString() : "reaction", COL_NAME_REACTIONS);
      CReaction *pRea = pModel->createReaction(TO_UTF8(name));

      if (pRea == NULL) continue;

      std::string key = pRea->getKey();
      emit notifyGUI(ListViews::REACTION, ListViews::ADD, key);

      if (index.isValid() && column == COL_EQUATION)
        {
          setEquation(pRea, index, value);
        }

      command->initializeUndoData(pRea, mCreatedKeys);
    }

  endInsertRows();
}

void CQReactionDM::addReactionRow(CReaction *pReaction)
{
  GET_MODEL_OR_RETURN(pModel);

  switchToWidget(CCopasiUndoCommand::REACTIONS);

  beginInsertRows(QModelIndex(), 1, 1);
  CReaction *pRea = pModel->createReaction(pReaction->getObjectName());
  std::string key = pRea->getKey();
  emit notifyGUI(ListViews::REACTION, ListViews::ADD, key);
  endInsertRows();
}

void CQReactionDM::deleteReactionRow(CReaction *pReaction)
{
  GET_MODEL_OR_RETURN(pModel);

  switchToWidget(CCopasiUndoCommand::REACTIONS);

  std::string key = pReaction->getKey();
  beginRemoveRows(QModelIndex(), 1, 1);
  pModel->removeReaction(pReaction->getKey());
  emit notifyGUI(ListViews::REACTION, ListViews::DELETE, key);
  emit notifyGUI(ListViews::REACTION, ListViews::DELETE, "");//Refresh all as there may be dependencies.
  endRemoveRows();
}

bool CQReactionDM::removeReactionRows(QModelIndexList rows, const QModelIndex&)
{
  if (rows.isEmpty())
    return false;

  GET_MODEL_OR(pModel, return false);

  switchToWidget(CCopasiUndoCommand::REACTIONS);

  //Build the list of pointers to items to be deleted
  //before actually deleting any item.
  QList <CReaction *> pReactions;
  QModelIndexList::const_iterator i;

  for (i = rows.begin(); i != rows.end(); ++i)
    {
      if (!isDefaultRow(*i) && pModel->getReactions()[(*i).row()])
        {
          pReactions.append(pModel->getReactions()[(*i).row()]);
        }
    }

  QList <CReaction *>::const_iterator j;

  for (j = pReactions.begin(); j != pReactions.end(); ++j)
    {
      CReaction * pReaction = *j;

      size_t delRow =
        pModel->getReactions().CCopasiVector< CReaction >::getIndex(pReaction);

      if (delRow == C_INVALID_INDEX)
        continue;

      QMessageBox::StandardButton choice =
        CQMessageBox::confirmDelete(NULL, "reaction",
                                    FROM_UTF8(pReaction->getObjectName()),
                                    pReaction->getDeletedObjects());

      if (choice == QMessageBox::Ok)
        removeRow((int) delRow);
    }

  return true;
}

bool CQReactionDM::insertReactionRows(QList <UndoReactionData *>& pData)
{
  switchToWidget(CCopasiUndoCommand::REACTIONS);

  QList <UndoReactionData *>::const_iterator j;

  for (j = pData.begin(); j != pData.end(); ++j)
    {
      addReactionRow(*j);
    }

  return true;
}

void CQReactionDM::addReactionRow(UndoReactionData *pData)
{
  GET_MODEL_OR_RETURN(pModel);

  CReaction *pReaction = pData->restoreObjectIn(pModel);

  if (pReaction == NULL)
    return;

  beginInsertRows(QModelIndex(), 1, 1);
  emit notifyGUI(ListViews::REACTION, ListViews::ADD, pReaction->getKey());
  endInsertRows();
  emit notifyGUI(ListViews::MODEL, ListViews::CHANGE, "");//Refresh all as there may be dependencies.
}

void CQReactionDM::deleteReactionRow(UndoReactionData * pData)
{
  switchToWidget(CCopasiUndoCommand::REACTIONS);

  GET_MODEL_OR_RETURN(pModel);

  if (pModel->getReactions().getIndex(pData->getName()) == C_INVALID_INDEX)
    return;

  CReaction *pReaction = pModel->getReactions()[pData->getName()];
  std::string key = pReaction->getKey();

  beginRemoveRows(QModelIndex(), 1, 1);
  pModel->removeReaction(pReaction);
  ReactionChangeCommand::removeCreatedObjects(pData->getAdditionalKeys(), pModel, NULL);
  emit notifyGUI(ListViews::REACTION, ListViews::DELETE, key);
  emit notifyGUI(ListViews::REACTION, ListViews::DELETE, "");//Refresh all as there may be dependencies.
  endRemoveRows();
}

void CQReactionDM::deleteReactionRows(QList <UndoReactionData *>& pData)
{
  switchToWidget(CCopasiUndoCommand::REACTIONS);

  QList <UndoReactionData *>::const_iterator j;

  for (j = pData.begin(); j != pData.end(); ++j)
    {
      deleteReactionRow(*j);
    }
}

bool CQReactionDM::clear()
{
  mpUndoStack->push(new RemoveAllReactionRowsCommand(this, QModelIndex()));
  return true;
}

bool CQReactionDM::removeAllReactionRows()
{
  return removeRows(0, rowCount() - 1);
}

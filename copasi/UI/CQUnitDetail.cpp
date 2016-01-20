// Copyright (C) 2010 - 2015 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 - 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2006 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#include "CQUnitDetail.h"

//#include "CQExpressionWidget.h"

#include "UI/CQMessageBox.h"
#include "UI/qtUtilities.h"

#include "model/CModel.h"
//#include "model/CModelValue.h"
//#include "function/CExpression.h"
#include "report/CKeyFactory.h"
#include "report/CCopasiRootContainer.h"
#include "UI/CQValidatorUnit.h"

////UNDO framework classes
//#ifdef COPASI_UNDO
//#include "model/CReactionInterface.h"
//#include "undoFramework/DeleteGlobalQuantityCommand.h"
//#include "undoFramework/CreateNewGlobalQuantityCommand.h"
////#include "undoFramework/GlobalQuantityTypeChangeCommand.h"
//#include "undoFramework/UndoGlobalQuantityData.h"
//#include "undoFramework/UndoReactionData.h"
//#include "undoFramework/UndoEventData.h"
//#include "undoFramework/UndoSpecieData.h"
//#include "undoFramework/UndoEventAssignmentData.h"
//#include "copasiui3window.h"
//#endif

/*
 *  Constructs a CQUnitDetail which is a child of 'parent', with the
 *  name 'name'.'
 */
CQUnitDetail::CQUnitDetail(QWidget* parent, const char* name)
  : CopasiWidget(parent, name),
    mKeyToCopy(""),
    mpUnit(NULL)
{
  setupUi(this);

  init();

//#ifdef COPASI_UNDO
//  CopasiUI3Window *  pWindow = dynamic_cast<CopasiUI3Window * >(parent->parent());
//  setUndoStack(pWindow->getUndoStack());
//#endif
}

/*
 *  Destroys the object and frees any allocated resources
 */
CQUnitDetail::~CQUnitDetail()
{
  destroy();
  // no need to delete child widgets, Qt does it all for us
}

/// Slot to create a new unit; activated whenever the New button is clicked
void CQUnitDetail::slotBtnNew()
{
 std::string name = "unit_1";
 int i = 1;
 CUnit* pUnit;
 CCopasiVectorN<CUnit> * unitList
   = CCopasiRootContainer::getUnitList();

 while (unitList->getIndex(name) != C_INVALID_INDEX)
   {
     i++;
     name = "unit_";
     name += TO_UTF8(QString::number(i));
   }

 CCopasiRootContainer::getUnitList()->add(pUnit = new CUnit(name), true);

 std::string key = pUnit->getKey();
 protectedNotify(ListViews::UNIT, ListViews::ADD, key);
 // enter(key);
 mpListView->switchToOtherWidget(C_INVALID_INDEX, key);
}

void CQUnitDetail::slotBtnCopy()
{
  mKeyToCopy = mKey;
}

//void CQUnitDetail::slotBtnDelete()
//{
////#ifdef COPASI_UNDO
////  mpUndoStack->push(new DeleteGlobalQuantityCommand(this));
////#else
//  assert(CCopasiRootContainer::getDatamodelList()->size() > 0);
//  CCopasiDataModel* pDataModel = (*CCopasiRootContainer::getDatamodelList())[0];
//  assert(pDataModel != NULL);
//  CModel * pModel = pDataModel->getModel();

//  if (pModel == NULL)
//    return;

//  if (mpUnit == NULL)
//    return;

//  QMessageBox::StandardButton choice =
//    CQMessageBox::confirmDelete(this, "unit",
//                                FROM_UTF8(mpUnit->getObjectName()),
//                                mpUnit->getDeletedObjects());

//  switch (choice)
//    {
//      case QMessageBox::Ok:
//      {
//        pDataModel->getModel()->re(mKey);
//        mpUnit = NULL;

//#undef DELETE
//        protectedNotify(ListViews::UNIT, ListViews::DELETE, mKey);
//        protectedNotify(ListViews::UNIT, ListViews::DELETE, "");//Refresh all as there may be dependencies.
//        break;
//      }

//      default:
//        break;
//    }

//////#endif
//}

//! Slot for being activated whenever Delete button is clicked
void CQUnitDetail::slotBtnDelete()
{

  assert(CCopasiRootContainer::getDatamodelList()->size() > 0);
  CCopasiDataModel* pDataModel = (*CCopasiRootContainer::getDatamodelList())[0];
  assert(pDataModel != NULL);
  CModel * pModel = pDataModel->getModel();

  if (pModel == NULL)
    return;

  CCopasiVectorN<CUnit>* pUnits = CCopasiRootContainer::getUnitList();

  if (pUnits == NULL)
    return;

  CUnit * pUnit = dynamic_cast<CUnit *>(CCopasiRootContainer::getKeyFactory()->get(mKey));

  if (pUnit == NULL)
    return;

  CCopasiObject::DataObjectSet uses = pModel->getUnitSymbolUsage(pUnit->getSymbol());

  if (!uses.empty())
  {
    QString text = "It is being used in . . .\n";

    CCopasiObject::DataObjectSet::const_iterator it = uses.begin();
    CCopasiObject::DataObjectSet::const_iterator end = uses.end();

    for (; it != end; ++it)
    {
      text += FROM_UTF8((*it)->getObjectName()) + "\n";
    }

    CQMessageBox::information(this,
                              "Unable to delete " + FROM_UTF8(pUnit->getObjectName()),
                              text,
                              QMessageBox::Ok, QMessageBox::Ok);
  }
  else
  {
    int ret = CQMessageBox::question(this, tr("Confirm Delete"), "Delete " + FROM_UTF8(pUnit->getObjectName()) + "?",
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (ret == QMessageBox::Yes)
    {
      delete pUnit;
      protectedNotify(ListViews::UNIT, ListViews::DELETE, mKey);
    }
  }

  return;
}


/*!
    If the simulation type is changed then COPASI will automatically adjust its appearance,
    especially correlating to the Expression Widget and its buttons.
 */
//void CQUnitDetail::slotTypeChanged(int type)
//{
//  switch ((CModelEntity::Status) mItemToType[type])
//    {
//      case CModelEntity::FIXED:
//        // hide label, widget, and all buttons
//        mpLblExpression->hide();
//        mpExpressionEMW->hide();

//        // enable the option of use Initial Expression
//        mpBoxUseInitialExpression->setEnabled(true);
//        slotInitialTypeChanged(mpBoxUseInitialExpression->isChecked());

//        // we don't need to update the expression widget as it is already hidden

//        break;

//      case CModelEntity::ASSIGNMENT:
//        // show label, widget, and correct buttons
//        mpLblExpression->show();   // show the label
//        mpExpressionEMW->show();  // show the widget

//        // disable the option of use Initial Expression
//        mpBoxUseInitialExpression->setEnabled(false);
//        slotInitialTypeChanged(false);

//        // update the expression widget
//        mpExpressionEMW->updateWidget();

//        break;

//      case CModelEntity::ODE:
//        // show label, widget, and correct buttons
//        mpLblExpression->show();   // show the label
//        mpExpressionEMW->show();  // show the widget

//        // enable the option of use Initial Expression
//        mpBoxUseInitialExpression->setEnabled(true);
//        slotInitialTypeChanged(mpBoxUseInitialExpression->isChecked());

//        // update the expression widget
//        mpExpressionEMW->updateWidget();

//        break;

//      default:
//        break;
//    }
//}

///*!
//    This function is used in case of not FIXED type
// */
//void CQUnitDetail::slotExpressionValid(bool valid)
//{
//  mExpressionValid = valid;
//}

//void CQUnitDetail::slotInitialExpressionValid(bool valid)
//{
//  mInitialExpressionValid = valid;
//}

void CQUnitDetail::init()
{
//  mpComboBoxType->insertItem(mpComboBoxType->count(), FROM_UTF8(CModelEntity::StatusName[CModelEntity::FIXED]));
//  mpComboBoxType->insertItem(mpComboBoxType->count(), FROM_UTF8(CModelEntity::StatusName[CModelEntity::ASSIGNMENT]));
//  mpComboBoxType->insertItem(mpComboBoxType->count(), FROM_UTF8(CModelEntity::StatusName[CModelEntity::ODE]));

//  mItemToType.push_back(CModelEntity::FIXED);
//  mItemToType.push_back(CModelEntity::ASSIGNMENT);
//  mItemToType.push_back(CModelEntity::ODE);

//  mExpressionValid = false;
//  mpExpressionEMW->mpExpressionWidget->setExpressionType(CQExpressionWidget::TransientExpression);

//  mInitialExpressionValid = false;
//  mpInitialExpressionEMW->mpExpressionWidget->setExpressionType(CQExpressionWidget::InitialExpression);

  mpEditDefinition->setValidator(new CQValidatorUnit(mpEditDefinition));
}

void CQUnitDetail::destroy()
{}

bool CQUnitDetail::update(ListViews::ObjectType  objectType,
                          ListViews::Action action,
                          const std::string & key)
{
  switch (objectType)
    {
      case ListViews::MODEL:

        // For a new model we need to remove references to no longer existing metabolites
        if (action == ListViews::ADD)
          {
            mKey = "";
            mpObject = NULL;
            mpUnit = NULL;
          }

        break;

      case ListViews::MODELVALUE:

        // If the currently displayed metabolite is deleted we need to remove its references.
        if (action == ListViews::DELETE && mKey == key)
          {
            mKey = "";
            mpObject = NULL;
            mpUnit = NULL;
          }

        break;

      case ListViews::STATE:
        break;

      default:
        return true;
        break;
    }

  if (isVisible() && !mIgnoreUpdates)
    load();

  return true;
}

bool CQUnitDetail::leave()
{
//  if ((CModelEntity::Status) mItemToType[mpComboBoxType->currentIndex()] != CModelEntity::FIXED)
//    {
//      // -- Expression --
//      mpExpressionEMW->updateWidget();
//    }

//  if (mpBoxUseInitialExpression->isChecked())
//    {
//      // -- Initial Expression --
//      mpInitialExpressionEMW->updateWidget();
//    }

  save();

  return true;
}

bool CQUnitDetail::enterProtected()
{
  mpUnit = NULL;

  if (mKeyToCopy != "")
    {
      mpUnit = dynamic_cast<CUnit *>(CCopasiRootContainer::getKeyFactory()->get(mKeyToCopy));
      mKeyToCopy = "";
    }
  else
    {
      mpUnit = dynamic_cast<CUnit *>(mpObject);
    }

  if (!mpUnit)
    {
      mpListView->switchToOtherWidget(115, "");
      return false;
    }

  load();

  // This is needed, in the case the unit is copied.
  mpUnit = dynamic_cast<CUnit *>(mpObject);

  return true;
}

/*!
    Load any saved values to the screen
 */
void CQUnitDetail::load()
{
  if (mpUnit == NULL) return;

  // Definition
  mpEditDefinition->setText(FROM_UTF8(mpUnit->getDefinition()));

  // Name
  mpEditName->setText(FROM_UTF8(mpUnit->getObjectName()));

  // Definition
  mpEditSymbol->setText(FROM_UTF8(mpUnit->getSymbol()));

//  // Expression
//  mpExpressionEMW->mpExpressionWidget->setExpression(mpModelValue->getExpression());

//  // Update Expression Widget
//  mpExpressionEMW->updateWidget();

//  // Initial Expression
//  mpInitialExpressionEMW->mpExpressionWidget->setExpression(mpModelValue->getInitialExpression());

//  // Update Initial Expression Widget
//  mpInitialExpressionEMW->updateWidget();

//  // Type dependent display of values
////  slotTypeChanged(mpComboBoxType->currentIndex());

//  // Use Initial Expression
//  if (mpModelValue->getStatus() == CModelEntity::ASSIGNMENT ||
//      mpModelValue->getInitialExpression() == "")
//    {
//      mpBoxUseInitialExpression->setChecked(false);
//      //      slotInitialTypeChanged(false);
//    }
//  else
//    {
//      mpBoxUseInitialExpression->setChecked(true);
//      //      slotInitialTypeChanged(true);
//    }

  mChanged = false;
}

/*!
    Save any inputs and set them as initial setting values
 */
void CQUnitDetail::save()
{
  if (mpUnit == NULL) return;

  // set status
//  if (mpModelValue->getStatus() != (CModelEntity::Status) mItemToType[mpComboBoxType->currentIndex()])
//    {
//      mpModelValue->setStatus((CModelEntity::Status) mItemToType[mpComboBoxType->currentIndex()]);
//      mChanged = true;
//    }

  // set initial value
//  if (QString::number(mpModelValue->getInitialValue(), 'g', 10) != mpEditInitialValue->text() &&
//      mpModelValue->getStatus() != CModelEntity::ASSIGNMENT)
//    {
//      mpModelValue->setInitialValue(mpEditInitialValue->text().toDouble());
//      mChanged = true;
//    }

  // set expression
//  if (mpModelValue->getExpression() != mpExpressionEMW->mpExpressionWidget->getExpression())
//    {
//      //      mpModelValue->setExpression(((CQExpressionWidget *)mpEditExpression->widget(0))->getExpression());
//      mpModelValue->setExpression(mpExpressionEMW->mpExpressionWidget->getExpression());
//      mChanged = true;
//    }

  // set initial expression
//  if ((CModelEntity::Status) mItemToType[mpComboBoxType->currentIndex()] != CModelEntity::ASSIGNMENT)
//    {
//      if (mpBoxUseInitialExpression->isChecked() &&
//          mpModelValue->getInitialExpression() != mpInitialExpressionEMW->mpExpressionWidget->getExpression())
//        {
//          mpModelValue->setInitialExpression(mpInitialExpressionEMW->mpExpressionWidget->getExpression());
//          mChanged = true;
//        }
//      else if (!mpBoxUseInitialExpression->isChecked() &&
//               mpModelValue->getInitialExpression() != "")
//        {
//          mpModelValue->setInitialExpression("");
//          mChanged = true;
//        }
//    }

  // set unit name
  if (mpUnit->getObjectName() != TO_UTF8(mpEditName->text()))
    {
      mChanged = (mChanged || mpUnit->setObjectName(TO_UTF8(mpEditDefinition->text())));
    }

  // set unit symbol
  if (mpUnit->getSymbol() != TO_UTF8(mpEditSymbol->text()))
    {
      mpUnit->setSymbol(TO_UTF8(mpEditSymbol->text()));
      mChanged = true;
    }

  if (mChanged)
    {
      assert(CCopasiRootContainer::getDatamodelList()->size() > 0);
      (*CCopasiRootContainer::getDatamodelList())[0]->changed();
      protectedNotify(ListViews::UNIT, ListViews::CHANGE, mKey);
    }

  mChanged = false;
}

/*!
    If the initial expression is chosen to be used by checking the mpBoxUseInitialExpression check box being represented by
    the boolean parameter useInitialAssignment (true if checked; false otherwise), COPASI will show the Initial Expression
    Widget with its correct push buttons. Otherwise, the widget and its buttons will be hidden.
 */
//void CQUnitDetail::slotInitialTypeChanged(bool useInitialAssignment)
//{
//  if (useInitialAssignment)  // use Initial Expression (ie. the mpBoxUseInitialExpression is checked)
//    {
//      // show label, widget, and the correct buttons
//      mpLblInitialExpression->show();  // show the label
//      mpInitialExpressionEMW->show(); // show the widget

//      // enable the option of use Initial Value
//      mpEditInitialValue->setEnabled(false);

//      // update the Initial Expression Widget
//      mpInitialExpressionEMW->updateWidget();
//    }
//  else  // mpBoxUseInitialExpression is not checked
//    {
//      // hide label, widget, and all buttons
//      mpLblInitialExpression->hide();
//      mpInitialExpressionEMW->hide();

//      // enable the option of use Initial Value
//      mpEditInitialValue->setEnabled((CModelEntity::Status) mItemToType[mpComboBoxType->currentIndex()] != CModelEntity::ASSIGNMENT);

//      // we don't need to update the Initial Expression Widget
//    }
//}

//Undo methods
#ifdef COPASI_UNDO

void CQUnitDetail::createNewGlobalQuantity()
{
  // save the current setting values
  leave();

  // standard name
  std::string name = "quantity_1";

  // if the standard name already exists then creating the new event will fail
  // thus, a growing index will automatically be added to the standard name
  int i = 1;
  assert(CCopasiRootContainer::getDatamodelList()->size() > 0);

  while (!(mpModelValue = (*CCopasiRootContainer::getDatamodelList())[0]->getModel()->createModelValue(name)))
    {
      i++;
      name = "quantity_";
      name += TO_UTF8(QString::number(i));
    }

  std::string key = mpModelValue->getKey();
  protectedNotify(ListViews::MODELVALUE, ListViews::ADD, key);
  mpListView->switchToOtherWidget(C_INVALID_INDEX, key);
}

void CQUnitDetail::deleteGlobalQuantity()
{

  assert(CCopasiRootContainer::getDatamodelList()->size() > 0);
  CCopasiDataModel* pDataModel = (*CCopasiRootContainer::getDatamodelList())[0];
  assert(pDataModel != NULL);
  CModel * pModel = pDataModel->getModel();

  if (pModel == NULL)
    return;

  if (mpModelValue == NULL)
    return;

  QMessageBox::StandardButton choice =
    CQMessageBox::confirmDelete(this, "quantity",
                                FROM_UTF8(mpModelValue->getObjectName()),
                                mpModelValue->getDeletedObjects());

  switch (choice)
    {
      case QMessageBox::Ok:
      {
        pDataModel->getModel()->removeModelValue(mKey);
        mpModelValue = NULL;

#undef DELETE
        protectedNotify(ListViews::MODELVALUE, ListViews::DELETE, mKey);
        protectedNotify(ListViews::MODELVALUE, ListViews::DELETE, "");//Refresh all as there may be dependencies.
        break;
      }

      default:
        break;
    }

  mpListView->switchToOtherWidget(115, "");
}

void CQUnitDetail::deleteGlobalQuantity(UndoGlobalQuantityData *pGlobalQuantityData)
{
  assert(CCopasiRootContainer::getDatamodelList()->size() > 0);
  CCopasiDataModel* pDataModel = (*CCopasiRootContainer::getDatamodelList())[0];
  assert(pDataModel != NULL);

  CModel * pModel = pDataModel->getModel();
  assert(pModel != NULL);

  CModelValue * pGQ = pModel->getModelValues()[pGlobalQuantityData->getName()];
  std::string key = pGQ->getKey();
  pModel->removeModelValue(key);
  mpModelValue = NULL;

#undef DELETE
  protectedNotify(ListViews::MODELVALUE, ListViews::DELETE, key);
  protectedNotify(ListViews::MODELVALUE, ListViews::DELETE, "");//Refresh all as there may be dependencies.

  mpListView->switchToOtherWidget(115, "");
}

void CQUnitDetail::addGlobalQuantity(UndoGlobalQuantityData *pSData)
{
  assert(CCopasiRootContainer::getDatamodelList()->size() > 0);
  CCopasiDataModel* pDataModel = (*CCopasiRootContainer::getDatamodelList())[0];
  assert(pDataModel != NULL);

  CModel * pModel = pDataModel->getModel();
  assert(pModel != NULL);

  //reinsert the Global Quantity
  /*  CModelValue *pGlobalQuantity =  pModel->createModelValue(pSData->getName(), pSData->getInitialValue());
    pGlobalQuantity->setStatus(pSData->getStatus());
    std::string key = pGlobalQuantity->getKey();
    protectedNotify(ListViews::MODELVALUE, ListViews::ADD, key);  */

  CModelValue *pGlobalQuantity =  pModel->createModelValue(pSData->getName());
  pGlobalQuantity->setStatus(pSData->getStatus());

  if (pSData->getStatus() != CModelEntity::ASSIGNMENT)
    {
      pGlobalQuantity->setInitialValue(pSData->getInitialValue());
    }

  // set the expression
  if (pSData->getStatus() != CModelEntity::FIXED)
    {
      pGlobalQuantity->setExpression(pSData->getExpression());
      pGlobalQuantity->getExpressionPtr()->compile();
    }

  // set initial expression
  if (pSData->getStatus() != CModelEntity::ASSIGNMENT)
    {
      pGlobalQuantity->setInitialExpression(pSData->getInitialExpression());
      pGlobalQuantity->getInitialExpressionPtr()->compile();
    }

  protectedNotify(ListViews::MODELVALUE, ListViews::ADD, pGlobalQuantity->getKey());

  //restore the reactions the Global Quantity dependent on
  //restore the reactions
  QList <UndoGlobalQuantityData *>::const_iterator k;

  //reinsert all the species
  QList <UndoSpecieData *> *pSpecieData = pSData->getSpecieDependencyObjects();

  if (!pSpecieData->empty())
    {
      QList <UndoSpecieData *>::const_iterator i;

      for (i = pSpecieData->begin(); i != pSpecieData->end(); ++i)
        {
          UndoSpecieData * sData = *i;

          //need to make sure species doesn't exist in the model already
          CMetab *pSpecie =  pModel->createMetabolite(sData->getName(), sData->getCompartment(), sData->getIConc(), sData->getStatus());

          if (pSpecie)
            {

              if (sData->getStatus() != CModelEntity::ASSIGNMENT)
                {
                  pSpecie->setInitialConcentration(sData->getIConc());
                }

              if (sData->getStatus() == CModelEntity::ODE || sData->getStatus() == CModelEntity::ASSIGNMENT)
                {
                  pSpecie->setExpression(sData->getExpression());
                  pSpecie->getExpressionPtr()->compile();
                }

              // set initial expression
              if (sData->getStatus() != CModelEntity::ASSIGNMENT)
                {
                  pSpecie->setInitialExpression(sData->getInitialExpression());
                  pSpecie->getInitialExpressionPtr()->compile();
                }

              protectedNotify(ListViews::METABOLITE, ListViews::ADD, pSpecie->getKey());
            }
        }
    }

  QList <UndoReactionData *> *reactionData = pSData->getReactionDependencyObjects();

  QList <UndoReactionData *>::const_iterator j;

  for (j = reactionData->begin(); j != reactionData->end(); ++j)
    {

      UndoReactionData * rData = *j;

      //need to make sure reaction doesn't exist in the model already
      if (pModel->getReactions().getIndex(rData->getName()) == C_INVALID_INDEX)
        {
          CReaction *pRea =  pModel->createReaction(rData->getName());
          rData->getRi()->createMetabolites();
          rData->getRi()->createOtherObjects();
          rData->getRi()->writeBackToReaction(pRea);
          protectedNotify(ListViews::REACTION, ListViews::ADD, pRea->getKey());
        }
    }

  //reinsert the dependency events
  QList <UndoEventData *> *pEventData = pSData->getEventDependencyObjects();

  if (!pEventData->empty())
    {
      QList <UndoEventData *>::const_iterator ev;

      for (ev = pEventData->begin(); ev != pEventData->end(); ++ev)
        {
          UndoEventData * eData = *ev;

          if (pModel->getEvents().getIndex(eData->getName()) == C_INVALID_INDEX)
            {
              CEvent *pEvent =  pModel->createEvent(eData->getName());

              if (pEvent)
                {
                  std::string key = pEvent->getKey();
                  //set the expressions
                  pEvent->setTriggerExpression(eData->getTriggerExpression());
                  pEvent->setDelayExpression(eData->getDelayExpression());
                  pEvent->setPriorityExpression(eData->getPriorityExpression());

                  QList <UndoEventAssignmentData *> *assignmentData = eData->getEventAssignmentData();
                  QList <UndoEventAssignmentData *>::const_iterator i;

                  for (i = assignmentData->begin(); i != assignmentData->end(); ++i)
                    {
                      UndoEventAssignmentData * assignData = *i;

                      CCopasiObject * pObject = NULL;
                      bool speciesExist = false;
                      size_t ci;

                      for (ci = 0; ci < pModel->getCompartments().size(); ci++)
                        {
                          CCompartment * pCompartment = pModel->getCompartments()[ci];

                          if (pCompartment->getMetabolites().getIndex(assignData->getName()) != C_INVALID_INDEX)
                            speciesExist = true;
                        }

                      if (speciesExist)
                        {
                          size_t index = pModel->findMetabByName(assignData->getName());
                          pObject =  pModel->getMetabolites()[index];
                        }
                      else if (pModel->getModelValues().getIndex(assignData->getName()) != C_INVALID_INDEX)
                        {
                          pObject = pModel->getModelValues()[assignData->getName()];
                        }
                      else if (pModel->getReactions().getIndex(assignData->getName()) != C_INVALID_INDEX)
                        {
                          pObject = pModel->getReactions()[assignData->getName()];
                        }

                      const CModelEntity * pEntity = dynamic_cast< const CModelEntity * >(pObject);

                      CEventAssignment *eventAssign = new CEventAssignment(pObject->getKey(), pEvent->getObjectParent());

                      eventAssign->setExpression(assignData->getExpression());

                      eventAssign->getExpressionPtr()->compile();

                      pEvent->getAssignments().add(eventAssign);
                    }

                  protectedNotify(ListViews::EVENT, ListViews::ADD, key);
                }
            }
        }
    }

  mpListView->switchToOtherWidget(C_INVALID_INDEX, pGlobalQuantity->getKey());
}

void CQUnitDetail::globalQuantityTypeChanged(int type)
{
  ; //TODO
}
#endif

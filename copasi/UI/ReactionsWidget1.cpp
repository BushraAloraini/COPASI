// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/ReactionsWidget1.cpp,v $
//   $Revision: 1.195.10.3 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2009/02/10 14:25:16 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/*********************************************************************
 **  $ CopasiUI/ReactionsWidget1.cpp
 **  $ Author  : Mudita Singhal
 **
 ** This file is used to create the GUI FrontPage for the  information
 ** obtained from the data model about the Recations----It is Basically
 ** the Second level of Reactions.
 **********************************************************************/

#include <qgroupbox.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qtoolbar.h>
#include <qwidget.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qtooltip.h>

#include "copasi.h"
#include "ReactionsWidget1.h"
#include "listviews.h"
#include "parametertable.h"
#include "MyLineEdit.h"
#include "qtUtilities.h"
#include "ChemEqValidator.h"
#include "FunctionWidget1.h"
#include "CQMessageBox.h"
#include "utilities/CCopasiVector.h"
#include "CopasiDataModel/CCopasiDataModel.h"
#include "model/CModel.h"
#include "function/CFunctionDB.h"
#include "function/CFunctionParameters.h"
#include "function/CFunctionParameter.h"
#include "CopasiDataModel/CCopasiDataModel.h"
#include "report/CKeyFactory.h"

/*
 *  Constructs a ReactionsWidget which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */

ReactionsWidget1::ReactionsWidget1(QWidget *parent, const char * name, WFlags f)
    : CopasiWidget(parent, name, f),
    objKey(""),
    mpRi(NULL)
{
  if (!name)
    setName("ReactionsWidget1");
  setCaption(trUtf8("ReactionsWidget1"));

  ReactionsWidget1Layout = new QGridLayout(this, 1, 1, 11, 6, "ReactionsWidget1Layout");

  TextLabel4 = new QLabel(this, "TextLabel4");
  TextLabel4->setText(trUtf8("Name"));
  TextLabel4->setAlignment(int(QLabel::AlignVCenter
                               | QLabel::AlignRight));
  ReactionsWidget1Layout->addWidget(TextLabel4, 0, 0);

  TextLabel7 = new QLabel(this, "TextLabel7");
  TextLabel7->setText(trUtf8("Symbol Definition"));
  TextLabel7->setAlignment(int(QLabel::AlignVCenter
                               | QLabel::AlignRight));
  ReactionsWidget1Layout->addWidget(TextLabel7, 8, 0);

  //Buttons:
  Layout1 = new QHBoxLayout(0, 0, 6, "Layout1");

  commitChanges = new QPushButton(this, "commitChanges");
  commitChanges->setText(trUtf8("Commit"));
  Layout1->addWidget(commitChanges);

  cancelChanges = new QPushButton(this, "cancelChanges");
  cancelChanges->setText(trUtf8("Revert"));
  Layout1->addWidget(cancelChanges);

  newReaction = new QPushButton(this, "newReaction");
  newReaction->setText(trUtf8("&New"));
  Layout1->addWidget(newReaction);

  deleteReaction = new QPushButton(this, "deleteReaction");
  deleteReaction->setText(trUtf8("&Delete"));
  Layout1->addWidget(deleteReaction);

  ReactionsWidget1Layout->addMultiCellLayout(Layout1, 11, 11, 0, 3);

  //

  Line2 = new QFrame(this, "Line2");
  Line2->setFrameShape(QFrame::HLine);
  Line2->setFrameShadow(QFrame::Sunken);
  Line2->setFrameShape(QFrame::HLine);
  ReactionsWidget1Layout->addMultiCellWidget(Line2, 7, 7, 0, 3);

  /*Line1 = new QFrame(this, "Line1");
  Line1->setFrameShape(QFrame::HLine);
  Line1->setFrameShadow(QFrame::Sunken);
  Line1->setFrameShape(QFrame::HLine);
  ReactionsWidget1Layout->addMultiCellWidget(Line1, 11, 11, 0, 3);*/

  /*Line3 = new QFrame(this, "Line3");
  Line3->setFrameShape(QFrame::HLine);
  Line3->setFrameShadow(QFrame::Sunken);
  Line3->setFrameShape(QFrame::HLine);
  ReactionsWidget1Layout->addMultiCellWidget(Line3, 1, 1, 0, 3);*/

  // kinetics line
  TextLabel6 = new QLabel(this, "TextLabel6");
  TextLabel6->setText(trUtf8("Rate Law"));
  TextLabel6->setAlignment(int(QLabel::AlignVCenter
                               | QLabel::AlignRight));
  ReactionsWidget1Layout->addWidget(TextLabel6, 5, 0);

  ComboBox1 = new QComboBox(FALSE, this, "ComboBox1");
  ComboBox1->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  ReactionsWidget1Layout->addMultiCellWidget(ComboBox1, 5, 5, 1, 2);

  newKinetics = new QPushButton(this, "newKinetics");
  newKinetics->setText(trUtf8("&New Rate Law"));
  ReactionsWidget1Layout->addWidget(newKinetics, 5, 3);

  TextLabel8 = new QLabel(this, "TextLabel8");
  TextLabel8->setText(trUtf8("Flux"));
  TextLabel8->setAlignment(int(QLabel::AlignVCenter
                               | QLabel::AlignRight));
  ReactionsWidget1Layout->addWidget(TextLabel8, 6, 0);

  LineEdit3 = new QLineEdit(this, "LineEdit3");
  LineEdit3->setEnabled(FALSE);
  ReactionsWidget1Layout->addMultiCellWidget(LineEdit3, 6, 6, 1, 2);

  //name
  LineEdit1 = new MyLineEdit(this, "LineEdit1");
  ReactionsWidget1Layout->addMultiCellWidget(LineEdit1, 0, 0, 1, 3);

  // equation line
  TextLabel5 = new QLabel(this, "TextLabel5");
  TextLabel5->setText(trUtf8("Chemical Equation"));
  TextLabel5->setAlignment(int(QLabel::AlignVCenter
                               | QLabel::AlignRight));
  ReactionsWidget1Layout->addWidget(TextLabel5, 2, 0);

  LineEdit2 = new MyLineEdit(this, "LineEdit2");
  LineEdit2->setValidator(new ChemEqValidator(LineEdit2));
  ReactionsWidget1Layout->addMultiCellWidget(LineEdit2, 2, 2, 1, 3);

  CheckBox = new QCheckBox(this, "CheckBox");
  CheckBox->setText(trUtf8("Reversible"));
  ReactionsWidget1Layout->addWidget(CheckBox, 3, 1);

  mpMultiCompartment = new QCheckBox(this, "mpMultiCompartment");
  mpMultiCompartment->setText(trUtf8("Multi Compartment"));
  mpMultiCompartment->setEnabled(false);
  ReactionsWidget1Layout->addWidget(mpMultiCompartment, 3, 2);

  Line4 = new QFrame(this, "Line4");
  Line4->setFrameShape(QFrame::HLine);
  Line4->setFrameShadow(QFrame::Sunken);
  Line4->setFrameShape(QFrame::HLine);
  ReactionsWidget1Layout->addMultiCellWidget(Line4, 4, 4, 0, 3);

  table = new ParameterTable(this, "table");
  ReactionsWidget1Layout->addMultiCellWidget(table, 8, 9, 1, 3);

  QSpacerItem* spacer = new QSpacerItem(80, 101, QSizePolicy::Minimum, QSizePolicy::Expanding);
  ReactionsWidget1Layout->addItem(spacer, 10, 0);

  setTabOrder(LineEdit1, LineEdit2);
  setTabOrder(LineEdit2, CheckBox);
  setTabOrder(CheckBox, mpMultiCompartment);
  setTabOrder(mpMultiCompartment, ComboBox1);
  setTabOrder(ComboBox1, newKinetics);
  setTabOrder(newKinetics, table);
  setTabOrder(table, commitChanges);
  setTabOrder(commitChanges, cancelChanges);
  setTabOrder(cancelChanges, newReaction);
  setTabOrder(newReaction, deleteReaction); //TODO !!!!!

  connect(commitChanges, SIGNAL(clicked()), this, SLOT(slotBtnOKClicked()));
  connect(cancelChanges, SIGNAL(clicked()), this, SLOT(slotBtnCancelClicked()));
  connect(newReaction, SIGNAL(clicked()), this, SLOT(slotBtnNewClicked()));
  connect(deleteReaction, SIGNAL(clicked()), this, SLOT(slotBtnDeleteClicked()));

  connect(newKinetics, SIGNAL(clicked()), this, SLOT(slotNewFunction()));

  connect(CheckBox, SIGNAL(clicked()), this, SLOT(slotCheckBoxClicked()));
  connect(ComboBox1, SIGNAL(activated(const QString &)), this, SLOT(slotComboBoxSelectionChanged(const QString &)));
  connect(LineEdit2, SIGNAL(edited()), this, SLOT(slotLineEditChanged()));
  connect(LineEdit1, SIGNAL(edited()), this, SLOT(slotNameChanged()));

  //connect(table, SIGNAL(signalChanged(int, int, Qstring)), this, SLOT(slotTableChanged(int, int, QString)));
}

ReactionsWidget1::~ReactionsWidget1()
{}

/* This function loads the reactions widget when its name is
   clicked in the tree   */
bool ReactionsWidget1::loadFromReaction(const CReaction* reaction)
{
  if (!reaction) return false;

  // this loads the reaction into a CReactionInterface object.
  // the gui works on this object and later writes back the changes to the reaction
  pdelete(mpRi);
  mpRi = new CReactionInterface(CCopasiDataModel::Global->getModel());

  mpRi->initFromReaction(reaction->getKey());

  // update the widget.
  FillWidgetFromRI();

  return true; //TODO: really check
}

bool ReactionsWidget1::saveToReaction()
{
  CReaction* reac = dynamic_cast< CReaction * >(GlobalKeys.get(objKey));
  if (reac == NULL) return true;

  if (!LineEdit2->isValid()) return false;
  LineEdit2->slotForceUpdate();

  if (!mpRi->isValid()) return false;

  CModel * pModel = CCopasiDataModel::Global->getModel();
  if (pModel == NULL) return false;

  unsigned C_INT32 ReactionIndex = pModel->getReactions().getIndex(mpRi->getReactionName());

  // Before we save any changes we must check whether any local reaction parameters,
  // which are used in any mathematical expression in the model are removed.
  // If that is the case the user must have option to cancel the changes or remove the
  // affected expressions.
  std::set< const CCopasiObject * > DeletedParameters = mpRi->getDeletedParameters();
  if (DeletedParameters.size() != 0)
    {
      QString parameterList = "Are you sure you want to delete listed PARAMETERS ?\n";
      QString affectedCompartmentList = "Following COMPARTMENT(S) reference above PARAMETERS and will be deleted -\n";
      QString affectedMetabList = "Following METABOLITE(S) reference above PARAMETERS and will be deleted -\n";
      QString affectedReacList = "Following REACTION(S) reference above PARAMETERS and will be deleted -\n";
      QString affectedValueList = "Following MODEL VALUE(S) reference above PARAMETERS and will be deleted -\n";

      bool compartmentFound = false;
      bool metabFound = false;
      bool reacFound = false;
      bool valueFound = false;

      std::set< const CCopasiObject * >::const_iterator itParameter, endParameter = DeletedParameters.end();

      for (itParameter = DeletedParameters.begin(); itParameter != endParameter; ++itParameter) //all parameters
        {
          parameterList.append(FROM_UTF8((*itParameter)->getObjectName()));
          parameterList.append(", ");

          std::set< const CCopasiObject * > DeletedObjects;
          DeletedObjects.insert((*itParameter)->getObject(CCopasiObjectName("Reference=Value")));

          std::set< const CCopasiObject * > Reactions;
          std::set< const CCopasiObject * > Metabolites;
          std::set< const CCopasiObject * > Values;
          std::set< const CCopasiObject * > Compartments;

          pModel->appendDependentModelObjects(DeletedObjects,
                                              Reactions, Metabolites, Compartments, Values);

          if (Reactions.size() > 0)
            {
              reacFound = true;
              std::set< const CCopasiObject * >::const_iterator it, itEnd = Reactions.end();
              for (it = Reactions.begin(); it != itEnd; ++it)
                {
                  affectedReacList.append(FROM_UTF8((*it)->getObjectName()));
                  affectedReacList.append(", ");
                }

              affectedReacList.remove(affectedReacList.length() - 2, 2);
              affectedReacList.append("  ---> ");
              affectedReacList.append(FROM_UTF8((*itParameter)->getObjectName()));
              affectedReacList.append("\n");
            }

          if (Metabolites.size() > 0)
            {
              metabFound = true;
              std::set< const CCopasiObject * >::const_iterator it, itEnd = Metabolites.end();
              for (it = Metabolites.begin(); it != itEnd; ++it)
                {
                  affectedMetabList.append(FROM_UTF8((*it)->getObjectName()));
                  affectedMetabList.append(", ");
                }

              affectedMetabList.remove(affectedMetabList.length() - 2, 2);
              affectedMetabList.append("  ---> ");
              affectedMetabList.append(FROM_UTF8((*itParameter)->getObjectName()));
              affectedMetabList.append("\n");
            }

          if (Values.size() > 0)
            {
              valueFound = true;
              std::set< const CCopasiObject * >::const_iterator it, itEnd = Values.end();
              for (it = Values.begin(); it != itEnd; ++it)
                {
                  affectedValueList.append(FROM_UTF8((*it)->getObjectName()));
                  affectedValueList.append(", ");
                }

              affectedValueList.remove(affectedValueList.length() - 2, 2);
              affectedValueList.append("  ---> ");
              affectedValueList.append(FROM_UTF8((*itParameter)->getObjectName()));
              affectedValueList.append("\n");
            }

          if (Compartments.size() > 0)
            {
              compartmentFound = true;
              std::set< const CCopasiObject * >::const_iterator it, itEnd = Compartments.end();
              for (it = Compartments.begin(); it != itEnd; ++it)
                {
                  affectedCompartmentList.append(FROM_UTF8((*it)->getObjectName()));
                  affectedCompartmentList.append(", ");
                }

              affectedCompartmentList.remove(affectedCompartmentList.length() - 2, 2);
              affectedCompartmentList.append("  ---> ");
              affectedCompartmentList.append(FROM_UTF8((*itParameter)->getObjectName()));
              affectedCompartmentList.append("\n");
            }
        }

      parameterList.remove(parameterList.length() - 2, 2);

      QString msg = parameterList;

      if (compartmentFound)
        {
          msg.append("\n \n");
          msg.append(affectedCompartmentList);
        }

      if (metabFound)
        {
          msg.append("\n \n");
          msg.append(affectedMetabList);
        }

      if (reacFound)
        {
          msg.append("\n \n");
          msg.append(affectedReacList);
        }

      if (valueFound)
        {
          msg.append("\n \n");
          msg.append(affectedValueList);
        }

      C_INT32 choice = 0;
      if (metabFound || reacFound || valueFound || valueFound)
        choice = CQMessageBox::question(this,
                                        "CONFIRM DELETE",
                                        msg,
                                        "Continue", "Cancel", 0, 1, 1);

      switch (choice)
        {
          // Yes or Enter
        case 0:
          for (itParameter = DeletedParameters.begin(); itParameter != endParameter; ++itParameter) //all parameters
            pModel->removeLocalReactionParameter((*itParameter)->getKey());
          break;

          // No or Escape
        default:
          return true;
          break;
        }
    }

  // We need to check whether the current reaction still exists, since it is possible that
  // removing a local reaction parameter triggers its deletion.
  reac = dynamic_cast< CReaction * >(GlobalKeys.get(objKey));
  if (reac == NULL)
    {
      unsigned C_INT32 size = CCopasiDataModel::Global->getModel()->getReactions().size();

      mpRi->setFunctionWithEmptyMapping("");

      if (size > 0)
        enter(CCopasiDataModel::Global->getModel()->getReactions()[std::min(ReactionIndex, size - 1)]->getKey());
      else
        enter("");

      protectedNotify(ListViews::REACTION, ListViews::DELETE, objKey);
      return true;
    }

  //first check if new metabolites need to be created
  bool createdMetabs = mpRi->createMetabolites();
  bool createdObjects = mpRi->createOtherObjects();

  mpRi->setReactionName((const char *)LineEdit1->text().utf8());

  //this writes all changes to the reaction
  if (!mpRi->writeBackToReaction(NULL))
    {
      if (mpRi->getReactionName() != reac->getObjectName())
        {
          QString msg;
          msg = "Unable to rename reaction '" + FROM_UTF8(reac->getObjectName()) + "'\n"
                + "to '" + FROM_UTF8(mpRi->getReactionName()) + "' since a reation with that name already exists.";

          CQMessageBox::information(this,
                                    "Unable to rename Reaction",
                                    msg,
                                    QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);

          mpRi->setReactionName(reac->getObjectName());
          LineEdit1->setText(FROM_UTF8(mpRi->getReactionName()));
        }
    }

  //CCopasiDataModel::Global->getModel()->compile();

  //this tells the gui what it needs to know.
  if (createdObjects)
    protectedNotify(ListViews::MODEL, ListViews::CHANGE, "");
  else
    {
      if (createdMetabs) protectedNotify(ListViews::METABOLITE, ListViews::ADD, "");
      protectedNotify(ListViews::REACTION, ListViews::CHANGE, objKey);
    }

  //TODO: detect rename events (mpRi->writeBackToReaction has to do this)

  // :TODO Bug 322: This should only be called when actual changes have been saved.
  CCopasiDataModel::Global->changed();

  return true;
}

void ReactionsWidget1::slotBtnCancelClicked()
{enter(objKey);}

void ReactionsWidget1::slotBtnOKClicked()
{
  // This assures that even the last edit is regocnized.
  commitChanges->setFocus();

  saveToReaction();
  CReaction* reac = dynamic_cast< CReaction * >(GlobalKeys.get(objKey));

  if (reac == NULL) return;
  loadFromReaction(reac);
}

void ReactionsWidget1::slotCheckBoxClicked()
{
  LineEdit2->slotForceUpdate();

  // tell the reaction interface
  mpRi->setReversibility(CheckBox->isChecked(), "");

  // update the widget
  FillWidgetFromRI();
}

// the function has been changed
void ReactionsWidget1::slotComboBoxSelectionChanged(const QString & p2)
{
  // tell the reaction interface
  mpRi->setFunctionAndDoMapping((const char *)p2.utf8());

  // update the widget
  FillWidgetFromRI();
}

/*This function is called when the "Chemical Reaction" LineEdit is changed.*/
void ReactionsWidget1::slotLineEditChanged()
{
  //std::string rName = (const char *)LineEdit1->text().utf8();

  std::string eq = (const char *)LineEdit2->text().utf8();

  //first check if the string is a valid equation
  if (!CChemEqInterface::isValidEq(eq))
    {
      return;  // abort further processing
    }

  // tell the reaction interface
  //mpRi->setReactionName(rName);

  mpRi->setChemEqString(eq, "");

  // update the widget
  FillWidgetFromRI();
}

void ReactionsWidget1::slotNameChanged()
{
  std::string rName = (const char *)LineEdit1->text().utf8();
  mpRi->setReactionName(rName);
}

// added 5/19/04
void ReactionsWidget1::slotBtnNewClicked()
{
  slotBtnOKClicked();

  std::string name = "reaction";
  int i = 0;
  while (!CCopasiDataModel::Global->getModel()->createReaction(name))
    {
      i++;
      name = "reaction_";
      name += QString::number(i).utf8();
    }
  protectedNotify(ListViews::REACTION, ListViews::ADD);
  enter(CCopasiDataModel::Global->getModel()->getReactions()[name]->getKey());
  //pListView->switchToOtherWidget(mKeys[row]);
}

// Just added 5/18/04
void ReactionsWidget1::slotBtnDeleteClicked()
{
  CModel * pModel = CCopasiDataModel::Global->getModel();
  if (pModel == NULL)
    return;

  CReaction * pReaction =
    dynamic_cast< CReaction * >(GlobalKeys.get(objKey));
  if (pReaction == NULL) return;

  QString reactionList = "Are you sure you want to delete listed REACTION(S) ?\n";
  QString affectedCompartmentList = "Following COMPARTMENT(S) reference above REACTION(S) and will be deleted -\n";
  QString affectedMetabList = "Following SPECIES reference above REACTION(S) and will be deleted -\n";
  QString affectedReacList = "Following REACTION(S) reference above REACTION(S) and will be deleted -\n";
  QString affectedValueList = "Following MODEL VALUE(S) reference above REACTION(S) and will be deleted -\n";

  bool compartmentFound = false;
  bool metabFound = false;
  bool reacFound = false;
  bool valueFound = false;

  reactionList.append(FROM_UTF8(pReaction->getObjectName()));
  reactionList.append(", ");

  std::set< const CCopasiObject * > Reactions;
  std::set< const CCopasiObject * > Metabolites;
  std::set< const CCopasiObject * > Values;
  std::set< const CCopasiObject * > Compartments;

  pModel->appendDependentModelObjects(pReaction->getDeletedObjects(),
                                      Reactions, Metabolites, Compartments, Values);

  if (Reactions.size() > 0)
    {
      reacFound = true;
      std::set< const CCopasiObject * >::const_iterator it, itEnd = Reactions.end();
      for (it = Reactions.begin(); it != itEnd; ++it)
        {
          affectedReacList.append(FROM_UTF8((*it)->getObjectName()));
          affectedReacList.append(", ");
        }

      affectedReacList.remove(affectedReacList.length() - 2, 2);
      affectedReacList.append("  ---> ");
      affectedReacList.append(FROM_UTF8(pReaction->getObjectName()));
      affectedReacList.append("\n");
    }

  if (Metabolites.size() > 0)
    {
      metabFound = true;
      std::set< const CCopasiObject * >::const_iterator it, itEnd = Metabolites.end();
      for (it = Metabolites.begin(); it != itEnd; ++it)
        {
          affectedMetabList.append(FROM_UTF8((*it)->getObjectName()));
          affectedMetabList.append(", ");
        }

      affectedMetabList.remove(affectedMetabList.length() - 2, 2);
      affectedMetabList.append("  ---> ");
      affectedMetabList.append(FROM_UTF8(pReaction->getObjectName()));
      affectedMetabList.append("\n");
    }

  if (Values.size() > 0)
    {
      valueFound = true;
      std::set< const CCopasiObject * >::const_iterator it, itEnd = Values.end();
      for (it = Values.begin(); it != itEnd; ++it)
        {
          affectedValueList.append(FROM_UTF8((*it)->getObjectName()));
          affectedValueList.append(", ");
        }

      affectedValueList.remove(affectedValueList.length() - 2, 2);
      affectedValueList.append("  ---> ");
      affectedValueList.append(FROM_UTF8(pReaction->getObjectName()));
      affectedValueList.append("\n");
    }

  if (Compartments.size() > 0)
    {
      compartmentFound = true;
      std::set< const CCopasiObject * >::const_iterator it, itEnd = Compartments.end();
      for (it = Compartments.begin(); it != itEnd; ++it)
        {
          affectedCompartmentList.append(FROM_UTF8((*it)->getObjectName()));
          affectedCompartmentList.append(", ");
        }

      affectedCompartmentList.remove(affectedCompartmentList.length() - 2, 2);
      affectedCompartmentList.append("  ---> ");
      affectedCompartmentList.append(FROM_UTF8(pReaction->getObjectName()));
      affectedCompartmentList.append("\n");
    }

  reactionList.remove(reactionList.length() - 2, 2);

  QString msg = reactionList;

  if (compartmentFound)
    {
      msg.append("\n \n");
      msg.append(affectedCompartmentList);
    }

  if (metabFound)
    {
      msg.append("\n \n");
      msg.append(affectedMetabList);
    }

  if (reacFound)
    {
      msg.append("\n \n");
      msg.append(affectedReacList);
    }

  if (valueFound)
    {
      msg.append("\n \n");
      msg.append(affectedValueList);
    }

  C_INT32 choice = 0;
  if (metabFound || reacFound || valueFound || valueFound)
    choice = CQMessageBox::question(this,
                                    "CONFIRM DELETE",
                                    msg,
                                    "Continue", "Cancel", 0, 1, 1);

  switch (choice)
    {
    case 0:                                                     // Yes or Enter
      {
        unsigned C_INT32 index
        = CCopasiDataModel::Global->getModel()->getReactions().getIndex(mpRi->getReactionName());

        CCopasiDataModel::Global->getModel()->removeReaction(objKey);
        unsigned C_INT32 size
        = CCopasiDataModel::Global->getModel()->getReactions().size();

        mpRi->setFunctionWithEmptyMapping("");

        if (size > 0)
          enter(CCopasiDataModel::Global->getModel()->getReactions()[std::min(index, size - 1)]->getKey());
        else
          enter("");

        protectedNotify(ListViews::REACTION, ListViews::DELETE, objKey);
        break;
      }
    default:                                                     // No or Escape
      break;
    }
}

void ReactionsWidget1::FillWidgetFromRI()
{
  LineEdit1->setText(FROM_UTF8(mpRi->getReactionName()));

  LineEdit2->setText(FROM_UTF8(mpRi->getChemEqString()));

  setFramework(mFramework);

  // the reversibility checkbox
  CheckBox->setChecked(false);
  if (mpRi->isReversible() == true)
    {
      CheckBox->setChecked(true);
    }

  mpMultiCompartment->setChecked(mpRi->isMulticompartment());

  // the function combobox
  QStringList comboEntries;
  vectorOfStrings2QStringList(mpRi->getListOfPossibleFunctions(), comboEntries);

  ComboBox1->clear();
  ComboBox1->insertStringList(comboEntries, -1);

  // if there is a current function the parameter table is initialized
  if (mpRi->getFunctionName() != "")
    {
      if (comboEntries.grep(FROM_UTF8(mpRi->getFunctionName())).size() == 0)
        ComboBox1->insertItem(FROM_UTF8(mpRi->getFunctionName()));

      ComboBox1->setCurrentText(FROM_UTF8(mpRi->getFunctionName()));
      QToolTip::add(ComboBox1, FROM_UTF8(mpRi->getFunctionDescription()));

      table->updateTable(*mpRi, *CCopasiDataModel::Global->getModel());
    }
  else
    {
      ComboBox1->insertItem("undefined");
      ComboBox1->setCurrentText("undefined");
      table->initTable();
    }

  //TODO isValid()
  commitChanges->setEnabled(mpRi->isValid());
}

void ReactionsWidget1::slotTableChanged(int index, int sub, QString newValue)
{
  // setValue
  if (mpRi->getUsage(index) == CFunctionParameter::PARAMETER)
    {
      if (sub != 0) return;

      if (mpRi->isLocalValue(index))
        mpRi->setLocalValue(index, newValue.toDouble()); // TODO: check
      else
        mpRi->setMapping(index, (const char *)newValue.utf8());
    }
  else if (mpRi->getUsage(index) == CFunctionParameter::VOLUME)
    {
      if (sub != 0) return;
      mpRi->setMapping(index, (const char *)newValue.utf8());
    }
  else
    {
      if (sub == 0) //here we assume that vector parameters cannot be edited
        {
          mpRi->setMapping(index, (const char *)table->text(table->mIndex2Line[index], 3).utf8());
        }
    }

  // update the widget
  int rrr = table->currentRow();
  int ccc = table->currentColumn();
  FillWidgetFromRI();
  table->setCurrentCell(rrr, ccc);
}

void ReactionsWidget1::slotParameterStatusChanged(int index, bool local)
{
  if (local)
    mpRi->setLocal(index);
  else
    mpRi->setMapping(index, "unknown"); //TODO keep global parameter

  // update the widget
  int rrr = table->currentRow();
  int ccc = table->currentColumn();
  FillWidgetFromRI();
  table->setCurrentCell(rrr, ccc);
}

void ReactionsWidget1::slotNewFunction()
{
  // FunctionWidget1 * fw = new FunctionWidget1(NULL);
  // fw->show();
  // TODO: we could think about calling the function widget as a dialogue here...

  std::string name = std::string("Rate Law for ") + (const char *)LineEdit1->text().utf8();
  std::string nname = name;
  int i = 0;
  CCopasiVectorN<CEvaluationTree>& FunctionList
  = CCopasiDataModel::Global->getFunctionList()->loadedFunctions();
  CFunction* pFunc;

  while (FunctionList.getIndex(nname) != C_INVALID_INDEX)
    {
      i++;
      nname = name + "_";
      nname += (const char *)QString::number(i).utf8();
    }

  CCopasiDataModel::Global->getFunctionList()->add(pFunc = new CKinFunction(nname), true);
  protectedNotify(ListViews::FUNCTION, ListViews::ADD);

  mpListView->switchToOtherWidget(0, pFunc->getKey());
}

bool ReactionsWidget1::update(ListViews::ObjectType objectType,
                              ListViews::Action C_UNUSED(action), const std::string & C_UNUSED(key))
{
  if (mIgnoreUpdates) return true;

  switch (objectType)
    {
    case ListViews::MODEL:
    case ListViews::STATE:
    case ListViews::COMPARTMENT:
    case ListViews::METABOLITE:
      return loadFromReaction(dynamic_cast< CReaction * >(GlobalKeys.get(objKey)));
      break;

    default:
      break;
    }
  return true;
}

bool ReactionsWidget1::leave()
{
  saveToReaction();
  return true; //always return true. That means that the widget can be
  //left without saving
}

bool ReactionsWidget1::enter(const std::string & key)
{
  objKey = key;
  CReaction* reac = dynamic_cast< CReaction * >(GlobalKeys.get(key));

  if (reac)
    return loadFromReaction(reac);

  mpListView->switchToOtherWidget(114, "");
  return false;
}

void ReactionsWidget1::setFramework(int framework)
{
  CopasiWidget::setFramework(framework);

  const CReaction * pReaction = dynamic_cast< CReaction * >(GlobalKeys.get(objKey));

  const CModel * pModel = NULL;
  if (pReaction != NULL)
    pModel = dynamic_cast<const CModel *>(pReaction->getObjectAncestor("Model"));

  QString Units;

  switch (mFramework)
    {
    case 0:
      if (pModel)
        Units = FROM_UTF8(pModel->getQuantityRateUnits());
      if (!Units.isEmpty())
        Units = " (" + Units + ")";
      TextLabel8->setText("Flux" + Units);
      if (pReaction != NULL)
        LineEdit3->setText(QString::number(pReaction->getFlux()));
      break;

    case 1:
      if (pModel)
        Units = FROM_UTF8(pModel->getFrequencyUnits());
      if (!Units.isEmpty())
        Units = " (" + Units + ")";
      TextLabel8->setText("Particle Flux" + Units);
      if (pReaction != NULL)
        LineEdit3->setText(QString::number(pReaction->getParticleFlux()));
      break;
    }
}

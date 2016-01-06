// Copyright (C) 2010 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 - 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2002 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/****************************************************************************
 **  $ CopasiUI/ReactionsWidget1.h
 **  $ Author  : Mudita Singhal
 **
 ** This is the header file for the Reactions Widget, i.e the second level
 ** of Reactions.
 **
 *****************************************************************************/

#ifndef REACTIONS_WIDGET1_H
#define REACTIONS_WIDGET1_H

#include "copasi/UI/ui_ReactionsWidget1.h"

#include "copasi/UI/copasiWidget.h"
#include "copasi/model/CReactionInterface.h"

class ParameterTable;
class MyLineEdit;

#include <copasi/undoFramework/CCopasiUndoCommand.h>
class ReactionChangeCommand;

class ReactionsWidget1 : public CopasiWidget, public Ui::ReactionsWidget1
{
  Q_OBJECT

  friend class DeleteReactionCommand;
  friend class CreateNewReactionCommand;

public:
  ReactionsWidget1(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0);
  ~ReactionsWidget1();

  virtual bool update(ListViews::ObjectType objectType, ListViews::Action action, const std::string & key);
  virtual bool leave();
  virtual void setFramework(int framework);
  void copy();

protected slots:
  virtual void slotBtnNew();
  virtual void slotBtnCopy(); //dummy, to bypass warnings from TabWidget connections
  virtual void slotBtnDelete();
  virtual void slotCheckBoxClicked();
  virtual void slotComboBoxSelectionChanged(const QString &);
  virtual void slotLineEditChanged();
  virtual void slotGotoFunction();
  virtual void slotNewFunction();

  //  public slots:
  virtual void slotTableChanged(int index, int sub, QString newValue);
  virtual void slotParameterStatusChanged(int index, bool local);

protected:
  virtual bool enterProtected();
  bool loadFromReaction(const CReaction* reaction);
  bool saveToReaction();
  void FillWidgetFromRI();

  CReactionInterface * mpRi;

  //additional functions for UNDO framework
  void deleteReaction();
  void addReaction(std::string & reaObjectName, CReactionInterface *pRi);
  void addReaction(UndoReactionData *pData);
  void createNewReaction();
  void deleteReaction(CReaction *pReaction);
public:
  bool changeReaction(const std::string& key,
                      CCopasiUndoCommand::Type type,
                      const QVariant& newValue,
                      const QVariant& newSecondValueValue,
                      ReactionChangeCommand* command
                     );
};

#endif // REACTIONSWIDGET1_H

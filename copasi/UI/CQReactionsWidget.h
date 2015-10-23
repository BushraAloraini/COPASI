// Copyright (C) 2010 - 2015 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

#ifndef CQReactionsWidget_h
#define CQReactionsWidget_h

#include <QtCore/QVariant>

#include "copasi/UI/ui_CQReactionsWidget.h"

#include "copasi/UI/CQSortFilterProxyModel.h"
#include "copasi/UI/CQReactionDM.h"

class CQReactionsWidget : public CopasiWidget, public Ui::CQReactionsWidget
{
  Q_OBJECT

public:
  CQReactionsWidget(QWidget* parent = 0, const char* name = 0);
  ~CQReactionsWidget();

  virtual bool update(ListViews::ObjectType objectType, ListViews::Action action, const std::string & key);
  virtual bool leave();
  virtual void setFramework(int framework);

private:
  CQReactionDM* mpReactionDM;
  CQSortFilterProxyModel *mpProxyModel;
  void deleteSelectedReactions();
  void updateDeleteBtns();

protected:
  virtual bool enterProtected();
  virtual void keyPressEvent(QKeyEvent* ev);

protected slots:
  virtual void slotBtnNewClicked();
  virtual void slotBtnDeleteClicked();
  virtual void slotBtnClearClicked();
  virtual void slotSelectionChanged(const QItemSelection& selected,
                                    const QItemSelection& deselected);
  virtual void slotDoubleClicked(const QModelIndex proxyIndex);
  virtual void dataChanged(const QModelIndex& topLeft,
                           const QModelIndex& bottomRight);
  virtual void slotFilterChanged();

};

#endif // CQReactionsWidget_h

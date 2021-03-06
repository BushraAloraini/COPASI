// Copyright (C) 2017 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and University of
// of Connecticut School of Medicine.
// All rights reserved.

// Copyright (C) 2011 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

#ifndef CLNAResultWidget_H__
#define CLNAResultWidget_H__

#include "UI/copasiWidget.h"
#include <QGridLayout>

class CLNAResultSubwidget;

class CLNAResultWidget : public CopasiWidget
{
  Q_OBJECT

public:
  CLNAResultWidget(QWidget *parent = 0, const char *name = 0, Qt::WindowFlags fl = 0);
  ~CLNAResultWidget();

  virtual bool update(ListViews::ObjectType objectType, ListViews::Action action, const std::string &key);
  virtual bool leave();
  bool loadFromBackend();

protected slots:

protected:
  virtual bool enterProtected();
  bool saveToBackend();

  QGridLayout *mWidgetLayout;

  CLNAResultSubwidget *mCentralWidget;
};

#endif

// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/CQTaskMethodWidget.h,v $
//   $Revision: 1.1.2.1 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2011/01/04 13:57:48 $
// End CVS Header

// Copyright (C) 2010 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

#ifndef COPASI_CQTaskMethodWidget
#define COPASI_CQTaskMethodWidget

#include <qvariant.h>

#include "copasi/UI/ui_CQTaskMethodWidget.h"

class CCopasiTask;
class CCopasiMethod;

class CQTaskMethodWidget: public QWidget, public Ui::CQTaskMethodWidget
{
  Q_OBJECT

public:
  CQTaskMethodWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);

  virtual ~CQTaskMethodWidget();

  void setTask(CCopasiTask * pTask);

  void setValidMethods(const unsigned int * validMehtods);

  void enableMethodParameter(const bool & enable);

  bool loadMethod();

  bool saveMethod();


protected slots:
  virtual void languageChange();

  void changeMethod(int);

protected:
  CCopasiTask * mpTask;
  CCopasiMethod * mpMethod;
};

#endif // COPASI_CQTaskMethodWidget

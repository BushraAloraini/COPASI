// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/Attic/CMIRIAMModelWidget.h,v $
//   $Revision: 1.3 $
//   $Name:  $
//   $Author: ssahle $
//   $Date: 2007/10/29 13:45:14 $
// End CVS Header

// Copyright (C) 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#ifndef COPASI_CMIRIAMMODELWIDGET
#define COPASI_CMIRIAMMODELWIDGET

#include "copasiWidget.h"

class CAuthorsWidget;
class QPushButton;

class CMIRIAMModelWidget : public CopasiWidget
  {
    Q_OBJECT
  public:
    CMIRIAMModelWidget(QWidget *parent, const char * name = 0, WFlags f = 0);

  private:
    CAuthorsWidget *mAuthorsWidget;

    QPushButton* btnOK;
    QPushButton* btnCancel;
    QPushButton* btnDelete;
    QPushButton* btnNew;
    QPushButton* btnClear;

  protected slots:
    virtual void slotBtnOKClicked();
    virtual void slotBtnCancelClicked();
    virtual void slotBtnDeleteClicked();
    virtual void slotBtnNewClicked();
    virtual void slotBtnClearClicked();

    virtual void slotEnableOKAndCancel(bool e);
  };

#endif // COPASI_CMIRIAMMODELWIDGET

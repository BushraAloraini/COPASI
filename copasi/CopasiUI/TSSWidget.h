/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/CopasiUI/Attic/TSSWidget.h,v $
   $Revision: 1.7 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2005/10/13 13:43:45 $
   End CVS Header */

#ifndef TSSWIDGET_H
#define TSSWIDGET_H

#include "copasi.h" 
//#include <qvariant.h>
//#include <qwidget.h>
#include "TaskWidget.h"

//class QVBoxLayout;
//class QHBoxLayout;
//class QGridLayout;
//class QCheckBox;
//class QFrame;
//class QLabel;
//class QLineEdit;
//class QPushButton;
//class QRadioButton;
//class QTable;

class TSSWidget : public TaskWidget
  {
    Q_OBJECT

  public:
    //virtual bool update(ListViews::ObjectType objectType, ListViews::Action action, const std::string & key);
    //virtual bool leave();
    //virtual bool enter(const std::string & key = "");

    TSSWidget(QWidget* parent = 0, const char* name = 0, WFlags fl = 0);
    ~TSSWidget();

    virtual bool runTask();

  protected:
    virtual bool loadTask();
    virtual bool saveTask();
    virtual CCopasiMethod * createMethod(const CCopasiMethod::SubType & type);

    //QCheckBox* taskJacobian;
    //QCheckBox* taskStability;
    //QLabel* taskDescriptionLabel;
    //QLabel* parameterValueLabel;
    //QCheckBox* bExecutable;
    // QCheckBox* setInitialState;
    //QLineEdit* taskName;
    //QTable* parameterTable;
    //QPushButton* bRunButton;
    //QPushButton* cancelChange;
    //QPushButton* ExportFileButton;
    //QPushButton* reportDefinitionButton;

  public slots:
    //virtual void CancelButtonClicked();
    //virtual void CommitButtonClicked();
    //virtual void ExportToFileButtonClicked();
    //virtual void parameterValueChanged();
    //virtual void RunButtonChecked();
    //virtual void runTask();
    //virtual void ReportDefinitionClicked();

  protected:
  };

#endif // STEADYSTATEWIDGET_H

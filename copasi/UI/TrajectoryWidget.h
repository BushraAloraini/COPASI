/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/Attic/TrajectoryWidget.h,v $
   $Revision: 1.35 $
   $Name:  $
   $Author: shoops $
   $Date: 2006/04/27 01:27:46 $
   End CVS Header */

// Copyright � 2005 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/********************************************************
Author: Liang Xu
Version : 1.xx  <first>
Description:
Date: 02/03
Comment : TrajectoryWidget
Contact: Please contact lixu1@vt.edu.
 *********************************************************/
#ifndef TRAJECTORYWIDGET_H
#define TRAJECTORYWIDGET_H

#include "copasi.h"
#include <qvariant.h>
#include <qwidget.h>
#include "copasiWidget.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QCheckBox;
class QComboBox;
class QFrame;
class QLabel;
class QLineEdit;
class QPushButton;
class QTable;
class CTrajectoryTask;
class CTrajectoryProblem;
class CTrajectoryMethod;
class CModel;

class TrajectoryWidget : public CopasiWidget
  {
    Q_OBJECT

  public:
    TrajectoryWidget(QWidget* parent = 0, const char* name = 0, WFlags fl = 0);
    ~TrajectoryWidget();

    virtual bool update(ListViews::ObjectType objectType, ListViews::Action action, const std::string & key);
    virtual bool leave();
    virtual bool enter(const std::string & key = "");

  signals:

  protected slots:
    virtual void CancelChange();
    virtual void CommitChange();
    virtual void UpdateMethod(const bool & update = true);
    virtual void ReportDefinitionClicked();
    virtual void outputDefinitionClicked();

    virtual void DurationSlot();
    virtual void StepsizeSlot();
    virtual void NumStepsSlot();

    //checks whether the time series will be too huge
    void checkTimeSeries();

  public slots:
    virtual void runTrajectoryTask();

  private:

    std::string objKey;
    void loadTrajectoryTask();
    void saveTrajectoryTask();
    void loadMethodParameters();

    //QWidget* pParent;
    QGridLayout* TrajectoryWidgetLayout;
    QHBoxLayout* Layout2;
    QFrame* line8;
    QTable* parameterTable;
    QLabel* taskNameLabel;
    QFrame* line7;
    QFrame* line7_2;
    QComboBox* ComboBox1;
    QCheckBox* bExecutable, *setInitialState;
    QCheckBox* bStoreTimeSeries;
    QLabel* parameterValueLabel;
    QLabel* TextLabel1;
    QLabel* TextLabel1_3_2;
    QLineEdit* nStepSize;
    QLabel* TextLabel1_2_2;
    QLineEdit* nDuration;
    QLineEdit* nStepNumber;
    QLabel* TextLabel1_2;
    QFrame* line6;
    QPushButton* bRunTask;
    QPushButton* cancelChange;
    QPushButton* reportDefinitionButton;
    QPushButton* outputDefinitionButton;
    QLineEdit* mLineEditStartOutput;
    QCheckBox* mCheckBoxStartOutput;
    CTrajectoryProblem * mpProblem;
  };

#endif // TRAJECTORYWIDGET_H

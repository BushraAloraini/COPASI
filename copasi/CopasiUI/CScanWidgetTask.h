/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/CopasiUI/Attic/CScanWidgetTask.h,v $
   $Revision: 1.6 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2006/03/28 23:09:05 $
   End CVS Header */

/****************************************************************************
 ** Form interface generated from reading ui file 'CScanWidgetTask.ui'
 **
 ** Created: Tue Mar 28 14:23:46 2006
 **      by: The User Interface Compiler ($Id: CScanWidgetTask.h,v 1.6 2006/03/28 23:09:05 shoops Exp $)
 **
 ** WARNING! All changes made in this file will be lost!
 ****************************************************************************/

#ifndef CSCANWIDGETTASK_H
 #define CSCANWIDGETTASK_H

#include <qvariant.h>
 #include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QFrame;
class QLabel;
class QComboBox;
class QToolButton;
class QCheckBox;
class CModel;
class CCopasiObject;
class CScanProblem;

class CScanWidgetTask : public QWidget
  {
    Q_OBJECT

  public:
    CScanWidgetTask(QWidget* parent = 0, const char* name = 0, WFlags fl = 0);
    ~CScanWidgetTask();

    QFrame* frame;
    QLabel* labelTitle;
    QComboBox* comboType;
    QToolButton* buttonEdit;
    QCheckBox* checkInitialConditions;
    QCheckBox* checkOutput;

    virtual bool initFromScanProblem(CScanProblem * pg, const CModel * model);
    virtual bool saveToScanProblem(CScanProblem * pg) const;

  protected:
    CCopasiObject* mpObject;
    const CModel* mpModel;

    QVBoxLayout* CScanWidgetTaskLayout;
    QVBoxLayout* frameLayout;
    QHBoxLayout* layout1;
    QSpacerItem* spacer1;
    QSpacerItem* spacer2;
    QHBoxLayout* layout2;
    QSpacerItem* spacer3;
    QSpacerItem* spacer4;

  protected slots:
    virtual void languageChange();

  private:
    void init();

  private slots:
    virtual void typeChanged(int n);
  };

#endif // CSCANWIDGETTASK_H

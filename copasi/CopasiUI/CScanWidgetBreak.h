/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/CopasiUI/Attic/CScanWidgetBreak.h,v $
   $Revision: 1.1 $
   $Name:  $
   $Author: ssahle $ 
   $Date: 2005/02/27 20:27:39 $
   End CVS Header */

/****************************************************************************
 ** Form interface generated from reading ui file 'CScanWidgetBreak.ui'
 **
 ** Created: Sa Feb 26 19:16:27 2005
 **      by: The User Interface Compiler ($Id: CScanWidgetBreak.h,v 1.1 2005/02/27 20:27:39 ssahle Exp $)
 **
 ** WARNING! All changes made in this file will be lost!
 ****************************************************************************/

#ifndef CSCANWIDGETBREAK_H
#define CSCANWIDGETBREAK_H

#include <qvariant.h>
#include <qwidget.h>
#include "utilities/CCopasiParameterGroup.h"
#include "scan/CScanProblem.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QFrame;
class QComboBox;
class QLabel;

class CScanWidgetBreak : public QWidget
  {
    Q_OBJECT

  public:
    CScanWidgetBreak(QWidget* parent = 0, const char* name = 0, WFlags fl = 0);
    ~CScanWidgetBreak();

    QFrame* frame;
    QComboBox* comboBoxReport;
    QLabel* textLabelReport;
    QComboBox* comboBoxPlot;
    QLabel* labelTitle;
    QLabel* textLabelPlot;

    virtual bool initFromScanItem(CCopasiParameterGroup * pg);
    virtual bool saveToScanItem(CScanProblem * pg) const;

  protected:
    QVBoxLayout* CScanWidgetBreakLayout;
    QGridLayout* frameLayout;
    QSpacerItem* spacer1;

  protected slots:
    virtual void languageChange();

  private:
    void init();
  };

#endif // CSCANWIDGETBREAK_H

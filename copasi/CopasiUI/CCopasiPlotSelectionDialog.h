/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/CopasiUI/Attic/CCopasiPlotSelectionDialog.h,v $
   $Revision: 1.3.2.1 $
   $Name:  $
   $Author: shoops $
   $Date: 2006/05/19 12:55:45 $
   End CVS Header */

// Copyright � 2005 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#ifndef CCopasiPlotSelectionDialog_H__
#define CCopasiPlotSelectionDialog_H__

#include "qdialog.h"
#include <vector>

class QWidget;
class QPushButton;
class QSplitter;
class QCheckBox;
class QVBox;
class QHBoxLayout;
class QVBoxLayout;
class QLabel;
class CCopasiObject;
class CModel;
class CCopasiSelectionWidget;

class CCopasiPlotSelectionDialog: public QDialog
  {
    Q_OBJECT

  public:
    CCopasiPlotSelectionDialog(QWidget * parent = 0, const char * name = 0, bool modal = FALSE, WFlags f = 0);
    virtual ~CCopasiPlotSelectionDialog();
    void setOutputVectors(std::vector<CCopasiObject*>* outputVector1, std::vector<CCopasiObject*>* outputVector2);
    void setModel(CModel* model);

  protected slots:
    void slotOKButtonClicked();
    void slotCancelButtonClicked();
    void slotExpertCheckBoxToggled(bool checked);

  protected:
    void setTabOrder();

  protected:
    QPushButton* mpOKButton;
    QPushButton* mpCancelButton;
    QCheckBox* mpExpertCheckBox;
    CCopasiSelectionWidget* mpXAxisSelectionWidget;
    CCopasiSelectionWidget* mpYAxisSelectionWidget;
    QSplitter* mpSplitter;
    QHBoxLayout* mpButtonBox;
    QVBoxLayout* mpMainLayout;
    QLabel* mpXAxisLabel;
    QLabel* mpYAxisLabel;
    QVBox* mpXAxisSelectionBox;
    QVBox* mpYAxisSelectionBox;
    std::vector<CCopasiObject*>* mpXAxisOutputVector;
    std::vector<CCopasiObject*>* mpYAxisOutputVector;
  };

#endif // CPlotSelectionDialog_H__

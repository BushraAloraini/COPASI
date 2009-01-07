// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/plotUI/Attic/plotwidget1.h,v $
//   $Revision: 1.19 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2009/01/07 19:03:24 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/****************************************************************************
 ** Form interface generated from reading ui file 'plotwidget1.ui'
 **
 ** Created: Mon Sep 29 10:43:24 2003
 **      by: The User Interface Compiler ($Id: plotwidget1.h,v 1.19 2009/01/07 19:03:24 shoops Exp $)
 **
 ** WARNING! All changes made in this file will be lost!
 ****************************************************************************/

#ifndef PLOTWIDGET1_H
#define PLOTWIDGET1_H

#include <vector>
#include <string>
#include <fstream>
#include <qvariant.h>
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QLabel>
#include <Q3Frame>

#include "UI/copasiWidget.h"
#include "plot/CPlotSpecification.h"

class Q3VBoxLayout;
class Q3HBoxLayout;
class Q3GridLayout;
class QSpacerItem;
class QComboBox;
class QLineEdit;
class QCheckBox;
class QLabel;
class Q3Frame;
class QPushButton;
class QTabWidget;
class QWidget;
class QToolButton;

class PlotWindow;
class Curve2DWidget;

class PlotWidget1 : public CopasiWidget
  {
    Q_OBJECT

  public:
    PlotWidget1(QWidget* parent = 0, const char* name = 0, Qt::WFlags fl = 0);
    ~PlotWidget1();

    virtual bool update(ListViews::ObjectType objectType,
                        ListViews::Action action, const std::string & key);
    virtual bool leave();
    virtual bool enter(const std::string & key = "");

    /**
     * creates tabs for histograms from a list of objects. objects[0] is skipped,
     * so objects.size()-1 histogram descriptions are generated.
     * incr is used for all of them.
     * This method is called from HistoWidget, so that when the user chooses several
     * objects from the object selection widget several histograms can be generated
     * accordingly.
     */
    void createHistograms(std::vector<const CCopasiObject* >objects, const C_FLOAT64 & incr);

  protected:
    bool loadFromPlotSpec(const CPlotSpecification *);
    bool saveToPlotSpec();

    /**
     * creates curve widget and adds it to the tab bar
     */
    void addCurveTab(const std::string & title,
                     const CPlotDataChannelSpec & x,
                     const CPlotDataChannelSpec & y);

    /**
     * creates histogram widget and adds it to the tab bar
     */
    void addHisto1DTab(const std::string & title,
                       const CPlotDataChannelSpec & x,
                       const C_FLOAT64 & incr);

    std::string objKey;

    CPlotItem::Type mType;

    Q3GridLayout* PlotWidget1Layout;
    Q3HBoxLayout* layoutTitle;
    Q3HBoxLayout* layoutButtons;
    Q3HBoxLayout* layoutCurves;
    QSpacerItem* spacerCurves;

    QComboBox* comboType;
    QLineEdit* titleLineEdit;
    QCheckBox* activeCheckBox;
    QLabel* labelTitle;
    QLabel* labelType;
    QLabel* labelScale;
    QCheckBox* checkLogX;
    QCheckBox* checkLogY;
    Q3Frame* lineButtons;
    QTabWidget* tabs;
    QLabel* labelCurves;
    QToolButton* addCurveButton;
    QToolButton* addHistoButton;
    QToolButton* deleteCurveButton;
    Q3Frame* line1;

    QPushButton* startPlotButton;
    QPushButton* deletePlotButton;
    QPushButton* addPlotButton;
    QPushButton* resetButton;

  protected slots:

    /**
     * slots that are connected to the buttons for adding curves/histograms
     */
    void addCurveSlot();
    void addHistoSlot();

    /**
     * deletes the current tab
     */
    void removeCurve();

    /**
     *
     */
    void commitPlot();

    /**
     * deletes the current plot spec.
     */
    void deletePlot();

    /**
     * create a new plot definition
     */
    void addPlot();

    /**
     * cancels changes to the plot definition
     */
    void resetPlot();

    void typeChanged();

  protected:
    /**
     * this specifically handles the creation of a 2d curve. It is called when
     * the corresponding button is pressed and the plot is actually 2D.
     * The dialogs for choosing objects for the axes is called from here and one
     * or several curve descriptions are generated.
     */
    void addCurve2D();

    /**
     * this specifically handles the creation of a histogram. It is called when
     * the corresponding button is pressed and the plot is actually 2D.
     * An empty histogram description is generated.
     */
    void addHisto1D();
  };

#endif // PLOTWIDGET1_H

// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/listviews.h,v $
//   $Revision: 1.143 $
//   $Name:  $
//   $Author: tjohann $
//   $Date: 2008/07/02 07:59:05 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/****************************************************************************
 **  $ CopasiUI/listviews.h                Modified on : 8th March, 2002
 **  $ Author  : Ankur Gupta
 *****************************************************************************/
#ifndef LISTVIEWS_H
#define LISTVIEWS_H

#include <set>
#include <qsplitter.h>
#include <qlistview.h>

#include "copasi.h"
#include "UI/Tree.h"

class CTabWidget;

class Refresh;
class CCopasiObject;

class DataModelGUI;
class CQCompartment;
class CompartmentsWidget;
class CQDifferentialEquations;
#ifdef COPASI_DEBUG
class CQEventsWidget;
class CQEventWidget1;
#endif // COPASI_DEBUG
class FunctionWidget1;
class FunctionWidget;
class CMathModel;
class CQMetabolite;
class MetabolitesWidget;
class CQEFMWidget;
//class ModelWidget;
class ModelValuesWidget;
class CQMoietiesTaskResult;
class CQMoietiesTaskWidget;
class ParametersWidget;
class ReactionsWidget1;
class ReactionsWidget;
class SteadyStateWidget;
class ScanWidget;
class SensitivitiesWidget;
class CQSensResultWidget;
class StateWidget;
class CQTrajectoryWidget;
class TableDefinition;
#ifdef COPASI_TSS
class TSSWidget;
#endif // COPASI_TSS
#ifdef COPASI_TSSA
class CQTSSAWidget;
class CQTSSAResultWidget;
class CQTSSAResultSubWidget;
#endif
#ifdef COPASI_DEBUG
class CQUpdatesWidget;
#endif
class CQOptimizationWidget;
class OptimizationResultWidget;
class CopasiWidget;
class PlotWidget1;
class PlotWidget;
class CQSplashWidget;
class TimeSeriesWidget;
class CQFittingWidget;
class CQFittingResult;
class CQMCAWidget;
class CMCAResultWidget;
class CQReportDefinition;
class CQLyapWidget;
class CQLyapResultWidget;
class CQModelValue;
class CQMathMatrixWidget;

#ifdef WITH_LAYOUT
class CQCopasiLayoutWidget;
#endif

//*********************************************************************************

class FolderListItem : public QListViewItem
  {
  public:
    FolderListItem(QListView *parent, const IndexedNode *f, bool recurs = true);
    FolderListItem(FolderListItem *parent, const IndexedNode *f, bool recurs = true);
    void createSubFolders();
    void deleteSubFolders();

    bool setFolder(const IndexedNode * folder);
    const IndexedNode * getFolder() const;
    QString key(int, bool) const;

  protected:
    const IndexedNode *mpFolder;
    QString mSortKey;
  };

//********************************************************************************

class ListViews : public QSplitter
  {
    Q_OBJECT

    friend class CopasiUI3Window;

  public:
    ListViews(QWidget *parent = 0, const char *name = 0);
    ~ListViews();

    // CHANGE does not include RENAME
    enum Action {CHANGE = 0, ADD, DELETE, RENAME};
    enum ObjectType {METABOLITE = 0, COMPARTMENT, REACTION, FUNCTION, MODEL, STATE, REPORT, PLOT, MODELVALUE, EVENT, MIRIAM};

    static void setDataModel(DataModelGUI* dm);
    static DataModelGUI* getDataModel() {return dataModel;};
    static bool notify(ObjectType objectType, Action action, const std::string & key = "");
    static bool commit();
    void switchToOtherWidget(C_INT32 id, const std::string & key);
    static void switchAllListViewsToWidget(C_INT32 id, const std::string & key);

    void storeCurrentItem();
    void restoreCurrentItem();
    static void storeCurrentItemInAllListViews();
    static void restoreCurrentItemInAllListViews();
    CopasiWidget* findWidgetFromId(const C_INT32 & id) const;

  private:
    CMathModel *mpMathModel;

    CopasiWidget* findWidgetFromItem(FolderListItem* item) const;

    void ConstructNodeWidgets();
    void setupFolders();

    void setTheRightPixmap(QListViewItem* lvi);

    FolderListItem* findListViewItem(C_INT32 id, std::string key); //should always return a valid item

  private slots:
    void slotFolderChanged(QListViewItem*);

  private:
    static DataModelGUI* dataModel;
    static std::vector< Refresh * > mUpdateVector;
    static std::set< const CCopasiObject * > mChangedObjects;
    static int mFramework;

    QListViewItem* lastSelection;
    CopasiWidget* currentWidget;
    std::string lastKey;

    std::string mSaveObjectKey;
    C_INT32 mSaveFolderID;

    static std::set<ListViews *> mListOfListViews;
    bool attach();
    bool detach();

    bool updateCurrentWidget(ObjectType objectType, Action action, const std::string & key = "");
    static bool updateDataModelAndListviews(ObjectType objectType, Action action, const std::string & key);
    static bool updateAllListviews(C_INT32 id);

    void notifyChildWidgets(ObjectType objectType,
                            Action action,
                            const std::string & key);

    static void notifyAllChildWidgets(ObjectType objectType,
                                      Action action,
                                      const std::string & key);

    static void refreshInitialValues();
    static void buildChangedObjects();

    void setChildWidgetsFramework(int framework);

    static void setFramework(int framework);

    //the widgets
    QListView *folders;

    CMCAResultWidget* mpCMCAResultWidget;
    CQMCAWidget* mpCQMCAWidget;
    CompartmentsWidget *compartmentsWidget;
    CTabWidget *compartmentsWidget1;
    CQSplashWidget *defaultWidget;
    CQDifferentialEquations *differentialEquations;
#ifdef COPASI_DEBUG
    CQEventsWidget *eventsWidget;
    CQEventWidget1 *eventWidget1;
#endif // COPASI_DEBUG
    FunctionWidget *functionWidget;
    CTabWidget *functionWidget1;
    CQLyapWidget *lyapWidget;
    CQLyapResultWidget *lyapResultWidget;
    MetabolitesWidget *metabolitesWidget;
    CTabWidget *metabolitesWidget1;
    CTabWidget *modelWidget;
    ModelValuesWidget *modelValuesWidget;
    CTabWidget *mpModelValueWidget;
    CQEFMWidget *modesWidget;
    CQMoietiesTaskResult *mpMoietiesTaskResult;
    CQMoietiesTaskWidget *mpMoietiesTaskWidget;
    CQOptimizationWidget *optimizationWidget;
    OptimizationResultWidget *optResultWidgetS;
    OptimizationResultWidget *optResultWidgetT;
    CQFittingWidget *paramFittingWidget;
    CQFittingResult *mpFittingResultWidget;
    ParametersWidget *parametersWidget;
    PlotWidget *plotWidget;
    PlotWidget1 *plotWidget1;
    ReactionsWidget *reactionsWidget;
    CTabWidget *reactionsWidget1;
    ScanWidget *scanWidget;
    SensitivitiesWidget *sensWidget;
    CQSensResultWidget *sensResultWidget;
    StateWidget *stateWidget;
    SteadyStateWidget *steadystateWidget;
    TableDefinition *tableDefinition;
    CQReportDefinition *tableDefinition1;
#ifdef COPASI_TSS
    TSSWidget *tssWidget;
#endif // COPASI_TSS
    TimeSeriesWidget *timeSeriesWidget;
    CQTrajectoryWidget *trajectoryWidget;
#ifdef COPASI_TSSA
    CQTSSAWidget *tssaWidget;
    CQTSSAResultWidget *tssaResultWidget;
#endif // COPASI_DEBUG
#ifdef COPASI_DEBUG
    CQUpdatesWidget *mpUpdatesWidget;
#endif // COPASI_DEBUG
#ifdef WITH_LAYOUT
    CQCopasiLayoutWidget * mpCopasiLayoutWidget;
#endif
    CQMathMatrixWidget * mpMathMatrixWidget;
  };

#endif

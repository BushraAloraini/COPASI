/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/listviews.h,v $
   $Revision: 1.105 $
   $Name:  $
   $Author: ssahle $
   $Date: 2006/05/04 10:59:57 $
   End CVS Header */

// Copyright � 2005 by Pedro Mendes, Virginia Tech Intellectual
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
#include "Tree.h"

class DataModelGUI;

//class CSteadyStateTask;
//class CTrajectoryTask;

//class CompartmentSymbols;
//class ConstantSymbols;
//class FixedMetaboliteSymbols;
//class MetaboliteSymbols;
class CompartmentsWidget1;
class CompartmentsWidget;
class DifferentialEquations;
class FunctionSymbols;
class FunctionWidget1;
class FunctionWidget;
class CMathModel;
class MetabolitesWidget1;
class MetabolitesWidget;
class ModesWidget;
class ModelWidget;
class ModelValuesWidget;
class MoietyWidget1;
class MoietyWidget;
class ParametersWidget;
class ReactionsWidget1;
class ReactionsWidget;
class SteadyStateWidget;
class ScanWidget;
class SensitivitiesWidget;
class StateWidget;
class TrajectoryWidget;
class TableDefinition;
class TableDefinition1;
class TSSWidget;
class CQOptimizationWidget;
class OptimizationResultWidget;
class CopasiWidget;
class PlotWidget1;
class PlotWidget;
class CopasiDefaultWidget;
class TimeSeriesWidget;
class CQFittingWidget;
class CQFittingResult;
class CMCAWidget;
class CMCAResultWidget;
class CQReportDefinition;
class CQLyapWidget;

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
    enum ObjectType {METABOLITE = 0, COMPARTMENT, REACTION, FUNCTION, MODEL, STATE, REPORT, PLOT, MODELVALUE};

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

  private:
    CMathModel *mpMathModel;

    CopasiWidget* findWidgetFromItem(FolderListItem* item) const;

    void ConstructNodeWidgets();
    void setupFolders();

    void setTheRightPixmap(QListViewItem* lvi);

    FolderListItem* findListViewItem(int id, std::string key); //should always return a valid item

  private slots:
    void slotFolderChanged(QListViewItem*);

  private:
    static DataModelGUI* dataModel;
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

    //the widgets
    QListView *folders;

    //    CompartmentSymbols *compartmentSymbols;
    //    MetaboliteSymbols *metaboliteSymbols;
    //    ConstantSymbols *constantSymbols;
    //    FixedMetaboliteSymbols *fixedMetaboliteSymbols;
    CMCAResultWidget* mpCMCAResultWidget;
    CMCAWidget* mpCMCAWidget;
    CompartmentsWidget *compartmentsWidget;
    CompartmentsWidget1 *compartmentsWidget1;
    CopasiDefaultWidget *defaultWidget;
    DifferentialEquations *differentialEquations;
    FunctionSymbols *functionSymbols;
    FunctionWidget *functionWidget;
    FunctionWidget1 *functionWidget1;
    CQLyapWidget *lyapWidget;
    MetabolitesWidget *metabolitesWidget;
    MetabolitesWidget1 *metabolitesWidget1;
    ModelWidget *modelWidget;
    ModelValuesWidget *modelValuesWidget;
    ModesWidget *modesWidget;
    MoietyWidget *moietyWidget;
    MoietyWidget1 *moietyWidget1;
    CQOptimizationWidget *optimizationWidget;
    OptimizationResultWidget *optResultWidgetS;
    OptimizationResultWidget *optResultWidgetT;
    CQFittingWidget *paramFittingWidget;
    CQFittingResult *mpFittingResultWidget;
    ParametersWidget *parametersWidget;
    PlotWidget *plotWidget;
    PlotWidget1 *plotWidget1;
    ReactionsWidget *reactionsWidget;
    ReactionsWidget1 *reactionsWidget1;
    ScanWidget *scanWidget;
    SensitivitiesWidget *sensWidget;
    StateWidget *stateWidget;
    SteadyStateWidget *steadystateWidget;
    TableDefinition *tableDefinition;
    CQReportDefinition *tableDefinition1;
    TSSWidget *tssWidget;
    TimeSeriesWidget *timeSeriesWidget;
    TrajectoryWidget *trajectoryWidget;
  };

#endif

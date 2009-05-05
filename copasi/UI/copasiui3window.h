// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/copasiui3window.h,v $
//   $Revision: 1.78 $
//   $Name:  $
//   $Author: ssahle $
//   $Date: 2009/05/05 23:57:16 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#include <string>

#include <qmainwindow.h>
#include <qdialog.h>
#include <qtoolbar.h>
#include "qvariant.h"
//Added by qt3to4:
#include <QLabel>
#include <Q3PopupMenu>
#include <QCloseEvent>

#ifdef COPASI_SBW_INTEGRATION

#include <qapplication.h>
#include <qevent.h>

#define WIN32_LEAN_AND_MEAN
#include <SBW/SBW.h>
#undef DELETE
#undef ERROR
#undef TRUE
#undef FALSE
using namespace SystemsBiologyWorkbench;
#else
class SBWListener;
#endif  // COPASI_SBW_INTEGRATION

#include "../MIRIAM/CMIRIAMResource.h"

class QToolButton;
class Q3ListViewItem;
class ListViews;
class DataModelGUI;
class SliderDialog;
class CQTrajectoryWidget;
class SteadyStateWidget;
class ScanWidget;
class CQMCAWidget;
class QPushButton;
class QLabel;
class Q3HBox;
class Q3ScrollView;
class QAction;
class ObjectBrowserDialog;
class QComboBox;
class CMIRIAMResources;
class CMIRIAMResourceObject;

class CopasiUI3Window : public QMainWindow
#ifdef COPASI_SBW_INTEGRATION
    // A SBW listener can catch messages from SBW ... used here to allow COPASI to be shut down
    , public SBWListener
#endif // COPASI_SBW_INTEGRATION

{
  Q_OBJECT

public:
  static CopasiUI3Window * create();
  ~CopasiUI3Window();
  //void enable_object_browser_menu();
  //void disable_object_browser_menu();
  DataModelGUI* getDataModel();
  //void saveFile();

  ListViews* getMainWidget();

  void checkPendingMessages();
  void suspendAutoSave(const bool & suspend);
  void importSBMLFromString(const std::string & sbmlDocumentText);
  std::string exportSBMLToString();

protected:
  CopasiUI3Window();
  DataModelGUI* dataModel; // to keep track of the data model..
  ListViews *listViews;
  QComboBox * mpBoxSelectFramework;

public slots:
  void slotShowSliders(bool flag);

  /**
   * This should only be called by the destructor of the object browser dialog
   * otherwise a memory leak occurs
   */
  void slotObjectBrowserDialogWasClosed();

  void slotPreferences();
  void autoSave();

protected slots:
  void slotFileOpen(QString file = QString::null);
  void slotFileExamplesCopasiFiles(QString file = QString::null);
  void slotFileExamplesSBMLFiles(QString file = QString::null);
  bool slotFileSave();
  bool slotFileSaveAs(QString str = QString::null);
  void newDoc();
  void slotFilePrint();
  void slotImportSBML(QString file = QString::null);
  void slotExportSBML();
  void slotExportMathModel();
  void slotTutorialWizard();
  void about();
  void license();
  void aboutQt();
  void slotQuit();
  void slotConvertToIrreversible();
  void closeEvent(QCloseEvent* e);
  void listViewsFolderChanged(Q3ListViewItem* item);
  void slotOpenRecentFile(int index);
  void slotOpenRecentSBMLFile(int index);
  bool slotRegistration();
  void slotCheckModel();
  void slotApplyInitialState();
  void slotUpdateInitialState();
  void slotFrameworkChanged(int index);
  void slotCapture();
  // handle the custom events
  void customEvent(QCustomEvent *);
  // start an analyzer when selected from the SBW menu
  void startSBWAnalyzer(int nId);
  void slotUpdateMIRIAM();

  /**
   * This should only be called via signal by the corresponding QAction mpaObjectBrowser.
   * Otherwise the QAction will not be up to date.
   */
  void slotShowObjectBrowserDialog(bool flag);

private:
  int newFlag;
  QString FixedTitle;
  //QToolButton * msave_button;
  //QPopupMenu * tools;

  void createActions();
  void createToolBar();
  void createMenuBar();

  QAction* mpaNew;
  QAction* mpaOpen;
  QAction* mpaOpenCopasiFiles;
  QAction* mpaOpenSBMLFiles;
  QAction* mpaSave;
  QAction* mpaSaveAs;
  QAction* mpaImportSBML;
  QAction* mpaExportSBML;
  QAction* mpaExportODE;
  QAction* mpaObjectBrowser;
  QAction* mpaSliders;
  QAction* mpaCheckModel;
  QAction* mpaApplyInitialState;
  QAction* mpaUpdateInitialState;
  QAction *mpaCapture;
  QAction* mpaUpdateMIRIAM;

  void CleanUp();

  SliderDialog* mpSliders;
  ObjectBrowserDialog * mpObjectBrowser;

  bool mSaveAsRequired;
  void updateTitle();
  QTimer *mpAutoSaveTimer;
  bool mSuspendAutoSave;

  Q3PopupMenu * mpMenuRecentFiles;
  Q3PopupMenu * mpMenuExamples;

  void refreshRecentFileMenu();

  Q3PopupMenu * mpMenuRecentSBMLFiles;
  void refreshRecentSBMLFileMenu();

  Q3PopupMenu * mpTools;

  CMIRIAMResources * mpMIRIAMResources;

#ifdef COPASI_SBW_INTEGRATION
public:
  /**
   * This event is triggered by SBW asking COPASI to import an SBML document provided as a string
   */
  class QSBWSBMLEvent : public QCustomEvent
  {
  public:
    /**
     * Constructor
     * @param const std::string & SBMLModel
     */
    QSBWSBMLEvent(const std::string & SBMLModel);

    /**
     * Retrieve the SBML model
     * @return const std::string & SBMLModel
     */
    const std::string & getSBMLModel() const;

  private:
    /**
     * A string holding the SBML model
     */
    std::string mSBML;
  };

  /**
   * This event is triggered by SBW asking COPASI shut down.
   */
  class QSBWShutdownEvent: public QCustomEvent
  {
  public:
    QSBWShutdownEvent();
  };

  // We expose 2 methods to SBW, one to load an SBML file
  SystemsBiologyWorkbench::DataBlockWriter doAnalysis(SystemsBiologyWorkbench::Module from,
      SystemsBiologyWorkbench::DataBlockReader reader);

  // and another to return the SBML file COAPSI is currently working with
  SystemsBiologyWorkbench::DataBlockWriter getSBML(SystemsBiologyWorkbench::Module from,
      SystemsBiologyWorkbench::DataBlockReader reader);

  // those methods are registered here
  static void registerMethods(SystemsBiologyWorkbench::MethodTable<CopasiUI3Window> & table);

  // as part of the SBWListener we tell SBW here, that we want to react on the shutdown event
  virtual void onShutdown();

private:
  /**
   * Connect to SBW
   */
  void connectSBW();

  /**
   * Register COPASI as a module ins SBW
   */
  void registerSBW();

  /**
   * Refresh the SBW menu.
   */
  void refreshSBWMenu();

  /**
   * Retrieve the list of all services from the SBW broker
   * @param const std::string & category
   * @param const bool & recursive
   * @return std::vector< SystemsBiologyWorkbench::DataBlockReader > services
   */
  std::vector< SystemsBiologyWorkbench::DataBlockReader > findServices(const std::string & category,
      const bool & recursive);

  /**
   * The SBW module which handles the interaction with the SBW broker
   */
  SystemsBiologyWorkbench::ModuleImpl * mpSBWModule;

  /**
   * A list of SBW analyzer modules
   */
  QStringList mAnalyzerModules;

  /**
   * A list of the corresponding SBW services
   */
  QStringList mAnalyzerServices;

  /**
   * The SBW menu
   */
  Q3PopupMenu * mpMenuSBW;

  /**
   * The id of the SBW menu
   */
  int mIdMenuSBW;

  /**
   * This variable indicates whether COPASI is to ignore SBW shutdown events
   */
  bool mIgnoreSBWShutdownEvent;

#endif // COPASI_SBW_INTEGRATION
#ifdef COPASI_LICENSE_COM
  bool checkRegistration();
#endif // COPASI_LICENSE_COM
};

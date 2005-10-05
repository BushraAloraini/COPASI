######################################################################
# $Revision: 1.104 $ $Author: ssahle $ $Date: 2005/10/05 13:53:12 $  
######################################################################

TEMPLATE = app

include(../common.pri)

DEPENDPATH += .. 
INCLUDEPATH += ..

COPASI_LIBS = \
         commandline \
         copasiDM \
         copasiXML \
         elementaryFluxModes \
         function \
         mathmodel \
         mml \
         model \
         optimization \
         plot \
         randomGenerator \
         report \
         sbmlimport \
         scan \
         steadystate \
         trajectory \
         tss \
         utilities \
         wizard

contains(BUILD_OS, WIN32) {
  RC_FILE = CopasiUI.rc

  LIBS += $$join(COPASI_LIBS, ".lib  ../lib/", ../lib/, .lib)

  TARGETDEPS += $$join(COPASI_LIBS, ".lib  ../lib/", ../lib/, .lib)

  release {
    distribution.extra = bash ../../admin/mkbuild.sh $${BUILD_OS}
  }
} 

contains(BUILD_OS, Linux) {
  LIBS = -L../lib \
         -Wl,--start-group \
         $$join(COPASI_LIBS, " -l", -l) \
         -Wl,--end-group \
         $${LIBS}

  TARGETDEPS += $$join(COPASI_LIBS, ".a  ../lib/lib", ../lib/lib, .a)

  LIBS += -Wl,-lqt-mt \
          -Wl,-lXcursor \
          -Wl,-lXft \
          -Wl,-lfontconfig \
          -Wl,-lpthread

  release {
    dynamic_LFLAGS = $${QMAKE_LFLAGS}
    dynamic_LFLAGS -= -static

    dynamic_LIBS = -Wl,-Bstatic $${LIBS} -Wl,-Bdynamic 
    dynamic_LIBS -= -Wl,-lqt-mt
    dynamic_LIBS -= -Wl,-lXcursor
    dynamic_LIBS -= -Wl,-lXft
    dynamic_LIBS -= -Wl,-lfontconfig
    dynamic_LIBS -= -Wl,-lpthread
 
    dynamic.target   = CopasiUI-dynamic
    dynamic.depends  = $(OBJECTS) $(OBJMOC) $(OBJCOMP) $${TARGETDEPS}
    dynamic.commands = \
      $(LINK) $${dynamic_LFLAGS} -L$(QTDIR)/lib -L/usr/X11R6/lib \
              -o $@ $(OBJECTS) $(OBJMOC) $(OBJCOMP) $${dynamic_LIBS} \
              -Wl,--start-group -Wl,-Bstatic \
              -lqt-mt -lXrender -lXrandr -lXcursor -lXinerama -lXft \ 
              -lfreetype -lfontconfig -lSM -lICE -lXext -lX11 -lm \
              -Wl,--end-group -Wl,-Bdynamic \
              -ldl -lpthread && \
              strip $@

    QMAKE_EXTRA_UNIX_TARGETS += dynamic

    distribution.extra = make $${dynamic.target}; \
                         ../../admin/mkbuild.sh $${BUILD_OS}

  }
}

contains(BUILD_OS, SunOS) {
  QMAKE_LFLAGS += -z rescan

  LIBS = -L../lib \
         $$join(COPASI_LIBS, " -l", -l) \
         $${LIBS}

  TARGETDEPS += $$join(COPASI_LIBS, ".a  ../lib/lib", ../lib/lib, .a)

  LIBS += -lICE -ldl

  release {
    distribution.extra = ../../admin/mkbuild.sh $${BUILD_OS}
  }
}  

contains(BUILD_OS, Darwin){
  QMAKE_LFLAGS += -Wl,-search_paths_first
  
  COPASI_LIBS += randomGenerator
  COPASI_LIBS += function
  
  LIBS = $$join(COPASI_LIBS, ".a  ../lib/lib", ../lib/lib, .a) \
         $$LIBS
  
  TARGETDEPS += $$join(COPASI_LIBS, ".a  ../lib/lib", ../lib/lib, .a)

  release {
    distribution.extra = ../../admin/mkbuild.sh $${BUILD_OS}
  }
}

# Input
HEADERS += \
           AboutDialog.h \
           CCopasiPlotSelectionDialog.h \
           CCopasiSelectionDialog.h \
           CCopasiSelectionWidget.h \
           CCopasiSimpleSelectionTree.h \
           ChemEqValidator.h \
           CMCAWidget.h \
           CMCAResultWidget.h \
           CReportDefinitionSelect.h \
           CompartmentSymbols.h \
           CompartmentsWidget.h \
           CompartmentsWidget1.h \
           ConstantSymbols.h \
           CopasiDefaultWidget.h \
           CopasiFileDialog.h \
           CopasiSlider.h \
           CopasiTableWidget.h \
           copasiWidget.h \
           copasiui3window.h \
           CProgressBar.h \
           CQReportListItem.h \
           CScanContainerWidget.h \
           CTimeSeriesTable.h \
           DataModelGUI.h \
           DifferentialEquations.h \
           FixedMetaboliteSymbols.h \
           FunctionItemWidget.h \
           FunctionSymbols.h \
           FunctionWidget.h \
           FunctionWidget1.h \
           MetaboliteSymbols.h \
           MetabolitesWidget.h \
           MetabolitesWidget1.h \
           ModelWidget.h \
           ModelValuesWidget.h \
           ModesWidget.h \
           MoietyWidget.h \
           MoietyWidget1.h \
           MyLineEdit.h \
           listviews.h \
           ObjectBrowserDialog.h \
           ObjectBrowserItem.h \
           ObjectBrowserWidget.h \
           OptimizationItemWidget.h \
           OptimizationWidget.h \
           ParametersWidget.h \
           ParamFittingItemWidget.h \
           ParamFittingWidget.h \
           parametertable.h \
           PlotWidget.h \
           qtUtilities.h \
           ReactionsWidget.h \
           ReactionsWidget1.h \
           ScanItemWidget.h \
           ScanScrollView.h \
           ScanWidget.h \
           SliderDialog.h \
           StateWidget.h \
           SteadyStateWidget.h \
           TableDefinition.h \
           TableDefinition1.h \
           TaskWidget.h \
           TimeSeriesWidget.h \
           TrajectoryWidget.h \
           Tree.h \
           TSSWidget.h

# INTERFACES += CompartmentsWidget.ui \
#               FunctionWidget1.ui \
#               MetabolitesWidget1.ui \
#               MoietyWidget1.ui \
#               ObjectBrowser.ui \
#               objectdebug.ui \
#               ReactionsWidget1.ui \
#               SteadyStateWidget.ui \
#               TrajectoryWidget.ui

SOURCES += \
           AboutDialog.cpp \
           CCopasiPlotSelectionDialog.cpp \
           CCopasiSelectionDialog.cpp \
           CCopasiSelectionWidget.cpp \
           CCopasiSimpleSelectionTree.cpp \
           CMCAWidget.cpp \
           CMCAResultWidget.cpp \
           CReportDefinitionSelect.cpp \
           CompartmentSymbols.cpp \
           CompartmentsWidget.cpp \
           CompartmentsWidget1.cpp \
           ConstantSymbols.cpp \
           CopasiDefaultWidget.cpp \
           CopasiFileDialog.cpp \
           CopasiSlider.cpp \
           CopasiTableWidget.cpp \
           copasiWidget.cpp \
           copasiui3window.cpp \
           CProgressBar.cpp \
           CQReportListItem.cpp \
           CScanContainerWidget.cpp \
           CTimeSeriesTable.cpp \
           DataModelGUI.cpp \
           DifferentialEquations.cpp \
           FixedMetaboliteSymbols.cpp \
           FunctionItemWidget.cpp \
           FunctionSymbols.cpp \
           FunctionWidget.cpp \
           FunctionWidget1.cpp \
           listviews.cpp \
           main.cpp \
           MetaboliteSymbols.cpp \
           MetabolitesWidget.cpp \
           MetabolitesWidget1.cpp \
           ModelValuesWidget.cpp \
           ModelWidget.cpp \
           ModesWidget.cpp \
           MoietyWidget.cpp \
           MoietyWidget1.cpp \
           MyLineEdit.cpp \
           ObjectBrowserDialog.cpp \
           ObjectBrowserItem.cpp \
           ObjectBrowserWidget.cpp \
           OptimizationItemWidget.cpp \
           OptimizationWidget.cpp \
           ParametersWidget.cpp \
           ParamFittingItemWidget.cpp \
           ParamFittingWidget.cpp \
           parametertable.cpp \
           PlotWidget.cpp \
           qtUtilities.cpp \
           ReactionsWidget.cpp \
           ReactionsWidget1.cpp \
           ScanItemWidget.cpp \
           ScanScrollView.cpp \
           ScanWidget.cpp \
           SliderDialog.cpp \
           StateWidget.cpp \
           SteadyStateWidget.cpp \
           TableDefinition.cpp \
           TableDefinition1.cpp \
           TaskWidget.cpp \
           TimeSeriesWidget.cpp \
           TrajectoryWidget.cpp \
           Tree.cpp \
           TSSWidget.cpp

# FORMS += TimeSeriesSubwidget.ui
# FORMS += StateSubwidget.ui
# FORMS += CMCAResultSubwidget.ui
# FORMS += SimpleSelectionWidget.ui        
# FORMS += SliderSettingsDialog.ui    
# FORMS += objectdebug.ui       
# FORMS += CUpDownSubwidget.ui
# FORMS += CScanWidgetBreak.ui
# FORMS += CScanWidgetRandom.ui
# FORMS += CScanWidgetRepeat.ui
# FORMS += CScanWidgetScan.ui
# FORMS += CScanWidgetTask.ui
# FORMS += DefaultplotDialog.ui
# FORMS += CQProgressItem.ui
# FORMS += CQProgressItemBar.ui
# FORMS += CQProgressItemText.ui
# FORMS += CQProgressDialog.ui
# FORMS += CQReportDefinition.ui
# FORMS += CQTextDialog.ui
# FORMS += CQFileDialogBtnGrp.ui

# headers generated from .ui files   
HEADERS += \
           CMCAResultSubwidget.h \
           CQFileDialogBtnGrp.h \
           CQProgressItem.h \
           CQProgressItem.ui.h \
           CQProgressItemBar.ui.h \
           CQProgressItemBar.h \
           CQProgressItemText.h \
           CQProgressItemText.ui.h \
           CQProgressDialog.h \
           CQProgressDialog.ui.h \
           CQReportDefinition.h \
           CQReportDefinition.ui.h \
           CQTextDialog.h \
           CQTextDialog.ui.h \
           CUpDownSubwidget.h \
           CScanWidgetBreak.h \
           CScanWidgetRandom.h \
           CScanWidgetRepeat.h \
           CScanWidgetScan.h \
           CScanWidgetTask.h  \ 
           DefaultplotDialog.h \
           objectdebug.h \
           SliderSettingsDialog.h \
           StateSubwidget.h \
           TimeSeriesSubwidget.h 

# sources generated from .ui files   
SOURCES += \
           CMCAResultSubwidget.cpp \
           CQFileDialogBtnGrp.cpp \
           CQProgressItem.cpp \
           CQProgressItemBar.cpp \
           CQProgressItemText.cpp \
           CQProgressDialog.cpp \
           CQReportDefinition.cpp \
           CQTextDialog.cpp \
           CUpDownSubwidget.cpp \
           CScanWidgetBreak.cpp \
           CScanWidgetRandom.cpp \
           CScanWidgetRepeat.cpp \
           CScanWidgetScan.cpp \
           CScanWidgetTask.cpp \  
           DefaultplotDialog.cpp \
           objectdebug.cpp \
           SliderSettingsDialog.cpp \
           StateSubwidget.cpp \
           TimeSeriesSubwidget.cpp 

release {
  distribution.path = .
  distribution.file = CopasiUI

  INSTALLS += distribution
}

// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/DataModelGUI.h,v $
//   $Revision: 1.29 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2008/12/18 19:57:54 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#ifndef DATAMODELGUI_H
#define DATAMODELGUI_H

#include <qobject.h>
#include <qapplication.h>

#include "UI/Tree.h"
#include "plotUI/COutputHandlerPlot.h"

//class CMathModel;
class QTimer;
class CMIRIAMResources;

class DataModelGUI
  {
  private:
    IndexedTree mTree; // create the  object of the tree

    //CMathModel * mpMathModel;
    //bool mMathModelUpdateScheduled;

    QApplication *mpApp;

    void linkDataModelToGUI();

    COutputHandlerPlot mOutputHandlerPlot;

  public:
    DataModelGUI();

    void populateData();

    void updateCompartments();
    void updateMetabolites();
    void updateReactions();
    void updateModelValues();
    void updateFunctions();
    void updateEvents();
    void updateReportDefinitions();
    void updatePlots();

    const IndexedNode * getRootNode() const;
    const IndexedNode * getNode(const int & id) const;

    bool loadModel(const std::string & fileName);
    bool createModel();
    bool saveModel(const std::string & fileName, bool overwriteFile = false);

    bool updateMIRIAM(CMIRIAMResources & miriamResources);

    bool importSBMLFromString(const std::string & sbmlDocumentText);
    bool importSBML(const std::string & fileName);
    std::string exportSBMLToString();
    bool exportSBML(const std::string & fileName, bool overwriteFile , int sbmlLevel, int sbmlVersion, bool exportIncomplete, bool exportCOPASIMIRIAM = true);
    bool exportMathModel(const std::string & fileName, const std::string & filter, bool overwriteFile = false);

    COutputDefinitionVector & getPlotDefinitionList();

    //CMathModel * getMathModel() {return mpMathModel;}
    //bool updateMathModel();
    //bool scheduleMathModelUpdate(const bool & update = true);

    void setQApp(QApplication* app);
    QApplication* getQApp() const;
  };

#endif

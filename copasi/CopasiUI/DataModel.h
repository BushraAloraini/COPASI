/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/CopasiUI/Attic/DataModel.h,v $
   $Revision: 1.54 $
   $Name:  $
   $Author: gauges $ 
   $Date: 2004/10/25 13:27:40 $
   End CVS Header */

/****************************************************************************
 **  $ CopasiUI/DataModel.h                 Modified on : 28th March, 2002
 **  $ Author  : Ankur Gupta
 *****************************************************************************/
#ifndef DATAMODEL_H
#define DATAMODEL_H

#include "copasi.h"

class CReadConfig;

class CModel;
class CSteadyStateTask;
class CTrajectoryTask;
class CScanTask;
class CMCATask;
class CReportDefinitionVector;
class CPlotSpec2Vector;
class COptFunction;

//******************************************************************************

class DataModel
  {
  protected:
    CModel * model;
    bool mChanged;

    CSteadyStateTask* steadystatetask;
    CTrajectoryTask* trajectorytask;
    CScanTask* scantask;
    CReportDefinitionVector* reportdefinitions;
    CPlotSpec2Vector* plotspecs;
    COptFunction* pOptFunction;
    CMCATask* mpCMCATask;

  public:
    DataModel();

    void loadModel(const char* fileName);
    void createModel();
    void saveModel(const char* fileName);

    void importSBML(const char* fileName);
    void exportSBML(const char* fileName);

    CModel* getModel() {return model;}

    bool isChanged() const;
    void changed(const bool & changed = true);

    inline CSteadyStateTask* getSteadyStateTask() {return steadystatetask;}
    inline CTrajectoryTask* getTrajectoryTask() {return trajectorytask;}
    inline CMCATask* getMCATask() {return mpCMCATask;}
    inline CReportDefinitionVector* getReportDefinitionVectorAddr(){return reportdefinitions;}
    inline CPlotSpec2Vector* getPlotSpecVectorAddr(){return plotspecs;}
  };

#endif

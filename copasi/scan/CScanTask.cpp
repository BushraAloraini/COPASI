/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/scan/CScanTask.cpp,v $
   $Revision: 1.57 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2005/08/30 15:40:33 $
   End CVS Header */

/**
 * CScanTask class.
 *
 * This class implements a scan task which is comprised of a
 * of a problem and a method.
 *  
 */
#include "copasi.h"
#include "CScanTask.h"
#include "CScanProblem.h"
#include "CScanMethod.h"
#include "utilities/CReadConfig.h"
#include "report/CKeyFactory.h"
#include "report/CReport.h"

#include "model/CModel.h"
#include "model/CState.h"

#include "trajectory/CTrajectoryTask.h"
#include "trajectory/CTrajectoryProblem.h"
#include "steadystate/CSteadyStateTask.h"
#include "steadystate/CSteadyStateProblem.h"
#include "utilities/COutputHandler.h"
#include "utilities/CProcessReport.h"
#include "CopasiDataModel/CCopasiDataModel.h"

CScanTask::CScanTask(const CCopasiContainer * pParent):
    CCopasiTask(CCopasiTask::scan, pParent)
{
  mpProblem = new CScanProblem(this);
  mpMethod = CScanMethod::createMethod();
  this->add(mpMethod, true);
  //  mpMethod->setObjectParent(this);
  ((CScanMethod *) mpMethod)->setProblem((CScanProblem *) mpProblem);
}

CScanTask::CScanTask(const CScanTask & src,
                     const CCopasiContainer * pParent):
    CCopasiTask(src, pParent)
{
  mpProblem = new CScanProblem(* (CScanProblem *) src.mpProblem, this);
  mpMethod = CScanMethod::createMethod();
  this->add(mpMethod, true);
  //  mpMethod->setObjectParent(this);
  ((CScanMethod *) mpMethod)->setProblem((CScanProblem *) mpProblem);
}

CScanTask::~CScanTask()
{cleanup();}

void CScanTask::cleanup()
{
  //pdelete(mpProblem);
  //pdelete(mpMethod);
  //  pdelete(mpOutEnd);
  //-pdelete(mReport);
}

bool CScanTask::initialize(const OutputFlag & of,
                           std::ostream * pOstream)
{
  CScanProblem* pProblem =
    dynamic_cast<CScanProblem *>(mpProblem);

  assert(pProblem);

  bool success = true;

  if (!pProblem->getModel()->compileIfNecessary()) success = false;

  //  pdelete(mpOutEnd);
  //mpOut = & out;
  // added by Liang for Scan Report
  //mReport.open(mpOut);
  //mReport.compile();

  // for Steadystate Report
  //  if (pProblem->processSteadyState())
  //    pProblem->getSteadyStateTask()->initialize(mpOut);

  // for Trajectory Report
  //  if (pProblem->processTrajectory())
  //    pProblem->getTrajectoryTask()->initialize(mpOut);

  //initialize reporting
  if (!CCopasiTask::initialize(of, pOstream)) success = false;

  return success;
}

void CScanTask::load(CReadConfig & C_UNUSED(configBuffer))
{
  CScanProblem* pProblem =
    dynamic_cast<CScanProblem *>(mpProblem);
  assert(pProblem);

  //pProblem->load(configBuffer);
}

bool CScanTask::process(const bool & /* useInitialValues */)
{
  if (!mpProblem) fatalError();
  if (!mpMethod) fatalError();

  mpMethod->isValidProblem(mpProblem);

  CScanProblem * pProblem = dynamic_cast<CScanProblem *>(mpProblem);
  if (!pProblem) fatalError();
  CScanMethod * pMethod = dynamic_cast<CScanMethod *>(mpMethod);
  if (!pMethod) fatalError();

  bool success = true;

  initSubtask();

  pMethod->setProblem(pProblem);

  //TODO: reports

  //initialize the method (parsing the ScanItems)
  if (!pMethod->init()) return false;

  //init progress bar
  mProgress = 0;
  /*
    if (mpCallBack) 
      mpCallBack->init(pMethod->getTotalNumberOfSteps(),
                       "performing parameter scan...",
                       true);
  */

  if (mpCallBack)
    {
      mpCallBack->setName("performing parameter scan...");

      unsigned C_INT32 totalSteps = pMethod->getTotalNumberOfSteps();
      mpCallBack->addItem("Number of Steps",
                          CCopasiParameter::UINT,
                          &mProgress,
                          &totalSteps);
    }
  //init output handler (plotting)
  if (mpOutputHandler) mpOutputHandler->init();

  //calling the scanner, output is done in the callback
  if (!pMethod->scan()) success = false;

  //finishing progress bar and output
  if (mpCallBack) mpCallBack->finish();
  if (mpOutputHandler) mpOutputHandler->finish();

  return success;
}

bool CScanTask::processCallback()
{
  //do tasks
  //CSteadyStateProblem * ssProblem = dynamic_cast<CSteadyStateProblem*>(mpSubtask->getProblem());
  //if (ssProblem)
  //{ssProblem->setInitialState(CCopasiDataModel::Global->getModel()->getInitialState());}
  //CTrajectoryProblem * ttProblem = dynamic_cast<CTrajectoryProblem*>(mpSubtask->getProblem());
  //if (ttProblem)
  //  {ttProblem->setInitialState(CCopasiDataModel::Global->getModel()->getInitialState());}
  mpSubtask->process(!mAdjustInitialConditions);

  //do output
  if (mpOutputHandler && (!mOutputInSubtask)) mpOutputHandler->doOutput();

  //do progress bar
  ++mProgress;
  if (mpCallBack) return mpCallBack->progress();

  return true;
}

bool CScanTask::outputSeparatorCallback(bool isLast)
{
  if ((!isLast) || mOutputInSubtask)
    if (mpOutputHandler) return mpOutputHandler->doSeparator();
  return true;
}

bool CScanTask::initSubtask()
{
  if (!mpProblem) fatalError();
  CScanProblem * pProblem = dynamic_cast<CScanProblem *>(mpProblem);
  if (!pProblem) fatalError();

  //get the parameters from the problem
  CCopasiTask::Type type = *(CCopasiTask::Type*) pProblem->getValue("Subtask").pUINT;

  //CTrajectoryProblem* trajProblem;
  //CSteadyStateProblem* ssProblem;
  switch (type)
    {
    case CCopasiTask::steadyState:
      mpSubtask = dynamic_cast<CCopasiTask*>
                  ((*CCopasiDataModel::Global->getTaskList())["Steady-State"]);
      //ssProblem = dynamic_cast<CSteadyStateProblem*>(mpSubtask->getProblem());
      //ssProblem->setInitialState(CCopasiDataModel::Global->getModel()->getInitialState());
      break;

    case CCopasiTask::timeCourse:
      mpSubtask = dynamic_cast<CCopasiTask*>
                  ((*CCopasiDataModel::Global->getTaskList())["Time-Course"]);
      //trajProblem = dynamic_cast<CTrajectoryProblem*>(mpSubtask->getProblem());
      //trajProblem->setInitialState(CCopasiDataModel::Global->getModel()->getInitialState());
      break;

    default:
      mpSubtask = NULL;
    }

  /*
  if (type == CCopasiTask::steadyState)
    {
      mpSubtask=const_cast<CCopasiTask*>
                   (dynamic_cast<const CCopasiTask*>
                     (CCopasiContainer::Root->getObject(CCopasiObjectName("Task=Steady-State"))));
    }
  else
    {mpSubtask=NULL;}*/

  if (!mpSubtask) return false;

  mpSubtask->getProblem()->setModel(CCopasiDataModel::Global->getModel());
  mpSubtask->setCallBack(NULL);
  mpSubtask->initialize(NO_OUTPUT, NULL);

  mOutputInSubtask = * pProblem->getValue("Output in subtask").pBOOL;
  if (type != CCopasiTask::timeCourse)
    mOutputInSubtask = false;

  mAdjustInitialConditions = * pProblem->getValue("Adjust initial conditions").pBOOL;

  return true;
}

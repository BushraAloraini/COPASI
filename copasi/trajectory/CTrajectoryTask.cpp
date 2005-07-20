/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/trajectory/CTrajectoryTask.cpp,v $
   $Revision: 1.60 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2005/07/20 21:10:42 $
   End CVS Header */

/**
 * CTrajectoryTask class.
 *
 * This class implements a trajectory task which is comprised of a
 * of a problem and a method. Additionally calls to the reporting 
 * methods are done when initialized.
 *  
 * Created for Copasi by Stefan Hoops 2002
 */

#include <string>

#include "copasi.h"

#include "CTrajectoryTask.h"
#include "CTrajectoryProblem.h"
#include "CTrajectoryMethod.h"
#include "model/CModel.h"
#include "model/CState.h"
#include "report/CKeyFactory.h"
#include "report/CReport.h"
#include "utilities/COutputHandler.h"
#include "utilities/CProcessReport.h"
#include "utilities/CCopasiException.h"

#define XXXX_Reporting

bool fle(const C_FLOAT64 & d1, const C_FLOAT64 & d2)
{return (d1 <= d2);}

bool fl(const C_FLOAT64 & d1, const C_FLOAT64 & d2)
{return (d1 < d2);}

bool ble(const C_FLOAT64 & d1, const C_FLOAT64 & d2)
{return (d1 >= d2);}

bool bl(const C_FLOAT64 & d1, const C_FLOAT64 & d2)
{return (d1 > d2);}

CTrajectoryTask::CTrajectoryTask(const CCopasiContainer * pParent):
    CCopasiTask(CCopasiTask::timeCourse, pParent),
    mpState(NULL),
    mTimeSeriesRequested(true),
    mDoOutput(OUTPUT_COMPLETE)
{
  mpProblem = new CTrajectoryProblem(this);
  mpMethod =
    CTrajectoryMethod::createTrajectoryMethod(CCopasiMethod::deterministic,
        (CTrajectoryProblem *) mpProblem);
  this->add(mpMethod, true);
  //mpMethod->setObjectParent(this);
}

/*CTrajectoryTask::CTrajectoryTask(const CTrajectoryTask & src,
                                 const CCopasiContainer * pParent):
    CCopasiTask(src, pParent),
    mpState(src.mpState),
    mTimeSeriesRequested(src.mTimeSeriesRequested)
{fatalError();}*/

CTrajectoryTask::~CTrajectoryTask()
{
  cleanup();
}

void CTrajectoryTask::cleanup()
{
  pdelete(mpState);
}

void CTrajectoryTask::load(CReadConfig & configBuffer)
{
  configBuffer.getVariable("Dynamics", "bool", &mScheduled,
                           CReadConfig::LOOP);

  pdelete(mpProblem);
  mpProblem = new CTrajectoryProblem(this);
  ((CTrajectoryProblem *) mpProblem)->load(configBuffer);

  pdelete(mpMethod);
  mpMethod = CTrajectoryMethod::createTrajectoryMethod();
  this->add(mpMethod, true);
  //mpMethod->setObjectParent(this);
  ((CTrajectoryMethod *)mpMethod)->setProblem((CTrajectoryProblem *) mpProblem);
}

bool CTrajectoryTask::initialize(std::ostream * pOstream)
{
  assert(mpProblem && mpMethod);

  CTrajectoryProblem* pProblem =
    dynamic_cast<CTrajectoryProblem *>(mpProblem);
  assert(pProblem);

  bool success = true;

  if (!mReport.open(pOstream)) success = false;
  if (!mReport.compile()) success = false;
  if (!pProblem->getModel()->compileIfNecessary()) success = false;
  //  if (!pProblem->
  //      setInitialState(pProblem->getModel()->getInitialState()))
  //    success = false;

  //pProblem->setInitialState(pProblem->getModel()->getInitialState());

  return success;
}

bool CTrajectoryTask::process()
{
  unsigned C_INT32 FailCounter = 0;

  assert(/*mpProblem && */mpMethod);
  mpMethod->isValidProblem(mpProblem);

  CTrajectoryProblem * pProblem = (CTrajectoryProblem *) mpProblem;
  CTrajectoryMethod * pMethod = (CTrajectoryMethod *) mpMethod;

  //the following is a hack that has to disappear soon.
  pProblem->setInitialState(pProblem->getModel()->getInitialState());

  mTimeSeriesRequested = pProblem->timeSeriesRequested();

  pdelete(mpState);
  mpState = new CState(pProblem->getInitialState());

  pMethod->setCurrentState(mpState);
  pMethod->setProblem(pProblem);

  C_FLOAT64 StepSize = pProblem->getStepSize();
  C_FLOAT64 NextTimeToReport;

  bool (*LE)(const C_FLOAT64 &, const C_FLOAT64 &);
  bool (*L)(const C_FLOAT64 &, const C_FLOAT64 &);
  if (StepSize < 0.0)
    {
      LE = &ble;
      L = &bl;
    }
  else
    {
      LE = &fle;
      L = &fl;
    }

  const C_FLOAT64 & Time = mpState->getTime();
  const C_FLOAT64 & EndTime = pProblem->getEndTime();
  const C_FLOAT64 & StartTime = pProblem->getStartTime();

  C_FLOAT64 ActualStepSize;
  C_FLOAT64 StepNumber = (EndTime - StartTime) / StepSize;

  unsigned C_INT32 StepCounter = 1;

  C_FLOAT64 outputStartTime = pProblem->getOutputStartTime();

  if (StepSize == 0.0 && pProblem->getEndTime() != pProblem->getStartTime())
    {
      CCopasiMessage(CCopasiMessage::ERROR, MCTrajectoryProblem + 1, StepSize);
      return false;
    }
  bool flagProceed = true;
  C_FLOAT64 handlerFactor = 100.0 / (pProblem->getEndTime() - pProblem->getStartTime());

  /*
    if (mpCallBack)
      mpCallBack->init(1000,
                              "performing simulation...",
                              true);
  */
  C_FLOAT64 Percentage = 0;
  unsigned C_INT32 hProcess;

  if (mpCallBack)
    {
      mpCallBack->setName("performing simulation...");
      C_FLOAT64 hundred = 100;
      hProcess = mpCallBack->addItem("Completion",
                                     CCopasiParameter::DOUBLE,
                                     &Percentage,
                                     &hundred);
    }

  if (mDoOutput)
    {
      pProblem->getModel()->setState(mpState);
      pProblem->getModel()->updateRates();
    }

  if (mDoOutput == OUTPUT_COMPLETE)
    {
      pProblem->getModel()->setState(mpState);
      pProblem->getModel()->updateRates();
      mReport.printHeader();
      if (mpOutputHandler) mpOutputHandler->init();
      if (mTimeSeriesRequested) mTimeSeries.init(pProblem->getStepNumber(), mpState);
    }

  if ((*LE)(outputStartTime, Time) && (mDoOutput))
    {
      mReport.printBody();
      if (mpOutputHandler) mpOutputHandler->doOutput();
      if (mTimeSeriesRequested) mTimeSeries.add();
    }

  // We start the integration
  // This is numerically more stable then adding pProblem->getStepSize().
  NextTimeToReport = (EndTime - StartTime) * StepCounter++ / StepNumber;
  try
    {
      ActualStepSize = pMethod->step(StepSize, mpState);
    }
  catch (CCopasiException Exception)
    {
      if (mpCallBack) mpCallBack->finish(hProcess);
      throw CCopasiException(Exception.getMessage());
    }

  if (mpCallBack)
    {
      Percentage = (Time - pProblem->getStartTime()) * handlerFactor;
      flagProceed = mpCallBack->progress(hProcess);
    }

  if ((mDoOutput) &&
      (*LE)(outputStartTime, Time) &&
      !(*L)(Time, NextTimeToReport * (1 - 100 * DBL_EPSILON)))
    {
      pProblem->getModel()->setState(mpState);
      pProblem->getModel()->updateRates();
      mReport.printBody();
      if (mpOutputHandler) mpOutputHandler->doOutput();
      if (mTimeSeriesRequested) mTimeSeries.add();
    }

  if ((*L)(Time, NextTimeToReport * (1 - 100 * DBL_EPSILON)))
    {
      /* Here we will do conditional event processing */
      FailCounter++;
      StepSize = NextTimeToReport - Time;
      if (mpCallBack)
        flagProceed = mpCallBack->proceed();
    }
  else
    {
      FailCounter = 0;
      // This is numerically more stable then adding pProblem->getStepSize().
      NextTimeToReport = (EndTime - StartTime) * StepCounter++ / StepNumber;
      // Make sure that we do not overstep
      if ((*L)(EndTime, NextTimeToReport)) NextTimeToReport = EndTime;
      StepSize = NextTimeToReport - Time;
    }

  while ((*L)(Time, EndTime * (1 - 100 * DBL_EPSILON)) && flagProceed)
    {
      try
        {
          ActualStepSize = pMethod->step(StepSize);
        }
      catch (CCopasiException Exception)
        {
          if (mpCallBack) mpCallBack->finish(hProcess);
          throw CCopasiException(Exception.getMessage());
        }

      //std::cout << EndTime << "  " << Time << "  " << EndTime-Time << std::endl;

      if (mpCallBack)
        {
          Percentage = (Time - pProblem->getStartTime()) * handlerFactor;
          flagProceed = mpCallBack->progress(hProcess);
        }

      if ((mDoOutput) &&
          (*LE)(outputStartTime, Time) &&
          !(*L)(Time, NextTimeToReport * (1 - 100 * DBL_EPSILON)))
        {
          pProblem->getModel()->setState(mpState);
          pProblem->getModel()->updateRates();
          mReport.printBody();
          if (mpOutputHandler) mpOutputHandler->doOutput();
          if (mTimeSeriesRequested) mTimeSeries.add();
        }

      if ((*L)(Time, NextTimeToReport * (1 - 100 * DBL_EPSILON)))
        {
          /* Here we will do conditional event processing */

          FailCounter++;
          if (FailCounter > 10)
            CCopasiMessage(CCopasiMessage::EXCEPTION, MCTrajectoryMethod + 12);

          StepSize = NextTimeToReport - Time;
          if (mpCallBack)
            flagProceed = mpCallBack->proceed();
        }
      else
        {
          FailCounter = 0;
          // This is numerically more stable then adding pProblem->getStepSize().
          NextTimeToReport = (EndTime - StartTime) * StepCounter++ / StepNumber;
          // Make sure that we do not overstep
          if ((*L)(EndTime, NextTimeToReport)) NextTimeToReport = EndTime;
          StepSize = NextTimeToReport - Time;
        }
    }

  //pProblem->setEndState(new CState(*mpState));

  if (mpCallBack) mpCallBack->finish(hProcess);
  pProblem->getModel()->setState(mpState);
  pProblem->getModel()->updateRates();

  if ((*LE)(outputStartTime, Time) && (mDoOutput))
    {
      mReport.printFooter();
      if (mpOutputHandler) mpOutputHandler->finish();
      if (mTimeSeriesRequested) mTimeSeries.finish();
    }

  return true;
}

//virtual
bool CTrajectoryTask::processForScan(bool useInitialConditions, bool doOutput)
{
  assert(/*mpProblem && */mpMethod);
  mpMethod->isValidProblem(mpProblem);

  CTrajectoryProblem* pProblem =
    dynamic_cast<CTrajectoryProblem *>(mpProblem);
  assert(pProblem);

  //set flag for output
  OutputFlag storeOutput = mDoOutput;
  if (doOutput)
    mDoOutput = OUTPUT;
  else
    mDoOutput = NO_OUTPUT;

  //handle initial conditions
  CState storeState;
  if (!useInitialConditions)
    {
      storeState = pProblem->getInitialState();
      pProblem->getModel()->setTime(pProblem->getInitialState().getTime());
      pProblem->setInitialState(pProblem->getModel()->getState());
      // pProblem->getInitialState().setTime(storeState.getTime());
    }

  //switch off time series storage
  bool storeTS = pProblem->timeSeriesRequested();
  pProblem->setTimeSeriesRequested(false);

  //do the calculation
  process();

  //restore ...
  if (!useInitialConditions) pProblem->setInitialState(storeState);
  mDoOutput = storeOutput;
  pProblem->setTimeSeriesRequested(storeTS);

  return true;
}

bool CTrajectoryTask::processSimple(bool singleStep) //without output
{
  assert(/*mpProblem && */mpMethod);
  mpMethod->isValidProblem(mpProblem); //TODO perhaps omit this check for performance reasons?

  CTrajectoryProblem * pProblem = (CTrajectoryProblem *) mpProblem;
  CTrajectoryMethod * pMethod = (CTrajectoryMethod *) mpMethod;

  //give the method a state to work on
  pdelete(mpState);
  mpState = new CState(pProblem->getInitialState());
  pMethod->setCurrentState(mpState);

  pMethod->setProblem(pProblem);

  C_FLOAT64 StepSize = pProblem->getEndTime() - pProblem->getInitialState().getTime();
  bool (*L)(const C_FLOAT64 &, const C_FLOAT64 &) = (StepSize < 0.0) ? &bl : &fl;

  bool flagProceed = false;
  C_FLOAT64 handlerFactor = 100.0 / (pProblem->getEndTime() - pProblem->getStartTime());

  C_FLOAT64 Percentage = 0;
  unsigned C_INT32 hProcess;
  if (mpCallBack)
    {
      mpCallBack->setName("performing simulation...");
      C_FLOAT64 hundred = 100;
      hProcess = mpCallBack->addItem("%",
                                     CCopasiParameter::DOUBLE,
                                     &Percentage,
                                     &hundred);
    }

  //first step
  pMethod->step(StepSize, &pProblem->getInitialState());

  if (mpCallBack)
    {
      Percentage = (mpState->getTime() - pProblem->getStartTime()) * handlerFactor;
      flagProceed = mpCallBack->progress(hProcess);
    }

  if (mpState->getTime() == pProblem->getEndTime()) return true; //end reached in one step
  if (singleStep) return false; //end not reached but only one step requested

  //more Steps if necessary
  while ((*L)(mpState->getTime(), pProblem->getEndTime()) && (!flagProceed))
    {
      StepSize = pProblem->getEndTime() - mpState->getTime();
      pMethod->step(StepSize);

      if (mpCallBack)
        {
          Percentage = (mpState->getTime() - pProblem->getStartTime()) * handlerFactor;
          flagProceed = mpCallBack->progress(hProcess);
        }
    }

  //pProblem->setEndState(new CState(*mpState));

  if (mpCallBack) mpCallBack->finish(hProcess);

  return true;
}

bool CTrajectoryTask::setMethodType(const int & type)
{
  CCopasiMethod::SubType Type = (CCopasiMethod::SubType) type;

  if (!CTrajectoryMethod::isValidSubType(Type)) return false;
  if (mpMethod->getSubType() == Type) return true;

  pdelete (mpMethod);
  mpMethod =
    CTrajectoryMethod::createTrajectoryMethod(Type,
        (CTrajectoryProblem *) mpProblem);
  this->add(mpMethod, true);
  //mpMethod->setObjectParent(this);

  return true;
}

CState * CTrajectoryTask::getState()
{return mpState;}

const CTimeSeries & CTrajectoryTask::getTimeSeries() const
  {return mTimeSeries;}

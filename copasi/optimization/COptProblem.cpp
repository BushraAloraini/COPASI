/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/optimization/COptProblem.cpp,v $
   $Revision: 1.33 $
   $Name:  $
   $Author: anuragr $ 
   $Date: 2005/05/13 18:04:46 $
   End CVS Header */

/**
 *  File name: COptProblem.cpp
 *
 *  Programmer: Yongqun He
 *  Contact email: yohe@vt.edu
 *  Purpose: This is the source file of the COptProblem class.
 *           It specifies the optimization problem with its own members and
 *           functions. It's used by COptAlgorithm class and COptimization class
 */
#include <float.h>

#include "copasi.h"
#include "COptProblem.h"
#include "COptItem.h"

#include "steadystate/CSteadyStateTask.h"
#include "trajectory/CTrajectoryTask.h"
#include "steadystate/CSteadyStateProblem.h"
#include "trajectory/CTrajectoryProblem.h"

#include "model/CModel.h"
#include "model/CCompartment.h"

#include "report/CCopasiObjectReference.h"
#include "utilities/CProcessReport.h"

//  Default constructor
COptProblem::COptProblem(const CCopasiContainer * pParent):
    CCopasiProblem(CCopasiTask::optimization, pParent),
    mpSteadyState(NULL),
    mpTrajectory(NULL),
    mOptItemList()
{
  addGroup("OptimizationItemList");
  addGroup("OptimizationConstraintList");

  addParameter("SteadyState", CCopasiParameter::STRING, (std::string) "");
  addParameter("Trajectory", CCopasiParameter::STRING, (std::string) "");
  addParameter("ObjectiveFunction", CCopasiParameter::STRING, (std::string) "");
  addParameter("Maximize", CCopasiParameter::BOOL, (bool) false);
}

// copy constructor
COptProblem::COptProblem(const COptProblem& src,
                         const CCopasiContainer * pParent):
    CCopasiProblem(src, pParent),
    mpSteadyState(src.mpSteadyState),
    mpTrajectory(src.mpTrajectory),
    mOptItemList()
{}

// Destructor
COptProblem::~COptProblem()
{
  std::vector<COptItem *>::iterator it = mOptItemList.begin();
  std::vector<COptItem *>::iterator end = mOptItemList.end();

  for (; it != end; ++it)
    pdelete(*it);
}

bool COptProblem::setModel(CModel * pModel)
{
  mpModel = pModel;
  return true;
}

bool COptProblem::setCallBack(CProcessReport * pCallBack)
{
  CCopasiProblem::setCallBack(pCallBack);

  mpCallBack->addItem("Simulation Counter",
                      CCopasiParameter::DOUBLE,
                      getObject(CCopasiObjectName("Reference=Simulation Counter")));

  return true;
}

void COptProblem::initObjects()
{
  addObjectReference("Simulation Counter", mCounter, CCopasiObject::ValueDbl);
}

bool COptProblem::initialize()
{
  if (!mpModel) return false;
  mpModel->compileIfNecessary();

  std::vector< CCopasiContainer * > ContainerList;
  ContainerList.push_back(mpModel);
  ContainerList.push_back(getObjectParent());

  unsigned C_INT32 i;
  unsigned C_INT32 Size = mOptItemList.size();
  mUpdateMethods.resize(Size);
  mSolutionVariables.resize(Size);

  std::vector< COptItem * >::iterator it = mOptItemList.begin();
  std::vector< COptItem * >::iterator end = mOptItemList.end();

  for (i = 0; it != end; ++it, i++)
    {
      if (!(*it)->compile(ContainerList)) return false;
      mUpdateMethods[i] = (*it)->getUpdateMethod();
    }

  return true;
}

bool COptProblem::checkParametricConstraints()
{
  std::vector< COptItem * >::const_iterator it = mOptItemList.begin();
  std::vector< COptItem * >::const_iterator end = mOptItemList.end();

  for (; it != end; ++it)
    if (!(*it)->checkConstraint()) return false;

  return true;
}

bool COptProblem::checkFunctionalConstraints()
{
  return true; // :TODO:
}

/**
 * calculate() decides whether the problem is a steady state problem or a
 * trajectory problem based on whether the pointer to that type of problem
 * is null or not.  It then calls the process() method for that type of 
 * problem.  Currently process takes ofstream& as a parameter but it will
 * change so that process() takes no parameters.
 */
bool COptProblem::calculate()
{
  if (mpSteadyState != NULL)
    {
      ((CSteadyStateProblem *) mpSteadyState->getProblem())->
      setInitialState(mpSteadyState->getProblem()->getModel()->getInitialState());
      mpSteadyState->process();
    }

  if (mpTrajectory != NULL)
    {
      ((CTrajectoryProblem *) mpTrajectory->getProblem())->
      setInitialState(mpTrajectory->getProblem()->getModel()->getInitialState());
      mpTrajectory->process();
    }

  // :TODO: mCalculateValue = mpFunction->calvValue(NULL);

  mCounter += 1;

  if (mpCallBack) return mpCallBack->progress();
  return true;
}

const C_FLOAT64 & COptProblem::getCalculateValue() const
{return mCalculateValue;}

void COptProblem::setSolutionVariables(const CVector< C_FLOAT64 > & variables)
{mSolutionVariables = variables;}

const CVector< C_FLOAT64 > & COptProblem::getSolutionVariables() const
  {return mSolutionVariables;}

void COptProblem::setSolutionValue(const C_FLOAT64 & value)
{mSolutionValue = value;}

const C_FLOAT64 & COptProblem::getSolutionValue() const
  {return mSolutionValue;}

// set the type of problem : Steady State OR Trajectory
void COptProblem::setProblemType(ProblemType type)
{
  // :TODO:
  if (type == SteadyState)
    mpSteadyState = new CSteadyStateTask(/*this*/NULL);
  if (type == Trajectory)
    mpTrajectory = new CTrajectoryTask(/*this*/NULL);
}

COptItem COptProblem::getOptItem(const unsigned C_INT32 & index)
{
  assert (index < mOptItemList.size());
  return *mOptItemList[index];
}

const unsigned C_INT32 COptProblem::getOptItemSize() const
  {return ((CCopasiParameterGroup *) getValue("OptimizationItemList"))->size();}

COptItem COptProblem::addOptItem(const CCopasiObjectName & objectCN)
{
  unsigned C_INT32 index = getOptItemSize();
  CCopasiParameterGroup * pOptimizationItemList
  = (CCopasiParameterGroup *) getValue("OptimizationItemList");

  pOptimizationItemList->addGroup("OptimizationItem");

  CCopasiParameterGroup * pOptItem =
    (CCopasiParameterGroup *) pOptimizationItemList->getValue(index);

  assert(pOptItem != NULL);

  COptItem * pTmp = new COptItem(*pOptItem);
  if (!pTmp->initialize(objectCN)) fatalError();

  mOptItemList.push_back(pTmp);

  return *pTmp;
}

bool COptProblem::removeOptItem(const unsigned C_INT32 & index)
{
  if (!(index < mOptItemList.size())) return false;

  pdelete(mOptItemList[index]);
  mOptItemList.erase(mOptItemList.begin() + index);

  return ((CCopasiParameterGroup *) getValue("OptimizationItemList"))->removeParameter(index);
}

bool COptProblem::swapOptItem(const unsigned C_INT32 & iFrom,
                              const unsigned C_INT32 & iTo)
{
  if (!(iFrom < mOptItemList.size()) ||
      !(iTo < mOptItemList.size()))
    return false;

  COptItem * pTmp = mOptItemList[iFrom];
  mOptItemList[iFrom] = mOptItemList[iTo];
  mOptItemList[iTo] = pTmp;

  return ((CCopasiParameterGroup *) getValue("OptimizationItemList"))->swap(iFrom, iTo);
}

const std::vector< COptItem * > & COptProblem::getOptItemList() const
{return mOptItemList;}

const std::vector< UpdateMethod * > & COptProblem::getCalculateVariableUpdateMethods() const
  {return mUpdateMethods;}

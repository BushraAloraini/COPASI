/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/optimization/COptProblem.cpp,v $
   $Revision: 1.63 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2005/09/15 18:45:26 $
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
#include "COptTask.h"
#include "COptProblem.h"
#include "COptItem.h"

#include "function/CFunctionDB.h"

#include "CopasiDataModel/CCopasiDataModel.h"

#include "steadystate/CSteadyStateTask.h"
#include "trajectory/CTrajectoryTask.h"
#include "steadystate/CSteadyStateProblem.h"
#include "trajectory/CTrajectoryProblem.h"

#include "model/CModel.h"
#include "model/CCompartment.h"

#include "report/CCopasiObjectReference.h"
#include "report/CKeyFactory.h"

#include "utilities/CProcessReport.h"

//  Default constructor
COptProblem::COptProblem(const CCopasiContainer * pParent):
    CCopasiProblem(CCopasiTask::optimization, pParent),
    mpSteadyState(NULL),
    mpTrajectory(NULL),
    mpFunction(NULL),
    mOptItemList()
{
  addParameter("Steady-State", CCopasiParameter::KEY, (std::string) "");
  addParameter("Time-Course", CCopasiParameter::KEY, (std::string) "");
  addParameter("ObjectiveFunction", CCopasiParameter::KEY, (std::string) "");
  addParameter("Maximize", CCopasiParameter::BOOL, (bool) false);

  addGroup("OptimizationItemList");
  addGroup("OptimizationConstraintList");

  initObjects();
}

// copy constructor
COptProblem::COptProblem(const COptProblem& src,
                         const CCopasiContainer * pParent):
    CCopasiProblem(src, pParent),
    mpSteadyState(src.mpSteadyState),
    mpTrajectory(src.mpTrajectory),
    mpFunction(NULL),
    mOptItemList()
{
  if (src.mpFunction)
    {
      createObjectiveFunction();
      mpFunction->setInfix(src.mpFunction->getInfix());
      setValue("ObjectiveFunction", mpFunction->getKey());
    }

  initObjects();
}

// Destructor
COptProblem::~COptProblem()
{
  std::vector<COptItem *>::iterator it = mOptItemList.begin();
  std::vector<COptItem *>::iterator end = mOptItemList.end();

  for (; it != end; ++it)
    pdelete(*it);

  if (mpFunction && CCopasiDataModel::Global &&
      CCopasiDataModel::Global->getFunctionList() &&
      CCopasiDataModel::Global->getFunctionList()->loadedFunctions()[mpFunction->getObjectName()] == mpFunction)
    CCopasiDataModel::Global->getFunctionList()->loadedFunctions().remove(mpFunction->getObjectName());

  pdelete(mpFunction);
}

bool COptProblem::setModel(CModel * pModel)
{
  mpModel = pModel;
  return true;
}

bool COptProblem::setCallBack(CProcessReport * pCallBack)
{
  CCopasiProblem::setCallBack(pCallBack);

  if (pCallBack)
    {
      mhSolutionValue =
        mpCallBack->addItem("Best Value",
                            CCopasiParameter::DOUBLE,
                            & mSolutionValue);
      mhCounter =
        mpCallBack->addItem("Simulation Counter",
                            CCopasiParameter::UINT,
                            & mCounter);
    }

  return true;
}

void COptProblem::initObjects()
{
  addObjectReference("Simulation Counter", mCounter, CCopasiObject::ValueInt);
  addObjectReference("Best Value", mSolutionValue, CCopasiObject::ValueDbl);
  addVectorReference("Best Parameters", mSolutionVariables, CCopasiObject::ValueDbl);
}

#ifdef WIN32 
// warning C4056: overflow in floating-point constant arithmetic
// warning C4756: overflow in constant arithmetic
# pragma warning (disable: 4056 4756)
#endif

bool COptProblem::initialize()
{
  if (!mpModel) return false;
  mpModel->compileIfNecessary();

  mpReport = NULL;
  mCounter = 0;
  mSolutionValue = DBL_MAX * 2;

  std::vector< CCopasiContainer * > ContainerList;
  ContainerList.push_back(mpModel);

  COptTask * pTask = dynamic_cast<COptTask *>(getObjectParent());
  if (pTask)
    {
      ContainerList.push_back(pTask);
      mpReport = &pTask->getReport();
      if (!mpReport->getStream()) mpReport = NULL;
    }

  mpSteadyState =
    dynamic_cast<CSteadyStateTask *>(GlobalKeys.get(* getValue("Steady-State").pKEY));
  mpTrajectory =
    dynamic_cast<CTrajectoryTask *>(GlobalKeys.get(* getValue("Time-Course").pKEY));

  if (!mpSteadyState && !mpTrajectory) return false;

  if (mpSteadyState)
    {
      mpSteadyState->initialize(CCopasiTask::NO_OUTPUT, NULL);
      ContainerList.push_back(mpSteadyState);
    }
  if (mpTrajectory)
    {
      mpTrajectory->initialize(CCopasiTask::NO_OUTPUT, NULL);
      ContainerList.push_back(mpTrajectory);
    }

  buildOptItemListFromParameterGroup();

  unsigned C_INT32 i;
  unsigned C_INT32 Size = mOptItemList.size();

  mUpdateMethods.resize(Size);
  mSolutionVariables.resize(Size);
  mOriginalVariables.resize(Size);

  std::vector< COptItem * >::iterator it = mOptItemList.begin();
  std::vector< COptItem * >::iterator end = mOptItemList.end();

  if (it == end)
    {
      CCopasiMessage(CCopasiMessage::ERROR, MCOptimization + 6);
      return false;
    }

  for (i = 0; it != end; ++it, i++)
    {
      if (!(*it)->compile(ContainerList)) return false;
      mUpdateMethods[i] = (*it)->getUpdateMethod();
      mOriginalVariables[i] = *(*it)->getObjectValue();
    }

  createObjectiveFunction();
  if (!mpFunction) return false;

  if (!mpFunction->compile(ContainerList))
    {
      CCopasiMessage(CCopasiMessage::ERROR, MCOptimization + 5);
      return false;
    }

  return true;
}

bool COptProblem::restore()
{
  unsigned C_INT32 i, imax = mOptItemList.size();

  // set the original paramter values
  for (i = 0; i < imax; i++)
    (*mUpdateMethods[i])(mOriginalVariables[i]);

  return true;
}

#ifdef WIN32
# pragma warning (default: 4056 4756)
#endif

bool COptProblem::checkParametricConstraints()
{
  std::vector< COptItem * >::const_iterator it = mOptItemList.begin();
  std::vector< COptItem * >::const_iterator end = mOptItemList.end();

  for (; it != end; ++it)
    if ((*it)->checkConstraint()) return false;

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
  mCounter += 1;
  bool success = false;

  try
    {
      if (mpSteadyState != NULL)
        {
          //((CSteadyStateProblem *) mpSteadyState->getProblem())->
          //setInitialState(mpSteadyState->getProblem()->getModel()->getInitialState());
          success = mpSteadyState->process(true);
        }

      if (mpTrajectory != NULL)
        {
          //((CTrajectoryProblem *) mpTrajectory->getProblem())->
          //setInitialState(mpTrajectory->getProblem()->getModel()->getInitialState());
          success = mpTrajectory->process(true);
        }

      mCalculateValue = mpFunction->calcValue();
    }

  catch (...)
    {
      success = false;
    }

  if (!success) mCalculateValue = DBL_MAX;

  if (mpCallBack) return mpCallBack->progress(mhCounter);

  return true;
}

const C_FLOAT64 & COptProblem::getCalculateValue() const
{return mCalculateValue;}

void COptProblem::setSolutionVariables(const CVector< C_FLOAT64 > & variables)
{mSolutionVariables = variables;}

const CVector< C_FLOAT64 > & COptProblem::getSolutionVariables() const
  {return mSolutionVariables;}

bool COptProblem::setSolutionValue(const C_FLOAT64 & value)
{
  mSolutionValue = value;
  if (mpCallBack) return mpCallBack->progress(mhSolutionValue);

  return true;
}

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

COptItem & COptProblem::getOptItem(const unsigned C_INT32 & index)
{
  if (mOptItemList.size() != getOptItemSize())
    buildOptItemListFromParameterGroup();

  assert(index < mOptItemList.size());

  return *mOptItemList[index];
}

const unsigned C_INT32 COptProblem::getOptItemSize() const
{return getValue("OptimizationItemList").pGROUP->size();}

COptItem & COptProblem::addOptItem(const CCopasiObjectName & objectCN)
{
  if (mOptItemList.size() != getOptItemSize())
    buildOptItemListFromParameterGroup();

  unsigned C_INT32 index = getOptItemSize();

  CCopasiParameterGroup * pOptimizationItemList = (CCopasiParameterGroup *) getParameter("OptimizationItemList");

  pOptimizationItemList->addGroup("OptimizationItem");

  CCopasiParameterGroup * pOptItem =
    (CCopasiParameterGroup *) pOptimizationItemList->getParameter(index);

  assert(pOptItem != NULL);

  COptItem * pTmp = new COptItem(*pOptItem);

  //pTmp->setLowerBound(

  if (!pTmp->initialize(objectCN)) fatalError();

  mOptItemList.push_back(pTmp);

  return *pTmp;
}

bool COptProblem::removeOptItem(const unsigned C_INT32 & index)
{
  if (mOptItemList.size() != getOptItemSize())
    buildOptItemListFromParameterGroup();

  if (!(index < mOptItemList.size())) return false;

  pdelete(mOptItemList[index]);
  mOptItemList.erase(mOptItemList.begin() + index);

  return ((CCopasiParameterGroup *) getParameter("OptimizationItemList"))->removeParameter(index);
}

bool COptProblem::swapOptItem(const unsigned C_INT32 & iFrom,
                              const unsigned C_INT32 & iTo)
{
  if (mOptItemList.size() != getOptItemSize())
    buildOptItemListFromParameterGroup();

  if (!(iFrom < mOptItemList.size()) ||
      !(iTo < mOptItemList.size()))
    return false;

  COptItem * pTmp = mOptItemList[iFrom];
  mOptItemList[iFrom] = mOptItemList[iTo];
  mOptItemList[iTo] = pTmp;

  return ((CCopasiParameterGroup *) getParameter("OptimizationItemList"))->swap(iFrom, iTo);
}

const std::vector< COptItem * > & COptProblem::getOptItemList() const
  {
    if (mOptItemList.size() != getOptItemSize())
      const_cast<COptProblem *>(this)->buildOptItemListFromParameterGroup();

    return mOptItemList;
  }

const std::vector< UpdateMethod * > & COptProblem::getCalculateVariableUpdateMethods() const
{return mUpdateMethods;}

bool COptProblem::setObjectiveFunction(const std::string & infix)
{
  if (!mpFunction) createObjectiveFunction();

  return mpFunction->setInfix(infix);
}

const std::string COptProblem::getObjectiveFunction()
{
  if (!mpFunction) createObjectiveFunction();

  return mpFunction->getInfix();
}

bool COptProblem::createObjectiveFunction()
{
  mpFunction =
    dynamic_cast<CExpression *>(GlobalKeys.get(* getValue("ObjectiveFunction").pKEY));

  CCopasiVectorN<CEvaluationTree> & FunctionList =
    CCopasiDataModel::Global->getFunctionList()->loadedFunctions();

  if (!mpFunction)
    {
      std::ostringstream Name;
      Name << "Objective Function";

      int i = 0;

      while (FunctionList.getIndex(Name.str()) != C_INVALID_INDEX)
        {
          i++;
          Name.str("");
          Name << "Objective Function " << i;
        }

      mpFunction = new CExpression(Name.str(), this);
      FunctionList.add(mpFunction, false);

      setValue("ObjectiveFunction", mpFunction->getKey());
    }
  else
    {
      mpFunction->setObjectParent(this);

      if (FunctionList.getIndex(mpFunction->getObjectName()) == C_INVALID_INDEX)
        FunctionList.add(mpFunction, false);
    }

  return true;
}

bool COptProblem::buildOptItemListFromParameterGroup()
{
  bool success = true;

  std::vector< COptItem * >::iterator it = mOptItemList.begin();
  std::vector< COptItem * >::iterator end = mOptItemList.end();

  for (; it != end; ++it)
    pdelete(*it);

  unsigned i, imax = getOptItemSize();
  mOptItemList.resize(imax);

  std::vector<CCopasiParameter *> & List = *getValue("OptimizationItemList").pGROUP;

  for (i = 0; i < imax; i++)
    {
      mOptItemList[i] = new COptItem(* (CCopasiParameterGroup *) List[i]);
      if (!mOptItemList[i]->isValid()) success = false;
    }

  return true;
}

void COptProblem::print(std::ostream * ostream) const
{*ostream << *this;}

void COptProblem::printResult(std::ostream * ostream) const
  {
    *ostream << "    Objective Function Value: " << mSolutionValue << std::endl;
    *ostream << std::endl;

    std::vector< COptItem * >::const_iterator itItem =
      mOptItemList.begin();
    std::vector< COptItem * >::const_iterator endItem =
      mOptItemList.end();

    unsigned C_INT32 i;

    for (i = 0; itItem != endItem; ++itItem, i++)
      *ostream << "    " << (*itItem)->getObjectDisplayName() << ": "
      << mSolutionVariables[i] << std::endl;
  }

std::ostream &operator<<(std::ostream &os, const COptProblem & o)
{
  os << "Problem Description:" << std::endl;

  os << "Subtask: " << std::endl;

  if (o.mpSteadyState)
    o.mpSteadyState->getDescription().print(&os);
  else if (o.mpTrajectory)
    o.mpTrajectory->getDescription().print(&os);
  else
    os << "No Subtask specified.";

  os << std::endl;

  os << "Objective Function:" << std::endl;
  os << "    " << o.mpFunction->getDisplayString() << std::endl;
  os << std:: endl;

  os << "List of Optimization Items:" << std::endl;

  std::vector< COptItem * >::const_iterator itItem =
    o.mOptItemList.begin();
  std::vector< COptItem * >::const_iterator endItem =
    o.mOptItemList.end();

  for (; itItem != endItem; ++itItem)
    os << "    " << **itItem << std::endl;

  return os;
}

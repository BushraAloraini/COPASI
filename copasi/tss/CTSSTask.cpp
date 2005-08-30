/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/tss/Attic/CTSSTask.cpp,v $
   $Revision: 1.5 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2005/08/30 15:40:49 $
   End CVS Header */

/**
 * CTSSTask class.
 *
 */

#include "copasi.h"

#include "CTSSTask.h"
#include "CTSSProblem.h"
#include "CTSSMethod.h"
#include "model/CModel.h"
#include "model/CState.h" 
//#include "model/CMetabNameInterface.h"
#include "report/CKeyFactory.h"
#include "report/CReport.h"

#define XXXX_Reporting

CTSSTask::CTSSTask(const CCopasiContainer * pParent):
    CCopasiTask(CCopasiTask::tss, pParent)
{
  mpProblem = new CTSSProblem(this);
  mpMethod = CTSSMethod::createTSSMethod(CCopasiMethod::tssMethod);
  this->add(mpMethod, true);
  //mpMethod->setObjectParent(this);
  //((CSteadyStateMethod *) mpMethod)->setProblem((CSteadyStateProblem *) mpProblem);
}

CTSSTask::CTSSTask(const CTSSTask & src,
                   const CCopasiContainer * pParent):
    CCopasiTask(src, pParent)
{
  mpProblem =
    new CTSSProblem(* (CTSSProblem *) src.mpProblem, this);
  mpMethod =
    CTSSMethod::createTSSMethod(src.mpMethod->getSubType());
  this->add(mpMethod, true);
  //mpMethod->setObjectParent(this);
  //((CSteadyStateMethod *) mpMethod)->setProblem((CSteadyStateProblem *) mpProblem);
}

CTSSTask::~CTSSTask()
{}

void CTSSTask::cleanup()
{}

void CTSSTask::print(std::ostream * ostream) const {(*ostream) << (*this);}

bool CTSSTask::initialize(const OutputFlag & of, std::ostream * pOstream)
{
  assert(mpProblem && mpMethod);

  CTSSProblem* pProblem =
    dynamic_cast<CTSSProblem *>(mpProblem);
  assert(pProblem);

  bool success = true;

  if (CCopasiTask::initialize(of, pOstream)) success = false;

  if (!pProblem->getModel()->compileIfNecessary()) success = false;
  //pProblem->setInitialState(pProblem->getModel()->getInitialState());

  return success;
}

bool CTSSTask::process(const bool & /* useInitialValues */)
{
  assert(/*mpProblem && */mpMethod);
  mpMethod->isValidProblem(mpProblem);

  CTSSProblem* pProblem =
    dynamic_cast<CTSSProblem *>(mpProblem);
  assert(pProblem);

  CTSSMethod* pMethod =
    dynamic_cast<CTSSMethod *>(mpMethod);
  assert(pMethod);

  mReport.printHeader();

  bool success = pMethod->process(mpCallBack);

  mReport.printBody();
  mReport.printFooter();

  return (success);
}

bool CTSSTask::processForScan(bool useInitialConditions, bool doOutput)
{
  assert(/*mpProblem && */mpMethod);
  mpMethod->isValidProblem(mpProblem);

  CTSSProblem* pProblem =
    dynamic_cast<CTSSProblem *>(mpProblem);
  assert(pProblem);

  CTSSMethod* pMethod =
    dynamic_cast<CTSSMethod *>(mpMethod);
  assert(pMethod);

  bool success = pMethod->process(mpCallBack);

  return success;
}

std::ostream &operator<<(std::ostream &os, const CTSSTask &A)
{
  return os;
}

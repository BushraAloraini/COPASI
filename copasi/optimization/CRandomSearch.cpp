// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/optimization/CRandomSearch.cpp,v $
//   $Revision: 1.35.4.1.4.1 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2009/03/20 16:04:25 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/***************************************************************************
                  CRandomSearch.cpp  -  Random Optimizer
                     -------------------

Programmer           : Rohan Luktuke
email                : rluktuke@vt.edu
 ***************************************************************************/

/***************************************************************************
 * This is the implementation of the Random Algorithm for Optimization.  The
 * class is inherited from the COptAlgorithm class
 ***************************************************************************/

#include "copasi.h"

#include "COptMethod.h"
#include "COptProblem.h"
#include "COptItem.h"
#include "COptTask.h"

#include "CRandomSearch.h"

#include "report/CCopasiObjectReference.h"
#include "randomGenerator/CRandom.h"

CRandomSearch::CRandomSearch():
    COptMethod(CCopasiTask::optimization, CCopasiMethod::RandomSearch)
{
  addParameter("Number of Iterations", CCopasiParameter::UINT, (unsigned C_INT32) 100000);
  addParameter("Random Number Generator", CCopasiParameter::UINT, (unsigned C_INT32) CRandom::mt19937);
  addParameter("Seed", CCopasiParameter::UINT, (unsigned C_INT32) 0);

  initObjects();
}

CRandomSearch::CRandomSearch(const CRandomSearch & src):
    COptMethod(src)
{initObjects();}

/**
 * Destructor
 */
CRandomSearch::~CRandomSearch()
{//*** added similar to coptga
  cleanup();
}

void CRandomSearch::initObjects()
{
  addObjectReference("Current Iteration", mCurrentIteration, CCopasiObject::ValueInt);
}

#ifdef WIN32
// warning C4056: overflow in floating-point constant arithmetic
// warning C4756: overflow in constant arithmetic
# pragma warning (disable: 4056 4756)
#endif

/**
 * Optimizer Function
 * Returns: true if properly initialized
 * should return a boolean
 */
bool CRandomSearch::initialize()
{
  cleanup();

  if (!COptMethod::initialize()) return false;

  mIterations = * getValue("Number of Iterations").pUINT;
  mpRandom = CRandom::createGenerator(* (CRandom::Type *) getValue("Random Number Generator").pUINT,
                                      * getValue("Seed").pUINT);

  mBestValue = 2.0 * DBL_MAX;

  mVariableSize = mpOptItem->size();
  mIndividual.resize(mVariableSize);

  return true;
}

#ifdef WIN32
# pragma warning (default: 4056 4756)
#endif

/**
 * Optimizer Function
 * Returns: nothing
 * should return a boolean
 */
//C_INT32 CRandomSearch::optimize()
bool CRandomSearch::optimise()
{
  bool Continue = true;

  if (!initialize()) return false;

  unsigned C_INT32 j;

  // Initialize the population
  // first individual is the initial guess
  for (j = 0; j < mVariableSize; j++)
    {
      C_FLOAT64 & mut = mIndividual[j];
      COptItem & OptItem = *(*mpOptItem)[j];

      mut = OptItem.getStartValue();

      // force it to be within the bounds
      switch (OptItem.checkConstraint(mut))
        {
          case - 1:
            mut = *OptItem.getLowerBoundValue();
            break;

          case 1:
            mut = *OptItem.getUpperBoundValue();
            break;
        }

      // We need to set the value here so that further checks take
      // account of the value.
      (*(*mpSetCalculateVariable)[j])(mut);
    }

  Continue = evaluate(mIndividual);
  mBestValue = mValue;
  Continue = mpOptProblem->setSolution(mBestValue, mIndividual);

  // We found a new best value lets report it.
  //if (mpReport) mpReport->printBody();
  mpParentTask->output(COutputInterface::DURING);

  for (mCurrentIteration = 0; mCurrentIteration < mIterations && Continue; mCurrentIteration++)
    {
      for (j = 0; j < mVariableSize && Continue; j++)
        {
          // CALCULATE lower and upper bounds
          COptItem & OptItem = *(*mpOptItem)[j];
          C_FLOAT64 & mut = mIndividual[j];

          mut = OptItem.getRandomValue(mpRandom);

          // force it to be within the bounds
          switch (OptItem.checkConstraint(mut))
            {
              case - 1:
                mut = *OptItem.getLowerBoundValue();

                if (!OptItem.checkLowerBound(mut)) // Inequality
                  {
                    if (mut == 0.0)
                      mut = DBL_MIN;
                    else
                      mut += mut * DBL_EPSILON;
                  }

                break;

              case 1:
                mut = *OptItem.getUpperBoundValue();

                if (!OptItem.checkUpperBound(mut)) // Inequality
                  {
                    if (mut == 0.0)
                      mut = - DBL_MIN;
                    else
                      mut -= mut * DBL_EPSILON;
                  }

                break;
            }

          // We need to set the value here so that further checks take
          // account of the value.
          (*(*mpSetCalculateVariable)[j])(mut);
        }

      Continue = evaluate(mIndividual);

      // COMPARE
      if (mValue < mBestValue)
        {
          mBestValue = mValue;
          Continue = mpOptProblem->setSolution(mBestValue, mIndividual);

          // We found a new best value lets report it.
          //if (mpReport) mpReport->printBody();
          mpParentTask->output(COutputInterface::DURING);
        }
    }

  return Continue;
}

// evaluate the fitness of one individual
bool CRandomSearch::evaluate(const CVector< C_FLOAT64 > & /* individual */)
{
  bool Continue = true;

  // We do not need to check whether the parametric constraints are fulfilled
  // since the parameters are created within the bounds.

  // evaluate the fitness
  Continue = mpOptProblem->calculate();

  // check wheter the functional constraints are fulfilled
  if (!mpOptProblem->checkFunctionalConstraints())
    mValue = 2.0 * DBL_MAX;
  else
    mValue = mpOptProblem->getCalculateValue();

  return Continue;
}

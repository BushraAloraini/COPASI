// Copyright (C) 2017 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and University of
// of Connecticut School of Medicine.
// All rights reserved.

// Copyright (C) 2010 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 - 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2002 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#include "copasi.h"

#include "CLsodaMethod.h"
#include "CTrajectoryProblem.h"

#include "CopasiDataModel/CCopasiDataModel.h"
#include "report/CCopasiRootContainer.h"
#include "math/CMathContainer.h"
#include "model/CModel.h"
#include "model/CState.h"

// Uncomment this line below to get numeric debug print out.
// #define DEBUG_NUMERICS 1

// Uncomment this line below to get processing flow output.
// #define DEBUG_FLOW 1

CLsodaMethod::CLsodaMethod(const CCopasiContainer * pParent,
                           const CTaskEnum::Method & methodType,
                           const CTaskEnum::Task & taskType):
  CTrajectoryMethod(pParent, methodType, taskType),
  mpReducedModel(NULL),
  mpRelativeTolerance(NULL),
  mpAbsoluteTolerance(NULL),
  mpMaxInternalSteps(NULL),
  mpMaxInternalStepSize(NULL),
  mData(),
  mpY(NULL),
  mpYdot(NULL),
  mNumRoots(0),
  mTime(),
  mLsodaStatus(1),
  mLastSuccessState(),
  mAtol(),
  mpAtol(NULL),
  mErrorMsg(),
  mLSODA(),
  mLSODAR(),
  mTask(),
  mDWork(),
  mIWork(),
  mJType(),
  mRootMask(),
  mDiscreteRoots(),
  mRootMasking(CLsodaMethod::NONE),
  mTargetTime(),
  mRootCounter(0),
  mPeekAheadMode(false)
{
  assert((void *) &mData == (void *) &mData.dim);

  mData.pMethod = this;
  initializeParameter();
}

CLsodaMethod::CLsodaMethod(const CLsodaMethod & src,
                           const CCopasiContainer * pParent):
  CTrajectoryMethod(src, pParent),
  mpReducedModel(NULL),
  mpRelativeTolerance(NULL),
  mpAbsoluteTolerance(NULL),
  mpMaxInternalSteps(NULL),
  mpMaxInternalStepSize(NULL),
  mData(src.mData),
  mpY(NULL),
  mpYdot(NULL),
  mNumRoots(src.mNumRoots),
  mTime(src.mTime),
  mLsodaStatus(src.mLsodaStatus),
  mLastSuccessState(src.mLastSuccessState),
  mAtol(src.mAtol),
  mpAtol(NULL),
  mErrorMsg(src.mErrorMsg.str()),
  mLSODA(),
  mLSODAR(),
  mTask(src.mTask),
  mDWork(src.mDWork),
  mIWork(src.mIWork),
  mJType(src.mJType),
  mRootMask(src.mRootMask),
  mDiscreteRoots(),
  mRootMasking(src.mRootMasking),
  mTargetTime(src.mTargetTime),
  mRootCounter(src.mRootCounter),
  mPeekAheadMode(src.mPeekAheadMode)
{
  assert((void *) &mData == (void *) &mData.dim);

  mData.pMethod = this;
  initializeParameter();
}

CLsodaMethod::~CLsodaMethod()
{}

void CLsodaMethod::initializeParameter()
{
  CCopasiParameter *pParm;

  mpReducedModel = assertParameter("Integrate Reduced Model", CCopasiParameter::BOOL, (bool) false);
  mpRelativeTolerance = assertParameter("Relative Tolerance", CCopasiParameter::UDOUBLE, (C_FLOAT64) 1.0e-6);
  mpAbsoluteTolerance = assertParameter("Absolute Tolerance", CCopasiParameter::UDOUBLE, (C_FLOAT64) 1.0e-12);
  mpMaxInternalSteps = assertParameter("Max Internal Steps", CCopasiParameter::UINT, (unsigned C_INT32) 10000);
  mpMaxInternalStepSize = assertParameter("Max Internal Step Size", CCopasiParameter::UDOUBLE, (C_FLOAT64) 0.0);

  // Check whether we have a method with the old parameter names
  if ((pParm = getParameter("LSODA.RelativeTolerance")) != NULL)
    {
      *mpRelativeTolerance = pParm->getValue< C_FLOAT64 >();
      removeParameter("LSODA.RelativeTolerance");

      if ((pParm = getParameter("LSODA.AbsoluteTolerance")) != NULL)
        {
          *mpAbsoluteTolerance = pParm->getValue< C_FLOAT64 >();
          removeParameter("LSODA.AbsoluteTolerance");
        }

      if ((pParm = getParameter("LSODA.AdamsMaxOrder")) != NULL)
        {
          removeParameter("LSODA.AdamsMaxOrder");
        }

      if ((pParm = getParameter("LSODA.BDFMaxOrder")) != NULL)
        {
          removeParameter("LSODA.BDFMaxOrder");
        }

      if ((pParm = getParameter("LSODA.MaxStepsInternal")) != NULL)
        {
          *mpMaxInternalSteps = pParm->getValue< unsigned C_INT32 >();
          removeParameter("LSODA.MaxStepsInternal");
        }
    }

  // Check whether we have a method with "Use Default Absolute Tolerance"
  if ((pParm = getParameter("Use Default Absolute Tolerance")) != NULL)
    {
      C_FLOAT64 NewValue;

      if (pParm->getValue< bool >())
        {
          // The default
          NewValue = 1.e-12;
        }
      else
        {
          C_FLOAT64 OldValue = *mpAbsoluteTolerance;
          CCopasiDataModel* pDataModel = getObjectDataModel();
          assert(pDataModel != NULL);
          CModel * pModel = pDataModel->getModel();

          if (pModel == NULL)
            // The default
            NewValue = 1.e-12;
          else
            {
              const CCopasiVectorNS< CCompartment > & Compartment = pModel->getCompartments();
              size_t i, imax;
              C_FLOAT64 Volume = std::numeric_limits< C_FLOAT64 >::max();

              for (i = 0, imax = Compartment.size(); i < imax; i++)
                if (Compartment[i].getValue() < Volume)
                  Volume = Compartment[i].getValue();

              if (Volume == std::numeric_limits< C_FLOAT64 >::max())
                // The default
                NewValue = 1.e-12;
              else
                // Invert the scaling as best as we can
                NewValue = OldValue / (Volume * pModel->getQuantity2NumberFactor());
            }
        }

      *mpAbsoluteTolerance = NewValue;
      removeParameter("Use Default Absolute Tolerance");
    }

  // These parameters are no longer supported.
  removeParameter("Adams Max Order");
  removeParameter("BDF Max Order");
}

bool CLsodaMethod::elevateChildren()
{
  initializeParameter();
  return true;
}

// virtual
void CLsodaMethod::stateChange(const CMath::StateChange & change)
{
  if (change == CMath::FixedEventTarget)
    {
      // The only thing which changed are fixed event targets which do not effect the simulation
      // thus we can continue from the saved state after updating the fixed event targets;
      memcpy(mSavedState.ContainerState.array(), mContainerState.array(), mpContainer->getCountFixedEventTargets() * sizeof(C_FLOAT64));
    }
  else if (change & (CMath::State | CMath::ContinuousSimulation | CMath::EventSimulation))
    {
      // We need to restart the integrator
      mLsodaStatus = 1;

      mTime = *mpContainerStateTime;
      mPeekAheadMode = false;
      mSavedState.Status = FAILURE;

      destroyRootMask();
    }
}

CTrajectoryMethod::Status CLsodaMethod::step(const double & deltaT)
{
  if (mData.dim == 1 && mNumRoots == 0) //just do nothing if there are no variables except time
    {
      mTime += deltaT;
      *mpContainerStateTime = mTime;

      return NORMAL;
    }

  C_FLOAT64 StartTime = mTime;
  C_FLOAT64 EndTime = mTime + deltaT;

  if (mTargetTime != EndTime)
    {
      // We have a new end time and reset the root counter.
      mTargetTime = EndTime;
      mRootCounter = 0;

      if (mTask == 5)
        {
          mDWork[0] = mTargetTime;
        }
    }
  else
    {
      // We are called with the same end time which means a root has previously been
      // found. We increase the root counter and check whether the limit is reached.
      mRootCounter++;

      if (mRootCounter > *mpMaxInternalSteps)
        {
          return FAILURE;
        }
    }

  C_INT ITOL = 2; // mRtol scalar, mAtol vector
  C_INT one = 1;
  C_INT DSize = (C_INT) mDWork.size();
  C_INT ISize = (C_INT) mIWork.size();

  // The return status of the integrator.
  Status Status = NORMAL;

  mLastSuccessState = mContainerState;

  if (mRootsFound.size() > 0)
    {
      if (mSavedState.Status != FAILURE)
        {
          resetState();
        }

      if (mLsodaStatus != 3)
        {
          mLSODAR(&EvalF, //  1. evaluate F
                  &mData.dim, //  2. number of variables
                  mpY, //  3. the array of current concentrations
                  &mTime, //  4. the current time
                  &EndTime, //  5. the final time
                  &ITOL, //  6. error control
                  mpRelativeTolerance, //  7. relative tolerance array
                  mpAtol, //  8. absolute tolerance array
                  &mTask, //  9. output by overshoot & interpolation
                  &mLsodaStatus, // 10. the state control variable
                  &one, // 11. further options (one)
                  mDWork.array(), // 12. the double work array
                  &DSize, // 13. the double work array size
                  mIWork.array(), // 14. the int work array
                  &ISize, // 15. the int work array size
                  &EvalJ, // 16. evaluate J (not given)
                  &mJType, // 17. type of j evaluation 2 internal full matrix
                  &EvalR, // 18. evaluate constraint functions
                  &mNumRoots, // 19. number of constraint functions g(i)
                  mRootsFound.array()); // 20. integer array of length NG for output of root information

          // There exist situations where LSODAR reports status = 3, which are actually status = -33
          // Obviously the trivial case is where LSODAR did not advance at all, i.e, the start time
          // equals the current time. It may also happen that a very small steps has been taken.
          // We reset short before we reach the internal step limit.
#ifdef DEBUG_FLOW
          std::cout << "mTime = " << mTime << ", EndTime = " << EndTime << ", mTask = " << mTask << ", mLsodaStatus = " << mLsodaStatus << ", mRootCounter = " << mRootCounter << std::endl;
          std::cout << "mDWork = " << CVectorCore< C_FLOAT64 >(22, mDWork.array()) << std::endl;
          std::cout << "mIWork = " << CVectorCore< C_INT >(20, mIWork.array()) << std::endl;
#endif // DEBUG_FLOW

          if (mLsodaStatus == 3 &&
              (mRootCounter > 0.99 * *mpMaxInternalSteps ||
               mTime == StartTime))
            {
              mLsodaStatus = -33;
              mRootCounter = 0;
            }

          // Try without over shooting.
          if ((mLsodaStatus <= 0 && mLsodaStatus != -33) ||
              !mpContainer->isStateValid())
            {
              if (mTask == 4 || mTask == 5)
                {
#ifdef DEBUG_FLOW
                  std::cout << "FAILURE: Attempt to prevent over shooting failed." << std::endl;
#endif // DEBUG_FLOW

                  Status = FAILURE;
                  mPeekAheadMode = false;

                  if (mLsodaStatus <= 0)
                    {
                      CCopasiMessage(CCopasiMessage::EXCEPTION, MCTrajectoryMethod + 6, mErrorMsg.str().c_str());
                    }
                  else
                    {
                      CCopasiMessage(CCopasiMessage::EXCEPTION, MCTrajectoryMethod + 25, mTime);
                    }
                }

              // We try to recover by preventing overshooting.
#ifdef DEBUG_NUMERICS
              std::cout << "State: " << mpContainer->getState(*mpReducedModel) << std::endl;
#endif // DEBUG_NUMERICS

              mContainerState = mLastSuccessState;

#ifdef DEBUG_NUMERICS
              std::cout << "State: " << mpContainer->getState(*mpReducedModel) << std::endl;
#endif // DEBUG_NUMERICS

              mTime = *mpContainerStateTime;
              mTask += 3;
              mDWork[0] = EndTime;
              stateChange(CMath::State);

#ifdef DEBUG_FLOW
              std::cout << "Attempting to prevent over shooting." << std::endl;
#endif // DEBUG_FLOW

              Status = step(deltaT);

              mTask -= 3;

              return Status;
            }
        }

      switch (mLsodaStatus)
        {
          case -33:

            switch (mRootMasking)
              {
                case NONE:
                case DISCRETE:
                  // Reset the integrator to the state before the failed integration.

                  mContainerState = mLastSuccessState;
                  mTime = *mpContainerStateTime;
                  mLsodaStatus = 1;

                  // Create a mask which hides all roots being constant and zero.
#ifdef DEBUG_FLOW
                  std::cout << "Creating Root Mask." << std::endl;
#endif // DEBUG_FLOW
                  createRootMask();

                  return step(deltaT);

                  break;

                case ALL:
                  break;
              }

            break;

          case 3:

            // If mLsodaStatus == 3 we have found a root. This needs to be indicated to
            // the caller as it is not sufficient to rely on the fact that T < TOUT
            if (mLsodaStatus == 3)
              {
                // It is sufficient to switch to 2. Eventual state changes due to events
                // are indicated via the method stateChanged()
                mLsodaStatus = 2;
                Status = ROOT;
              }

            // To detect simultaneous roots we have to peek ahead, i.e., continue
            // integration until the state changes are larger than the relative tolerances
            if (!mPeekAheadMode)
              {
#ifdef DEBUG_FLOW
                std::cout << "Starting peek ahead." << std::endl;
#endif // DEBUG_FLOW

                Status = peekAhead();

#ifdef DEBUG_FLOW
                std::cout << "Finishing peek ahead." << std::endl;
#endif // DEBUG_FLOW
              }

            // The break statement is intentionally missing since we
            // have to continue to check the root masking state.
          default:

            switch (mRootMasking)
              {
                case NONE:
                case DISCRETE:
                  break;

                case ALL:
                {
                  const bool * pDiscrete = mDiscreteRoots.array();
                  bool * pMask = mRootMask.array();
                  bool * pMaskEnd = pMask + mNumRoots;
                  bool Destroy = true;

                  for (; pMask != pMaskEnd; ++pMask, ++pDiscrete)
                    {
                      if (*pMask)
                        {
                          if (*pDiscrete)
                            {
                              Destroy = false;
                            }
                          else
                            {
                              *pMask = false;
                            }
                        }
                    }

                  if (Destroy)
                    {
#ifdef DEBUG_FLOW
                      std::cout << "Destroy Root Mask." << std::endl;
#endif // DEBUG_FLOW

                      destroyRootMask();
                    }
                  else
                    {
#ifdef DEBUG_FLOW
                      std::cout << "Root Mask Discrete." << std::endl;
#endif // DEBUG_FLOW

                      mRootMasking = DISCRETE;
                    }

                  // We have to restart the integrator
                  mLsodaStatus = 1;
                }

                break;
              }

            break;
        }
    }
  else
    {
      mLSODA(&EvalF, //  1. evaluate F
             &mData.dim, //  2. number of variables
             mpY, //  3. the array of current concentrations
             &mTime, //  4. the current time
             &EndTime, //  5. the final time
             &ITOL, //  6. error control
             mpRelativeTolerance, //  7. relative tolerance array
             mpAtol, //  8. absolute tolerance array
             &mTask, //  9. output by overshoot & interpolation
             &mLsodaStatus, // 10. the state control variable
             &one, // 11. further options (one)
             mDWork.array(), // 12. the double work array
             &DSize, // 13. the double work array size
             mIWork.array(), // 14. the int work array
             &ISize, // 15. the int work array size
             EvalJ, // 16. evaluate J (not given)
             &mJType);        // 17. the type of jacobian calculate (2)

      if (mLsodaStatus <= 0 ||
          !mpContainer->isStateValid())
        {
          if (mTask == 4 || mTask == 5)
            {
              Status = FAILURE;
              mPeekAheadMode = false;

              if (mLsodaStatus <= 0)
                {
                  CCopasiMessage(CCopasiMessage::EXCEPTION, MCTrajectoryMethod + 6, mErrorMsg.str().c_str());
                }
              else
                {
                  CCopasiMessage(CCopasiMessage::EXCEPTION, MCTrajectoryMethod + 25, mTime);
                }
            }

          // We try to recover by preventing overshooting.
#ifdef DEBUG_NUMERICS
          std::cout << "State: " << mpContainer->getState(*mpReducedModel) << std::endl;
#endif // DEBUG_NUMERICS

          mContainerState = mLastSuccessState;
#ifdef DEBUG_NUMERICS
          std::cout << "State: " << mpContainer->getState(*mpReducedModel) << std::endl;
#endif // DEBUG_NUMERICS

          mTime = *mpContainerStateTime;
          mTask += 3;
          mDWork[0] = EndTime;
          stateChange(CMath::State);

          Status = step(deltaT);
          mTask -= 3;

          return Status;
        }
    }

  *mpContainerStateTime = mTime;

  return Status;
}

void CLsodaMethod::start()
{
  CTrajectoryMethod::start();

  /* Reset lsoda */
  mLsodaStatus = 1;

  mTask = mpProblem == NULL ? 1 : mpProblem->getAutomaticStepSize() ? 5 : 1;
  mJType = 2;
  mErrorMsg.str("");

  mTime = *mpContainerStateTime;

  mTargetTime = mTime;
  mRootCounter = 0;
  mPeekAheadMode = false;

  mNumRoots = (C_INT) mpContainer->getRoots().size();
  mRootsFound.resize(mNumRoots);
  destroyRootMask();

  mAtol = mpContainer->initializeAtolVector(*mpAbsoluteTolerance, *mpReducedModel);

  // We ignore fixed event targets and start with the time
  mData.dim = (C_INT)(mContainerState.size() - mpContainer->getCountFixedEventTargets());
  mpY = mpContainerStateTime;
  mpYdot = mpContainer->getRate(*mpReducedModel).array() + mpContainer->getCountFixedEventTargets();
  mpAtol = mAtol.array() + mpContainer->getCountFixedEventTargets();

  /* Configure lsoda(r) */
  mDWork.resize(22 + mData.dim * std::max<C_INT>(16, mData.dim + 9) + 3 * mNumRoots);
  mDWork[4] = mDWork[6] = mDWork[7] = mDWork[8] = mDWork[9] = 0.0;

  mDWork[5] = *mpMaxInternalStepSize;

  mIWork.resize(20 + mData.dim);
  mIWork[4] = mIWork[6] = mIWork[9] = 0;

  mIWork[5] = *mpMaxInternalSteps;
  mIWork[7] = 12;
  mIWork[8] = 5;

  if (mNumRoots > 0)
    {
      mLSODAR.setOstream(mErrorMsg);
      mDiscreteRoots.initialize(mpContainer->getRootIsDiscrete());

      saveState();
    }
  else
    {
      mLSODA.setOstream(mErrorMsg);
    }

  return;
}

void CLsodaMethod::EvalF(const C_INT * n, const C_FLOAT64 * t, const C_FLOAT64 * y, C_FLOAT64 * ydot)
{static_cast<Data *>((void *) n)->pMethod->evalF(t, y, ydot);}

void CLsodaMethod::evalF(const C_FLOAT64 * t , const C_FLOAT64 * /* y */, C_FLOAT64 * ydot)
{
  *mpContainerStateTime = *t;

  mpContainer->updateSimulatedValues(*mpReducedModel);
  memcpy(ydot, mpYdot, mData.dim * sizeof(C_FLOAT64));

#ifdef DEBUG_NUMERICS
  std::cout << "State:     " << mpContainer->getState(false) << std::endl;
  std::cout << "Rate:      " << mpContainer->getRate(false) << std::endl;
#endif // DEBUG_NUMERICS

  return;
}

void CLsodaMethod::EvalR(const C_INT * n, const C_FLOAT64 * t, const C_FLOAT64 * y,
                         const C_INT * nr, C_FLOAT64 * r)
{static_cast<Data *>((void *) n)->pMethod->evalR(t, y, nr, r);}

void CLsodaMethod::evalR(const C_FLOAT64 * t, const C_FLOAT64 *  /* y */,
                         const C_INT *  nr, C_FLOAT64 * r)
{
  *mpContainerStateTime = *t;
  mpContainer->updateSimulatedValues(*mpReducedModel);

  CVectorCore< C_FLOAT64 > RootValues(*nr, r);
  RootValues = mpContainer->getRoots();

#ifdef DEBUG_NUMERICS
  std::cout << "State: " << mpContainer->getState(*mpReducedModel) << std::endl;
  std::cout << "Roots: " << RootValues << std::endl;
#endif // DEBUG_OUTPUT

  if (mRootMasking != NONE)
    {
      maskRoots(RootValues);
    }

#ifdef DEBUG_NUMERICS
  std::cout << "Roots: " << RootValues << std::endl;
#endif // DEBUG_NUMERICS
};

// static
void CLsodaMethod::EvalJ(const C_INT * n, const C_FLOAT64 * t, const C_FLOAT64 * y,
                         const C_INT * ml, const C_INT * mu, C_FLOAT64 * pd, const C_INT * nRowPD)
{static_cast<Data *>((void *) n)->pMethod->evalJ(t, y, ml, mu, pd, nRowPD);}

// virtual
void CLsodaMethod::evalJ(const C_FLOAT64 * t, const C_FLOAT64 * y,
                         const C_INT * ml, const C_INT * mu, C_FLOAT64 * pd, const C_INT * nRowPD)
{
  // TODO Implement me.
}

void CLsodaMethod::maskRoots(CVectorCore< C_FLOAT64 > & rootValues)
{
  const bool *pMask = mRootMask.array();
  const bool *pMaskEnd = pMask + mRootMask.size();
  C_FLOAT64 * pRoot = rootValues.array();

  for (; pMask != pMaskEnd; ++pMask, ++pRoot)
    {
      if (*pMask)
        {
          *pRoot = 1.0;
        }
    }
}

void CLsodaMethod::createRootMask()
{
  size_t NumRoots = mRootsFound.size();
  mRootMask.resize(NumRoots);
  CVector< C_FLOAT64 > RootValues;
  RootValues.resize(NumRoots);
  CVector< C_FLOAT64 > RootDerivatives;
  RootDerivatives.resize(NumRoots);

  mpContainer->updateSimulatedValues(*mpReducedModel);
  RootValues = mpContainer->getRoots();
  mpContainer->calculateRootDerivatives(RootDerivatives);

  bool *pMask = mRootMask.array();
  bool *pMaskEnd = pMask + mRootMask.size();
  C_FLOAT64 * pRootValue = RootValues.array();
  C_FLOAT64 * pRootDerivative = RootDerivatives.array();

  for (; pMask != pMaskEnd; ++pMask, ++pRootValue, ++pRootDerivative)
    {
      *pMask = (fabs(*pRootDerivative) < *mpAbsoluteTolerance ||
                fabs(*pRootValue) < 1e3 * std::numeric_limits< C_FLOAT64 >::min()) ? true : false;
    }

  mRootMasking = ALL;
}

void CLsodaMethod::destroyRootMask()
{
  mRootMask.resize(0);
  mRootMasking = NONE;
}

CTrajectoryMethod::Status CLsodaMethod::peekAhead()
{
  // Save the current state
  CVector< C_FLOAT64 > StartState = mContainerState;

  mPeekAheadMode = true;
  Status PeekAheadStatus = ROOT;

  CVector< C_FLOAT64 > CurrentRoots = mpContainer->getRoots();
#ifdef DEBUG_NUMERICS
  std::cout << "Current Roots:        " << mpContainer->getRoots() << std::endl;
#endif // DEBUG_NUMERICS

  CVector< C_INT > CombinedRootsFound = mRootsFound;
#ifdef DEBUG_OUTPUT
  std::cout << "Combined Roots found: " << CombinedRootsFound << std::endl;
#endif // DEBUG_NUMERICS

  C_FLOAT64 MaxPeekAheadTime = std::max(mTargetTime, mTime * (1.0 + 2.0 * *mpRelativeTolerance));

  while (mPeekAheadMode)
    {
      switch (step(MaxPeekAheadTime - mTime))
        {
          case NORMAL:

            if (mRootMasking != ALL)
              {
                // If we are here we are certain that the state has sufficiently changed.
                mPeekAheadMode = false;

                // It is possible that LSODA does not detect a root if the original root
                // value was very close to 0.0. We have a new root if we have a sign change.
                // This must have been at the start of the integration.
                C_INT * pCombinedRoot = CombinedRootsFound.array();
                C_INT * pCombinedRootEnd = pCombinedRoot + CombinedRootsFound.size();
                const C_FLOAT64 * pOldRoot = CurrentRoots.array();
                const C_FLOAT64 * pNewRoot = mpContainer->getRoots().array();

                for (; pCombinedRoot != pCombinedRootEnd; ++pCombinedRoot, ++pOldRoot, ++pNewRoot)
                  {
                    *pCombinedRoot |= (*pOldRoot **pNewRoot < 0.0);
                  }

                // Save the state for future use.
                saveState();
                mSavedState.Status = NORMAL;

                // Set the result to the state when peakAhead was entered.
                mContainerState = StartState;
                mTime = *mpContainerStateTime;

#ifdef DEBUG_NUMERICS
                std::cout << "Current Roots:        " << mpContainer->getRoots() << std::endl;
                std::cout << "Combined Roots found: " << CombinedRootsFound << std::endl;
#endif // DEBUG_OUTPUT
              }
            else
              {
                // If we are here LSODA returned with status -33, i.e.
                // time did not advanced due to to indistinguishable roots

                // Root masking has been enabled in step and the integrator is ready
                // to continue with the previous state.

                // Remove masked roots from the result set
                const bool *pMask = mRootMask.array();
                const bool *pMaskEnd = pMask + mRootMask.size();
                C_INT * pCombinedRoot = CombinedRootsFound.array();

                PeekAheadStatus = NORMAL;

                for (; pMask != pMaskEnd; ++pMask, ++pCombinedRoot)
                  {
                    if (*pMask)
                      {
                        *pCombinedRoot = 0;
                      }

                    if (*pCombinedRoot)
                      {
                        PeekAheadStatus = ROOT;
                      }
                  }
              }

            break;

          case ROOT:
          {
            // Check whether the new state is within the tolerances
            C_FLOAT64 * pOld = StartState.array();
            C_FLOAT64 * pOldEnd = pOld + StartState.size();
            C_FLOAT64 * pNew = mContainerState.array();
            C_FLOAT64 * pAtol = mAtol.array();

            for (; pOld != pOldEnd; ++pOld, ++pNew, ++pAtol)
              {
                if ((2.0 * fabs(*pNew - *pOld) > fabs(*pNew + *pOld) **mpRelativeTolerance) &&
                    fabs(*pNew) > *pAtol &&
                    fabs(*pOld) > *pAtol)
                  {
                    break;
                  }
              }

            if (pOld != pOldEnd)
              {
                // If we are here we are certain that the state has sufficiently changed.
                mPeekAheadMode = false;

                // Save the state for future use.
                saveState();
                mSavedState.Status = ROOT;

                // Set the result to the state when peakAhead was entered.
                mContainerState = StartState;
                mTime = *mpContainerStateTime;
              }
            else
              {
                // We found more roots within the desired tolerance
                // Combine all the roots
                C_INT * pRoot = mRootsFound.array();
                C_INT * pRootEnd = pRoot + mRootsFound.size();
                C_INT * pCombinedRoot = CombinedRootsFound.array();

                for (; pRoot != pRootEnd; ++pRoot, ++pCombinedRoot)
                  {
                    if (*pRoot > 0)
                      {
                        *pCombinedRoot = 1;
                      }
                  }

#ifdef DEBUG_NUMERICS
                std::cout << "Combined Roots found: " << CombinedRootsFound << std::endl;
#endif // DEBUG_NUMERICS
              }
          }
          break;

          case FAILURE:
            PeekAheadStatus = FAILURE;
            mPeekAheadMode = false;
            break;
        }
    }

  mRootsFound = CombinedRootsFound;

  return PeekAheadStatus;
}

void CLsodaMethod::saveState()
{
  *mpContainerStateTime = mTime;
  mSavedState.ContainerState = mContainerState;
  mSavedState.DWork = mDWork;
  mSavedState.IWork = mIWork;
  mSavedState.RootsFound = mRootsFound;
  mSavedState.Status = FAILURE;

  mLSODAR.saveState();
}

void CLsodaMethod::resetState()
{
  if (mSavedState.Status == ROOT)
    {
      mLsodaStatus = 3;
    }

  mContainerState = mSavedState.ContainerState;
  mTime = *mpContainerStateTime;
  mDWork = mSavedState.DWork;
  mIWork = mSavedState.IWork;
  mRootsFound = mSavedState.RootsFound;
  mSavedState.Status = FAILURE;

  mLSODAR.resetState();
}

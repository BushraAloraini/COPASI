// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/model/CMathModel.cpp,v $
//   $Revision: 1.3 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2009/05/21 15:34:38 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

#include "copasi.h"

#include "CMathModel.h"
#include "CModel.h"

CMathModel::CMathModel(const CCopasiContainer * pParent) :
    CCopasiContainer("MathModel", pParent, "CMathModel"),
    mpModel(NULL),
    mProcessQueue(),
    mEvents("ListOfMathEvents", this),
    mRootValues(),
    mRootRefreshes(),
    mRootIndex2Event()
{}

CMathModel::CMathModel(const CMathModel & src,
                       const CCopasiContainer * pParent) :
    CCopasiContainer(src, pParent),
    mpModel(src.mpModel),
    mProcessQueue(src.mProcessQueue),
    mEvents("ListOfMathEvents", this),
    mRootValues(),
    mRootRefreshes(),
    mRootIndex2Event()
{
  // Compile the math model.
  compile(mpModel);

  // TODO
  // The problem is that the pointers to events in the process queue point
  // to events in the source model. We can construct a mapping of source event pointers
  // to event pointers and update each process queue entry.

  // For the moment we enforce that the process queue is empty.
  assert(mProcessQueue.isEmpty());
}

// virtual
CMathModel::~CMathModel()
{}

bool CMathModel::compile(CModel * pModel)
{
  // For each CEvent we need to create a CMathEvent
  mEvents.clear();

  // We clear the sequence of root refreshes to avoid accessing invalid pointers.
  mRootRefreshes.clear();

  // We clear the map to avoid accessing no longer existing events.
  mRootIndex2Event.resize(0);

  mpModel = pModel;

  if (mpModel == NULL)
    return false;

  // Now we are ready to start the compilations.
  bool success = true;

  // The context in which this model is compiled.
  // TODO When the math model is the only thing required for simulation this should be changed
  // to this.
  std::vector< CCopasiContainer * > Context(1);
  Context[0] = mpModel;

  CCopasiVectorN< CEvent >::const_iterator itEvent = mpModel->getEvents().begin();
  CCopasiVectorN< CEvent >::const_iterator endEvent = mpModel->getEvents().end();

  size_t RootFinderCount = 0;

  for (; itEvent != endEvent; ++itEvent)
    {
      CMathEvent * pEvent = new CMathEvent();
      mEvents.add(pEvent, true);
      success &= pEvent->compile(*itEvent, Context);

      RootFinderCount += pEvent->getMathTrigger().getRootFinders().size();
    }

  // We need to create a CVector of pointers to the current root finder values
  mRootValues.resize(RootFinderCount);
  C_FLOAT64 ** ppRootValue = mRootValues.array();

  // We need create a map of root finder indexes to events.
  mRootIndex2Event.resize(RootFinderCount);
  CMathEvent ** ppRootIndex = mRootIndex2Event.array();

  std::set< const CCopasiObject * > RootValuesDependencies;

  CCopasiVector< CMathEvent >::const_iterator itMathEvent = mEvents.begin();
  CCopasiVector< CMathEvent >::const_iterator endMathEvent = mEvents.end();

  // for each event
  for (; itMathEvent != endMathEvent; ++itMathEvent)
    {
      CCopasiVector< CMathTrigger::CRootFinder >::const_iterator itRootFinder =
        (*itMathEvent)->getMathTrigger().getRootFinders().begin();
      CCopasiVector< CMathTrigger::CRootFinder >::const_iterator endRootFinder =
        (*itMathEvent)->getMathTrigger().getRootFinders().end();

      // for each root finder
      for (; itRootFinder != endRootFinder; ++itRootFinder, ++ppRootValue, ++ppRootIndex)
        {
          // Update the vector of pointers to current root values
          *ppRootValue = (*itRootFinder)->getRootValuePtr();

          // Build the mapping from root values indexes to CMathEvents
          *ppRootIndex = *itMathEvent;

          // Get the list of direct dependencies which we need to compile
          // the refresh sequence
          std::set< const CCopasiObject * > Dependencies =
            (*itRootFinder)->getDirectDependencies();

          std::set< const CCopasiObject * >::const_iterator itDependency =
            Dependencies.begin();
          std::set< const CCopasiObject * >::const_iterator endDependency =
            Dependencies.end();

          for (; itDependency != endDependency; ++itDependency)
            {
              RootValuesDependencies.insert(*itDependency);
            }
        }
    }

  // We need to compile a refresh sequence which calculates all root values.
  mRootRefreshes = buildRequiredRefreshList(RootValuesDependencies);

  return success;
}

bool CMathModel::initialize()
{
  bool success = true;

  success &= mProcessQueue.initialize(this);

  // TODO CRITICAL When do we initialize the roots, i.e., calculate their initial state.
  // This should be done when we apply the initial state.
  return false;
}

void CMathModel::evaluateRoots(CVector< double > & rootValues)
{
  // Apply all needed refresh calls to calculate the current root values.
  std::vector< Refresh * >::const_iterator itRefresh = mRootRefreshes.begin();
  std::vector< Refresh * >::const_iterator endRefresh = mRootRefreshes.end();

  while (itRefresh != endRefresh)
    (**itRefresh++)();

  // Copy the current values to the output vector.
  assert(rootValues.size() == mRootValues.size());

  C_FLOAT64 * pTarget = rootValues.array();
  C_FLOAT64 **pSrc = mRootValues.array();
  C_FLOAT64 **pSrcEnd = pSrc + mRootValues.size();

  for (; pSrc != pSrcEnd; ++pSrc, ++pTarget)
    {
      *pTarget = **pSrc;
    }

  return;
}

void CMathModel::processQueue(const C_FLOAT64 & time, const bool & equality)
{
  mProcessQueue.process(time, equality);
  return;
}

void CMathModel::processRoots(const CVector< C_INT > & roots)
{
  assert(roots.size() == mRootIndex2Event.size());

  // All events associated with the found roots need to be evaluated whether they fire.
  // In case one fires the corresponding event needs to be scheduled in the process queue.

  const C_INT *pRoot = roots.array();
  const C_INT *pRootEnd = pRoot + roots.size();

  CMathEvent ** ppEvent = mRootIndex2Event.array();
  CMathEvent * pProcessedEvent = NULL;

  // We go through the list of roots and process the events
  // which need to be checked whether they fire.
  for (; pRoot != pRootEnd; ++pRoot, ++ppEvent)
    {
      // Process the events for which we have found a root.
      // A found root is indicated by roots[i] = 1 or 0 otherwise.
      // We need to process each event at most once.
      if (*pRoot > 0 && *ppEvent != pProcessedEvent)
        {
          pProcessedEvent = *ppEvent;
          pProcessedEvent->processRoot();
        }
    }

  return;
}

const C_FLOAT64 & CMathModel::getProcessQueueExecutionTime() const
{
  return mProcessQueue.getProcessQueueExecutionTime();
}

std::vector< Refresh * > CMathModel::buildRequiredRefreshList(const std::set< const CCopasiObject * > & requiredObjects) const
{
  // We assume that all values determined by ODEs and reactions are current.
  // This means for reduced models moieties must have been applied.
  std::set< const CCopasiObject * > UpToDate;

  return CCopasiObject::buildUpdateSequence(requiredObjects, UpToDate);
}

std::vector< Refresh * > CMathModel::buildDependendRefreshList(const std::set< const CCopasiObject * > & changedObjects) const
{
  if (mpModel == NULL)
    return std::vector< Refresh * >();

  // We cannot use the default dependencies for a species which concentration was changed
  // by an event as the concentration is the dependent value. This must not be done for
  // species which where not changed by the event, i.e., the solution must be context specific.
  // This implies that the methods getDirectDependencies and its callers must be aware of the species
  // concentrations in the list of the changed objects.

  // CCopasiObject::mDependencies is private this means the only place where can be accessed is
  // CCopasiObject itself. We need to replace all read access the mDirectDependencies with
  // getDirectDependencies in CCopasiObject and than make this method context sensitive. This new
  // method will be virtual and must be overloaded for the concentration and particle number in species.

  // We build a list of all simulated objects which depend on the changed values.
  std::set< const CCopasiObject * > RequiredObjects;

  // The values which need to be updated are all values needed for simulation.
  // These are all objects which are required to calculate the reaction rates and
  // the right hand side of ODEs in other words all rates of entities of type reaction
  // or ODE.

  // Due to the context sensitivity, i.e., the changed concentration is no longer dependent
  // on the particle number and compartment volume, we will miss all particle numbers of changed
  // species which do not directly appear in any calculation.

  // First ODEs and species particle numbers
  CModelEntity *const* ppEntity = mpModel->getStateTemplate().getEntities();
  CModelEntity *const* ppEndEntity = ppEntity + mpModel->getStateTemplate().size();

  const CMetab * pSpecies;

  for (; ppEntity != ppEndEntity; ++ppEntity)
    {
      switch ((*ppEntity)->getStatus())
        {
          case CModelEntity::ODE:
          case CModelEntity::FIXED:

            if ((*ppEntity)->getRateReference()->dependsOn(changedObjects, changedObjects))
              {
                RequiredObjects.insert((*ppEntity)->getRateReference());
              }

            pSpecies = dynamic_cast< const CMetab * >(*ppEntity);

            // The break statement is intentionally missing since we need to check
            // the particle values.
          case CModelEntity::REACTIONS:
            pSpecies = dynamic_cast< const CMetab * >(*ppEntity);

            if (pSpecies != NULL &&
                pSpecies->getValueReference()->dependsOn(changedObjects, changedObjects))
              {
                RequiredObjects.insert(pSpecies->getValueReference());
              }

            break;

          case CModelEntity::ASSIGNMENT:
            // Assignments may have to be recalculated but only if an ODE or a
            // reaction flux depends on it.
            break;

          case CModelEntity::TIME:
            // The model time will not advance during an event. For discrete time
            // simulation the users should add a global quantity representing the time.
            break;
        }
    }

  // Since we have discontinuous change we have to evaluate the total amount of the moieties
  // which participant changed concentration.

  CCopasiVector< CMoiety >::const_iterator itMoiety = mpModel->getMoieties().begin();
  CCopasiVector< CMoiety >::const_iterator endMoiety = mpModel->getMoieties().end();

  for (; itMoiety != endMoiety; ++itMoiety)
    {
      if ((*itMoiety)->getValueReference()->dependsOn(changedObjects, changedObjects))
        {
          RequiredObjects.insert((*itMoiety)->getValueReference());
        }
    }

  std::set< const CCopasiObject * > UpToDate;
  return CCopasiObject::buildUpdateSequence(RequiredObjects, UpToDate, changedObjects);
}

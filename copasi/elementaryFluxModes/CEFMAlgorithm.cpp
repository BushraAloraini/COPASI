/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/elementaryFluxModes/CEFMAlgorithm.cpp,v $
   $Revision: 1.16 $
   $Name:  $
   $Author: shoops $
   $Date: 2006/09/12 13:21:04 $
   End CVS Header */

// Copyright � 2005 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/**
 *  CEFMAlgorithm class.
 *  Used to calculate elementary flux modes
 *
 *  Created for Copasi by Stefan Hoops 2002-05-08
 * (C) Stefan Hoops 2002
 */

#include "copasi.h"

#include "CEFMAlgorithm.h"
#include "CEFMProblem.h"
#include "CEFMTask.h"
#include "CFluxMode.h"
#include "CTableauMatrix.h"

#include "model/CModel.h"
#include "utilities/CProcessReport.h"
#include "report/CCopasiObjectReference.h"

CEFMAlgorithm::CEFMAlgorithm(const CCopasiContainer * pParent):
    CEFMMethod(CCopasiTask::fluxMode, CCopasiMethod::EFMAlgorithm, pParent),
    mpModel(NULL),
    mStoi(0, 0),
    mReversible(0),
    mpCurrentTableau(NULL),
    mpNextTableau(NULL)
{initObjects();}

CEFMAlgorithm::CEFMAlgorithm(const CCopasiMethod::SubType subType, const CCopasiContainer * pParent):
    CEFMMethod(CCopasiTask::fluxMode, subType, pParent),
    mpModel(NULL),
    mStoi(0, 0),
    mReversible(0),
    mpCurrentTableau(NULL),
    mpNextTableau(NULL)
{initObjects();}

CEFMAlgorithm::CEFMAlgorithm(const CEFMAlgorithm & src,
                             const CCopasiContainer * pParent):
    CEFMMethod(src, pParent),
    mpModel(NULL),
    mStoi(0, 0),
    mReversible(0),
    mpCurrentTableau(NULL),
    mpNextTableau(NULL)
{initObjects();}

CEFMAlgorithm::~CEFMAlgorithm()
{
  DESTRUCTOR_TRACE;
  pdelete(mpCurrentTableau);
  pdelete(mpNextTableau);
}

void CEFMAlgorithm::initObjects()
{
  addObjectReference("Current Step", mStep, CCopasiObject::ValueInt);
}

bool CEFMAlgorithm::initialize()
{
  CEFMTask * pTask = dynamic_cast< CEFMTask *>(getObjectParent());
  if (pTask == NULL) return false;

  mpModel = pTask->getProblem()->getModel();
  if (mpModel == NULL) return false;

  mFluxModes.clear();

  /* ModelStoi is the transpose of the models stoichiometry matrix */
  const CTransposeView< CMatrix< C_FLOAT64 > > ModelStoi(mpModel->getStoi());

  unsigned C_INT32 row, numRows = ModelStoi.numRows();
  unsigned C_INT32 col, numCols = ModelStoi.numCols();

  /* Size the stoichiometry matrix passed to the algorithm */
  mStoi.resize(numRows);
  std::vector< std::vector< C_FLOAT64 > >::iterator it = mStoi.begin();
  std::vector< std::vector< C_FLOAT64 > >::iterator end = mStoi.end();
  for (; it != end; ++it)
    it->resize(numCols);

  /* Get the reactions from the model */
  /* Note: We have as many reactions as we have rows in ModelStoi */
  const CCopasiVectorNS < CReaction > & Reaction = mpModel->getReactions();

  /* Vector to keep track of the rearangements neccessary to put the */
  /* reversible reactions to the top of Stoi */
  mIndex.resize(numRows);

  /* Reversible reaction counter */
  mReversible = 0;
  unsigned C_INT32 Insert;
  unsigned C_INT32 InsertReversible = 0;
  unsigned C_INT32 InsertIrreversible = numRows - 1;

  /* Build the transpose of the stoichiometry matrix, */
  /* sort reversible reactions to the top, count them */
  /* and keep track of the rearrangement */
  for (row = 0; row < numRows; row++)
    {
      if (Reaction[row]->isReversible())
        {
          Insert = InsertReversible++;
          mReversible++;
        }
      else
        Insert = InsertIrreversible--;

      mIndex[Insert] = row;

      for (col = 0; col < numCols; col++)
        mStoi[Insert][col] = ModelStoi(row, col);
    }

  mStep = 0;
  mMaxStep = numCols;

  if (mpCallBack)
    mhSteps =
      mpCallBack->addItem("Current Step",
                          CCopasiParameter::UINT,
                          & mStep,
                          & mMaxStep);

  return true;
}

bool CEFMAlgorithm::calculate()
{
  bool Continue = true;

  if (!initialize()) return false;

  if (mStoi.size())
    {
      /* initialize the current tableu matrix */
      mpCurrentTableau = new CTableauMatrix(mStoi, mReversible);

      /* Do the iteration */

      for (mStep = 0; mStep < mMaxStep && Continue; mStep++)
        {
          calculateNextTableau();

          if (mpCallBack)
            Continue &= mpCallBack->progress(mhSteps);
        }

      /* Build the elementary flux modes to be returned */
      buildFluxModes(mFluxModes);

      /* Delete the current / final tableu matrix */
      pdelete(mpCurrentTableau);
    }

  return true;
}

void CEFMAlgorithm::calculateNextTableau()
{
  std::list< const CTableauLine * >::iterator a;
  std::list< const CTableauLine * >::iterator b;
  C_FLOAT64 ma, mb;

  mpNextTableau = new CTableauMatrix();

  /* Move all lines with zeros in the step column to the new tableu matrix */
  /* and remove them from the current tableau matrix */
  a = mpCurrentTableau->getFirst();

  while (a != mpCurrentTableau->getEnd())
    if ((*a)->getReaction(0) == 0.0)
      {
        /* We have to make sure that "a" points to the next element in the */
        /* list after the removal of itself */

        if (a == mpCurrentTableau->getFirst())
          {
            mpNextTableau->addLine(*a);
            mpCurrentTableau->removeLine(a);
            a = mpCurrentTableau->getFirst();
          }
        else
          {
            /* We have to remember the previous element so that a++ points to */
            /* past the removed one */
            b = a--;
            mpNextTableau->addLine(*b);
            mpCurrentTableau->removeLine(b);
            a++;
          }
      }
    else
      a++;

  /* Now we create all linear combinations of the remaining lines in the */
  /* current tableau */
  a = mpCurrentTableau->getFirst();

  while (a != mpCurrentTableau->getEnd())
    {
      b = a;
      b++;

      while (b != mpCurrentTableau->getEnd())
        {
          mb = (*a)->getReaction(0);

          /* We make sure that "mb" is positive */

          if (mb < 0.0)
            {
              mb *= -1;
              ma = (*b)->getReaction(0);
            }
          else
            ma = - (*b)->getReaction(0);

          /* The multiplier "ma" for irreversible reactions must be positive */
          if ((*a)->isReversible() || ma > 0.0)
            mpNextTableau->addLine(new CTableauLine(ma, **a, mb, **b));

          b++;
        }

      a++;
    }

  /* Assigne the next tableau to the current tableau and cleanup */
  pdelete(mpCurrentTableau);

  mpCurrentTableau = mpNextTableau;

  mpNextTableau = NULL;
}

void CEFMAlgorithm::buildFluxModes(std::vector < CFluxMode > & fluxModes)
{
  fluxModes.clear();

  std::list< const CTableauLine * >::iterator a = mpCurrentTableau->getFirst();
  std::list< const CTableauLine * >::iterator end = mpCurrentTableau->getEnd();

  while (a != end)
    {
      fluxModes.push_back(CFluxMode(*a));
      a++;
    }
}

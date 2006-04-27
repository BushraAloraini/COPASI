/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/trajectory/CStochNextReactionMethod.cpp,v $
   $Revision: 1.9 $
   $Name:  $
   $Author: shoops $
   $Date: 2006/04/27 01:32:17 $
   End CVS Header */

// Copyright � 2005 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#include "copasi.h"
#include "CStochNextReactionMethod.h"
#include "CTrajectoryMethod.h"
#include "model/CCompartment.h"
#include "model/CModel.h"

CStochNextReactionMethod::CStochNextReactionMethod()
    : CStochMethod()
{}

void CStochNextReactionMethod::initMethod(C_FLOAT64 start_time)
{
  setupPriorityQueue(start_time);
}

C_FLOAT64 CStochNextReactionMethod::doSingleStep(C_FLOAT64 C_UNUSED(time), C_FLOAT64 endTime)
{
  C_FLOAT64 steptime = mPQ.topKey();

  if (steptime >= endTime)
    {
      return endTime;
    }
  else
    {
      C_INT32 reaction_index = mPQ.topIndex();
      updateSystemState(reaction_index);
      updatePriorityQueue(reaction_index, steptime);
      //printDebugInfo();
      return steptime;
    }
}

//void CStochNextReactionMethod::printDebugInfo()
//{
//  cout << mPQ << endl;
//}

void CStochNextReactionMethod::setupPriorityQueue(C_FLOAT64 start_time)
{
  C_FLOAT64 time;

  mPQ.clear();
  for (unsigned C_INT32 i = 0; i < mpModel->getReactions().size(); i++)
    {
      time = start_time + generateReactionTime(i);
      mPQ.pushPair(i, time);
    }

  mPQ.buildHeap();
}

void CStochNextReactionMethod::updatePriorityQueue(C_INT32 reaction_index, C_FLOAT64 time)
{
  const std::set<C_INT32> & dep_nodes = mDG.getDependents(reaction_index);

  C_FLOAT64 new_time = time + generateReactionTime(reaction_index);

  mPQ.updateNode(reaction_index, new_time);

  std::set<C_INT32>::const_iterator di;

  for (di = dep_nodes.begin(); di != dep_nodes.end(); di++)
    {
      if (*di != reaction_index)
        {
          C_INT32 index = *di;
          C_FLOAT64 new_time;

          if (mAmuOld[index] > 0)
            {
              new_time = time + (mAmuOld[index] / mAmu[index]) * (mPQ.getKey(index) - time);
            }
          else
            {
              new_time = time + generateReactionTime(index);
            }

          mPQ.updateNode(index, new_time);
        }
    }
}

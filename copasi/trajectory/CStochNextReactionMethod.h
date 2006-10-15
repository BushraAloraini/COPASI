/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/trajectory/CStochNextReactionMethod.h,v $
   $Revision: 1.8 $
   $Name:  $
   $Author: gauges $
   $Date: 2006/10/15 08:31:13 $
   End CVS Header */

// Copyright � 2005 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#ifndef COPASI_CStochNextReactionMethod
#define COPASI_CStochNextReactionMethod

#include "trajectory/CStochMethod.h"
#include "utilities/CIndexedPriorityQueue.h"

class CStochNextReactionMethod: public CStochMethod
  {
    friend CStochMethod *
    CStochMethod::createStochMethod(CTrajectoryProblem * pProblem);

  private:
    // Private attributes

    /**
     * The set of putative reactions and associated times at which each reaction occurs.
     * This is represented as a priority queue, indexed on the reaction time.
     */
    CIndexedPriorityQueue mPQ;

    /**
     * Initialization.
     */
    void initMethod(C_FLOAT64 start_time);

  public:
    /**
     * Default constructor
     */
    CStochNextReactionMethod();

    /**
     * Do one iteration of the simulation
     * @return Current simulation time or -1 if error.
     */
    C_FLOAT64 doSingleStep(C_FLOAT64 time, C_FLOAT64 endTime);

    /**
     * print internal information (for debugging)
     */
    //void printDebugInfo();

  private:
    /**
     * Set up the priority queue.
     * @param start_time The time at which the simulation starts.
     */
    void setupPriorityQueue(C_FLOAT64 start_time = 0);
    /**
     * Update the priority queue
     */
    void updatePriorityQueue(C_INT32 reaction_index, C_FLOAT64 time);
  };

#endif

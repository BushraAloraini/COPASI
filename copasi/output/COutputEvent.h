/*****************************************************************************
 * PROGRAM NAME: COutputEvent.h
 * PROGRAMMER: Wei Sun wsun@vt.edu
 * PURPOSE: Handle Output Event 
 *****************************************************************************/

#ifndef COPASI_COutputEvent
#define COPASI_COutputEvent

#include <iostream>

class COutput;
class COutputList;
class CSteadyStateTask;

class COutputEvent
  {
  private:
    int mTime;

  public:
    /**
     * Default constructor. 
     */
    COutputEvent();

    /**
     * User defined constructor. 
     * @param time refers to the output interval
     * time = 0, header
     * time = 1, any time of simulation exceution
     * time = 2, the final result
     */
    COutputEvent(int time);

    /**
     * User defined constructor. 
     * @param time refers to the output interval
     */ 
    // COutputEvent(CSS_Solution &ss);
    COutputEvent(CSteadyStateTask &ss);

    /**
     * Deconstructor
     */
    ~COutputEvent();

    /**
     * Clean Up
     */
    void cleanup();

    /**
     * Copy output at the specified time 
     * @param tra refers to the CTrajectory object.
     */
    void print(const COutputList & list, std::ostream &fout) const;

    /**
     * Copy output at the specified time 
     * @param tra refers to the CSS_solution object.
     */
    void print(const CSteadyStateTask &ss, const COutputList & list, std::ostream &fout) const;
  };

#endif // COutputEvent

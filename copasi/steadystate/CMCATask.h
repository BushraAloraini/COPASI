/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/steadystate/CMCATask.h,v $
   $Revision: 1.5 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2005/12/15 16:57:10 $
   End CVS Header */

/**
 * CMCATask class.
 *
 * This class implements a metabolic control analysis task which is comprised of a
 * of a problem and a method. Additionally calls to the reporting 
 * methods are done when initialized.
 *  
 * Created for Copasi by Ralph Gauges 2004
 */

#ifndef COPASI_CMCATask_H__
#define COPASI_CMCATask_H__

#include <iostream>

#include "utilities/CCopasiTask.h"
#include "utilities/CMatrix.h"
#include "utilities/CReadConfig.h"
#include "CSteadyStateMethod.h"

class CMCAProblem;
class CState;

class CMCATask : public CCopasiTask
  {
    //Attributes
  private:
    /**
     * A pointer to the found elasticities
     */

    /**
     * A pointer to the found flux control coefficients.
     */

    /**
     * A pointer to the found concentration control coefficients.
     */

    //Operations
  public:

    /**
     * Default constructor
     * @param const CCopasiContainer * pParent (default: NULL)
     */
    CMCATask(const CCopasiContainer * pParent = & RootContainer);

    /**
     * Copy constructor
     * @param const CMCATask & src
     * @param const CCopasiContainer * pParent (default: NULL)
     */
    CMCATask(const CMCATask & src,
             const CCopasiContainer * pParent = NULL);

    /**
     * Destructor
     */
    virtual ~CMCATask();

    /**
     * Initialize the task. If an ostream is given this ostream is used
     * instead of the target specified in the report. This allows nested 
     * tasks to share the same output device.
     * @param const OutputFlag & of
     * @param std::ostream * pOstream (default: NULL)
     * @return bool success
     */
    virtual bool initialize(const OutputFlag & of, std::ostream * pOstream);

    /**
     * Process the task with or without initializing to the initial state.
     * @param const bool & useInitialValues
     * @return bool success
     */
    virtual bool process(const bool & useInitialValues);

    /**
     * Perform neccessary cleaup procedures
     */
    virtual bool restore();

    /**
     * Loads parameters for this solver with data coming from a
     * CReadConfig object. (CReadConfig object reads an input stream)
     * @param configbuffer reference to a CReadConfig object.
     */
    void load(CReadConfig & configBuffer);

    // Friend functions
    friend std::ostream &operator<<(std::ostream &os,
                                    const CMCATask &A);

  private:
    /**
     * cleanup()
     */
    void cleanup();
  };

#endif // COPASI_CMCATask_H__

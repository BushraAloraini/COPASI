/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/scan/CScanProblem.h,v $
   $Revision: 1.25 $
   $Name:  $
   $Author: ssahle $ 
   $Date: 2005/08/03 22:36:49 $
   End CVS Header */

/**
 *  CScanProblem class.
 */

#ifndef COPASI_CScanProblem
#define COPASI_CScanProblem

#include <string>

#include "utilities/CCopasiProblem.h"
#include "utilities/CReadConfig.h" 
//#include "model/CState.h"

//class CModel;

class CScanProblem : public CCopasiProblem
  {
  public:
    enum Type
    {
      SCAN_REPEAT = 0,
      SCAN_LINEAR,
      SCAN_RANDOM,
      SCAN_BREAK,
      SD_UNIFORM,
      SD_GAUSS,
      SD_BOLTZ,
      SD_REGULAR
    };

  private:

    /**
     *  The initial state, i.e., the starting conditions of the trajectroy/SS.
     */ 
    //CState mInitialState;

    /**
     *  The initial state, i.e., the starting conditions of the trajectroy/SS.
     */
    CCopasiParameterGroup * mpScanItems;

  public:

    /**
     * Default constructor.
     * @param const CCopasiContainer * pParent (default: NULL)
     */
    CScanProblem(const CCopasiContainer * pParent = NULL);

    /**
     * Copy constructor.
     * @param const CTrajectoryProblem & src
     * @paramconst CCopasiContainer * pParent (default: NULL)
     */
    CScanProblem(const CScanProblem & src,
                 const CCopasiContainer * pParent = NULL);

    /**
     *  Destructor.
     */
    ~CScanProblem();

    /**
     * Set the moddel the problem is dealing with.
     * @param "CModel *" pModel
     */ 
    //virtual bool setModel(CModel * pModel);

    /**
     *  Set the type of the subtask.
     */
    void setSubtask(CCopasiTask::Type type);

    /**
     *  Get the type of the subtask.
     */
    CCopasiTask::Type getSubtask() const;

    /**
     *  Set if output should be done after every step of the subtask.
     */
    void setOutputInSubtask(bool ois);

    /**
     *  Ask if output should be done after every step of the subtask.
     */
    const bool & getOutputInSubtask() const;

    /**
     *  Set if initial conditions should be set to the result of the previous run.
     */
    void setAdjustInitialConditions(bool aic);

    /**
     *  Ask if initial conditions should be set to the result of the previous run.
     */
    const bool & getAdjustInitialConditions() const;

    /**
     * Set the initial state of the problem.
     * @param "const CState &" initialState
     */ 
    //void setInitialState(const CState & initialState);

    /**
     * Set the initial state of the problem.
     * @param "const CStateX &" initialState
     */ 
    //void setInitialState(const CStateX & initialState);

    /**
     * Retrieve the initial state of the problem.
     * @return "const CState &" pInitialState
     */ 
    //const CState & getInitialState() const;

    unsigned C_INT32 getNumberOfScanItems() const;

    const CCopasiParameterGroup* getScanItem(unsigned C_INT32 index) const;
    CCopasiParameterGroup* getScanItem(unsigned C_INT32 index);

    CScanProblem::Type getScanItemType(unsigned C_INT32 index);

    CCopasiParameterGroup* createScanItem(CScanProblem::Type type, unsigned C_INT32 steps = 5, const CCopasiObject* obj = NULL);

    void clearScanItems();

    //void createDebugScan(CModel* model);

    /**
     * Load a scan problem
     * @param "CReadConfig &" configBuffer
     * @param "CReadConfig::Mode mode (Default: CReadConfig::NEXT)
     */
    void load(CReadConfig & configBuffer,
              CReadConfig::Mode mode = CReadConfig::NEXT);
  };

#endif

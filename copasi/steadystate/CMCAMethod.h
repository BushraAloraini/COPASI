/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/steadystate/CMCAMethod.h,v $
   $Revision: 1.14 $
   $Name:  $
   $Author: ssahle $ 
   $Date: 2005/11/08 16:18:26 $
   End CVS Header */

#ifndef COPASI_CMCAMethod_H__
#define COPASI_CMCAMethod_H__

#include <vector>
#include "utilities/CMatrix.h"
#include "utilities/CCopasiMethod.h"
#include "utilities/CAnnotatedMatrix.h"
#include "CSteadyStateMethod.h"

#define MCA_OK 0
#define MCA_SINGULAR 1

class CModel;

class CMCAMethod: public CCopasiMethod
  {
  private:

    CModel * mpModel;

    /**
     * MCA Matrices
     */
    CAnnotatedMatrixOld mUnscaledElasticities;

    CAnnotatedMatrixOld mUnscaledConcCC;

    CAnnotatedMatrixOld mUnscaledFluxCC;

    CAnnotatedMatrixOld mScaledElasticities;

    CAnnotatedMatrixOld mScaledConcCC;

    CAnnotatedMatrixOld mScaledFluxCC;

    /**
     * 1 if MCA coeffs are to be unscaled
     */
    C_INT16 mSSReder;

    /**
     * If need to evaluate the elasticites, ss_x
     * will be assigned in calculateTimeMCA()
     */
    std::vector <C_FLOAT64> mSsx;

    /**
     * Modulation factor for finite differences derivation
     */
    C_FLOAT64 mFactor;

    C_FLOAT64 mSteadyStateResolution;

    CSteadyStateMethod::ReturnCode mSSStatus;

  public:
    /**
     * Default constructor
     */
    CMCAMethod(const CCopasiContainer* pParent);

    /**
     * User defined constructor
     * @param refer to Model and factor
     */ 
    //    CMCAMethod(CModel & model, C_FLOAT64 factor, const CCopasiContainer* pParent);

    /**
     * Deconstructor
     */

    virtual ~CMCAMethod();

    const CAnnotatedMatrixOld & getUnscaledElasticities() const
      {return mUnscaledElasticities;}

    void calculateUnscaledElasticities(C_FLOAT64 res);

    const CAnnotatedMatrixOld & getUnscaledConcentrationCC() const
      {return mUnscaledConcCC;}

    int calculateUnscaledConcentrationCC();

    const CAnnotatedMatrixOld & getUnscaledFluxCC() const
      {return mUnscaledFluxCC;}

    void calculateUnscaledFluxCC(int condition);

    const CAnnotatedMatrixOld & getScaledElasticities() const
      {return mScaledElasticities;}

    const CAnnotatedMatrixOld & getScaledConcentrationCC() const
      {return mScaledConcCC;}

    const CAnnotatedMatrixOld & getScaledFluxCC() const
      {return mScaledFluxCC;}

    /**
     * Scales the coefficients (i.e. Kacser format, rather than Reder)
     */
    void scaleMCA(int condition, C_FLOAT64 res);

    /**
     * Set the Model
     */
    void setModel(CModel* model);

    /**
     * Get the Model
     */
    const CModel* getModel() const;

    /**
     * the steady state MCA entry point
     * @param ss_solution refer to steady-state solution
     * @param refer to the resolution
     */
    int CalculateMCA(CSteadyStateMethod::ReturnCode status, C_FLOAT64 res);

    /**
     * 
     */
    virtual bool process();

    /**
     * Read SSMCAUnscaled from configuration file
     */
    C_INT32 load(CReadConfig & configBuffer);

    void setSteadyStateStatus(CSteadyStateMethod::ReturnCode SSStatus);

    const CSteadyStateMethod::ReturnCode & getSteadyStateStatus() const
      {return mSSStatus;}

    void setFactor(C_FLOAT64 factor);

    void setSteadyStateResolution(C_FLOAT64 factor);

    /**
     * Check if the method is suitable for this problem
     * @return bool suitability of the method
     */
    virtual bool isValidProblem(const CCopasiProblem * pProblem);
  };
#endif // COPASI_CMca

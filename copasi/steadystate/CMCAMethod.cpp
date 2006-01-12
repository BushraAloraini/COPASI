/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/steadystate/CMCAMethod.cpp,v $
   $Revision: 1.27.2.4 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2006/01/12 16:50:22 $
   End CVS Header */

#include <cmath>
#include <limits.h>
#include <float.h>

#include "copasi.h"
#include "utilities/CReadConfig.h"
#include "utilities/utility.h"
#include "model/CModel.h"
#include "utilities/CCopasiTask.h"
#include "CMCAMethod.h"
#include "CMCAProblem.h"

//TODO: put all matrix resizing and annotations creation in one place, so
// that it has to be done only once if several MCA are calculated (e.g. in a scan)

/**
 * Default constructor
 */
CMCAMethod::CMCAMethod(const CCopasiContainer* pParent):
    CCopasiMethod(CCopasiTask::mca, CCopasiMethod::mcaMethodReder, pParent),
    mpModel(NULL)
{
  CONSTRUCTOR_TRACE;
  addParameter("MCA.ModulationFactor",
               CCopasiParameter::UDOUBLE, 1.0e-009);
  mFactor = 1.0e-9;
  mSSStatus = CSteadyStateMethod::notFound;
  mSteadyStateResolution = mFactor;

  initObjects();
}

void CMCAMethod::initObjects()
{
  CArrayAnnotation *
  tmp = new CArrayAnnotation("Unscaled elasticities", this,
                             new CCopasiMatrixInterface<CMatrix<C_FLOAT64> >(&mUnscaledElasticities));
  tmp->setOnTheFly(false);
  tmp->setDescription("Unscaled elasticity matrix");
  tmp->setDimensionDescription(0, "Reactions (reduced system)");
  tmp->setDimensionDescription(1, "Metabolites (reduced system)");
  mUnscaledElasticitiesAnn = tmp;

  tmp = new CArrayAnnotation("Unscaled concentration control coefficients", this,
                             new CCopasiMatrixInterface<CMatrix<C_FLOAT64> >(&mUnscaledConcCC));
  tmp->setOnTheFly(false);
  tmp->setDescription("Unscaled concentration control coefficients");
  tmp->setDimensionDescription(0, "Metabolites (reduced system)");
  tmp->setDimensionDescription(1, "Reactions (reduced system)");
  mUnscaledConcCCAnn = tmp;

  tmp = new CArrayAnnotation("Unscaled flux control coefficients", this,
                             new CCopasiMatrixInterface<CMatrix<C_FLOAT64> >(&mUnscaledFluxCC));
  tmp->setOnTheFly(false);
  tmp->setDescription("Unscaled flux control coefficients");
  tmp->setDimensionDescription(0, "Reactions (reduced system)");
  tmp->setDimensionDescription(1, "Reactions (reduced system)");
  mUnscaledFluxCCAnn = tmp;

  tmp = new CArrayAnnotation("Scaled elasticities", this,
                             new CCopasiMatrixInterface<CMatrix<C_FLOAT64> >(&mScaledElasticities));
  tmp->setOnTheFly(false);
  tmp->setDescription("Scaled elasticity matrix");
  tmp->setDimensionDescription(0, "Reactions (reduced system)");
  tmp->setDimensionDescription(1, "Metabolites (reduced system)");
  mScaledElasticitiesAnn = tmp;

  tmp = new CArrayAnnotation("Scaled concentration control coefficients", this,
                             new CCopasiMatrixInterface<CMatrix<C_FLOAT64> >(&mScaledConcCC));
  tmp->setOnTheFly(false);
  tmp->setDescription("Scaled concentration control coefficients");
  tmp->setDimensionDescription(0, "Metabolites (reduced system)");
  tmp->setDimensionDescription(1, "Reactions (reduced system)");
  mScaledConcCCAnn = tmp;

  tmp = new CArrayAnnotation("Scaled flux control coefficients", this,
                             new CCopasiMatrixInterface<CMatrix<C_FLOAT64> >(&mScaledFluxCC));
  tmp->setOnTheFly(false);
  tmp->setDescription("Scaled flux control coefficients");
  tmp->setDimensionDescription(0, "Reactions (reduced system)");
  tmp->setDimensionDescription(1, "Reactions (reduced system)");
  mScaledFluxCCAnn = tmp;
}

/**
 * Deconstructor
 */
CMCAMethod::~CMCAMethod()
{
  DESTRUCTOR_TRACE;
  //delSsipvt();
}

//that caclulates the elasticities as d(particle flux)/d(particle number)
//which is the same as d(flux of substance)/d(amount of substance)
void CMCAMethod::calculateUnscaledElasticities(C_FLOAT64 res)
{
  assert(mpModel);

  CCopasiVector<CMetab> & metabs = mpModel->getMetabolitesX();
  CCopasiVector<CReaction> & reacs = mpModel->getReactions();

  //mUnscaledElasticities.setup(reacs, metabs);
  unsigned C_INT32 numReacs = reacs.size();
  unsigned C_INT32 numMetabs = metabs.size();
  mUnscaledElasticities.resize(numReacs, numMetabs);

  //update annotated matrix
  mUnscaledElasticitiesAnn->resize();
  mUnscaledElasticitiesAnn->createAnnotationsFromCopasiVector(0, &reacs);
  mUnscaledElasticitiesAnn->createAnnotationsFromCopasiVector(1, &metabs);

  unsigned C_INT32 i, j;
  C_FLOAT64 store, temp;
  C_FLOAT64 K1, K2, K3;

  // constants for differentiation by finite differences
  K1 = 1 + mFactor;
  K2 = 1 - mFactor;
  K3 = 2 * mFactor;

  // Arrays to store function value
  std::vector<C_FLOAT64> f1; f1.resize(numReacs);
  std::vector<C_FLOAT64> f2; f2.resize(numReacs);

  // calculate elasticities
  for (j = 0; j < numMetabs; j++)
    {
      /**
       * if src[i+1] (x_ss[i+1]) is zero, the derivative will be calculated at a small
       * positive value (no point in considering negative values!).
       * let's stick with res*K1 (SSRes)
       */
      store = metabs[j]->getNumber();

      if (store < res)
        temp = res * K1;
      else
        temp = store;

      // let's take X_dx
      metabs[j]->setNumber(temp * K1);

      // Calculate the fluxes
      for (i = 0; i < numReacs; i++)
        f1[i] = reacs[i]->calculateParticleFlux();

      // now X-dx
      metabs[j]->setNumber(temp * K2);

      // calculate the fluxes
      for (i = 0; i < numReacs; i++)
        f2[i] = reacs[i]->calculateParticleFlux();

      // set column j of Dxv
      for (i = 0; i < numReacs; i++)
        mUnscaledElasticities[i][j] = (f1[i] - f2[i]) / (temp * K3); //TODO optimize

      // restore the value of (src[i])ss_x[i]
      metabs[j]->setNumber(store);
    }

  // make shure the fluxes are correct afterwords (needed for scaling of the MCA results)
  for (i = 0; i < numReacs; i++)
  {reacs[i]->calculateParticleFlux();}

  //std::cout << "elasticities" << std::endl;
  //std::cout << (CMatrix<C_FLOAT64>)mUnscaledElasticities << std::endl;
}

int CMCAMethod::calculateUnscaledConcentrationCC()
{
  assert(mpModel);

  unsigned C_INT32 i, j, k;
  unsigned C_INT32 dim;
  C_INT32 info;
  C_FLOAT64 **aux1, **aux2;

  // Create auxiliary matrices as big as needed
  if (mpModel->getNumVariableMetabs() > mpModel->getTotSteps())
    dim = mpModel->getNumVariableMetabs() + 1;
  else
    dim = mpModel->getTotSteps() + 1;

  // Create aux1 and aux2
  aux1 = (C_FLOAT64 **) malloc((dim) * sizeof(*aux1));
  aux2 = (C_FLOAT64 **) malloc((dim) * sizeof(*aux2));
  for (i = 0; i < dim; i++)
    {
      aux1[i] = (C_FLOAT64 *) malloc((dim) * sizeof(C_FLOAT64));
      aux2[i] = (C_FLOAT64 *) malloc((dim) * sizeof(C_FLOAT64));
    }

  // aux1 = rstoi * mDxv
  //CMatrix<C_FLOAT64> aux1; aux1.resize(mpModel->getNumIndependentMetabs(), mpModel->getIntMetab());
  for (i = 0; i < mpModel->getNumIndependentMetabs(); i++)
    for (j = 0; j < mpModel->getNumVariableMetabs(); j++)
      {
        aux1[i][j] = 0.0;

        for (k = 0; k < mpModel->getTotSteps(); k++)
          aux1[i][j] += (C_FLOAT64) mpModel->getRedStoi()[i][k] * mUnscaledElasticities[k][j];
      }

  //debug
  /*std::cout << "aux1 = redStoi * unscaledElasticities" << std::endl;
  for (i = 0; i < mpModel->getNumIndependentMetabs(); i++)
    {
      for (j = 0; j < mpModel->getNumVariableMetabs(); j++)
        std::cout << "  " << aux1[i][j];
      std::cout << std::endl;
    }
  std::cout << std::endl;*/

  // aux2 = aux1 * m1 (shifting indices for dgefa)
  //CMatrix<C_FLOAT64> aux2; aux2.resize(mpModel->getNumIndependentMetabs()+1, mpModel->getNumIndependentMetabs()+1);
  for (i = 0; i < mpModel->getNumIndependentMetabs(); i++)
    for (j = 0; j < mpModel->getNumIndependentMetabs(); j++)
      {
        aux2[i + 1][j + 1] = 0.0;
        for (k = 0; k < mpModel->getNumVariableMetabs(); k++)
          aux2[i + 1][j + 1] += aux1[i][k] * mpModel->getL()(k, j); //???
      }

  //debug
  //std::cout << "aux2 = aux1 * L,  equals reduced Jacobian" << std::endl;

#ifdef COPASI_DEBUG
  for (i = 0; i < mpModel->getNumIndependentMetabs(); i++)
    {
      for (j = 0; j < mpModel->getNumIndependentMetabs(); j++)
        std::cout << "  " << aux2[i + 1][j + 1];
      std::cout << std::endl;
    }
  //std::cout << std::endl;
#endif // COPASI_DEBUG

  // LU decomposition of aux2 (for inversion)
  // dgefa -> luX??
  C_INT32 * ssipvt;
  ssipvt = (C_INT32 *) malloc((mpModel->getNumIndependentMetabs() + 1) * sizeof(C_INT32));
  dgefa(aux2, mpModel->getNumIndependentMetabs(), ssipvt, &info);

  if (info != 0)
    {
      // matrix is singular
      // return now (mGamma[i][j] = 0)
      // delete matrices
      for (i = 0; i < dim; i++)
        {
          free((void *) aux1[i]);
          free((void *) aux2[i]);
        }
      free((void *) aux1);
      free((void *) aux2);
      pfree(ssipvt);

      return MCA_SINGULAR;
    }

  // set aux1 to the identity matrix (for inversion with dgesl)
  for (i = 0; i < mpModel->getNumIndependentMetabs(); i++)
    for (j = 0; j < mpModel->getNumIndependentMetabs(); j++)
      aux1[i + 1][j + 1] = (i == j) ? 1.0 : 0.0;

  // now invert aux2 (result in aux1)
  for (i = 0; i < mpModel->getNumIndependentMetabs(); i++)
    dgesl(aux2, mpModel->getNumIndependentMetabs(), ssipvt, aux1[i + 1], 1);

  //debug
  /*std::cout << "aux1 = inv(aux2)" << std::endl;
  for (i = 0; i < mpModel->getNumIndependentMetabs(); i++)
    {
      for (j = 0; j < mpModel->getNumIndependentMetabs(); j++)
        std::cout << "  " << aux1[i + 1][j + 1];
      std::cout << std::endl;
    }
  std::cout << std::endl;*/

  // aux2 = - ml * aux1 (shifting indeces back to 0 again)
  for (i = 0; i < mpModel->getNumVariableMetabs(); i++)
    for (j = 0; j < mpModel->getNumIndependentMetabs(); j++)
      {
        aux2[i][j] = 0.0;
        for (k = 0; k < mpModel->getNumIndependentMetabs(); k++)
          aux2[i][j] -= (C_FLOAT64)mpModel->getL()(i, k) * aux1[k + 1][j + 1];
      }

  //debug
  /*std::cout << "aux2 = -L*aux1" << std::endl;
  for (i = 0; i < mpModel->getNumVariableMetabs(); i++)
    {
      for (j = 0; j < mpModel->getNumIndependentMetabs(); j++)
        std::cout << "  " << aux2[i][j];
      std::cout << std::endl;
    }
  std::cout << std::endl;*/

  // mGamma = aux2 *RedStoi
  mUnscaledConcCC.resize(mpModel->getNumVariableMetabs(), mpModel->getTotSteps());
  for (i = 0; i < mpModel->getNumVariableMetabs(); i++)
    for (j = 0; j < mpModel->getTotSteps(); j++)
      {
        mUnscaledConcCC[i][j] = 0;
        for (k = 0; k < mpModel->getNumIndependentMetabs(); k++)
          mUnscaledConcCC[i][j] += aux2[i][k] * (C_FLOAT64) mpModel->getRedStoi()[k][j];
      }

  // delete matrices
  for (i = 0; i < dim; i++)
    {
      free((void *) aux1[i]);
      free((void *) aux2[i]);
    }
  free((void *) aux1);
  free((void *) aux2);
  pfree(ssipvt);

  //std::cout << "ConcCC  (= aux2*RedStoi = -L * redJac^-1 * redStoi)" << std::endl;
  //std::cout << (CMatrix<C_FLOAT64>)mUnscaledConcCC << std::endl;

  //update annotations
  mUnscaledConcCCAnn->resize();
  mUnscaledConcCCAnn->createAnnotationsFromCopasiVector(0, &mpModel->getMetabolitesX());
  mUnscaledConcCCAnn->createAnnotationsFromCopasiVector(1, &mpModel->getReactions());

  return MCA_OK;
}

void CMCAMethod::calculateUnscaledFluxCC(int condition)
{
  assert(mpModel);
  unsigned C_INT32 i, j, k;

  mUnscaledFluxCC.resize(mpModel->getTotSteps(), mpModel->getTotSteps());

  if (condition == MCA_SINGULAR)
    {
      for (i = 0; i < mUnscaledFluxCC.numRows(); i++)
        for (j = 0; j < mUnscaledFluxCC.numCols(); j++)
          mUnscaledFluxCC[i][j] = 0.0;
    }
  else
    {
      // unscaledFluxCC = I + unscaledElasticities * unscaledConcCC
      for (i = 0; i < mUnscaledFluxCC.numRows(); i++)
        for (j = 0; j < mUnscaledFluxCC.numCols(); j++)
          {
            mUnscaledFluxCC[i][j] = (i == j) ? 1.0 : 0.0;

            for (k = 0; k < mUnscaledElasticities.numCols(); k++)
              mUnscaledFluxCC[i][j] += mUnscaledElasticities[i][k] * mUnscaledConcCC[k][j];
          }
    }

  //update annotations
  mUnscaledFluxCCAnn->resize();
  mUnscaledFluxCCAnn->createAnnotationsFromCopasiVector(0, &mpModel->getReactions());
  mUnscaledFluxCCAnn->createAnnotationsFromCopasiVector(1, &mpModel->getReactions());
}

void CMCAMethod::scaleMCA(int condition, C_FLOAT64 res)
{
  assert(mpModel);
  // if previous calcutations failed return now
  if (condition != MCA_OK)
    return;

  unsigned C_INT32 i, j;

  // Scale Elasticities
  mScaledElasticities.resize(mUnscaledElasticities.numRows(), mUnscaledElasticities.numCols());
  for (i = 0; i < mpModel->getTotSteps(); i++)
    for (j = 0; j < mpModel->getNumMetabs(); j++)
      {
        // change the use of Col[] and Row[] to mSteps and mMetabolites
        // change the use of ICol[] and IRow[] to mStepsX and mMetabolitesX

        if (fabs(mpModel->getReactions()[i]->getFlux()
                 *mpModel->getMetabolitesX()[j]->getCompartment()->getVolumeInv()) >= res)
          {
            mScaledElasticities[i][j] = mUnscaledElasticities[i][j]
                                        * mpModel->getMetabolitesX()[j]->getNumber()
                                        / mpModel->getReactions()[i]->getParticleFlux();
            //                                        * mpModel->getMetabolites()[j]->getConcentration()
            //                                        * mpModel->getMetabolites()[j]->getCompartment()->getVolume()
            //                                        / mpModel->getReactions()[i]->getFlux();
          }
        else
          mScaledElasticities[i][j] = DBL_MAX;
      }

  //update annotated matrix
  mScaledElasticitiesAnn->resize();
  mScaledElasticitiesAnn->createAnnotationsFromCopasiVector(0, &mpModel->getReactions());
  mScaledElasticitiesAnn->createAnnotationsFromCopasiVector(1, &mpModel->getMetabolitesX());

  //std::cout << "scElas " << std::endl;
  //std::cout << (CMatrix<C_FLOAT64>)mScaledElasticities << std::endl;
  if (mSSStatus != CSteadyStateMethod::found) return;

  // Scale ConcCC
  mScaledConcCC.resize(mUnscaledConcCC.numRows(), mUnscaledConcCC.numCols());
  for (i = 0; i < mpModel->getNumVariableMetabs(); i++)
    for (j = 0; j < mpModel->getTotSteps(); j++)
      {
        if (fabs(mpModel->getMetabolitesX()[i]->getConcentration()) >= res)
          mScaledConcCC[i][j] = mUnscaledConcCC[i][j]
                                * mpModel->getReactions()[j]->getParticleFlux()
                                / mpModel->getMetabolitesX()[i]->getNumber();
        //                                * mpModel->getReactions()[j]->getFlux()
        //                                / (mpModel->getMetabolites()[i]->getConcentration()
        //                                   *mpModel->getMetabolites()[j]->getCompartment()->getVolume());
        else
          mScaledConcCC[i][j] = DBL_MAX;
      }
  //std::cout << "scConcCC " << std::endl;
  //std::cout << (CMatrix<C_FLOAT64>)mScaledConcCC << std::endl;

  //update annotations
  mScaledConcCCAnn->resize();
  mScaledConcCCAnn->createAnnotationsFromCopasiVector(0, &mpModel->getMetabolitesX());
  mScaledConcCCAnn->createAnnotationsFromCopasiVector(1, &mpModel->getReactions());

  // Scale FluxCC
  mScaledFluxCC.resize(mUnscaledFluxCC.numRows(), mUnscaledFluxCC.numCols());
  for (i = 0; i < mpModel->getTotSteps(); i++)
    for (j = 0; j < mpModel->getTotSteps(); j++)
      {
        C_FLOAT64 tmp = mpModel->getReactions()[i]->getLargestCompartment().getVolumeInv();

        if (fabs(mpModel->getReactions()[i]->getFlux()*tmp) >= res)
          mScaledFluxCC[i][j] = mUnscaledFluxCC[i][j]
                                * mpModel->getReactions()[j]->getFlux()
                                / mpModel->getReactions()[i]->getFlux();
        else
          mScaledFluxCC[i][j] = DBL_MAX;
      }
  //std::cout << "scFluxCC " << std::endl;
  //std::cout << (CMatrix<C_FLOAT64>)mScaledFluxCC << std::endl;

  //update annotations
  mScaledFluxCCAnn->resize();
  mScaledFluxCCAnn->createAnnotationsFromCopasiVector(0, &mpModel->getReactions());
  mScaledFluxCCAnn->createAnnotationsFromCopasiVector(1, &mpModel->getReactions());
}

/**
 * Set the Model
 */
void CMCAMethod::setModel(CModel* model)
{
  mpModel = model;
}

/**
 * the steady state MCA entry point
 * @param ss_solution refer to steady-state solution
 * @param refer to the resolution
 */
int CMCAMethod::CalculateMCA(CSteadyStateMethod::ReturnCode C_UNUSED(status), C_FLOAT64 res)
{
  assert(mpModel);
  int ret = MCA_OK;

  calculateUnscaledElasticities(res);

  if (mSSStatus == CSteadyStateMethod::found)
    {
      ret = calculateUnscaledConcentrationCC();
      calculateUnscaledFluxCC(ret);
    }

  scaleMCA(ret, res);

  return ret;
}

/**
 * Read SSMCAUnscaled from configuration file
 */
C_INT32 CMCAMethod::load(CReadConfig & configBuffer)
{
  C_INT32 Fail = 0;

  if ((Fail = configBuffer.getVariable("SSMCAUnscaled", "C_INT16",
                                       (void *) & mSSReder,
                                       CReadConfig::LOOP)))
    return Fail;

  return Fail;
}

bool CMCAMethod::process()
{
  // check if current state is a steady state
  // if not, calculate TimeMCA only
  if (1 /*mIsSteadyState*/)
    {
      CalculateMCA(mSSStatus, mSteadyStateResolution);
    }
  else
    {
      //   CalculateTimeMCA(mSteadyStateResolution);
    }
  return true;
}

/*bool CMCAMethod::isSteadyState() const
  {
    return this->mIsSteadyState;
  }*/

void CMCAMethod::setSteadyStateStatus(CSteadyStateMethod::ReturnCode SSStatus)
{
  mSSStatus = SSStatus;
}

void CMCAMethod::setFactor(C_FLOAT64 factor)
{
  this->mFactor = factor;
}

void CMCAMethod::setSteadyStateResolution(C_FLOAT64 resolution)
{
  this->mSteadyStateResolution = resolution;
}

const CModel* CMCAMethod::getModel() const
  {
    return this->mpModel;
  }

//virtual
bool CMCAMethod::isValidProblem(const CCopasiProblem * pProblem)
{
  if (!CCopasiMethod::isValidProblem(pProblem)) return false;

  const CMCAProblem * pP = dynamic_cast<const CMCAProblem *>(pProblem);
  if (!pP)
    {
      //not a TrajectoryProblem
      CCopasiMessage(CCopasiMessage::EXCEPTION, "Problem is not an MCA problem.");
      return false;
    }

  return true;
}

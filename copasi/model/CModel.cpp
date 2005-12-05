/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/model/CModel.cpp,v $
   $Revision: 1.239 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2005/12/05 19:45:15 $
   End CVS Header */

/////////////////////////////////////////////////////////////////////////////
// CModel
// model.cpp : interface of the CModel class
//
/////////////////////////////////////////////////////////////////////////////
#ifdef SunOS
# include <ieeefp.h>
#else
# include <float.h>
#endif

#include "copasi.h"

#include <string>
#include <vector>
#include <limits.h>
#include <float.h>
#include <cmath>
#include <algorithm>

// #define DEBUG_MATRIX

#include "CCompartment.h"
#include "CMetab.h"
#include "CModel.h"
#include "CState.h"
#include "CModelValue.h"
#include "function/CFunctionDB.h"
#include "report/CCopasiObjectReference.h"
#include "report/CKeyFactory.h"
#include "utilities/CCopasiException.h"
#include "utilities/CCopasiMessage.h"
#include "utilities/CCopasiVector.h"
#include "CopasiDataModel/CCopasiDataModel.h"
#include "utilities/CVector.h"
#include "utilities/CluX.h"
#include "utilities/utility.h"
#include "utilities/CProcessReport.h"
#include "CReactionInterface.h"
#include "clapackwrap.h"
#include "utilities/CAnnotatedMatrix.h"

#ifdef COPASI_DEBUG
#define CCHECK {check();}
#else
#define CCHECK
#endif

const char * CModel::VolumeUnitNames[] =
  {"m\xc2\xb3", "l", "ml", "\xc2\xb5l", "nl", "pl", "fl", NULL};

const char * CModel::TimeUnitNames[] =
  {"d", "h", "min", "s", "ms", "\xc2\xb5s", "ns", "ps", "fs", NULL};

const char * CModel::QuantityUnitNames[] =
  {"Mol", "mMol", "\xc2\xb5Mol", "nMol", "pMol", "fMol", "#", NULL};

CModel::CModel():
    CCopasiContainer("New Model", &RootContainer, "Model"),
    mKey(GlobalKeys.add("Model", this)),
    mComments(),
    mVolumeUnit(ml),
    mTimeUnit(s),
    mQuantityUnit(mMol),
    mCompartments("Compartments", this),
    mMetabolites("Metabolites", this),
    mMetabolitesX("Reduced Model Metabolites", this),
    mMetabolitesInd("Independent Metabolites", this),
    //mMetabolitesDep("Dependent Metabolites", this),
    mMetabolitesVar("Variable Metabolites", this),
    mSteps("Reactions", this),
    mStepsX("Reduced Model Reactions", this),
    //mStepsInd("Independent Reactions", this),
    mFluxes(),
    mFluxesX(),
    mParticleFluxes(),
    mParticleFluxesX(),
    mValues("Values", this),
    mInitialTime(0),
    mTime(0),
    mTransitionTime(0),
    mMoieties("Moieties", this),
    mStoi(),
    mRedStoi(),
    mL(),
    mLView(mL, mMetabolitesInd),
    mQuantity2NumberFactor(1.0),
    mNumber2QuantityFactor(1.0),
    mStateTemplate(),
    mpCompileHandler(NULL)
{
  initObjects();

  unsigned C_INT32 i, imax = mSteps.size();

  for (i = 0; i < imax; i++)
    mSteps[i]->compile(/*mCompartments*/);

  initializeMetabolites();

  forceCompile();

  /* This following 2 lines added by Liang Xu
  Becaues of the failure to initialize the parameter when creating a new models
  */
  setQuantityUnit(mQuantityUnit); // set the factors
  setVolumeUnit(mVolumeUnit); // set the factors

  CONSTRUCTOR_TRACE;
}

CModel::CModel(const CModel & src):
    CCopasiContainer(src),
    mKey(GlobalKeys.add("Model", this)),
    mComments(src.mComments),
    mVolumeUnit(src.mVolumeUnit),
    mTimeUnit(src.mTimeUnit),
    mQuantityUnit(src.mQuantityUnit),
    mCompartments(src.mCompartments, this),
    mMetabolites(src.mMetabolites, this),
    mMetabolitesX(src.mMetabolitesX, this),
    mMetabolitesInd(src.mMetabolitesInd, this),
    //mMetabolitesDep(src.mMetabolitesDep, this),
    mMetabolitesVar(src.mMetabolitesVar, this),
    mSteps(src.mSteps, this),
    mStepsX(src.mStepsX, this),
    //mStepsInd(src.mStepsInd, this),
    mFluxes(src.mFluxes),
    mFluxesX(src.mFluxesX),
    mParticleFluxes(src.mParticleFluxes),
    mParticleFluxesX(src.mParticleFluxesX),
    mValues(src.mValues, this),
    mInitialTime(src.mInitialTime),
    mTime(src.mTime),
    mTransitionTime(src.mTransitionTime),
    mMoieties(src.mMoieties, this),
    mStoi(src.mStoi),
    mRedStoi(src.mRedStoi),
    mL(src.mL),
    mLView(mL, mMetabolitesInd),
    mQuantity2NumberFactor(src.mQuantity2NumberFactor),
    mNumber2QuantityFactor(src.mNumber2QuantityFactor),
    mStateTemplate(),
    mpCompileHandler(NULL)
{
  CONSTRUCTOR_TRACE;

  unsigned C_INT32 i, imax = mSteps.size();

  for (i = 0; i < imax; i++)
    mSteps[i]->compile(/*mCompartments*/);

  initializeMetabolites();

  forceCompile();
  initObjects();
}

CModel::~CModel()
{
  GlobalKeys.remove(mKey);
  //cleanup();
  DESTRUCTOR_TRACE;
}

void CModel::cleanup()
{
  /* The real objects */
  mCompartments.cleanup();
  mSteps.cleanup();
  mMoieties.cleanup();

  /* The references */
  mStepsX.resize(0);
  //mStepsInd.resize(0);
  mMetabolites.resize(0);
  mMetabolitesX.resize(0);
  mMetabolitesInd.resize(0);
  //mMetabolitesDep.resize(0);
  mMetabolitesVar.resize(0);
  mFluxes.resize(0);
  mFluxesX.resize(0);
  mParticleFluxes.resize(0);
  mParticleFluxesX.resize(0);
}

C_INT32 CModel::load(CReadConfig & configBuffer)
{
  C_INT32 Size = 0;
  C_INT32 Fail = 0;
  unsigned C_INT32 i;
  std::string tmp;

  // For old Versions we must read the list of Metabolites beforehand
  if ((Fail = configBuffer.getVariable("TotalMetabolites", "C_INT32",
                                       &Size, CReadConfig::LOOP)))
    return Fail;

  // :TODO: Remove OldMetabolites as part of the data model.
  CCopasiDataModel::Global->pOldMetabolites->load(configBuffer, Size);

  if ((Fail = configBuffer.getVariable("Title", "string", &tmp,
                                       CReadConfig::LOOP)))
    return Fail;
  setObjectName(tmp);

  try
    {
      Fail = configBuffer.getVariable("Comments", "multiline", &mComments,
                                      CReadConfig::SEARCH);
    }

  catch (CCopasiException Exception)
    {
      if ((MCReadConfig + 1) == Exception.getMessage().getNumber())
        mComments = "";
      else
        throw Exception;
    }

  try
    {
      Fail = configBuffer.getVariable("TimeUnit", "string", &tmp,
                                      CReadConfig::LOOP);
    }
  catch (CCopasiException Exception)
    {
      if ((MCReadConfig + 1) == Exception.getMessage().getNumber())
        tmp = ""; //unknown?
      else
        throw Exception;
    }

  setTimeUnit(tmp); // set the factors

  try
    {
      Fail = configBuffer.getVariable("ConcentrationUnit", "string", &tmp,
                                      CReadConfig::LOOP);
    }
  catch (CCopasiException Exception)
    {
      if ((MCReadConfig + 1) == Exception.getMessage().getNumber())
        tmp = "";  //unknown?
      else
        throw Exception;
    }

  setQuantityUnit(tmp); // set the factors

  try
    {
      Fail = configBuffer.getVariable("VolumeUnit", "string", &tmp,
                                      CReadConfig::LOOP);
    }
  catch (CCopasiException Exception)
    {
      if ((MCReadConfig + 1) == Exception.getMessage().getNumber())
        tmp = ""; //unknown?
      else
        throw Exception;
    }

  setVolumeUnit(tmp); // set the factors

  mInitialTime = 0;

  if ((Fail = configBuffer.getVariable("TotalCompartments", "C_INT32", &Size,
                                       CReadConfig::LOOP)))
    return Fail;

  mCompartments.load(configBuffer, Size);

  // Create the correct compartment / metabolite relationships
  CMetab *pMetabolite;

  for (i = 0; i < CCopasiDataModel::Global->pOldMetabolites->size(); i++)
    {
      pMetabolite = new CMetab;
      mCompartments[(*CCopasiDataModel::Global->pOldMetabolites)[i]->getIndex()]->
      addMetabolite(pMetabolite);

      (*pMetabolite) = *(*CCopasiDataModel::Global->pOldMetabolites)[i];
    }

  //DebugFile << mCompartments;       //debug

  initializeMetabolites();

  if ((Fail = CCopasiDataModel::Global->getFunctionList()->load(configBuffer))) // slow
    return Fail;

  if ((Fail = configBuffer.getVariable("TotalSteps", "C_INT32", &Size,
                                       CReadConfig::LOOP)))
    return Fail;

  mSteps.load(configBuffer, Size); // slow

  // DebugFile << std::endl << mSteps << std::endl;  //debug

  for (i = 0; i < mSteps.size(); i++)
    mSteps[i]->compile(/*mCompartments*/);

  // DebugFile << "After compiling " << std::endl << mSteps << std::endl;   //debug

  CCopasiDataModel::Global->pOldMetabolites->cleanup();

  setCompileFlag();
  return Fail;
}

bool CModel::compile()
{
  CMatrix< C_FLOAT64 > LU;

  unsigned C_INT32 CompileStep = 0;
  unsigned C_INT32 hCompileStep;
  if (mpCompileHandler)
    {
      mpCompileHandler->setName("Compiling model...");
      unsigned C_INT32 totalSteps = 7;
      hCompileStep = mpCompileHandler->addItem("Step",
                     CCopasiParameter::UINT,
                     & CompileStep,
                     &totalSteps);
    }
  unsigned C_INT32 i, imax = mSteps.size();
  for (i = 0; i < imax; i++)
    mSteps[i]->compile(/*mCompartments*/);

  CompileStep = 1;
  if (mpCompileHandler && !mpCompileHandler->progress(hCompileStep)) return false;

  buildStoi();
  CompileStep = 2;
  if (mpCompileHandler && !mpCompileHandler->progress(hCompileStep)) return false;

  buildLinkZero();
  CompileStep = 3;
  if (mpCompileHandler && !mpCompileHandler->progress(hCompileStep)) return false;

  /*
    lUDecomposition(LU);
    CompileStep = 2;
    if (mpCompileHandler && !mpCompileHandler->progress(hCompileStep)) return false;
  */

  buildRedStoi();
  CompileStep = 4;
  if (mpCompileHandler && !mpCompileHandler->progress(hCompileStep)) return false;

  setMetabolitesStatus();
  CompileStep = 5;
  if (mpCompileHandler && !mpCompileHandler->progress(hCompileStep)) return false;

  /*
    buildL(LU);
    CompileStep = 5;
    if (mpCompileHandler && !mpCompileHandler->progress(hCompileStep)) return false;
  */

  buildMoieties();
  CompileStep = 6;
  if (mpCompileHandler && !mpCompileHandler->progress(hCompileStep)) return false;

  buildStateTemplate();
  CompileStep = 7;
  if (mpCompileHandler && !mpCompileHandler->progress(hCompileStep)) return false;

  if (mpCompileHandler) mpCompileHandler->finish(hCompileStep);

  mCompileIsNecessary = false;
  return true;
}

void CModel::setCompileFlag(bool flag)
{
  mCompileIsNecessary = flag;
  //if (flag) std::cout << "* model dirty flag set. " << std::endl;

  if (flag) initializeMetabolites();
}

bool CModel::compileIfNecessary()
{
  /*
  std::cout << "** compiling a CModel is requested. ";
  if (mCompileIsNecessary)
    std::cout << "It will be done...." << std::endl;
  else
    std::cout << " " << std::endl;
  */

  if (mCompileIsNecessary)
    return compile();
  return true;
}

bool CModel::forceCompile()
{
  setCompileFlag();
  return compileIfNecessary();
}

void CModel::buildStoi()
{
  unsigned C_INT32 i, j, k, imax;
  std::string key;

  initializeMetabolites();

  mFluxes.resize(mSteps.size());
  mParticleFluxes.resize(mSteps.size());

  for (i = 0; i < mSteps.size(); i++)
    {
      mFluxes[i] = & mSteps[i]->getFlux();
      mParticleFluxes[i] = & mSteps[i]->getParticleFlux();
    }

  imax = mMetabolites.size();
  mStoi.resize(imax - mNumFixed, mSteps.size());

  unsigned C_INT32 hProcess;
  if (mpCompileHandler)
    {
      i = 0;
      unsigned C_INT32 imax = mStoi.numCols();
      hProcess = mpCompileHandler->addItem("Building Stoichiometry",
                                           CCopasiParameter::UINT,
                                           &i,
                                           &imax);
    }

  for (i = 0; i < (unsigned C_INT32) mStoi.numCols(); i++)
    {
      if (mpCompileHandler && !mpCompileHandler->progress(hProcess)) return;

      const CCopasiVector < CChemEqElement > &Structure
      = mSteps[i]->getChemEq().getBalances();

      for (j = 0; j < (unsigned C_INT32) mStoi.numRows(); j++)
        {
          mStoi[j][i] = 0.0;
          key = mMetabolites[j]->getKey(); //TODO use reference?

          for (k = 0; k < Structure.size(); k++)
            if (Structure[k]->getMetaboliteKey() == key)
              {
                mStoi[j][i] = Structure[k]->getMultiplicity();
                break;
              }
        }
    }

  // We need to have all unused and fixed metabolites at the end of mMetabolites.
  // However we can only detect unused metabolites after building the
  // stoichiometry matrix.
  handleUnusedMetabolites();

#ifdef DEBUG_MATRIX
  DebugFile << "Stoichiometry Matrix" << std::endl;
  DebugFile << mStoi << std::endl;
#endif

  if (mpCompileHandler)
    mpCompileHandler->finish(hProcess);

  return;
}

bool CModel::handleUnusedMetabolites()
{
  unsigned C_INT32 i, imax = mStoi.numRows();
  unsigned C_INT32 j, jmax = mStoi.numCols();
  std::vector< unsigned C_INT32 > Unused;

  for (i = 0; i < imax; i++)
    {
      for (j = 0; j < jmax; j++)
        if (mStoi[i][j] != 0.0) break;

      if (j == jmax) Unused.push_back(i);
    }

  unsigned C_INT32 k = 0, kmax = Unused.size();

  if (kmax == 0) return false;

  // We treat unused variables in the same way as fixed, i.e.
  // they will be sorted to the end of the metabolite list.
  mNumFixed += kmax;

  CMatrix< C_FLOAT64 > Stoi(imax - kmax, jmax);
  std::vector< CMetab * > VariableMetabolites(imax - kmax);
  std::vector< CMetab * > UnusedMetabolites(kmax);
  std::vector< unsigned C_INT32 >::const_iterator itUnused = Unused.begin();
  std::vector< unsigned C_INT32 >::const_iterator endUnused = Unused.end();
  CMetab * pMetab;

  // Build new stoichiometry Matrix
  for (i = 0; i < imax; i++)
    {
      if (itUnused != endUnused && i == *itUnused)
        {
          pMetab = mMetabolites[i];
          if (listReactionsDependentOnMetab(pMetab->getKey()).size())
            pMetab->setStatus(CModelEntity::FIXED);
          else
            pMetab->setStatus(CModelEntity::UNUSED);

          UnusedMetabolites[k] = pMetab;

          k++;
          ++itUnused;
        }
      else
        {
          VariableMetabolites[i - k] = mMetabolites[i];

          // The row needs to be copied to the new stoichiometry matrix
          for (j = 0; j < jmax; j++)
            Stoi[i - k][j] = mStoi[i][j];
        }
    }

  // Reorder metabolites
  for (i = 0, imax = VariableMetabolites.size(); i < imax; i++)
    mMetabolites[i] = VariableMetabolites[i];

  for (k = 0; k < kmax; k++)
    mMetabolites[i + k] = UnusedMetabolites[k];

  // We need also to update the metabolites for the reduced system.
  for (i = 0, imax = mMetabolites.size(); i < imax; i++)
    mMetabolitesX[i] = mMetabolites[i];

  // Update stoichiometry matrix
  mStoi = Stoi;
  return true;
}

void CModel::lUDecomposition(CMatrix< C_FLOAT64 > & LU)
{
  unsigned C_INT32 i;

  LU = mStoi;

  LUfactor(LU, mRowLU, mColLU, mpCompileHandler);

  // permutate Metabolites and Steps to match rearangements done during
  // LU decomposition

  for (i = 0; i < mRowLU.size(); i++)
    mMetabolitesX[i] = mMetabolites[mRowLU[i]];

  mStepsX.resize(mSteps.size(), false);
  mFluxesX.resize(mStepsX.size());
  mParticleFluxesX.resize(mStepsX.size());

  for (i = 0; i < mSteps.size(); i++)
    {
      mStepsX[i] = mSteps[mColLU[i]];
      mFluxesX[i] = &mStepsX[i]->getFlux();
      mParticleFluxesX[i] = &mStepsX[i]->getParticleFlux();
    }

#ifdef DEBUG_MATRIX
  DebugFile << "Metabolite reordering " << mRowLU << std::endl;
  DebugFile << "Reaction reordering " << mColLU << std::endl;
  DebugFile << "LU Decomposistion" << std::endl;
  DebugFile << LU;
#endif

  return;
}

void CModel::setMetabolitesStatus()
{
  unsigned C_INT32 i;
  unsigned C_INT32 iIndependent = 0, iVariable = 0;

  unsigned C_INT32 imax = mMetabolitesX.size() - mL.numRows();

  for (i = 0; i < imax; i++)
    mMetabolitesX[i]->setStatus(CModelEntity::REACTIONS);

  iIndependent = i;

  for (; i < iIndependent + mL.numRows(); i++)
    mMetabolitesX[i]->setStatus(CModelEntity::DEPENDENT);

  iVariable = i;

  assert(mNumFixed + iVariable == mMetabolites.size());

  mMetabolitesInd.resize(iIndependent, false);
  mMetabolitesVar.resize(iVariable, false);

  for (i = 0; i < iIndependent; i++)
    {
      mMetabolitesInd[i] = mMetabolitesX[i];
      mMetabolitesVar[i] = mMetabolitesX[i];
    }

  for (; i < iVariable; i++)
    mMetabolitesVar[i] = mMetabolitesX[i];

  return;
}

void CModel::buildRedStoi()
{
  C_INT32 i, imax = mMetabolites.size() - mL.numRows();
  C_INT32 j, jmax = mStepsX.size();                // wei for compiler

  mRedStoi.resize(imax, jmax);
  /* just have to swap rows and colums */
  for (i = 0; i < imax; i++)
    for (j = 0; j < jmax; j++)
      mRedStoi(i, j) = mStoi(mRowLU[i], mColLU[j]);

  imax = mMetabolites.size();
  mMetabolitesX.resize(imax, false);
  for (i = 0; i < imax; i++)
    mMetabolitesX[i] = mMetabolites[mRowLU[i]];

  imax = mSteps.size();
  mStepsX.resize(mSteps.size(), false);
  mFluxesX.resize(mStepsX.size());
  mParticleFluxesX.resize(mStepsX.size());

  for (i = 0; i < imax; i++)
    {
      mStepsX[i] = mSteps[mColLU[i]];
      mFluxesX[i] = &mStepsX[i]->getFlux();
      mParticleFluxesX[i] = &mStepsX[i]->getParticleFlux();
    }

#ifdef XXXX
  for (i = 0; i < imax; i++)
    for (j = 0; j < jmax; j++)
      {
        /* Since L[i,k] = 1 for k = i and L[i,k] = 0 for k > i
           we have to avoid L[i,k] where k >= i, i.e.. k < i.
           Similarly, since U[k,j] = 0 for k > j
           we have to avoid U[k,j] where k > j, i.e., k <= j. */

        if (j < i)
          {
            Sum = 0.0;
            kmax = j + 1;
          }
        else
          {
            /* For j < i we are missing a part of the sum: */
            /* Sum +=  LU[i][j]; since L[i,i] = 1 */
            Sum = LU[i][j];
            kmax = i;
          }

        for (k = 0; k < kmax; k++)
          Sum += LU[i][k] * LU[k][j];

        mRedStoi[i][j] = Sum;
      }
#endif // XXXX

#ifdef DEBUG_MATRIX
  DebugFile << "Reduced Stoichiometry Matrix" << std::endl;
  DebugFile << mRedStoi << std::endl;
#endif
  //std::cout << "Reduced Stoichiometry Matrix" << std::endl;
  //std::cout << mRedStoi << std::endl;

  return;
}

void CModel::buildL(const CMatrix< C_FLOAT64 > & LU)
{
  C_INT N = mMetabolitesInd.size();
  C_INT LDA = std::max((C_INT) 1, N);
  C_INT Info;

  unsigned C_INT32 i, imin, imax;
  unsigned C_INT32 j, jmax;
  unsigned C_INT32 k;
  C_FLOAT64 * sum;

  CMatrix< C_FLOAT64 > R(N, N);

  for (i = 1; i < (unsigned C_INT32) N; i++)
    for (j = 0; j < i; j++)
      R(i, j) = LU(i, j);

#ifdef DEBUG_MATRIX
  DebugFile << "L" << std::endl;
  DebugFile << R << std::endl;
#endif

  /* to take care of differences between fortran's and c's memory  acces,
     we need to take the transpose, i.e.,the upper triangular */
  char cL = 'U';
  char cU = 'U'; /* 1 in the diaogonal of R */

  /* int dtrtri_(char *uplo,
   *             char *diag, 
   *             integer *n, 
   *             doublereal * A,
   *             integer *lda, 
   *             integer *info);   
   *  -- LAPACK routine (version 3.0) --
   *     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
   *     Courant Institute, Argonne National Lab, and Rice University
   *     March 31, 1993
   *
   *  Purpose
   *  =======
   *
   *  DTRTRI computes the inverse of a real upper or lower triangular
   *  matrix A.
   *
   *  This is the Level 3 BLAS version of the algorithm.
   *
   *  Arguments
   *  =========
   *
   *  uplo    (input) CHARACTER*1
   *          = 'U':  A is upper triangular;
   *          = 'L':  A is lower triangular.
   *
   *  diag    (input) CHARACTER*1
   *          = 'N':  A is non-unit triangular;
   *          = 'U':  A is unit triangular.
   *
   *  n       (input) INTEGER
   *          The order of the matrix A.  n >= 0.
   *
   *  A       (input/output) DOUBLE PRECISION array, dimension (lda,n)
   *          On entry, the triangular matrix A.  If uplo = 'U', the
   *          leading n-by-n upper triangular part of the array A contains
   *          the upper triangular matrix, and the strictly lower
   *          triangular part of A is not referenced.  If uplo = 'L', the
   *          leading n-by-n lower triangular part of the array A contains
   *          the lower triangular matrix, and the strictly upper
   *          triangular part of A is not referenced.  If diag = 'U', the
   *          diagonal elements of A are also not referenced and are
   *          assumed to be 1.
   *          On exit, the (triangular) inverse of the original matrix, in
   *          the same storage format.
   *
   *  lda     (input) INTEGER
   *          The leading dimension of the array A.  lda >= max(1,n).
   *
   *  info    (output) INTEGER
   *          = 0: successful exit
   *          < 0: if info = -i, the i-th argument had an illegal value
   *          > 0: if info = i, A(i,i) is exactly zero.  The triangular
   *               matrix is singular and its inverse can not be computed.
   */
  dtrtri_(&cL, &cU, &N, R.array(), &LDA, &Info);
  if (Info) fatalError();

#ifdef DEBUG_MATRIX
  DebugFile << "L inverse" << std::endl;
  DebugFile << R << std::endl;
#endif

  mL.resize(getNumDependentMetabs(), getNumIndependentMetabs());

  imin = getNumIndependentMetabs(), imax = getNumVariableMetabs();
  jmax = getNumIndependentMetabs();

  // Construct L_0
  for (i = imin; i < imax; i++)
    for (j = 0; j < jmax; j++)
      {
        sum = & mL(i - imin, j);
        *sum = LU(i, j);

        for (k = j + 1; k < jmax; k++)
          *sum += LU(i, k) * R(k, j);

        if (fabs(*sum) < DBL_EPSILON) *sum = 0.0;
      }

#ifdef DEBUG_MATRIX
  DebugFile << "Link Matrix:" << std::endl;
  DebugFile << mLView << std::endl;
#endif // DEBUG_MATRIX
}

void CModel::updateMoietyValues()
{
  C_INT32 i, imax = mMoieties.size();
  for (i = 0; i < imax; ++i)
    mMoieties[i]->setInitialValue();
}

void CModel::buildMoieties()
{
  unsigned C_INT32 i;
  unsigned C_INT32 imin = mMetabolitesInd.size();
  unsigned C_INT32 imax = imin + getNumDependentMetabs();
  unsigned C_INT32 j, jmax = mMetabolitesInd.size();

  CMoiety *pMoiety;

  mMoieties.cleanup();

  for (i = imin; i < imax; i++)
    {
      if (mMetabolitesX[i]->getStatus() == CModelEntity::UNUSED) continue;

      pMoiety = new CMoiety;
      pMoiety->setName(mMetabolitesX[i]->getObjectName());
      pMoiety->add(1.0, mMetabolitesX[i]);

      for (j = 0; j < jmax; j++)
        {
          if (mLView(i, j) != 0.0)
            pMoiety->add(- mLView(i, j), mMetabolitesX[j]);
        }

      //pMoiety->setInitialValue();
      //      DebugFile << pMoiety->getDescription() << " = "
      //      << pMoiety->getNumber() << std::endl;

      mMoieties.add(pMoiety, true);
    }
  updateMoietyValues();
  return;
}

#ifdef WIN32 
// warning C4056: overflow in floating-point constant arithmetic
// warning C4756: overflow in constant arithmetic
# pragma warning (disable: 4056 4756)
#endif

void CModel::setTransitionTimes()
{
  unsigned C_INT32 i, imax = mMetabolites.size();
  unsigned C_INT32 j, jmax = mSteps.size();

  C_FLOAT64 TotalFlux_p, TotalFlux_n, min_flux, PartialFlux;
  C_FLOAT64 TransitionTime;

  mTransitionTime = 0.0;

  for (i = 0; i < imax; i++)
    {
      if (CModelEntity::FIXED == mMetabolites[i]->getStatus())
        mMetabolites[i]->setTransitionTime(2 * DBL_MAX);
      else
        {
          TotalFlux_p = 0.0;
          for (j = 0; j < jmax; j++)
            {
              PartialFlux = mStoi[i][j] * *mParticleFluxes[j];

              if (PartialFlux > 0.0)
                TotalFlux_p += PartialFlux;
            }

          //if (TotalFlux == 0.0) //TODO discuss
          TotalFlux_n = 0.0;
          for (j = 0; j < jmax; j++)
            {
              PartialFlux = - mStoi[i][j] * *mParticleFluxes[j];

              if (PartialFlux > 0.0)
                TotalFlux_n += PartialFlux;
            }

          if (TotalFlux_p < TotalFlux_n)
            min_flux = TotalFlux_p;
          else
            min_flux = TotalFlux_n;

          if (min_flux == 0.0)
            TransitionTime = 2 * DBL_MAX;
          else
            TransitionTime = mMetabolites[i]->getNumber() / min_flux;

          mMetabolites[i]->setTransitionTime(TransitionTime);
          //mMetabolites[i]->setNumberRate(TotalFlux);

          if (!finite(TransitionTime))
            mTransitionTime = TransitionTime;
          else
            mTransitionTime += TransitionTime;
        }
    }
}

#ifdef WIN32
# pragma warning (default: 4056 4756)
#endif

//this is supposed to be so fast it can be called often to be kept up to date
//all the time. At the moment it creates the mMetabolites and sorts the fixed
//metabs to the end
void CModel::initializeMetabolites()
{
  unsigned C_INT32 i, j;

  // Create a vector of pointers to all metabolites.
  // Note, the metabolites physically exist in the compartments.
  mMetabolites.resize(0);

  for (i = 0; i < mCompartments.size(); i++)
    for (j = 0; j < mCompartments[i]->getMetabolites().size(); j++)
      {
        mMetabolites.add(mCompartments[i]->getMetabolites()[j]);
      }

  unsigned C_INT32 imax = mMetabolites.size();

  mMetabolitesX.resize(imax, false);

  // We reorder mMetabolitesX so that the fixed metabolites appear at the end.
  for (i = 0, mNumFixed = 0; i < imax; i++)
    if (mMetabolites[i]->getStatus() == CModelEntity::FIXED)
      mMetabolitesX[imax - ++mNumFixed] = mMetabolites[i];
    else
      mMetabolitesX[i - mNumFixed] = mMetabolites[i];

  // Update mMetabolites to reflect the reordering.
  // We need to to this to allow the use of the full model for simulation.
  // :TODO: This most definitely breaks output when reading Gepasi files.
  //        CGlobal::OldMetabolites is still in the expexted order !!!
  for (i = 0; i < imax; i++)
    mMetabolites[i] = mMetabolitesX[i];
}

//**********************************************************************

CCopasiVectorNS < CReaction > & CModel::getReactions()
{return mSteps;}

const CCopasiVectorNS < CReaction > & CModel::getReactions() const
  {return mSteps;}

CCopasiVectorN< CReaction > & CModel::getReactionsX()
{CCHECK return mStepsX;}

const CCopasiVectorN< CReaction > & CModel::getReactionsX() const
  {CCHECK return mStepsX;}

CCopasiVector< CMetab > & CModel::getMetabolites()
{return mMetabolites;}

const CCopasiVector< CMetab > & CModel::getMetabolites() const
  {return mMetabolites;}

CCopasiVector< CMetab > & CModel::getMetabolitesX()
{CCHECK return mMetabolitesX;}

const CCopasiVector< CMetab > & CModel::getMetabolitesX() const
  {CCHECK return mMetabolitesX;}

const CCopasiVector< CMetab > & CModel::getMetabolitesInd() const
  {CCHECK return mMetabolitesInd;}

//CCopasiVector< CMetab > & CModel::getMetabolitesDep()
//  {CCHECK return mMetabolitesDep;}

const CCopasiVector< CMetab > & CModel::getVariableMetabolites() const
  {CCHECK return mMetabolitesVar;}

const CCopasiVectorN< CModelValue > & CModel::getModelValues() const
  {return mValues;}

CCopasiVectorN< CModelValue > & CModel::getModelValues()
{return mValues;}

//********

unsigned C_INT32 CModel::getNumMetabs() const
  {return mMetabolites.size();}

unsigned C_INT32 CModel::getNumVariableMetabs() const
  {return mMetabolites.size() - mNumFixed;}

unsigned C_INT32 CModel::getNumIndependentMetabs() const
  {CCHECK return mMetabolitesInd.size();}

unsigned C_INT32 CModel::getNumDependentMetabs() const
  {CCHECK return getNumVariableMetabs() - getNumIndependentMetabs();}

unsigned C_INT32 CModel::getTotSteps() const
  {return mSteps.size();}

unsigned C_INT32 CModel::getNumModelValues() const
  {return mValues.size();}

const std::string & CModel::getComments() const
  {return mComments;}

const std::string & CModel::getKey() const
  {return mKey;}

CCopasiVectorNS < CCompartment > & CModel::getCompartments()
{return mCompartments;}

const CCopasiVectorNS < CCompartment > & CModel::getCompartments() const
  {return mCompartments;}

/**
 *  Get the Reduced Stoichiometry Matrix of this Model
 */
const CMatrix < C_FLOAT64 >& CModel::getRedStoi() const
  {CCHECK return mRedStoi;}

/**
 *  Get the Stoichiometry Matrix of this Model
 */
const CMatrix < C_FLOAT64 >& CModel::getStoi() const
  {CCHECK return mStoi;}

const CCopasiVector < CMoiety > & CModel::getMoieties() const
  {return mMoieties;} //TODO: resolv when to recalculate moieties...

//const CCopasiVectorN< CReaction > & CModel::getStepsX() const
//  {CCHECK return mStepsX;}

const CModel::CLinkMatrixView & CModel::getL() const
  {CCHECK return mLView;}

const CModel::CStateTemplate & CModel::getStateTemplate() const
  {CCHECK return mStateTemplate;}

bool CModel::setTitle(const std::string &title)
{
  if (title == "")
    return setObjectName("NoTitle");
  return setObjectName(title);
}

void CModel::setComments(const std::string &comments)
{mComments = comments;}

void CModel::setInitialTime(const C_FLOAT64 & time)
{mInitialTime = time;}

const C_FLOAT64 & CModel::getInitialTime() const
  {return mInitialTime;}

void CModel::setTime(const C_FLOAT64 & time)
{mTime = time;}

const C_FLOAT64 & CModel::getTime() const
  {return mTime;}

const CVector<unsigned C_INT32> & CModel::getMetabolitePermutation() const
  {CCHECK return mRowLU;}

const CVector<unsigned C_INT32> & CModel::getReactionPermutation() const
  {CCHECK return mColLU;}

//**********************************************************************

/**
 *        Returns the index of the metab
 */
C_INT32 CModel::findMetabByName(const std::string & Target) const
  {
    unsigned C_INT32 i, s;
    std::string name;

    s = mMetabolites.size();
    for (i = 0; i < s; i++)
      {
        name = mMetabolites[i]->getObjectName();
        if (name == Target)
          return i;
      }
    return - 1;
  }

/**
 *        Returns the index of the Moiety
 */
C_INT32 CModel::findMoiety(std::string &Target) const
  {
    unsigned C_INT32 i, s;
    std::string name;

    s = mMoieties.size();
    for (i = 0; i < s; i++)
      {
        name = mMoieties[i]->getObjectName();
        if (name == Target)
          return i;
      }
    return - 1;
  }

//**********************************************************************

void CModel::applyInitialValues()
{
  mTime = mInitialTime;

  unsigned C_INT32 i, imax;

  /* Set the volumes */
  for (i = 0, imax = mCompartments.size(); i < imax; ++i)
    mCompartments[i]->setValue(mCompartments[i]->getInitialValue());

  /* Set the metabolites */
  for (i = 0, imax = mMetabolites.size(); i < imax; ++i)
    mMetabolites[i]->setValue(mMetabolites[i]->getInitialValue());

  /* Set the global parameters */
  for (i = 0, imax = mValues.size(); i < imax; ++i)
    mValues[i]->setValue(mValues[i]->getInitialValue());
}

bool CModel::buildStateTemplate()
{
  bool success = false;

  mStateTemplate.cleanup();

  mStateTemplate.add(getKey()); // Time

  unsigned C_INT32 i, imax;

  for (i = 0, imax = mCompartments.size(); i < imax; i++)
    mStateTemplate.add(mCompartments[i]->getKey());

  for (i = 0, imax = mMetabolites.size(); i < imax; i++)
    mStateTemplate.add(mMetabolites[i]->getKey());

  for (i = 0, imax = mValues.size(); i < imax; i++)
    mStateTemplate.add(mValues[i]->getKey());

  return success;
}

CState CModel::getState() const
  {
    unsigned C_INT32 i, imax;
    CState s(this);

    /* Set the time */
    s.setTime(mTime);

    /* Set the volumes */
    C_FLOAT64 * Dbl = const_cast<C_FLOAT64 *>(s.getVolumeVector().array());
    for (i = 0, imax = mCompartments.size(); i < imax; i++, Dbl++)
      *Dbl = mCompartments[i]->getVolume();

    /* Set the variable Metabolites */
    Dbl = const_cast<C_FLOAT64 *>(s.getVariableNumberVector().array());
    for (i = 0, imax = getNumVariableMetabs(); i < imax; i++, Dbl++)
      *Dbl = mMetabolites[i]->getNumber();

    /* Set the fixed Metabolites */
    Dbl = const_cast<C_FLOAT64 *>(s.getFixedNumberVector().array());
    for (i = getNumVariableMetabs(), imax = getNumMetabs(); i < imax; i++, Dbl++)
      *Dbl = mMetabolites[i]->getNumber();

    /* Set the global parameters */
    Dbl = const_cast<C_FLOAT64 *>(s.getGlobalParameterVector().array());
    for (i = 0, imax = mValues.size(); i < imax; i++, Dbl++)
      *Dbl = mValues[i]->getValue();

    return s;
  }

CState CModel::getInitialState() const
  {
    unsigned C_INT32 i, imax;
    CState s(this);

    /* Set the time */
    s.setTime(mInitialTime);

    /* Set the volumes */
    C_FLOAT64 * Dbl = const_cast<C_FLOAT64 *>(s.getVolumeVector().array());
    for (i = 0, imax = mCompartments.size(); i < imax; i++, Dbl++)
      *Dbl = mCompartments[i]->getInitialVolume();

    /* Set the variable Metabolites */
    Dbl = const_cast<C_FLOAT64 *>(s.getVariableNumberVector().array());
    for (i = 0, imax = getNumVariableMetabs(); i < imax; i++, Dbl++)
      *Dbl = mMetabolites[i]->getInitialNumber();

    /* Set the fixed Metabolites */
    Dbl = const_cast<C_FLOAT64 *>(s.getFixedNumberVector().array());
    for (i = getNumVariableMetabs(), imax = getNumMetabs(); i < imax; i++, Dbl++)
      *Dbl = mMetabolites[i]->getInitialNumber();

    /* Set the global parameters */
    Dbl = const_cast<C_FLOAT64 *>(s.getGlobalParameterVector().array());
    for (i = 0, imax = mValues.size(); i < imax; i++, Dbl++)
      *Dbl = mValues[i]->getInitialValue();

    //     DebugFile << "getInitialState " << mInitialTime;
    //     for (i = 0, imax = mMetabolitesX.size(); i < imax; i++)
    //       DebugFile << " " << mMetabolitesX[i]->getInitialConcentration();
    //     DebugFile << std::endl;

    return s;
  }

CStateX CModel::getInitialStateX() const
  {
    CCHECK
    unsigned C_INT32 i, imax;
    CStateX s(this);

    /* Set the time */
    s.setTime(mInitialTime);

    /* Set the volumes */
    C_FLOAT64 * Dbl = const_cast<C_FLOAT64 *>(s.getVolumeVector().array());
    for (i = 0, imax = mCompartments.size(); i < imax; i++, Dbl++)
      *Dbl = mCompartments[i]->getInitialVolume();

    /* Set the independent variable Metabolites */
    Dbl = const_cast<C_FLOAT64 *>(s.getVariableNumberVector().array());
    for (i = 0, imax = getNumIndependentMetabs(); i < imax; i++, Dbl++)
      *Dbl = mMetabolitesX[i]->getInitialNumber();

    /* Set the dependent variable Metabolites */
    Dbl = const_cast<C_FLOAT64 *>(s.getDependentNumberVector().array());
    for (i = getNumIndependentMetabs(), imax = getNumVariableMetabs(); i < imax; i++, Dbl++)
      *Dbl = mMetabolitesX[i]->getInitialNumber();

    /* Set the fixed Metabolites */
    Dbl = const_cast<C_FLOAT64 *>(s.getFixedNumberVector().array());
    for (i = getNumVariableMetabs(), imax = getNumMetabs(); i < imax; i++, Dbl++)
      *Dbl = mMetabolitesX[i]->getInitialNumber();

    /* Set the global parameters */
    Dbl = const_cast<C_FLOAT64 *>(s.getGlobalParameterVector().array());
    for (i = 0, imax = mValues.size(); i < imax; i++, Dbl++)
      *Dbl = mValues[i]->getInitialValue();

    //     DebugFile << "getInitialStateX " << mInitialTime;
    //     for (i = 0, imax = mMetabolitesX.size(); i < imax; i++)
    //       DebugFile << " " << mMetabolitesX[i]->getInitialConcentration();
    //     DebugFile << std::endl;

    return s;
  }

void CModel::setInitialState(const CState * state)
{
#ifdef COPASI_DEBUG
  state->check("CModel::setInitialState()");
#endif

  unsigned C_INT32 i, imax;

  /* Set the volumes */
  const C_FLOAT64 * Dbl = state->getVolumeVector().array();

  for (i = 0, imax = mCompartments.size(); i < imax; i++, Dbl++)
    mCompartments[i]->setVolume(*Dbl);

  /* Set the variable metabolites */
  Dbl = state->getVariableNumberVector().array();
  for (i = 0, imax = getNumVariableMetabs(); i < imax; i++, Dbl++)
    mMetabolites[i]->setInitialNumber(*Dbl);

  /* Set the fixed metabolites */
  Dbl = state->getFixedNumberVector().array();
  for (i = getNumVariableMetabs(), imax = getNumMetabs(); i < imax; i++, Dbl++)
    mMetabolites[i]->setInitialNumber(*Dbl);

  /* Set the global parameters */
  Dbl = state->getGlobalParameterVector().array();
  for (i = 0, imax = mValues.size(); i < imax; i++, Dbl++)
    mValues[i]->setInitialValue(*Dbl);

  /* We need to update the initial values for moieties */
  for (i = 0, imax = mMoieties.size(); i < imax; i++)
    mMoieties[i]->setInitialValue();

  return;
}

void CModel::setInitialStateX(const CStateX * state)
{
  CCHECK
  unsigned C_INT32 i, imax;

  /* Set the volumes */
  const C_FLOAT64 * Dbl = state->getVolumeVector().array();
  for (i = 0, imax = mCompartments.size(); i < imax; i++, Dbl++)
    mCompartments[i]->setVolume(*Dbl);

  /* Set the independent variable metabolites */
  Dbl = state->getVariableNumberVector().array();
  for (i = 0, imax = getNumIndependentMetabs(); i < imax; i++, Dbl++)
    mMetabolitesX[i]->setInitialNumber(*Dbl);

  /* Set the dependent variable metabolites */
  Dbl = state->getDependentNumberVector().array();
  for (i = getNumIndependentMetabs(), imax = getNumVariableMetabs(); i < imax; i++, Dbl++)
    mMetabolitesX[i]->setInitialNumber(*Dbl);

  /* Set the fixed metabolites */
  Dbl = state->getFixedNumberVector().array();
  for (i = getNumVariableMetabs(), imax = getNumMetabs(); i < imax; i++, Dbl++)
    mMetabolitesX[i]->setInitialNumber(*Dbl);

  /* Set the global parameters */
  Dbl = state->getGlobalParameterVector().array();
  for (i = 0, imax = mValues.size(); i < imax; i++, Dbl++)
    mValues[i]->setInitialValue(*Dbl);

  /* We need to update the initial values for moieties */
  for (i = 0, imax = mMoieties.size(); i < imax; i++)
    mMoieties[i]->setInitialValue();

  return;
}

void CModel::setState(const CState * state)
{
#ifdef COPASI_DEBUG
  state->check("CModel::setState()");
#endif
  unsigned C_INT32 i, imax;
  const C_FLOAT64 * Dbl;

  mTime = state->getTime();

#ifdef XXXX // This gets enabled when we have dynamic volume changes
  Dbl = state->getVolumeVector();
  for (i = 0, imax = mCompartments.size(); i < imax; i++, Dbl++)
    mCompartments[i]->setVolume(*Dbl);
#endif // XXXX

  /* Set the variable metabolites */
  Dbl = state->getVariableNumberVector().array();
  for (i = 0, imax = getNumVariableMetabs(); i < imax; i++, Dbl++)
    mMetabolites[i]->setNumber(*Dbl);
  //   DebugFile << "setState " << mTime;
  //   for (i = 0, imax = mMetabolitesX.size(); i < imax; i++)
  //     DebugFile << " " << mMetabolitesX[i]->getConcentration();
  //   DebugFile << std::endl;
  return;
}

void CModel::setStateX(const CStateX * state)
{
  CCHECK
  unsigned C_INT32 i, imax;
  const C_FLOAT64 * Dbl;

  mTime = state->getTime();

#ifdef XXXX // This gets enabled when we have dynamic volume changes
  Dbl = state->getVolumeVector();
  for (i = 0, imax = mCompartments.size(); i < imax; i++, Dbl++)
    mCompartments[i]->setVolume(*Dbl);
#endif // XXXX

  /* Set the independent variable metabolites */
  Dbl = state->getVariableNumberVector().array();
  for (i = 0, imax = getNumIndependentMetabs(); i < imax; i++, Dbl++)
    mMetabolitesX[i]->setNumber(*Dbl);

  /* We need to update the dependent metabolites by using moieties */
  /* This changes need to be reflected in the current state */
  C_FLOAT64 NumberDbl;
  for (i = 0, imax = mMoieties.size(); i < imax; i++)
    {
      NumberDbl = mMoieties[i]->dependentNumber();
      mMetabolitesVar[i + getNumIndependentMetabs()]->setNumber(NumberDbl);
      (const_cast<CStateX *>(state))->setDependentNumber(i, NumberDbl);
    }
  //   DebugFile << "setStateX " << mTime;
  //   for (i = 0, imax = mMetabolitesX.size(); i < imax; i++)
  //     DebugFile << " " << mMetabolitesX[i]->getConcentration();
  //   DebugFile << std::endl;
  return;
}

void CModel::updateDepMetabNumbers(CStateX const & state) const
  {
    CCHECK
    (const_cast< CModel * >(this))->setStateX(&state);
    //TODO this could be done more efficiently
  }

void CModel::updateRates()
{
  CCHECK
  unsigned C_INT32 j, jmax = mSteps.size();
  for (j = 0; j < jmax; ++j)
    mSteps[j]->calculate();

  // Calculate ydot = Stoi * v
  unsigned C_INT32 i, imax = getNumVariableMetabs();
  C_FLOAT64 tmp;
  for (i = 0; i < imax; ++i)
    {
      tmp = 0.0;
      for (j = 0; j < jmax; ++j)
        tmp += mStoi[i][j] * *mParticleFluxes[j];
      mMetabolites[i]->setNumberRate(tmp);
    }

  setTransitionTimes();
}

//**********************************************************************

/*void CModel::getRates(const CState * state, C_FLOAT64 * rates)
{
  setState(state);
 
  unsigned C_INT32 i, imax;
 
  for (i = 0, imax = mSteps.size(); i < imax; i++)
    rates[i] = mSteps[i]->calculate();
 
  return;
}
 
void CModel::getRatesX(const CStateX * state, C_FLOAT64 * rates)
{
  setStateX(state);
 
  unsigned C_INT32 i, imax;
 
  for (i = 0, imax = mStepsX.size(); i < imax; i++)
    rates[i] = mStepsX[i]->calculate();
 
  return;
}*/

void CModel::getDerivatives_particles(const CState * state, CVector< C_FLOAT64 > & derivatives)
{
  setState(state);

  unsigned C_INT32 i, imax = getNumVariableMetabs();
  unsigned C_INT32 j, jmax = mSteps.size();

  assert (derivatives.size() == imax);

  for (j = 0; j < jmax; j++)
    mSteps[j]->calculate();

  // Calculate ydot = Stoi * v
  for (i = 0; i < imax; i++)
    {
      derivatives[i] = 0.0;

      for (j = 0; j < jmax; j++)
        derivatives[i] += mStoi[i][j] * *mParticleFluxes[j];
    }
}

void CModel::getDerivativesX_particles(const CStateX * state, CVector< C_FLOAT64 > & derivatives)
{
  setStateX(state);

  unsigned C_INT32 i, imax = mMetabolitesInd.size();
  unsigned C_INT32 j, jmax = mStepsX.size();

  assert (derivatives.size() == imax);

  for (j = 0; j < jmax; j++)
    mStepsX[j]->calculate();

  // Calculate ydot = RedStoi * v
  for (i = 0; i < imax; i++)
    {
      derivatives[i] = 0.0;

      for (j = 0; j < jmax; j++)
        derivatives[i] += mRedStoi[i][j] * *mParticleFluxesX[j];
    }
}

//**********************************************************************

/*unsigned C_INT32 CModel::unitCompare(const std::string & name,
                                     const char ** units
                                     const unsigned C_INT32 unique)
{
  unsigned C_INT32 i, j;
  std::string Unit;
 
  //for (i = 0; *units; i++, units++)
  //  {
  //    Unit = *units;
  //    for (j = Unit.length(); j >= unique || j == Unit.length(); j--)
  //      if (Unit.substr(0, j) == name.substr(0, j)) return i;
  //}
  
  for (i = 0; *units; i++, units++)
    {
      Unit = *units;
        if (Unit == name) return i;
    }
 
  return i;
}*/

bool CModel::setVolumeUnit(const std::string & name)
{
  int unit = toEnum(name.c_str(), VolumeUnitNames);

  if (-1 == unit)
    return setVolumeUnit(ml);
  else
    return setVolumeUnit((CModel::VolumeUnit) unit);
}

bool CModel::setVolumeUnit(const CModel::VolumeUnit & unit)
{
  mVolumeUnit = unit;
  return true;

  /*bool success = true;
   
    switch (unit)
      {
      case m3:
      case l:
      case ml:
      case microl:
      case nl:
      case pl:
      case fl:
        mVolumeUnit = VolumeUnitName[unit];
        break;
   
      default:
        mVolumeUnit = VolumeUnitName[ml];
        success = false;
      }
   
    return success;*/
}

std::string CModel::getVolumeUnitName() const
  {
    return VolumeUnitNames[mVolumeUnit];
  }

CModel::VolumeUnit CModel::getVolumeUnitEnum() const
  {
    return mVolumeUnit;
  }

//****

bool CModel::setTimeUnit(const std::string & name)
{
  int unit = toEnum(name.c_str(), TimeUnitNames);

  if (-1 == unit)
    return setTimeUnit(s);
  else
    return setTimeUnit((CModel::TimeUnit) unit);
}

bool CModel::setTimeUnit(const CModel::TimeUnit & unit)
{
  mTimeUnit = unit;
  return true;

  //   bool success = true;
  //
  //   switch (unit)
  //     {
  //     case d:
  //     case h:
  //     case min:
  //     case s:
  //     case ms:
  //     case micros:
  //     case ns:
  //     case ps:
  //     case fs:
  //       mTimeUnit = TimeUnitName[unit];
  //       break;
  //
  //     default:
  //       mTimeUnit = TimeUnitName[s];
  //       success = false;
  //}
  //
  //   return success;
}

std::string CModel::getTimeUnitName() const
  {
    return TimeUnitNames[mTimeUnit];
  }

CModel::TimeUnit CModel::getTimeUnitEnum() const
  {
    return mTimeUnit;
  }

//****

bool CModel::setQuantityUnit(const std::string & name)
{
  int unit = toEnum(name.c_str(), QuantityUnitNames);

  if (-1 == unit)
    return setQuantityUnit(mMol);
  else
    return setQuantityUnit((CModel::QuantityUnit) unit);
}

bool CModel::setQuantityUnit(const CModel::QuantityUnit & unit)
{
  bool success = true;
  mQuantityUnit = unit;

  switch (unit)
    {
    case Mol:
      mQuantity2NumberFactor = AVOGADRO;
      break;

    case mMol:
      mQuantity2NumberFactor = AVOGADRO * 1E-3;
      break;

    case microMol:
      mQuantity2NumberFactor = AVOGADRO * 1E-6;
      break;

    case nMol:
      mQuantity2NumberFactor = AVOGADRO * 1E-9;
      break;

    case pMol:
      mQuantity2NumberFactor = AVOGADRO * 1E-12;
      break;

    case fMol:
      mQuantity2NumberFactor = AVOGADRO * 1E-15;
      break;

    case number:
      mQuantity2NumberFactor = 1.0;
      break;

    default:
      mQuantityUnit = number;
      mQuantity2NumberFactor = 1.0;
      success = false;
    }

  mNumber2QuantityFactor = 1.0 / mQuantity2NumberFactor;

  //adapt particle numbers
  C_INT32 i, imax = mMetabolites.size();
  for (i = 0; i < imax; ++i)
    {
      //update particle numbers
      mMetabolites[i]->setInitialConcentration(mMetabolites[i]->getInitialConcentration());
      mMetabolites[i]->setConcentration(mMetabolites[i]->getConcentration());
    }

  return success;
}

std::string CModel::getQuantityUnitName() const
  {
    return QuantityUnitNames[mQuantityUnit];
  }

CModel::QuantityUnit CModel::getQuantityUnitEnum() const
  {
    return mQuantityUnit;
  }

const C_FLOAT64 & CModel::getQuantity2NumberFactor() const
  {return mQuantity2NumberFactor;}

const C_FLOAT64 & CModel::getNumber2QuantityFactor() const
  {return mNumber2QuantityFactor;}

//*****

std::string CModel::getConcentrationUnitName() const
  {
    return getQuantityUnitName() + "/" + getVolumeUnitName();
  }

std::string CModel::getConcentrationRateUnitName() const
  {
    return getQuantityUnitName()
    + "/(" + getVolumeUnitName()
    + "*" + getTimeUnitName() + ")";
  }

std::string CModel::getQuantityRateUnitName() const
  {
    return getQuantityUnitName()
    + "/" + getTimeUnitName();
  }

//**********************************************************************

std::set<std::string> CModel::listReactionsDependentOnMetab(const std::string & key)
{
  std::set<std::string> Keys;
  const CCopasiVectorN<CReaction> & Reactions = getReactions();
  C_INT32 j, jmax = Reactions.size();

  for (j = 0; j < jmax; j++)
    {
      const CCopasiVector <CChemEqElement> &Substrates = Reactions[j]->getChemEq().getSubstrates();
      C_INT32 i, imax = Substrates.size();
      for (i = 0; i < imax; i++)
        if (key == Substrates[i]->getMetaboliteKey())
          Keys.insert(Reactions[j]->getKey());

      const CCopasiVector <CChemEqElement> &Products = Reactions[j]->getChemEq().getProducts();
      imax = Products.size();
      for (i = 0; i < imax; i++)
        if (key == Products[i]->getMetaboliteKey())
          Keys.insert(Reactions[j]->getKey());

      const CCopasiVector <CChemEqElement> &Modifiers = Reactions[j]->getChemEq().getModifiers();
      imax = Modifiers.size();
      for (i = 0; i < imax; i++)
        if (key == Modifiers[i]->getMetaboliteKey())
          Keys.insert(Reactions[j]->getKey());
    }

  return Keys;
}

std::set<std::string> CModel::listReactionsDependentOnCompartment(const std::string & key)
{
  std::set<std::string> compReacKeys, metabReacKeys;
  std::set<std::string>::iterator it, end;

  CCompartment* comp = dynamic_cast< CCompartment *>(GlobalKeys.get(key));
  const CCopasiVectorNS < CMetab > & Metabs = comp->getMetabolites();
  C_INT32 j, jmax = Metabs.size();

  for (j = 0; j < jmax; j++)
    {
      metabReacKeys = listReactionsDependentOnMetab(Metabs[j]->getKey());

      // Visual C++ does not implement insert(iterator first, iterator last)
      // Therfore the following does not compile:
      // compReacKeys.insert(metabReacKeys.begin(), metabReacKeys.end());
      for (it = metabReacKeys.begin(), end = metabReacKeys.end(); it != end; ++it)
        compReacKeys.insert(*it);
    }

  return compReacKeys;
}

std::set<std::string> CModel::listReactionsDependentOnFunction(const std::string & key)
{
  std::set<std::string> reacKeys;
  std::set<std::string>::iterator it, end;

  const CCopasiVectorN<CReaction> & Reactions = getReactions();
  C_INT32 j, jmax = Reactions.size();

  for (j = 0; j < jmax; j++)
    {
      if (key == Reactions[j]->getFunction().getKey())
        reacKeys.insert(Reactions[j]->getKey());
    }

  return reacKeys;
}

std::set<std::string> CModel::listReactionsDependentOnModelValue(const std::string & /* key */)
{
  std::set<std::string> Keys;

  /* :TODO:
  const CCopasiVectorN<CReaction> & Reactions = getReactions();
  C_INT32 j, jmax = Reactions.size();

  for (j = 0; j < jmax; j++)
     {
       const CCopasiVector <CChemEqElement> &Substrates = Reactions[j]->getChemEq().getSubstrates();
       C_INT32 i, imax = Substrates.size();
       for (i = 0; i < imax; i++)
         if (key == Substrates[i]->getMetaboliteKey())
           Keys.insert(Reactions[j]->getKey());

       const CCopasiVector <CChemEqElement> &Products = Reactions[j]->getChemEq().getProducts();
       imax = Products.size();
       for (i = 0; i < imax; i++)
         if (key == Products[i]->getMetaboliteKey())
           Keys.insert(Reactions[j]->getKey());

       const CCopasiVector <CChemEqElement> &Modifiers = Reactions[j]->getChemEq().getModifiers();
       imax = Modifiers.size();
       for (i = 0; i < imax; i++)
         if (key == Modifiers[i]->getMetaboliteKey())
           Keys.insert(Reactions[j]->getKey());
     }
  */

  return Keys;
}

//**********************************************************************

CMetab* CModel::createMetabolite(const std::string & name,
                                 const std::string & compartment,
                                 const C_FLOAT64 & iconc,
                                 const CMetab::Status & status)
{
  unsigned C_INT32 Index;

  if (mCompartments.size() == 0)
    return NULL;

  if (compartment == "")
    Index = 0;
  else if ((Index = mCompartments.getIndex(compartment)) == C_INVALID_INDEX)
    return NULL;

  if (mCompartments[Index]->getMetabolites().getIndex(name) != C_INVALID_INDEX)
    return NULL;

  CMetab * pMetab = new CMetab(name);

  if (!mCompartments[Index]->addMetabolite(pMetab))
    {
      delete pMetab;
      return NULL;
    }

  pMetab->setStatus(status);
  pMetab->setInitialConcentration(iconc);

  if (!mMetabolites.add(pMetab))
    return NULL;

  setCompileFlag();

  return pMetab;
}
bool CModel::removeMetabolite(const std::string & key)
{
  CMetab* pMetabolite =
    dynamic_cast<CMetab *>(GlobalKeys.get(key));

  if (!pMetabolite)
    return false;

  /* Before deleting the metabolite, delete all the reactions that are dependent */
  std::set<std::string> reacKeys = listReactionsDependentOnMetab(key);
  std::set<std::string>::const_iterator it, itEnd = reacKeys.end();
  for (it = reacKeys.begin(); it != itEnd; ++it)
    removeReaction(*it);

  /* Check if metabolite with that name exists */
  unsigned C_INT32 index = mMetabolites.getIndex(pMetabolite);
  if (index == C_INVALID_INDEX)
    return false;

  mMetabolites.remove(index);

  pdelete(pMetabolite);

  setCompileFlag();

  return true;
}

CCompartment* CModel::createCompartment(const std::string & name,
                                        const C_FLOAT64 & volume)
{
  // check if there is already a volume with this name
  if (mCompartments.getIndex(name) != C_INVALID_INDEX)
    return NULL;

  CCompartment * cpt = new CCompartment(name);

  cpt->setInitialVolume(volume);
  cpt->setVolume(volume);

  if (!mCompartments.add(cpt, true))
    {
      delete cpt;
      return NULL;
    }

  //compile();
  mStateTemplate.add(cpt->getKey());

  return cpt;
}

bool CModel::removeCompartment(const std::string & key)
{
  CCompartment *pCompartment =
    dynamic_cast< CCompartment * >(GlobalKeys.get(key));

  if (!pCompartment)
    return false;

  /* Delete the dependent Metabolites before deleting the Compartment */
  const CCopasiVectorNS <CMetab> &Metabs = pCompartment->getMetabolites();

  for (unsigned C_INT32 i = Metabs.size() - 1; i != C_INVALID_INDEX; i--)
    removeMetabolite(Metabs[i]->getKey());

  //Check if Compartment with that name exists
  unsigned C_INT32 index =
    mCompartments.CCopasiVector< CCompartment >::getIndex(pCompartment);
  if (index == C_INVALID_INDEX)
    return false;

  mCompartments.CCopasiVector< CCompartment >::remove(index);

  //compile();

  return true;
}

CReaction* CModel::createReaction(const std::string & name)
{
  if (mSteps.getIndex(name) != C_INVALID_INDEX)
    return NULL;

  CReaction * pReaction = new CReaction(name);

  if (!mSteps.add(pReaction, true))
    {
      delete pReaction;
      return NULL;
    }

  setCompileFlag();
  return pReaction;
}

/*bool CModel::addReaction(const CReaction & reaction)
{
  if (mSteps.getIndex(reaction.getObjectName()) != C_INVALID_INDEX)
    return false;
 
  mSteps.add(reaction);
  mSteps[reaction.getObjectName()]->compile();
 
  setCompileFlag();
  return true;
}*/

bool CModel::removeReaction(const std::string & key)
{
  CReaction * pReaction =
    dynamic_cast< CReaction * >(GlobalKeys.get(key));

  if (!pReaction)
    return false;

  //Check if Reaction exists
  unsigned C_INT32 index =
    mSteps.CCopasiVector< CReaction >::getIndex(pReaction);

  if (index == C_INVALID_INDEX)
    return false;

  mSteps.CCopasiVector< CReaction >::remove(index);

  setCompileFlag();
  mMoieties.resize(0);

  return true;
}

CModelValue* CModel::createModelValue(const std::string & name,
                                      const C_FLOAT64 & value)
{
  // check if there is already a value with this name
  if (mValues.getIndex(name) != C_INVALID_INDEX)
    return NULL;

  CModelValue * cmv = new CModelValue(name);

  cmv->setInitialValue(value);
  cmv->setValue(value);

  if (!mValues.add(cmv, true))
    {
      delete cmv;
      return NULL;
    }

  //compile();
  mStateTemplate.add(cmv->getKey());

  return cmv;
}

bool CModel::removeModelValue(const std::string & key)
{
  CModelValue *pModelValue =
    dynamic_cast< CModelValue * >(GlobalKeys.get(key));

  if (!pModelValue)
    return false;

  /* Before deleting the value, delete all the reactions that are dependent */
  std::set<std::string> reacKeys = listReactionsDependentOnModelValue(key);
  std::set<std::string>::const_iterator it, itEnd = reacKeys.end();
  for (it = reacKeys.begin(); it != itEnd; ++it)
    removeReaction(*it);

  //Check if Value with that name exists
  unsigned C_INT32 index =
    mValues.CCopasiVector< CModelValue >::getIndex(pModelValue);
  if (index == C_INVALID_INDEX)
    return false;

  mValues.CCopasiVector< CModelValue >::remove(index);

  //compile();

  return true;
}

bool CModel::convert2NonReversible()
{
  //TODO check if there are any reversible reactions
  //TODO warn the user
  //TODO tell the gui about changes
  //TODO generate report ?

  bool ret = true;

  std::vector<std::string> reactionsToDelete;

  CReaction *reac0, *reac1, *reac2;
  CReactionInterface ri1, ri2;
  std::string fn, rn1, rn2;

  //CModel* model = dynamic_cast< CModel * >(GlobalKeys.get(objKey));
  //if (!model) return false;

  CCopasiVectorN< CReaction > & steps = this->getReactions();

  unsigned C_INT32 i, imax = steps.size();
  for (i = 0; i < imax; ++i)
    if (steps[i]->isReversible())
      {
        ret = false;
        reac0 = steps[i];
        std::cout << i << "  ";

        //create the two new reactions
        reac1 = new CReaction(/* *reac0, &steps*/);
        rn1 = reac0->getObjectName() + " (forward)";
        reac1->setObjectName(rn1);
        steps.add(reac1);

        reac2 = new CReaction(/* *reac0, &steps*/);
        rn2 = reac0->getObjectName() + " (backward)";
        reac2->setObjectName(rn2);
        steps.add(reac2);

        ri1.initFromReaction(*this, reac0->getKey());
        ri1.setReactionName(rn1);

        ri2.initFromReaction(*this, reac0->getKey());
        ri2.setReactionName(rn2);

        //set the new function
        fn = reac0->getFunction().getObjectName();
        std::cout << fn << "  " << std::endl;

        if (fn == "Mass action (reversible)")
          {
            ri1.setReversibility(false, "Mass action (irreversible)");
            ri2.reverse(false, "Mass action (irreversible)");
          }
        else if (fn == "Constant flux (reversible)")
          {
            ri1.setReversibility(false, "Constant flux (irreversible)");
            ri2.reverse(false, "Constant flux (irreversible)");
          }
        else
          {
            //ri1.setReversibility(false);
            ri2.reverse(false, "Mass action (irreversible)");
          }

        ri1.writeBackToReaction(reac1, *this);
        ri2.writeBackToReaction(reac2, *this);

        //set the kinetic parameters

        if (fn == "Mass action (reversible)")
          {
            reac1->setParameterValue("k1", reac0->getParameterValue("k1"));
            reac2->setParameterValue("k1", reac0->getParameterValue("k2"));
            ret = true;
          }
        else
          {
            reac2->setParameterValue("k1", 0);
          }

        //remove the old reaction
        //mSteps.remove(reac0->getName());
        reactionsToDelete.push_back(reac0->getObjectName());
      }

  imax = reactionsToDelete.size();
  for (i = 0; i < imax; ++i)
    steps.remove(reactionsToDelete[i]);

  //protectedNotify(ListViews::MODEL, ListViews::CHANGE, objKey);

  return ret;
}

//**********************************************************************

void CModel::initObjects()
{
  addObjectReference("Time", mTime, CCopasiObject::ValueDbl);
  addObjectReference("Comments", mComments);
  //  add(&mCompartments);
  //  add(&mMetabolites);
  //  add(&mMetabolitesX);
  //  add(&mMetabolitesInd);
  //  add(&mMetabolitesDep);
  //  add(&mSteps);
  //  add(&mStepsX);
  //  //add(&mStepsInd);
  addVectorReference("Fluxes", mFluxes, CCopasiObject::ValueDbl);
  //  addVectorReference("Reduced Model Fluxes", mFluxesX);
  addVectorReference("Particle Fluxes", mParticleFluxes, CCopasiObject::ValueDbl);
  //  addVectorReference("Reduced Model Scaled Fluxes", mScaledFluxesX);
  // addObjectReference("Transition Time", mTransitionTime);
  addMatrixReference("Stoichiometry", mStoi, CCopasiObject::ValueDbl);
  addMatrixReference("Reduced Model Stoichiometry", mRedStoi, CCopasiObject::ValueDbl);
  // addVectorReference("Metabolite Interchanges", mRowLU);
  // addVectorReference("Reaction Interchanges", mColLU);
  // addMatrixReference("L", mL);
  addMatrixReference("Link Matrix", mLView, CCopasiObject::ValueDbl);
  addObjectReference("Quantity Unit", mQuantityUnit);
  addObjectReference("Quantity Conversion Factor", mQuantity2NumberFactor, CCopasiObject::ValueDbl);
  // addObjectReference("Inverse Quantity Conversion Factor",
  //                    mNumber2QuantityFactor);

  CArrayAnnotation * tmp = new CArrayAnnotation("Stoichiometry(ann)", this, new CCopasiMatrixInterface<CMatrix<C_FLOAT64> >(&mStoi));
  tmp->setOnTheFly(true);
  tmp->setDescription("Stoichiometry Matrix");
  tmp->setDimensionDescription(0, "Metabolites");
  tmp->setDimensionDescription(1, "Reactions");
  tmp->setCopasiVector(0, &mMetabolites);
  tmp->setCopasiVector(1, &mSteps);

  tmp = new CArrayAnnotation("Reduced stoichiometry(ann)", this, new CCopasiMatrixInterface<CMatrix<C_FLOAT64> >(&mRedStoi));
  tmp->setOnTheFly(true);
  tmp->setDescription("Reduced stoichiometry Matrix");
  tmp->setDimensionDescription(0, "Metabolites");
  tmp->setDimensionDescription(1, "Reactions");
  tmp->setCopasiVector(0, &mMetabolitesX);
  tmp->setCopasiVector(1, &mStepsX);

  tmp = new CArrayAnnotation("Link matrix(ann)", this, new CCopasiMatrixInterface<CLinkMatrixView>(&mLView));
  tmp->setOnTheFly(true);
  tmp->setDescription("Link matrix");
  tmp->setDimensionDescription(0, "Metabolites (full system)");
  tmp->setDimensionDescription(1, "Metabolites (reduced system)");
  tmp->setCopasiVector(0, &mMetabolites);
  tmp->setCopasiVector(1, &mMetabolitesX);
}

bool CModel::hasReversibleReaction() const
  {
    unsigned C_INT32 i, imax = mSteps.size();
    for (i = 0; i < imax; ++i) if (mSteps[i]->isReversible()) return true;

    return false;
  }

void CModel::setCompileHandler(CProcessReport* pHandler)
{mpCompileHandler = pHandler;}

CProcessReport* CModel::getCompileHandlerAddr()
{return mpCompileHandler;}

//**********************************************************************
//                   CLinkMatrixView
//**********************************************************************

const CModel::CLinkMatrixView::elementType CModel::CLinkMatrixView::mZero = 0.0;
const CModel::CLinkMatrixView::elementType CModel::CLinkMatrixView::mUnit = 1.0;

CModel::CLinkMatrixView::CLinkMatrixView(const CMatrix< C_FLOAT64 > & A,
    const CCopasiVector< CMetab > & independent):
    mA(A),
    mIndependent(independent)
{CONSTRUCTOR_TRACE;}

CModel::CLinkMatrixView::~CLinkMatrixView()
{DESTRUCTOR_TRACE;}

CModel::CLinkMatrixView &
CModel::CLinkMatrixView::operator = (const CModel::CLinkMatrixView & rhs)
{
  const_cast< CMatrix< C_FLOAT64 > &>(mA) = rhs.mA;
  const_cast< CCopasiVector< CMetab > &>(mIndependent) = rhs.mIndependent;

  return *this;
}

unsigned C_INT32 CModel::CLinkMatrixView::numRows() const
  {return mIndependent.size() + mA.numRows();}

unsigned C_INT32 CModel::CLinkMatrixView::numCols() const
  {return mA.numCols();}

std::ostream &operator<<(std::ostream &os,
                         const CModel::CLinkMatrixView & A)
{
  unsigned C_INT32 i, imax = A.numRows();
  unsigned C_INT32 j, jmax = A.numCols();
  os << "Matrix(" << imax << "x" << jmax << ")" << std::endl;

  for (i = 0; i < imax; i++)
    {
      for (j = 0; j < jmax; j++)
        os << "\t" << A(i, j);
      os << std::endl;
    }
  return os;
}

//**********************************************************************
//          CStateTemplate
//**********************************************************************

CModel::CStateTemplate::CStateTemplate() :
    mList(),
    mKeyMap(),
    mObjectMap()
{}

CModel::CStateTemplate::~CStateTemplate() {cleanup();}

bool CModel::CStateTemplate::cleanup()
{
  std::vector< std::pair< std::string, std::string > * >::iterator it =
    mList.begin();
  std::vector< std::pair< std::string, std::string > * >::iterator End =
    mList.end();

  for (; it != End; ++it)
    if (*it)
      {
        GlobalKeys.remove((*it)->first);
        delete *it;
        *it = NULL;
      }

  mList.clear();
  mKeyMap.clear();
  mObjectMap.clear();

  return true;
}

bool CModel::CStateTemplate::add(const std::string & objectKey)
{
  std::pair< std::string, std::string > * pAdd =
    new std::pair< std::string, std::string >
    (GlobalKeys.add("StateVariable", NULL), objectKey);

  mList.push_back(pAdd);

  mKeyMap[pAdd->first] = &objectKey;
  mObjectMap[objectKey] = &pAdd->first;

  return true;
}

std::string CModel::CStateTemplate::getObjectKey(const std::string & key) const
  {return *(*const_cast<std::map< std::string, const std::string * > *>(&mKeyMap))[key];}

std::string CModel::CStateTemplate::getKey(const std::string & objectKey) const
  {return *(*const_cast<std::map< std::string, const std::string * > *>(&mObjectMap))[objectKey];}

unsigned C_INT32 CModel::CStateTemplate::size() const {return mList.size();}

std::pair< std::string, std::string >
CModel::CStateTemplate::operator[](const unsigned C_INT32 & index) const
  {
    return * mList[index];
  }

std::string CModel::suitableForStochasticSimulation() const
  {
    unsigned C_INT32 i, reactSize = mSteps.size();
    C_INT32 multInt;
    unsigned C_INT32 j;
    C_FLOAT64 multFloat;
    //  C_INT32 metabSize = mMetabolites->size();

    for (i = 0; i < reactSize; i++) // for every reaction
      {
        // TEST getCompartmentNumber() == 1
        //if (mSteps[i]->getCompartmentNumber() != 1) return - 1;

        // TEST isReversible() == 0
        if (mSteps[i]->isReversible() != 0)
          return "At least one reaction is reversible. That means stochastic simulation is not possible. \nYou can try \"Tools|Convert to irreversible\" which will split the reversible reactions \n into two irreversible reactions. However you will have to fix the kinetics afterwards.";

        // TEST integer stoichometry
        // Iterate through each the metabolites
        // juergen: the number of rows of mStoi equals the number of non-fixed metabs!
        //  for (j=0; i<metabSize; j++)
        for (j = 0; j < mStoi.numRows(); j++)
          {
            multFloat = mStoi[j][i];
            multInt = static_cast<C_INT32>(floor(multFloat + 0.5)); // +0.5 to get a rounding out of the static_cast to int!
            if ((multFloat - multInt) > 0.01)
              return "Not all stoichiometries are integer numbers. \nThat means that discrete simulation is not possible.";
          }
      }

    for (i = 0; i < mMetabolites.size(); ++i)
      {
        if (mMetabolites[i]->getInitialNumber() > LLONG_MAX)
          return "At least one particle number in the inial state is too big.";
      }

    return ""; // Model is appropriate for hybrid simulation
  }

#ifdef COPASI_DEBUG
void CModel::check() const
  {
    if (mCompileIsNecessary)
    {std::cout << "******** compile should have been called" << std::endl;}
  }
#endif

void CModel::buildLinkZero()
{
  // Prior to a call to buildLinkZero the stoichiometry matrix mStoi must
  // have been constructed.

  mRedStoi = mStoi;

  C_INT M = mRedStoi.numCols();
  C_INT N = mRedStoi.numRows();
  C_INT LDA = std::max<C_INT>(1, M);

  CVector< C_INT > JPVT(N);
  JPVT = 0;

  unsigned C_INT32 Dim = std::min(M, N);
  if (Dim == 0) return;

  CVector< C_FLOAT64 > TAU(Dim);

  CVector< C_FLOAT64 > WORK(1);
  C_INT LWORK = -1;
  C_INT INFO;

  // QR factorization of the stoichimetry matrix
  /*
   *  -- LAPACK routine (version 3.0) --
   *     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
   *     Courant Institute, Argonne National Lab, and Rice University
   *     June 30, 1999
   *
   *  Purpose
   *  =======
   *
   *  DGEQP3 computes a QR factorization with column pivoting of a
   *  matrix A:  A*P = Q*R  using Level 3 BLAS.
   *
   *  Arguments
   *  =========
   *
   *  M       (input) INTEGER
   *          The number of rows of the matrix A. M >= 0.
   *
   *  N       (input) INTEGER
   *          The number of columns of the matrix A.  N >= 0.
   *
   *  A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)
   *          On entry, the M-by-N matrix A.
   *          On exit, the upper triangle of the array contains the
   *          min(M,N)-by-N upper trapezoidal matrix R; the elements below
   *          the diagonal, together with the array TAU, represent the
   *          orthogonal matrix Q as a product of min(M,N) elementary
   *          reflectors.
   *
   *  LDA     (input) INTEGER
   *          The leading dimension of the array A. LDA >= max(1,M).
   *
   *  JPVT    (input/output) INTEGER array, dimension (N)
   *          On entry, if JPVT(J).ne.0, the J-th column of A is permuted
   *          to the front of A*P (a leading column); if JPVT(J)=0,
   *          the J-th column of A is a free column.
   *          On exit, if JPVT(J)=K, then the J-th column of A*P was the
   *          the K-th column of A.
   *
   *  TAU     (output) DOUBLE PRECISION array, dimension (min(M,N))
   *          The scalar factors of the elementary reflectors.
   *
   *  WORK    (workspace/output) DOUBLE PRECISION array, dimension (LWORK)
   *          On exit, if INFO=0, WORK(1) returns the optimal LWORK.
   *
   *  LWORK   (input) INTEGER
   *          The dimension of the array WORK. LWORK >= 3*N+1.
   *          For optimal performance LWORK >= 2*N+(N+1)*NB, where NB
   *          is the optimal blocksize.
   *
   *          If LWORK = -1, then a workspace query is assumed; the routine
   *          only calculates the optimal size of the WORK array, returns
   *          this value as the first entry of the WORK array, and no error
   *          message related to LWORK is issued by XERBLA.
   *
   *  INFO    (output) INTEGER
   *          = 0: successful exit.
   *          < 0: if INFO = -i, the i-th argument had an illegal value.
   *
   *  Further Details
   *  ===============
   *
   *  The matrix Q is represented as a product of elementary reflectors
   *
   *     Q = H(1) H(2) . . . H(k), where k = min(m,n).
   *
   *  Each H(i) has the form
   *
   *     H(i) = I - tau * v * v'
   *
   *  where tau is a real/complex scalar, and v is a real/complex vector
   *  with v(1:i-1) = 0 and v(i) = 1; v(i+1:m) is stored on exit in
   *  A(i+1:m,i), and tau in TAU(i).
   *
   *  Based on contributions by
   *    G. Quintana-Orti, Depto. de Informatica, Universidad Jaime I, Spain
   *    X. Sun, Computer Science Dept., Duke University, USA
   *
   */

#ifdef DEBUG_MATRIX
  DebugFile << CTransposeView< CMatrix< C_FLOAT64 > >(mRedStoi) << std::endl;
#endif

  dgeqp3_(&M, &N, mRedStoi.array(), &LDA,
          JPVT.array(), TAU.array(), WORK.array(), &LWORK, &INFO);
  if (INFO < 0) fatalError();

  LWORK = WORK[0];
  WORK.resize(LWORK);

  dgeqp3_(&M, &N, mRedStoi.array(), &LDA,
          JPVT.array(), TAU.array(), WORK.array(), &LWORK, &INFO);
  if (INFO < 0) fatalError();

  unsigned C_INT32 i;
  mRowLU.resize(N);
  for (i = 0; i < N; i++)
    mRowLU[i] = JPVT[i] - 1;

  mColLU.resize(M);
  for (i = 0; i < M; i++)
    mColLU[i] = i;

#ifdef DEBUG_MATRIX
  DebugFile << "QR Factorization:" << std::endl;
  DebugFile << "Row permutation:\t" << mRowLU << std::endl;
  DebugFile << "Column permutation:\t" << mColLU << std::endl;
  DebugFile << CTransposeView< CMatrix< C_FLOAT64 > >(mRedStoi) << std::endl;
#endif

  C_INT independent = 0;
  while (independent < Dim &&
         fabs(mRedStoi(independent, independent)) > 100 * DBL_EPSILON) independent++;

  // Resize mL
  mL.resize(N - independent, independent);
  if (N == independent || independent == 0) return;

  /* to take care of differences between fortran's and c's memory  acces,
     we need to take the transpose, i.e.,the upper triangular */
  char cL = 'U';
  char cU = 'N'; /* 1 in the diaogonal of R */

  // Calculate Row Echelon form of R.
  // First invert R_1,1
  /* int dtrtri_(char *uplo,
   *             char *diag, 
   *             integer *n, 
   *             doublereal * A,
   *             integer *lda, 
   *             integer *info);   
   *  -- LAPACK routine (version 3.0) --
   *     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
   *     Courant Institute, Argonne National Lab, and Rice University
   *     March 31, 1993
   *
   *  Purpose
   *  =======
   *
   *  DTRTRI computes the inverse of a real upper or lower triangular
   *  matrix A.
   *
   *  This is the Level 3 BLAS version of the algorithm.
   *
   *  Arguments
   *  =========
   *
   *  uplo    (input) CHARACTER*1
   *          = 'U':  A is upper triangular;
   *          = 'L':  A is lower triangular.
   *
   *  diag    (input) CHARACTER*1
   *          = 'N':  A is non-unit triangular;
   *          = 'U':  A is unit triangular.
   *
   *  n       (input) INTEGER
   *          The order of the matrix A.  n >= 0.
   *
   *  A       (input/output) DOUBLE PRECISION array, dimension (lda,n)
   *          On entry, the triangular matrix A.  If uplo = 'U', the
   *          leading n-by-n upper triangular part of the array A contains
   *          the upper triangular matrix, and the strictly lower
   *          triangular part of A is not referenced.  If uplo = 'L', the
   *          leading n-by-n lower triangular part of the array A contains
   *          the lower triangular matrix, and the strictly upper
   *          triangular part of A is not referenced.  If diag = 'U', the
   *          diagonal elements of A are also not referenced and are
   *          assumed to be 1.
   *          On exit, the (triangular) inverse of the original matrix, in
   *          the same storage format.
   *
   *  lda     (input) INTEGER
   *          The leading dimension of the array A.  lda >= max(1,n).
   *
   *  info    (output) INTEGER
   *          = 0: successful exit
   *          < 0: if info = -i, the i-th argument had an illegal value
   *          > 0: if info = i, A(i,i) is exactly zero.  The triangular
   *               matrix is singular and its inverse can not be computed.
   */
  dtrtri_(&cL, &cU, &independent, mRedStoi.array(), &LDA, &INFO);
  if (INFO < 0) fatalError();

#ifdef DEBUG_MATRIX
  DebugFile << "Invert R_1,1:" << std::endl;
  DebugFile << CTransposeView< CMatrix< C_FLOAT64 > >(mRedStoi) << std::endl;
#endif

  unsigned C_INT32 j, k;

  // Compute Link_0 = inverse(R_1,1) * R_1,2
  C_FLOAT64 * pTmp1 = &mL(0, 0);
  C_FLOAT64 * pTmp2;
  C_FLOAT64 * pTmp3;

  for (j = 0; j < N - independent; j++)
    for (i = 0; i < independent; i++, pTmp1++)
      {
        pTmp2 = &mRedStoi(j + independent, i);
        pTmp3 = &mRedStoi(i, i);

        // assert(&mL(j, i) == pTmp3);
        *pTmp1 = 0.0;

        for (k = i; k < independent; k++, pTmp2++, pTmp3 += M)
          {
            // assert(&mRedStoi(j + independent, k) == pTmp2);
            // assert(&mRedStoi(k, i) == pTmp3);

            *pTmp1 += *pTmp3 * *pTmp2;
          }

        if (fabs(*pTmp1) < 100 * DBL_EPSILON) *pTmp1 = 0.0;
      }

#ifdef DEBUG_MATRIX
  DebugFile << "Link Zero Matrix:" << std::endl;
  DebugFile << mL << std::endl;
#endif // DEBUG_MATRIX

  return;
}

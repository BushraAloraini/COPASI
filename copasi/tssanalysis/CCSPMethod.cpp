// Copyright (C) 2010 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 - 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

//#define CSPDEBUG

#include <math.h>

#include "copasi.h"

#include "CCSPMethod.h"
#include "CTSSAProblem.h"
#include "CTSSATask.h"
#include "CopasiDataModel/CCopasiDataModel.h"
#include "report/CCopasiRootContainer.h"
#include "model/CModel.h"
#include "math/CMathContainer.h"
#include "utilities/CMatrix.h"
#include "utilities/CAnnotatedMatrix.h"
#include "report/CCopasiObjectReference.h"
#include "trajectory/CLsodaMethod.h"

#include "lapack/lapackwrap.h"        // CLAPACK
#include "lapack/blaswrap.h"           // BLAS

CCSPMethod::CCSPMethod(const CCopasiContainer * pParent,
                       const CTaskEnum::Method & methodType,
                       const CTaskEnum::Task & taskType):
  CTSSAMethod(pParent, methodType, taskType)
{
  initializeParameter();

  createAnnotationsM();
}

CCSPMethod::CCSPMethod(const CCSPMethod & src,
                       const CCopasiContainer * pParent):
  CTSSAMethod(src, pParent)
{
  initializeParameter();

  createAnnotationsM();
  //emptyVectors();
}

CCSPMethod::~CCSPMethod()
{}

void CCSPMethod::initializeParameter()
{
  CTSSAMethod::initializeParameter();

  assertParameter("Integrate Reduced Model", CCopasiParameter::BOOL, (bool) true);//->getValue().pBOOL;
  assertParameter("Ratio of Modes Separation", CCopasiParameter::UDOUBLE, (C_FLOAT64) 0.9);
  assertParameter("Maximum Relative Error", CCopasiParameter::UDOUBLE, (C_FLOAT64) 1.0e-3);
  assertParameter("Maximum Absolute Error", CCopasiParameter::UDOUBLE, (C_FLOAT64) 1.0e-6);
  assertParameter("Refinement Iterations Number", CCopasiParameter::UINT, (unsigned C_INT32) 1000);

  //createAnnotationsM();
  //emptyVectors();
}

/* multiply submatrix */
void CCSPMethod::smmult(CMatrix< C_FLOAT64 > & A, CMatrix< C_FLOAT64 > & B, CMatrix< C_FLOAT64 > & C, C_INT & n1, C_INT & n2, C_INT & n3)
{
  C_INT i, j, k;

  for (i = 0; i < n1 ; i++)
    for (j = 0; j < n3; j++)
      {
        C(i, j) = 0.;

        for (k = 0; k < n2; k++)
          C(i, j) += A(i, k) * B(k, j);
      }

  return;
}

/* subtract submatrix */
void CCSPMethod::smsubst(CMatrix< C_FLOAT64 > & A, CMatrix< C_FLOAT64 > & B, CMatrix< C_FLOAT64 > & C, C_INT & n1, C_INT & n2)
{
  C_INT i, j;

  for (i = 0; i < n1 ; i++)
    for (j = 0; j < n2; j++)
      C(i, j) = A(i, j) - B(i, j);

  return;
}

/* add submatrix */
void CCSPMethod::smadd(CMatrix< C_FLOAT64 > & A, CMatrix< C_FLOAT64 > & B, CMatrix< C_FLOAT64 > & C, C_INT & n1, C_INT & n2)
{
  C_INT i, j;

  for (i = 0; i < n1 ; i++)
    for (j = 0; j < n2; j++)
      C(i, j) = A(i, j) + B(i, j);

  return;
}

/* normalize submatrix */
void CCSPMethod::smnorm(C_INT & n, CMatrix< C_FLOAT64 > & A, CMatrix< C_FLOAT64 > & B, C_INT & n1)
{
  C_INT i, j;
  C_FLOAT64 c, d;

  for (j = 0; j < n1 ; j++)
    {
      c = 0.0;

      for (i = 0; i < n ; i++)
        {
          d = fabs(A(i, j));

          if (d > c) c = d;
        }

      for (i = 0; i < n ; i++)
        {
          A(i, j) = A(i, j) / c;
          B(j, i) = B(j, i) * c;
        }
    }

  return;
}

/* perturbate basis */
void CCSPMethod::perturbateA(C_INT & n, CMatrix< C_FLOAT64 > & A, C_FLOAT64 delta)
{
  C_INT i, j;

  for (j = 0; j < n ; j++)
    for (i = 0; i < n ; i++)
      A(i, j) = A(i, j) * delta;

  return;
}

void CCSPMethod::sminverse(C_INT & n, CMatrix< C_FLOAT64 > & A, CMatrix< C_FLOAT64 > & B)
{

  /*       int dgesv_(integer *n, integer *nrhs, doublereal *a, integer
   * *lda, integer *ipiv, doublereal *b, integer *ldb, integer *info)
   *
   *  -- LAPACK driver routine (version 3.0) --
   *     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
   *     Courant Institute, Argonne National Lab, and Rice University
   *     March 31, 1993
   *
   *
   *  Purpose
   *  =======
   *
   *  DGESV computes the solution to a real system of linear equations
   *     A * X = B,
   *  where A is an N-by-N matrix and X and B are N-by-NRHS matrices.
   *
   *  The LU decomposition with partial pivoting and row interchanges is
   *  used to factor A as
   *     A = P * L * U,
   *  where P is a permutation matrix, L is unit lower triangular, and U is
   *  upper triangular.  The factored form of A is then used to solve the
   *  system of equations A * X = B.
   *
   *  Arguments
   *  =========
   *
   *  N       (input) INTEGER
   *          The number of linear equations, i.e., the order of the
   *          matrix A.  N >= 0.
   *
   *  NRHS    (input) INTEGER
   *          The number of right hand sides, i.e., the number of columns
   *          of the matrix B.  NRHS >= 0.
   *
   *  A       (input/output) DOUBLE PRECISION array, dimension (LDA,N)
   *          On entry, the N-by-N coefficient matrix A.
   *          On exit, the factors L and U from the factorization
   *          A = P*L*U; the unit diagonal elements of L are not stored.
   *
   *  LDA     (input) INTEGER
   *          The leading dimension of the array A.  LDA >= max(1,N).
   *
   *  IPIV    (output) INTEGER array, dimension (N)
   *          The pivot indices that define the permutation matrix P;
   *          row i of the matrix was interchanged with row IPIV(i).
   *
   *  B       (input/output) DOUBLE PRECISION array, dimension (LDB,NRHS)
   *          On entry, the N-by-NRHS matrix of right hand side matrix B.
   *          On exit, if INFO = 0, the N-by-NRHS solution matrix X.
   *
   *  LDB     (input) INTEGER
   *          The leading dimension of the array B.  LDB >= max(1,N).
   *
   *
   * INFO    (output) INTEGER
   *          = 0:  successful exit
   *          < 0:  if INFO = -i, the i-th argument had an illegal value
   *          > 0:  if INFO = i, U(i,i) is exactly zero.  The factorization
   *                has been completed, but the factor U is exactly
   *                singular, so the solution could not be computed.
   */

  C_INT info = 0;

  C_INT * ipiv;
  ipiv = new C_INT [n];

  C_INT N = n;

  CMatrix<C_FLOAT64> TMP;
  TMP.resize(N, N);

  TMP = A;

  C_INT i, j;

  for (i = 0; i < N; i++)
    for (j = 0; j < N; j++)
      B(i, j) = 0.;

  for (i = 0; i < N; i++)
    B(i, i) = 1.;

  dgesv_(&N, &N, TMP.array(), &N, ipiv, B.array(), &N, &info);

  if (info != 0)
    {
      return;
    }

  return;
}
#if 0
/* find  the new number of fast  according to the time-scale separation ratio */
/*  TODO : equal,complex  eigenvalues are presenting !!! */
void CCSPMethod::findTimeScaleSeparation(C_INT & n, C_INT & k, CVector< C_FLOAT64 > & eigen, C_INT & info)
{

  C_INT i;
  C_FLOAT64 tmp;

  k = 0;
  i = 0;

  if (mTsc)
    for (i = 0; i < n; i++)
      {
        tmp = 1. / eigen[i];

        if (tmp < 0 && fabs(tmp) < mTsc)
          k++;

        else
          {
            if (tmp > 0) info = 1;

            break;
          }
      }
  else
    for (i = 0; i < n - 1; i++)
      {
        if (eigen[i] != eigen[i + 1])
          {
            tmp = eigen[i + 1 ] / eigen[i];

#if 0

            std::cout << "tsc[" << i << "]/tsc[" << i + 1 << "] " << tmp << std::endl;
            std::cout << "mEps " << mEps << std::endl;
#endif

            if (tmp > 0 && tmp < mEps)
              {
                k++;

                if (i)
                  if (eigen(i) == eigen(i - 1)) k++;
              }
            else
              {
                if (tmp < 0) info = 1;

                break;
              }
          }
        else
          {

#if 0
            std::cout << "the following time scales are equal:  " << std::endl;
            std::cout << "tsc[" << i << "] = tsc[" << i + 1 << "] " << std::endl;
#endif
          }
      }

  return;
}
#endif

/* find  the number of candidates to  fast  according to the time-scale separation ratio */
void CCSPMethod::findCandidatesNumber(C_INT & n, C_INT & k, CVector< C_FLOAT64 > & eigen, C_INT &  info)
{

  C_INT i;
  C_FLOAT64 tmp;

  k = 0;
  i = 0;

  if (eigen[0] == 0.) return;

#ifdef CSPDEBUG

  for (i = 0; i < n ; i++)
    std::cout << "eigen[" << i << "] " << eigen[i]  << std::endl;

  for (i = 0; i < n ; i++)
    std::cout << "tsc[" << i << "]  -" << 1 / eigen[i]  << std::endl;

#endif

  for (i = 0; i < n - 1; i++)
    {

      if (eigen[i + 1] == 0.) return;

      if (eigen[i] != eigen[i + 1])
        {
          tmp = eigen[i + 1 ] / eigen[i];

#ifdef CSPDEBUG

          std::cout << "tsc[" << i << "]/tsc[" << i + 1 << "] " << tmp << std::endl;
          std::cout << "mEps " << mEps << std::endl;
#endif

          if (tmp > 0 && tmp < mEps)
            {
              k++;

#ifdef CSPDEBUG
              std::cout << "k " << k << std::endl;
#endif

              if (i)
                if (eigen[i] == eigen[i - 1]) k++;
            }
          else
            {
              if (tmp < 0) info = 1;

              //if (tmp >= mEps) k++;

              break;
            }
        }
      else
        {

#ifdef CSPDEBUG
          std::cout << "the following time scales are equal:  " << std::endl;
          std::cout << "tsc[" << i << "] = tsc[" << i + 1 << "] " << std::endl;
#endif
        }
    }

#ifdef CSPDEBUG
  std::cout << "k " << k << std::endl;
#endif

  return;
}

void CCSPMethod::cspstep(const double & /* deltaT */, C_INT & N, C_INT & M, CMatrix< C_FLOAT64 > & A, CMatrix< C_FLOAT64 > & B)
{

  C_INT reacs_size = (C_INT) mpContainer->getReactions().size();
  emptyOutputData(N, N, reacs_size);

#ifdef CSPDEBUG
  std::cout << " *********************  New time step **********************" << std::endl;
  std::cout << " mTime " << *mpContainerStateTime << std::endl;

#endif

  CVector< C_FLOAT64 > g(mG.size());
  memcpy(g.array(), mG.array(), mG.size() * sizeof(C_FLOAT64));

  CVector< C_FLOAT64 > y = mY;

  CMatrix< C_FLOAT64 > A0;
  CMatrix< C_FLOAT64 > B0;
  CMatrix< C_FLOAT64 > J;

  g.resize(N);
  y.resize(N);

  A0.resize(N, N);
  B0.resize(N, N);
  J.resize(N, N);

  C_INT i, j;
  C_FLOAT64 * pSpeciesValue = mY.array();

  for (j = 0; j < N; j++, ++pSpeciesValue)
    {
      const CParticleReference* reference = static_cast<const CParticleReference*>(mpContainer->getMathObject(pSpeciesValue)->getDataObject());
      const CMetab * pSpeciesObject = static_cast<const CMetab *>(reference->getObjectParent());
      const CCompartment* comp = pSpeciesObject->getCompartment();

      mYerror[j] = mRerror * y[j] + mAerror * comp->getInitialValue();
    }

  J = mJacobian;

#ifdef CSPDEBUG
  std::cout << "current Jacobian " << std::endl;
  std::cout << J << std::endl;
#endif

  CMatrix<C_FLOAT64> ALA;
  CMatrix<C_FLOAT64> F;

  ALA.resize(N, N);
  F.resize(N, 1);

  /* csp iterations */
  C_INT iter = 0;

  CMatrix<C_FLOAT64> QF;
  CMatrix<C_FLOAT64> QSL;

  QF.resize(N, N);
  QSL.resize(N, N);

  mJacobian_initial.resize(N, N);
  mQ.resize(N, N);
  mR.resize(N, N);

  mQ = 0.;
  mR = 0.;

  mJacobian_initial = J;

  C_INT info = 0;

  schur(info);

  if (info)
    {
      CCopasiMessage(CCopasiMessage::WARNING,
                     MCTSSAMethod + 9, *mpContainerStateTime);
      return;
    }

  /* trial basis vectors */

  /* use the matrix of Schur vectors */

  A0 = mQ;
  B0 = 0;

  A = A0;
  B = B0;

  //perturbateA(N, A0, 0.99); // TEST

  smnorm(N, A0, B0, N);
  sminverse(N, A0, B0);

  /* ordered real parts of Eigen values */

  CVector<C_FLOAT64> eigen;
  CVector<C_FLOAT64> tsc;

  eigen.resize(N);
  tsc.resize(N);

  for (i = 0; i < N; i++)
    {
      eigen[i] = mR(i, i);

      tsc[i] = 1. / eigen[i];
    }

#ifdef CSPDEBUG
  std::cout << "CSP iteration:  " << std::endl;
  std::cout << "Eigen values ordered by increasing " << std::endl;

  for (i = 0; i < N; i++)
    std::cout << "eigen[" << i << "]  " << eigen[i] << std::endl;

#endif

#ifdef CSPDEBUG
  std::cout << "time scales :  " << std::endl;

  for (i = 0; i < N; i++)
    std::cout << fabs(tsc[i]) << std::endl;

#endif

  /* find the number of candidate to fast   */

  info = 0;

  //findTimeScaleSeparation(N, M, eigen, info);

  findCandidatesNumber(N, M, eigen, info);

#ifdef CSPDEBUG
  std::cout << " Candidates number " << M << std::endl;
#endif

  if (info)
    {
      /*  the fastest of slow modes has positive eigen value  */
      CCopasiMessage(CCopasiMessage::WARNING,
                     MCTSSAMethod + 15, *mpContainerStateTime);
    }

  if (M == N)
    {

      /* this case is not possible : */
      /*  if the ratio of time scale separation nearly 1, the time scale */
      /*  considered as slow */
#ifdef CSPDEBUG
      std::cout << "After time scales separation :  " << std::endl;
      std::cout << "the number of candidates to  fast modes is equal to the total number of modes" << std::endl;
#endif

      return;
    }

  if (M == 0)
    {

      /* After time scales separation : */
      /* no candidates to  fast modes */

      CCopasiMessage(CCopasiMessage::WARNING,
                     MCTSSAMethod + 12, *mpContainerStateTime);

      return;
    }

analyseMmodes:

#ifdef CSPDEBUG
  std::cout << " ************************************** Number of candidates to fast " << M << " ************************" << std::endl;
#endif

  iter = 0;

  A = A0;
  B = B0;

  /*   */
  /* ALA = B*J*A  */

  CMatrix<C_FLOAT64> TMP;
  TMP.resize(N, N);

#if 0
  CMatrix<C_FLOAT64> DBDT;

  DBDT.resize(N, N);

  DBDT = 0.;

  if (mTStep)
    for (i = 0; i < N; i++)
      for (j = 0; j < N; j++)
        {
          DBDT(i, j) = (B(i, j) - mB(i, j)) / deltaT;
        }

#ifdef CSPDEBUG
  std::cout << "time derivatives of B " << std::endl;
  std::cout << DBDT << std::endl;
#endif
#endif

  smmult(B, J, TMP, N, N, N);

#if 0
  /* TEST: time derivatives are present  */

  if (mTStep)
    for (i = 0; i < N; i++)
      for (j = 0; j < N; j++)
        {
          TMP(i, j) += DBDT(i, j);
        }

#endif

  smmult(TMP, A, ALA, N, N, N);

#ifdef CSPDEBUG
  std::cout << "B*J*A should converge to block-diagonal for an ideal basis:" << std::endl;
  std::cout << ALA << std::endl;

  std::cout << "considered time resolution of the solution  " << std::endl;
  std::cout << fabs(tsc[M]) << std::endl;   // to check this
#endif

  CMatrix<C_FLOAT64> TAUM;
  CMatrix<C_FLOAT64> ALAM;

  ALAM.resize(M, M);
  TAUM.resize(M, M);

  TAUM = 0;
  ALAM = 0;

  for (i = 0; i < M; i++)
    for (j = 0; j < M; j++)
      ALAM(i, j) = ALA(i, j);

  if (M > 1)
    sminverse(M, ALAM, TAUM);
  else
    TAUM(0, 0) = 1. / ALA(0, 0);

#if 1
  modesAmplitude(N, M, g, B, F);

#ifdef CSPDEBUG

  std::cout << " scaled amplitudes via  trial basis :  " << std::endl;

  for (i = 0; i < M; i++)
    {

      std::cout << F(i, 0) << std::endl;
    }

#endif

#ifdef CSPDEBUG
  std::cout << "  |A(i,m) * F(m,0) * tsc[M - 1]| , mYerror[i] " << std::endl;

  C_FLOAT64 tmp;

  for (j = 0; j < M; j++)
    {
      std::cout << " m " << j << std::endl;

      for (i = 0; i < N; i++)
        {
          tmp = fabs(A(i, j) * F(j, 0) * tsc[M - 1]);
          std::cout << A(i, j) << " * " << F(j, 0) << " * " << tsc[M - 1] << " = " << tmp << "        " << mYerror[i] << std::endl;
        }
    }

#endif

#endif

cspiteration:

  emptyOutputData(N, M, reacs_size);

#ifdef CSPDEBUG
  std::cout << "*********************************** CSP refinement iteration " << iter << "*******************************" << std::endl;
#endif

  CMatrix<C_FLOAT64> A1;
  CMatrix<C_FLOAT64> B1;

  A1.resize(N, N);
  B1.resize(N, N);

  basisRefinement(N, M, ALA, TAUM, A, B, A1, B1);

  /* recompute ALA  */

  ALA = 0;
  TMP = 0;

  smmult(B1, J, TMP, N, N, N);

#if 0
  DBDT = 0.;

  if (mTStep)
    for (i = 0; i < N; i++)
      for (j = 0; j < N; j++)
        {
          DBDT(i, j) = (B1(i, j) - mB(i, j)) / deltaT;
        }

#ifdef CSPDEBUG
  std::cout << "time derivatives of B " << std::endl;
  std::cout << DBDT << std::endl;
#endif

  /* TEST: time derivatives are present  */

  if (mTStep)
    for (i = 0; i < N; i++)
      for (j = 0; j < N; j++)
        {
          TMP(i, j) += DBDT(i, j);
        }

#endif

  smmult(TMP, A1, ALA, N, N, N);

#ifdef CSPDEBUG
  std::cout << "B1*J*A1 :   " << std::endl;
  std::cout << ALA << std::endl;
#endif

  C_INT result;
  result = isBlockDiagonal(N, M, ALA, 1.e-8);
#ifdef CSPDEBUG
  std::cout << " iteration result:   " << std::endl;
  std::cout << " 0 - convergence is achieved, 1 - convergence is  not achieved, -1  - iterations crucially disconverged " << std::endl;
  std::cout << result << std::endl;
#endif

  switch (result)
    {
      case 0: // iterations converged

        if (modesAreExhausted(N, M, tsc[M - 1], tsc[M] , g, A1, B1, F))
          {

#ifdef CSPDEBUG

            std::cout << " the M  modes are exhausted, M = " <<  M  << std::endl;
#endif

            //mAmplitude.resize(M);

            for (j = 0; j < N; j++)
              //for (j = 0; j < M; j++)
              mAmplitude[j] = F(j, 0);

            CSPradicalPointer(N, M, A1, B1);
            /**
            * compute  CSP Participation Index:
            * a measure of participation of the r-th elementary reaction to the balancing act of the i-th mode
            * It is assumed that forward and reverse reactions are counted as distinct
            *
            **/

            CSPParticipationIndex(N, M, tsc[M], B1); //NEW

            /**
             * compute CSP Importance Index :
             * a measure of relative importance  of the contribution of r-th elementary reaction
             * to the current reaction rate of the i-th species
             *
             **/

            /*  fast subspace projection matrix */
            smmult(A1, B1, QF, N, M, N);

            /* slow subspace projection matrix */
            smsubst(mI, QF, QSL, N, N);

            CSPImportanceIndex(N, tsc[M], QSL);

#ifdef CSPDEBUG
            CSPOutput(N, M, reacs_size);
#endif

            //setVectors(M);

            A = A1;
            B = B1;

            return;
          }
        else if (M > 1)
          {

            M --;

            if (tsc[M] == tsc[M - 1]) M --;

#ifdef CSPDEBUG
            std::cout << " iterations converged, but the criterion is not sutisfied for the given absolute an relative errors, the new number of candidates to fast  " << M << std::endl;
#endif

            if (M) goto analyseMmodes;
            else
              {

                /* No any fast exhausted modes was found on this time step */
                CCopasiMessage(CCopasiMessage::WARNING,
                               MCTSSAMethod + 12, *mpContainerStateTime);

                return;
              }
          }

        break;

      case 1: // next iteration

        if (iter < mIter)
          {

            modesAmplitude(N, M, g, B1, F);
//#ifdef  CSPDEBUG
#if 0

            std::cout << "scaled amplitudes via refined basis :  " << std::endl;

            for (i = 0; i < M; i++)
              {

                std::cout << F(i, 0) << std::endl;
              }

            std::cout << "  |A(i,m) * F(m,0) * tsc[M - 1]| , mYerror[i] " << std::endl;

            C_FLOAT64 tmp;

            for (j = 0; j < M; j++)
              {
                std::cout << " m " << j << std::endl;

                for (i = 0; i < N; i++)
                  {
                    tmp = fabs(A1(i, j) * F(j, 0) * tsc[M - 1]);
                    std::cout << A1(i, j) << " * " << F(j, 0) << " * " << tsc[M - 1] << " = " << tmp << "        " << mYerror[i] << std::endl;
                  }
              }

#endif
            iter ++;
            A = A1;
            B = B1;

            goto cspiteration;
          }
        else  if (iter >= mIter)
          {
            if (M > 1)
              {
                M --;

                if (tsc[M] == tsc[M - 1]) M --;

                if (M) goto analyseMmodes;
                else
                  {

                    /* No any fast exhausted modes was found on this time step */
                    CCopasiMessage(CCopasiMessage::WARNING,
                                   MCTSSAMethod + 12, *mpContainerStateTime);
                    return;
                  }
              }
          }

        break;

      case -1: //  iterations crucially disconverged

        if (M > 1)
          {
            M --;

            if (tsc[M] == tsc[M - 1]) M --;

            if (M) goto analyseMmodes;
            else
              {

                /* No any fast exhausted modes was found on this time step */

                CCopasiMessage(CCopasiMessage::WARNING,
                               MCTSSAMethod + 12, *mpContainerStateTime);
                return;
              }
          }

        break;

      default:
        /* No any fast exhausted modes was found on this time step */
        M = 0;

        CCopasiMessage(CCopasiMessage::WARNING,
                       MCTSSAMethod + 12, *mpContainerStateTime);
        break;
    }

  /* No any fast exhausted modes was found on this time step */

  M = 0;

  CCopasiMessage(CCopasiMessage::WARNING,
                 MCTSSAMethod + 12, *mpContainerStateTime);

  return;
}
/*  compute  the norm C  of the off-diagonal blocks   */
C_INT CCSPMethod::isBlockDiagonal(C_INT & N, C_INT & M, CMatrix< C_FLOAT64 > & ALA, C_FLOAT64 SMALL)
{
  C_INT i, j, imax, jmax, imaxl, jmaxl;
  C_FLOAT64 max = -1., maxl = -1.;
#ifdef CSPDEBUG
  std::cout << "blocks of ALA : " << std::endl;

  std::cout << "upper - left : " << std::endl;

  for (i = 0; i < M; i++)
    {
      for (j = 0 ; j < M; j++)
        std::cout << ALA(i, j) << "  ";

      std::cout << std::endl;
    }

  std::cout << "upper - right : " << std::endl;

  for (i = 0; i < M; i++)
    {
      for (j = M ; j < N; j++)
        std::cout << ALA(i, j) << "  ";

      std::cout << std::endl;
    }

  std::cout << "low - left : " << std::endl;

  for (i = M; i < N; i++)
    {
      for (j = 0 ; j < M; j++)
        std::cout << ALA(i, j) << "  ";

      std::cout << std::endl;
    }

  std::cout << "low - right : " << std::endl;

  for (i = M; i < N; i++)
    {
      for (j = M ; j < N; j++)
        std::cout << ALA(i, j) << "  ";

      std::cout << std::endl;
    }

#endif
  /* step #1: upper-right block */

  for (i = 0; i < M; i++)
    for (j = M ; j < N; j++)
      if (fabs(ALA(i, j)) > max)
        {
          max = fabs(ALA(i, j));
          imax = i; jmax = j;
        }

#if 1
  /* step #2: lower-left block */

  for (i = M; i < N; i++)
    for (j = 0 ; j < M; j++)
      if (fabs(ALA(i, j)) > maxl)
        {
          maxl = fabs(ALA(i, j));
          imaxl = i ; jmaxl = j;
        }

#ifdef CSPDEBUG
  std::cout << "norm C of the lower-left block of ALA is ALA(" << imaxl << "," << jmaxl << ") = " << maxl << std::endl;
  std::cout << "the low-left block : " << maxl << std::endl;
#endif

#endif

#ifdef CSPDEBUG
  std::cout << "the norm C of the upper-right block of ALA is ALA(" << imax << "," << jmax << ") = " << max << std::endl;
  std::cout << "the upper-right block : " << max << std::endl;
#endif

  C_INT result;

  result = 1;

  if (fabs(max) >= std::numeric_limits< C_FLOAT64 >::max() || fabs(maxl) >= std::numeric_limits< C_FLOAT64 >::max() || max < 0 || maxl < 0)
    {
#ifdef CSPDEBUG
      std::cout <<  " iterations  crucially  disconverged  " << std::endl;
#endif

      result =  -1;
    }
  else if (max <= SMALL) result = 0;

  return result;
}
void CCSPMethod::emptyOutputData(C_INT & N, C_INT & M, C_INT & R)
{

  C_INT i, m, r;
  //const CCopasiVector< CReaction > & reacs = Model.getReactions();

  for (m = 0; m < M; m++)
    for (i = 0; i < N; i++)
      {
        mAmplitude[i] = 0.;
        mRadicalPointer(i, m) = 0;
      }

  for (m = 0; m < M; m++)
    for (r = 0; r < R; r++)
      mFastReactionPointer(r, m) = 0;

  for (m = 0; m < M; m++)
    for (r = 0; r < R; r++)
      mFastReactionPointerNormed(r, m) = 0;

  for (i = 0; i < N; i++)
    for (r = 0; r < R; r++)
      mParticipationIndex(r, i) = 0;

  for (i = 0; i < N; i++)
    for (r = 0; r < R; r++)
      mParticipationIndexNormedRow(r, i) = 0;

  for (i = 0; i < N; i++)
    for (r = 0; r < R; r++)
      mParticipationIndexNormedColumn(r, i) = 0;

  for (r = 0; r < R; r++)
    mFastParticipationIndex[r] = 0;

  for (r = 0; r < R; r++)
    mSlowParticipationIndex[r] = 0;

  for (i = 0; i < N; i++)
    for (r = 0; r < R; r++)
      mImportanceIndex(r, i) = 0;

  for (i = 0; i < N; i++)
    for (r = 0; r < R; r++)
      mImportanceIndexNormedRow(r, i) = 0;

  return;
}

void CCSPMethod::step(const double & deltaT)
{
  C_INT N = mDim;

  C_INT M = 0;

  CMatrix<C_FLOAT64> A;
  CMatrix<C_FLOAT64> B;

  A.resize(N, N);
  B.resize(N, N);

  C_INT i, j;

  for (i = 0; i < N; i++)
    for (j = 0; j < N; j++)
      {
        A(i, j) = 0;
        B(i, j) = 0;
      }

  mpContainer->updateSimulatedValues(mReducedModel);
  mpContainer->calculateJacobian(mJacobian, 1e-6, mReducedModel);

  cspstep(deltaT, N, M, A, B);

  mB = B;
  mTStep = 1;

  if (M > 0)
    setVectors(M);
  else
    setVectorsToNaN();

  setAnnotationM(mCurrentStep); // need for ploting

  mCurrentStep += 1;

  /* integrate one time step */

  integrationStep(deltaT);

  return;
}

void CCSPMethod::start()
{
  CTSSAMethod::start();

  mReducedModel = getValue< bool >("Integrate Reduced Model");
  mpLsodaMethod->setValue("Integrate Reduced Model", mReducedModel);

  if (!mReducedModel)
    {
      mDim += mpContainer->getCountDependentSpecies();
    }

  emptyVectors();

  integrationMethodStart();

  /* CSP related staff */

  mY.initialize(mDim, mpFirstSpecies);
  mG.initialize(mDim, mpFirstSpeciesRate);

  mYerror.resize(mDim);
  mEps = getValue< C_FLOAT64 >("Ratio of Modes Separation");
  mRerror = getValue< C_FLOAT64 >("Maximum Relative Error");
  mAerror = getValue< C_FLOAT64 >("Maximum Absolute Error");
  mIter = getValue< unsigned C_INT32 >("Refinement Iterations Number");

  mpContainer->updateSimulatedValues(mReducedModel);

  mI.resize(mDim, mDim);
  mB.resize(mDim, mDim);

  mI = 0.0;
  mB = 0.0;

  C_INT i;

  for (i = 0; i < mDim; i++)
    mI(i, i) = 1.;

  mTStep = 0;
  mCSPbasis = 0;

  mAerror /= mpContainer->getModel().getNumber2QuantityFactor();

  /*  CSP Output  */

  size_t reacs_size = mpContainer->getReactions().size();

  mAmplitude.resize(mDim);
  mRadicalPointer.resize(mDim, mDim);
  mParticipationIndex.resize(reacs_size, mDim);
  mImportanceIndex.resize(reacs_size, mDim);
  mFastReactionPointer.resize(reacs_size, mDim);

  mParticipationIndexNormedRow.resize(reacs_size, mDim);
  mParticipationIndexNormedColumn.resize(reacs_size, mDim);

  mFastParticipationIndex.resize(reacs_size);
  mSlowParticipationIndex.resize(reacs_size);

  mImportanceIndexNormedRow.resize(reacs_size, mDim);
  mFastReactionPointerNormed.resize(reacs_size, mDim);

  CCopasiVector<CMetab>  metabs;
  metabs.resize(mDim);

#if 0
  CModelEntity* tmp;

  for (j = 0; j < mDim; j++)
    {

      tmp = Model.getStateTemplate().beginIndependent()[j];

      CMetab* metab = dynamic_cast< CMetab * >(tmp);

      metabs[j] = metab;
    }

  mImportanceIndexTab.resize(reacs_size, mDim);
  pImportanceIndexAnn->resize();
  pImportanceIndexAnn->setCopasiVector(0, &Model.getReactions());
  pImportanceIndexAnn->setCopasiVector(1, &metabs);

  mFastParticipationIndexTab.resize(reacs_size, 1);
  pFastParticipationIndexAnn->resize();
  pFastParticipationIndexAnn->setCopasiVector(0, &Model.getReactions());

  mSlowParticipationIndexTab.resize(reacs_size, 1);
  pSlowParticipationIndexAnn->resize();
  pSlowParticipationIndexAnn->setCopasiVector(0, &Model.getReactions());

  mVec_mFastParticipationIndex[0].resize(reacs_size, 1);
  mVec_mSlowParticipationIndex[0].resize(reacs_size, 1);
  mVec_mImportanceIndex[0].resize(reacs_size, mDim);

  mImportanceIndexTab = mVec_mImportanceIndex[0];
  mFastParticipationIndexTab = mVec_mFastParticipationIndex[0];
  mSlowParticipationIndexTab = mVec_mSlowParticipationIndex[0];
#endif

  mSetVectors = 0;

  return;
}

void CCSPMethod::CSPOutput(C_INT & N, C_INT & M, C_INT & R)
{
  const CModel & Model = mpContainer->getModel();

  C_INT i, m, r;
  const CCopasiVector< CReaction > & reacs = Model.getReactions();

  std::cout << "Amplitudes of reaction modes :" << std::endl;

  for (m = 0; m < M; m++)
    {
      std::cout << "reaction mode " << m << " :" << std::endl;

      for (i = 0; i < N; i++)
        {
          std::cout << " mode  " << i << "  : " << mAmplitude[i];

          std::cout << std::endl;
        }

      std::cout << std::endl;
      std::cout << "Radical Pointer: whenever is not a small number, species k is said to be CSP radical" << std::endl;

      for (i = 0; i < N; i++)
        std::cout <<
                  Model.getStateTemplate().beginIndependent()[i]->getObjectName()
                  << "   : " << mRadicalPointer(i, m) << std::endl;
    }

  std::cout << std::endl;
  std::cout << " Fast Reaction Pointer of the m-th reaction  mode : whenever is not a small number, " << std::endl;
  std::cout << " the r-th reaction is said to be a fast reaction  " << std::endl;

  for (m = 0; m < M; m++)
    {
      std::cout << "reaction mode " << m << " :" << std::endl;

      for (r = 0; r < R; r++)
        std::cout << reacs[r].getObjectName() << " :" << mFastReactionPointer(r, m) << std::endl;
    }

  std::cout << std::endl;
  std::cout << " Participation Index : is a mesure of participation of the r-th elementary reaction " << std::endl;
  std::cout << " to the balancing act of the i-th mode " << std::endl;

  for (i = 0; i < N; i++)
    {
      std::cout << "reaction mode " << i << " :" << std::endl;

      for (r = 0; r < R; r++)
        std::cout << reacs[r].getObjectName() << " :" << mParticipationIndex(r, i) << std::endl;
    }

  std::cout << std::endl;
  std::cout << " Importance Index: is a mesure of relative importance of the contribution of r-th elementary " << std::endl;
  std::cout << " reaction to the current reaction rate of i-th spiecies   " << std::endl;

  for (i = 0; i < N; i++)
    {
      std::cout <<
                Model.getStateTemplate().beginIndependent()[i]->getObjectName()
                << " :" << std::endl;

      for (r = 0; r < R; r++)
        std::cout << reacs[r].getObjectName() << " :" << mImportanceIndex(r, i) << std::endl;
    }

  return;
}

/* compute  CSP radical pointer and fast reaction pointer */
void CCSPMethod::CSPradicalPointer(C_INT & N, C_INT & M, CMatrix< C_FLOAT64 > & A, CMatrix< C_FLOAT64 > & B)
{
  C_INT i, j, m, r;

  C_INT reacs_size = (C_INT) mpContainer->getReactions().size();
  const CMatrix< C_FLOAT64 > & redStoi = mpContainer->getStoichiometry(mReducedModel);

  CMatrix<C_FLOAT64> A0;
  CMatrix<C_FLOAT64> B0;
  CMatrix<C_FLOAT64> TMP;

  A0.resize(N, reacs_size);
  B0.resize(reacs_size, N);
  TMP.resize(N, reacs_size);

  A0 = 0;
  B0 = 0;

  C_FLOAT64 tmp;

  for (r = 0; r < reacs_size; r++)
    {
      tmp = 0;

      for (i = 0; i < N; i++)
        {
          A0(i, r) = redStoi(i, r);
          tmp += A0(i, r) * A0(i, r);
        }

      for (i = 0; i < N; i++)
        B0(r, i) = A0(i, r) / tmp;
    }

  /*  m-th fast mode projection matrix */

  CMatrix<C_FLOAT64> QM;
  QM.resize(N, N);

  QM = 0.;

  for (m = 0; m < M ; m++)
    {

      CMatrix<C_FLOAT64> Qm;
      Qm.resize(N, N);

      for (i = 0; i < N; ++i)
        for (j = 0; j < N; ++j)
          {
            Qm(i, j) = A(i, m) * B(m, j);
          }

      /**
       * some comments on the Qm matrix: Qm(i,i) , i = 0,1,...,N,
       * is a measure of projection of i-th unit vector in the m-th mode,
       * whenever Qm(i,i) is not a small number, species m is said to be a CSP radical
       **/

      C_FLOAT64 tmp = 0.;

      for (i = 0; i < N ; i++)
        {
          mRadicalPointer(i, m) = Qm(i, i);

          tmp += Qm(i, i);
        }

      /* use stoichiometric vectors to build the fast reaction pointer */

      /**
       * Pmr  is a measure of projection of r-th stoichiometric vector in the m-th mode,
       * whenever Pmr is not a small number, the r-th reaction is said to be a fast reaction
       **/

      for (r = 0; r < reacs_size; r++)
        {
          C_FLOAT64 Pmr = 0.;

          for (i = 0; i < N; i++)
            {
              TMP(i, r) = 0.;

              for (j = 0; j < N; j++)
                TMP(i, r) += Qm(i, j) * A0(j, r);
            }

          for (j = 0; j < N; j++)
            Pmr += B0(r, j) * TMP(j, r);

          mFastReactionPointer(r, m) = Pmr;
        }
    }

  /* the Fast Reaction Pointer normed by column */

  for (m = 0; m < M ; m++)
    {
      C_FLOAT64 sum = 0.;

      for (r = 0; r < reacs_size; r++)
        sum += fabs(mFastReactionPointer(r, m));

      for (r = 0; r < reacs_size; r++)
        mFastReactionPointerNormed(r, m) = 100.*mFastReactionPointer(r, m) / sum;
    }

  return;
}

/**
 * compute  CSP Participation Index:
 * a measure of participation of the r-th elementary reaction to the balancing act of the i-th mode
 * It is assumed that forward and reverse reactions are counted as distinct
 **/
void CCSPMethod::CSPParticipationIndex(C_INT & N, C_INT & M, C_FLOAT64 & tauM1, CMatrix< C_FLOAT64 > & B0)
{
  C_INT i, r, j;
  C_INT reacs_size = (C_INT) mpContainer->getReactions().size();
  const CVector< double > & reac_fluxes = mpContainer->getParticleFluxes();
  const CMatrix< C_FLOAT64 > & redStoi = mpContainer->getStoichiometry(mReducedModel);

  //CVector<C_FLOAT64> flux;
  //flux.resize(reacs_size);

  CMatrix<C_FLOAT64> P;
  P.resize(N, reacs_size);

  CVector<C_FLOAT64> estim;
  estim.resize(N);

  CVector<C_FLOAT64> ampl;
  ampl.resize(N);

  //for (r = 0; r < reacs_size; ++r)
  //  flux[r] = * (C_FLOAT64 *) reacs[r].getParticleFluxObject()->getValuePointer();

  for (i = 0; i < N; ++i)
    {
      ampl[i] = 0;

      for (r = 0; r < reacs_size; ++r)
        {

          P(i, r) = 0;

          for (j = 0; j < N; ++j)
            P(i, r) += B0(i, j) * redStoi(j, r);

          ampl[i] += fabs(P(i, r) * reac_fluxes[r]);
        }

      C_FLOAT64 tmp = 0.0;

      for (j = 0; j < N; ++j)
        {
          tmp += B0(i, j) * mYerror[j];
        }

      estim[i] = fabs(tmp / tauM1);
    }

  for (i = 0; i < N; ++i)
    {

      for (r = 0; r < reacs_size; ++r)
        {
          P(i, r) *= reac_fluxes[r] / (ampl[i] + estim[i]);

          mParticipationIndex(r, i) = P(i, r);
        }
    }

  for (r = 0; r < reacs_size; ++r)
    {
      C_FLOAT64 PI = 0.;

      for (i = 0; i < M; ++i)
        PI += fabs(mParticipationIndex(r, i));

      mFastParticipationIndex[r] = PI;

      for (i = M; i < N; ++i)
        PI += fabs(mParticipationIndex(r, i));

      mFastParticipationIndex[r] /= PI;
      // TEST:  mFastParticipationIndex[r] = std::numeric_limits<C_FLOAT64>::quiet_NaN();

      for (i = M; i < N; ++i)
        mSlowParticipationIndex[r] += fabs(mParticipationIndex(r, i));

      mSlowParticipationIndex[r] /= PI;
    }

  /* the Participation Index normed by column */

  for (i = 0; i < N; ++i)
    {
      C_FLOAT64 sum = 0.;

      for (r = 0; r < reacs_size; ++r)
        sum +=  fabs(mParticipationIndex(r, i));

      for (r = 0; r < reacs_size; ++r)
        mParticipationIndexNormedColumn(r, i) = 100.* mParticipationIndex(r, i) / sum;
    }

  /* the Participation Index normed by row */

  for (r = 0; r < reacs_size; ++r)
    {
      C_FLOAT64 sum = 0.;

      for (i = 0; i < N; ++i)
        sum +=  fabs(mParticipationIndex(r, i));

      for (i = 0; i < N; ++i)
        mParticipationIndexNormedRow(r, i) = 100.* mParticipationIndex(r, i) / sum;
    }

  return;
}

/**
 * compute CSP Importance Index :
 * a measure of relative importance  of the contribution of r-th elementary reaction
 * to the current reaction rate of the i-th species
 **/

void CCSPMethod::CSPImportanceIndex(C_INT & N, C_FLOAT64 & tauM1, CMatrix< C_FLOAT64 > & Q)
{

  C_INT i, r;
  C_INT reacs_size = (C_INT) mpContainer->getReactions().size();
  const CVector< double > & reac_fluxes = mpContainer->getParticleFluxes();
  const CMatrix< C_FLOAT64 > & redStoi = mpContainer->getStoichiometry(mReducedModel);

  //CVector<C_FLOAT64> flux;
  //flux.resize(reacs_size);

  CMatrix<C_FLOAT64> S0;
  S0.resize(N, reacs_size);

  CMatrix<C_FLOAT64> S;
  S.resize(N, reacs_size);

  CMatrix<C_FLOAT64> I;
  I.resize(N, reacs_size);

  CVector<C_FLOAT64> estim;
  estim.resize(N);

  CVector<C_FLOAT64> g;
  g.resize(N);

  S = redStoi;

  smmult(Q, S, S0, N, N, reacs_size);

  //for (r = 0; r < reacs_size; ++r)
  //  flux[r] = * (C_FLOAT64 *) reacs[r].getParticleFluxObject()->getValuePointer();

  for (i = 0; i < N; ++i)
    {
      g[i] = 0;

      for (r = 0; r < reacs_size; ++r)
        g[i] += fabs(S0(i, r) * reac_fluxes[r]);

      estim[i] = fabs(mYerror[i] / tauM1);
    }

  for (i = 0; i < N; ++i)
    {

      for (r = 0; r < reacs_size; ++r)
        {
          I(i, r) = S0(i, r) * reac_fluxes[r] / (g[i] + estim[i]);

          mImportanceIndex(r, i) = I(i, r);
        }
    }

  /* the Importance Index normed by row */

  for (r = 0; r < reacs_size; ++r)
    {
      C_FLOAT64 sum = 0.;

      for (i = 0; i < N; ++i)
        sum +=  fabs(mImportanceIndex(r, i));

      for (i = 0; i < N; ++i)
        mImportanceIndexNormedRow(r, i) = 100.* mImportanceIndex(r, i) / sum;
    }

  return;
}

/* compute  amplitudes of fast and slow modes */
void CCSPMethod::modesAmplitude(C_INT & N, C_INT & /* M */, CVector< C_FLOAT64 > & g, CMatrix< C_FLOAT64 > & B, CMatrix< C_FLOAT64 > & F)
{

  C_INT i, j;

  /* evaluate amplitudes */

  for (i = 0; i < N; i++)
    {
      F(i, 0) = 0.;

      for (j = 0; j < N; j++)

        for (j = 0; j < N; j++)
          {
            F(i, 0) += B(i, j) * g[j];
          }
    }

  return;
}

/* correct for the contribution of the fast time-scales  to y  */
void CCSPMethod::yCorrection(C_INT & N, C_INT & M, CVector< C_FLOAT64 > & y, CMatrix< C_FLOAT64 > & TAUM, CMatrix< C_FLOAT64 > & F, CMatrix< C_FLOAT64 > & A)
{
  CMatrix<C_FLOAT64> TMP;
  CVector<C_FLOAT64> dy;

  TMP.resize(N, M);
  dy.resize(N);

  C_INT i, k;

  /* A*TAU*F  */

  smmult(A, TAUM, TMP, N, M, M);

  for (i = 0; i < N ; i++) dy[i] = 0.;

  for (i = 0; i < N ; i++)
    for (k = 0; k < M; k++)
      dy[i] += TMP(i, k) * F(k, 0);

  /* contribution of  fast time-scales */

  for (i = 0; i < N; i++)
    {
      y[i] -= dy[i];
    }

  return;
}

/* Refinement Procedure :
 * Lamm, Combustion Science and Technology, 1993.
 **/
void CCSPMethod::basisRefinement(C_INT & N, C_INT & M, CMatrix< C_FLOAT64 > & ALA, CMatrix< C_FLOAT64 > & TAU, CMatrix< C_FLOAT64 > & A, CMatrix< C_FLOAT64 > & B, CMatrix< C_FLOAT64 > & A0, CMatrix< C_FLOAT64 > & B0)
{

  C_INT i, j, n, m;

  CMatrix<C_FLOAT64> P;
  CMatrix<C_FLOAT64> Q;

  P.resize(N, N);
  Q.resize(N, N);

  P = 0.;
  Q = 0.;

  for (m = 0; m < M; m++)
    for (j = M; j < N; j++)
      {
        for (n = 0; n < M; n++)
          P(m, j) += TAU(m, n) * ALA(n, j);
      }

  for (j = M ; j < N; j++)
    for (m = 0; m < M; m++)
      {
        for (n = 0; n < M; n++)
          Q(j, m) += ALA(j, n) * TAU(n, m);
      }

  A0 = A;
  B0 = B;

  /* step #1 */

  for (m = 0; m < M; m++)
    for (i = 0; i < N; i++)
      for (j = M ; j < N; j++)
        B0(m, i) += P(m, j) * B(j, i);

  for (i = 0; i < N; i++)
    for (j = M ; j < N; j++)
      for (n = 0; n < M ; n++)
        A0(i, j) -= A(i, n) * P(n, j);

#if 1
  /* step #2  */

  A = A0;
  B = B0;

  for (i = M; i < N; i++)
    for (j = 0; j < N; j++)
      for (n = 0; n < M; n++)
        B0(i, j) -= Q(i, n) * B(n, j);

  for (i = 0; i < N; i++)
    for (m = 0; m < M; m++)
      for (j = M; j < N; j++)
        A0(i, m) += A(i, j) * Q(j, m);

#endif

  //smnorm(N, A0, B0, N);

  return;
}

/* "true" if each  of the analyzed M  modes is exhausted */
bool CCSPMethod::modesAreExhausted(C_INT & N, C_INT & M, C_FLOAT64 & tauM, C_FLOAT64 & /* tauM1 */ , CVector< C_FLOAT64 > & g, CMatrix< C_FLOAT64 > & A, CMatrix< C_FLOAT64 > & B, CMatrix< C_FLOAT64 > & F)
{
  C_INT i, j;
  C_FLOAT64 tmp;

  bool exhausted = true;

  modesAmplitude(N, M, g, B, F);

#ifdef CSPDEBUG
  std::cout << "  |A(i,m) * F(m,0) * tsc[M - 1]| , mYerror[i] " << std::endl;
#endif

  for (j = 0; j < M; j++)
    {

      for (i = 0; i < N; i++)
        {
          tmp = fabs(A(i, j) * F(j, 0) * tauM);

#ifdef CSPDEBUG
          std::cout << A(i, j) << " * " << F(j, 0) << " * " << tauM << " = " << tmp << "        " << mYerror[i] << std::endl;
#endif

          if (tmp >= mYerror[i]) exhausted = false;
        }
    }

  return exhausted;
}

/**
 *  Predefine the CArrayAnnotation for plots
 **/

void CCSPMethod::predefineAnnotation()
{
  const CModel & Model = mpContainer->getModel();

  mReducedModel = getValue< bool >("Integrate Reduced Model");

  C_INT N;

  if (mReducedModel)
    {
      N = Model.getNumIndependentReactionMetabs();
    }
  else
    {
      N = Model.getNumIndependentReactionMetabs() + Model.getNumDependentReactionMetabs();
    }

  CCopasiVector< CMetab >  metabs;
  //metabs.resize(N);

  C_INT j;

  for (j = 0; j < N; j++)
    {
      metabs.add(static_cast< CMetab * >(const_cast< CModelEntity * >(Model.getStateTemplate().beginIndependent()[j])), false);
    }

  mImportanceIndexTab.resize(Model.getReactions().size(),
                             N);

  pImportanceIndexAnn->resize(); // fixed
  pImportanceIndexAnn->setCopasiVector(0, Model.getReactions());
  pImportanceIndexAnn->setCopasiVector(1, metabs);

  mFastParticipationIndexTab.resize(Model.getReactions().size(), 1);
  pFastParticipationIndexAnn->resize();

  pFastParticipationIndexAnn->setCopasiVector(0, Model.getReactions());

  mSlowParticipationIndexTab.resize(Model.getReactions().size(), 1);
  pSlowParticipationIndexAnn->resize();

  pSlowParticipationIndexAnn->setCopasiVector(0, Model.getReactions());
}

/**
 * Create the CArraAnnotations for tables  in the CQTSSAResultSubWidget.
 * Input for each CArraAnnotations is a separate CMatrix.
 **/
void CCSPMethod::createAnnotationsM()
{
  tableNames.erase(tableNames.begin(), tableNames.end());

  std::string name;

  /* this table is not visible  more */
#if 0
  pAmplitudeAnn = new CArrayAnnotation("Amplitude", this,
                                       new CCopasiMatrixInterface<CMatrix<C_FLOAT64> >(&mAmplitudeTab), true);
  pAmplitudeAnn->setMode(0, pTmp1->STRINGS);
  pAmplitudeAnn->setMode(1, pTmp1->STRINGS);
  pAmplitudeAnn->setDescription(" ");
  pAmplitudeAnn->setDimensionDescription(0, "Fast Reaction Modes");
  pAmplitudeAnn->setDimensionDescription(1, "Amplitudes ");
#endif

  name = "Radical Pointer";
  tableNames.push_back(name);

  pRadicalPointerAnn = new CArrayAnnotation("Radical Pointer", this,
      new CCopasiMatrixInterface<CMatrix<C_FLOAT64> >(&mRadicalPointerTab), true);
  pRadicalPointerAnn->setMode(0, CArrayAnnotation::VECTOR);
  pRadicalPointerAnn->setMode(1, CArrayAnnotation::STRINGS);
  pRadicalPointerAnn->setDescription("Radical Pointer: whenever is not a small number, species k is said to be CSP radical ");
  pRadicalPointerAnn->setDimensionDescription(0, "Species");
  pRadicalPointerAnn->setDimensionDescription(1, "Fast Time Scales");

  mapTableToName[name] = pRadicalPointerAnn;

  //
  name = "Fast Reaction Pointer";
  tableNames.push_back(name);

  pFastReactionPointerAnn = new CArrayAnnotation("Fast Reaction Pointer", this,
      new CCopasiMatrixInterface<CMatrix<C_FLOAT64> >(&mFastReactionPointerTab), true);
  pFastReactionPointerAnn->setMode(0, CArrayAnnotation::VECTOR);
  pFastReactionPointerAnn->setMode(1, CArrayAnnotation::STRINGS);
  pFastReactionPointerAnn->setDescription("Fast Reaction Pointer of the m-th reaction  mode : whenever is not a small number, the r-th reaction is said to be a fast reaction");
  pFastReactionPointerAnn->setDimensionDescription(0, "Reactions");
  pFastReactionPointerAnn->setDimensionDescription(1, "Fast Time Scales");

  mapTableToName[name] = pFastReactionPointerAnn;

  //
  name = "Normed Fast Reaction Pointer";
  tableNames.push_back(name);

  pFastReactionPointerNormedAnn = new CArrayAnnotation("Normed Fast Reaction Pointer", this,
      new CCopasiMatrixInterface<CMatrix<C_FLOAT64> >(&mFastReactionPointerNormedTab), true);
  pFastReactionPointerNormedAnn->setMode(0, CArrayAnnotation::VECTOR);
  pFastReactionPointerNormedAnn->setMode(1, CArrayAnnotation::STRINGS);
  pFastReactionPointerNormedAnn->setDescription("Fast Reaction Pointer of the m-th reaction  mode : whenever is not a small number, the r-th reaction is said to be a fast reaction");
  pFastReactionPointerNormedAnn->setDimensionDescription(0, "Reactions");
  pFastReactionPointerNormedAnn->setDimensionDescription(1, "Fast Time Scales");

  mapTableToName[name] = pFastReactionPointerNormedAnn;

  //
  name = "Participation Index";
  tableNames.push_back(name);

  pParticipationIndexAnn = new CArrayAnnotation("Participation Index", this,
      new CCopasiMatrixInterface<CMatrix<C_FLOAT64> >(&mParticipationIndexTab), true);
  pParticipationIndexAnn->setMode(1, CArrayAnnotation::STRINGS);
  pParticipationIndexAnn->setMode(0, CArrayAnnotation::VECTOR);
  pParticipationIndexAnn->setDescription("Participation Index : is a measure of participation of the r-th elementary reaction to the balancing act of the i-th mode");
  pParticipationIndexAnn->setDimensionDescription(0, "Reactions");
  pParticipationIndexAnn->setDimensionDescription(1, "Time Scales");

  mapTableToName[name] = pParticipationIndexAnn;

  //
  name = "Normed Participation Index (by column)";
  tableNames.push_back(name);

  pParticipationIndexNormedColumnAnn = new CArrayAnnotation("Normed Participation Index (by column)", this,
      new CCopasiMatrixInterface<CMatrix<C_FLOAT64> >(&mParticipationIndexNormedColumnTab), true);
  pParticipationIndexNormedColumnAnn->setMode(1, CArrayAnnotation::STRINGS);
  pParticipationIndexNormedColumnAnn->setMode(0, CArrayAnnotation::VECTOR);
  pParticipationIndexNormedColumnAnn->setDescription("Participation Index : is a measure of participation of the r-th elementary reaction to the balancing act of the i-th mode");
  pParticipationIndexNormedColumnAnn->setDimensionDescription(0, "Reactions");
  pParticipationIndexNormedColumnAnn->setDimensionDescription(1, "Time Scales");

  mapTableToName[name] = pParticipationIndexNormedColumnAnn;

  //
  name = "Normed Participation Index (by row)";
  tableNames.push_back(name);

  pParticipationIndexNormedRowAnn = new CArrayAnnotation("Normed Participation Index (by row)", this,
      new CCopasiMatrixInterface<CMatrix<C_FLOAT64> >(&mParticipationIndexNormedRowTab), true);
  pParticipationIndexNormedRowAnn->setMode(1, CArrayAnnotation::STRINGS);
  pParticipationIndexNormedRowAnn->setMode(0, CArrayAnnotation::VECTOR);
  pParticipationIndexNormedRowAnn->setDescription("Participation Index : is a measure of participation of the r-th elementary reaction to the balancing act of the i-th mode");
  pParticipationIndexNormedRowAnn->setDimensionDescription(0, "Reactions");
  pParticipationIndexNormedRowAnn->setDimensionDescription(1, "Time Scales");

  mapTableToName[name] = pParticipationIndexNormedRowAnn;

  //
  name = "Fast Participation Index";
  tableNames.push_back(name);

  pFastParticipationIndexAnn = new CArrayAnnotation("Fast Participation Index", this,
      new CCopasiMatrixInterface<CMatrix<C_FLOAT64> >(&mFastParticipationIndexTab), true);
  pFastParticipationIndexAnn->setMode(0, CArrayAnnotation::VECTOR);
  pFastParticipationIndexAnn->setMode(1, CArrayAnnotation::STRINGS);
  pFastParticipationIndexAnn->setDescription(" Fast Participation Index : is a measure of participation of the r-th elementary reaction to the balancing act of fast modes");
  pFastParticipationIndexAnn->setDimensionDescription(0, "Reactions");
  pFastParticipationIndexAnn->setDimensionDescription(1, " ");

  mapTableToName[name] = pFastParticipationIndexAnn;

  //
  name = "Slow Participation Index";
  tableNames.push_back(name);

  pSlowParticipationIndexAnn = new CArrayAnnotation("Slow Participation Index", this,
      new CCopasiMatrixInterface<CMatrix<C_FLOAT64> >(&mSlowParticipationIndexTab), true);

  pSlowParticipationIndexAnn->setMode(0, CArrayAnnotation::VECTOR);
  pSlowParticipationIndexAnn->setMode(1, CArrayAnnotation::STRINGS);
  pSlowParticipationIndexAnn->setDescription("Slow Participation Index : is a measure of participation of the r-th elementary reaction to the balancing act of slow modes");
  pSlowParticipationIndexAnn->setDimensionDescription(0, "Reactions");
  pSlowParticipationIndexAnn->setDimensionDescription(1, " ");

  mapTableToName[name] = pSlowParticipationIndexAnn;

  //
  name = "Importance Index";
  tableNames.push_back(name);

  pImportanceIndexAnn = new CArrayAnnotation("Importance Index", this,
      new CCopasiMatrixInterface<CMatrix<C_FLOAT64> >(&mImportanceIndexTab), true);
  pImportanceIndexAnn->setMode(1, CArrayAnnotation::VECTOR);
  pImportanceIndexAnn->setMode(0, CArrayAnnotation::VECTOR);
  pImportanceIndexAnn->setDescription("Importance Index: is a measure of relative importance of the contribution of r-th elementary reaction to the current reaction rate of i-th species");
  pImportanceIndexAnn->setDimensionDescription(0, "Reactions");
  pImportanceIndexAnn->setDimensionDescription(1, "Species");

  mapTableToName[name] =  pImportanceIndexAnn;

  //
  name = "Normed Importance Index (by row)";
  tableNames.push_back(name);

  pImportanceIndexNormedRowAnn = new CArrayAnnotation("Normed Importance Index (by row)", this,
      new CCopasiMatrixInterface<CMatrix<C_FLOAT64> >(&mImportanceIndexNormedRowTab), true);
  pImportanceIndexNormedRowAnn->setMode(1, CArrayAnnotation::VECTOR);
  pImportanceIndexNormedRowAnn->setMode(0, CArrayAnnotation::VECTOR);
  pImportanceIndexNormedRowAnn->setDescription("Importance Index: is a measure of relative importance of the contribution of r-th elementary reaction to the current reaction rate of i-th species");
  pImportanceIndexNormedRowAnn->setDimensionDescription(0, "Reactions");
  pImportanceIndexNormedRowAnn->setDimensionDescription(1, "Species");

  mapTableToName[name] = pImportanceIndexNormedRowAnn;
}

/**
 * Set the every CArrayAnnotation for the requested step.
 * Set also the description of CArayAnnotation for both dimensions:
 *    - dimension description could consists of some std::strings
 *      some strings contain the Time Scale values for requested step
 *    - dimension description could consists of arrays of CommonNames
 **/
bool CCSPMethod::setAnnotationM(size_t step)
{
  const CModel & Model = mpContainer->getModel();

  std::string str;
  std::stringstream sstr;
  sstr.str("");
  sstr.clear();
  unsigned C_INT32 i;
  double timeScale;
  unsigned C_INT32 M;

  C_INT N;

  if (mReducedModel)
    {
      N = Model.getNumIndependentReactionMetabs();
    }
  else
    {
      N = Model.getNumIndependentReactionMetabs() + Model.getNumDependentReactionMetabs();
    }

  //TEST : if (step == 0) return false;

  //if (mVec_SlowModes.size() == 0) return false;

  //if (step > mVec_SlowModes.size()) return false;

  // TEST : step -= 1;

  if (mVec_SlowModes.size() <= step)
    {
      return false;
    }

  M = mVec_SlowModes[step];

  // fill pAmplitudeAnn

#if 0
  mAmplitudeTab.resize(mVec_mAmplitude[step].numRows(), 1);

  for (i = 0; i < mVec_mAmplitude[step].numRows(); i++)
    mAmplitudeTab(i, 0) = mVec_mAmplitude[step][i][0];

  pAmplitudeAnn->resize();

  for (i = 0; i < mVec_mAmplitude[step].numRows(); i++)
    {
      timeScale = mVec_TimeScale[step][i];

      if (i < M)
        sstr << "Fast: ";
      else
        sstr << "Slow: ";

      sstr << timeScale;
      str = sstr.str();
      pAmplitudeAnn->setAnnotationString(0, i, str);
      sstr.str("");
      sstr.clear();
    }

  sstr.str("");
  str = sstr.str();
  pAmplitudeAnn->setAnnotationString(1, 0, str);

#endif

  // fill pRadicalPointerAnn
  mRadicalPointerTab.resize(mVec_mRadicalPointer[step].numCols(),
                            mVec_mRadicalPointer[step].numRows());
  mRadicalPointerTab = mVec_mRadicalPointer[step];
  pRadicalPointerAnn->resize();

  CCopasiVector< CMetab >  metabs;
  //FIXED :  metabs.resize(mDim);
  //metabs.resize(N);

  C_INT j;

  // FIXED:  for (j = 0; j < mDim; j++)
  for (j = 0; j < N; j++)
    {
      metabs.add(static_cast< CMetab * >(const_cast< CModelEntity * >(Model.getStateTemplate().beginIndependent()[j])), false);
    }

  pRadicalPointerAnn->setCopasiVector(0, metabs);

#if 0
  std::cout << "metab.size " << mDim << std::endl;
  std::cout << "step " << step << std::endl;
  std::cout << "mVec_mRadicalPointer[step].numCols() " << mVec_mRadicalPointer[step].numCols() << std::endl;
  std::cout << "mVec_mRadicalPointer[step].numRows() " << mVec_mRadicalPointer[step].numRows() << std::endl;
  std::cout << "+++++++++++++++++++++++++++++++++++++++++++ " << std::endl;

#endif

  for (i = 0; i < mVec_mRadicalPointer[step].numCols(); i++)
    {
      timeScale = mVec_TimeScale[step][i];

      if (i < M)
        sstr << "Fast: ";
      else
        sstr << "Slow: ";

      sstr << timeScale;
      str = sstr.str();
      pRadicalPointerAnn->setAnnotationString(1, i, str);
      sstr.str("");
      sstr.clear();
    }

  // fill pFastReactionPointerAnn
  mFastReactionPointerTab.resize(mVec_mFastReactionPointer[step].numCols(),
                                 mVec_mFastReactionPointer[step].numRows());
  mFastReactionPointerTab = mVec_mFastReactionPointer[step];
  pFastReactionPointerAnn->resize();

  for (i = 0; i < mVec_mFastReactionPointer[step].numCols(); i++)
    {
      timeScale = mVec_TimeScale[step][i];

      if (i < M)
        sstr << "Fast: ";
      else
        sstr << "Slow: ";

      sstr << timeScale;
      str = sstr.str();
      pFastReactionPointerAnn->setAnnotationString(1, i, str);
      sstr.str("");
      sstr.clear();
    }

  pFastReactionPointerAnn->setCopasiVector(0, Model.getReactions());

  // fill pFastReactionPointerNormedAnn
  mFastReactionPointerNormedTab.resize(mVec_mFastReactionPointerNormed[step].numCols(),
                                       mVec_mFastReactionPointerNormed[step].numRows());
  mFastReactionPointerNormedTab = mVec_mFastReactionPointerNormed[step];
  pFastReactionPointerNormedAnn->resize();

  for (i = 0; i < mVec_mFastReactionPointerNormed[step].numCols(); i++)
    {
      timeScale = mVec_TimeScale[step][i];

      if (i < M)
        sstr << "Fast: ";
      else
        sstr << "Slow: ";

      sstr << timeScale;
      str = sstr.str();
      pFastReactionPointerNormedAnn->setAnnotationString(1, i, str);
      sstr.str("");
      sstr.clear();
    }

  pFastReactionPointerNormedAnn->setCopasiVector(0, Model.getReactions());

  // fill pParticipationIndexAnn
  mParticipationIndexTab.resize(mVec_mParticipationIndex[step].numCols(),
                                mVec_mParticipationIndex[step].numRows());
  mParticipationIndexTab = mVec_mParticipationIndex[step];
  pParticipationIndexAnn->resize();

  for (i = 0; i < mVec_mParticipationIndex[step].numCols(); i++)
    {
      timeScale = mVec_TimeScale[step][i];

      if (i < M)
        sstr << "Fast: ";
      else
        sstr << "Slow: ";

      sstr << timeScale;
      str = sstr.str();
      pParticipationIndexAnn->setAnnotationString(1, i, str);
      sstr.str("");
      sstr.clear();
    }

  pParticipationIndexAnn->setCopasiVector(0, Model.getReactions());

  // fill pParticipationIndexNormedColumnAnn
  mParticipationIndexNormedColumnTab.resize(mVec_mParticipationIndexNormedColumn[step].numCols(),
      mVec_mParticipationIndexNormedColumn[step].numRows());
  mParticipationIndexNormedColumnTab = mVec_mParticipationIndexNormedColumn[step];
  pParticipationIndexNormedColumnAnn->resize();

  for (i = 0; i < mVec_mParticipationIndexNormedColumn[step].numCols(); i++)
    {
      timeScale = mVec_TimeScale[step][i];

      if (i < M)
        sstr << "Fast: ";
      else
        sstr << "Slow: ";

      sstr << timeScale;
      str = sstr.str();
      pParticipationIndexNormedColumnAnn->setAnnotationString(1, i, str);
      sstr.str("");
      sstr.clear();
    }

  pParticipationIndexNormedColumnAnn->setCopasiVector(0, Model.getReactions());

  // fill pParticipationIndexNormedRowAnn
  mParticipationIndexNormedRowTab.resize(mVec_mParticipationIndexNormedRow[step].numCols(),
                                         mVec_mParticipationIndexNormedRow[step].numRows());
  mParticipationIndexNormedRowTab = mVec_mParticipationIndexNormedRow[step];
  pParticipationIndexNormedRowAnn->resize();

  for (i = 0; i < mVec_mParticipationIndexNormedRow[step].numCols(); i++)
    {
      timeScale = mVec_TimeScale[step][i];

      if (i < M)
        sstr << "Fast: ";
      else
        sstr << "Slow: ";

      sstr << timeScale;
      str = sstr.str();
      pParticipationIndexNormedRowAnn->setAnnotationString(1, i, str);
      sstr.str("");
      sstr.clear();
    }

  pParticipationIndexNormedRowAnn->setCopasiVector(0, Model.getReactions());

  // fill pFastParticipationIndexAnn

  mFastParticipationIndexTab.resize(Model.getReactions().size(), 1);
  mFastParticipationIndexTab = mVec_mFastParticipationIndex[step];
  pFastParticipationIndexAnn->resize();

  sstr << "   ";
  str = sstr.str();
  pFastParticipationIndexAnn->setAnnotationString(1, 0, str);
  pFastParticipationIndexAnn->setCopasiVector(0, Model.getReactions());

  // fill pSlowParticipationIndexAnn

  mSlowParticipationIndexTab.resize(Model.getReactions().size(),1);
  mSlowParticipationIndexTab = mVec_mSlowParticipationIndex[step];
  pSlowParticipationIndexAnn->resize();

  sstr << "   ";
  str = sstr.str();
  pSlowParticipationIndexAnn->setAnnotationString(1, 0, str);
  pSlowParticipationIndexAnn->setCopasiVector(0, Model.getReactions());

  // fill pmImportanceIndexAnn
  mImportanceIndexTab.resize(mVec_mImportanceIndex[step].numCols(),
                             mVec_mImportanceIndex[step].numRows());
  mImportanceIndexTab = mVec_mImportanceIndex[step];
  pImportanceIndexAnn->resize(); // fixed
  pImportanceIndexAnn->setCopasiVector(0, Model.getReactions());
  pImportanceIndexAnn->setCopasiVector(1, metabs);

// fill pmImportanceIndexNormedRowAnn
  mImportanceIndexNormedRowTab.resize(mVec_mImportanceIndexNormedRow[step].numCols(),
                                      mVec_mImportanceIndexNormedRow[step].numRows());
  mImportanceIndexNormedRowTab = mVec_mImportanceIndexNormedRow[step];
  pImportanceIndexNormedRowAnn->resize(); // fixed
  pImportanceIndexNormedRowAnn->setCopasiVector(0, Model.getReactions());
  pImportanceIndexNormedRowAnn->setCopasiVector(1, metabs);

  return true;
}

/**
 *  set vectors to NaN when the reduction was not possible
 **/
void CCSPMethod::setVectorsToNaN()
{

  mVec_TimeScale.push_back(mCurrentStep);
  mVec_TimeScale[mCurrentStep].resize(mDim);
  C_INT i, r, m, fast;
  C_INT reacs_size = (C_INT) mpContainer->getReactions().size();

  fast = mDim;

  for (i = 0; i < mDim; i++)
    mVec_TimeScale[mCurrentStep][i] = -1 / mR(i, i);

  mVec_SlowModes.push_back(mCurrentStep);
  mVec_SlowModes[mCurrentStep] = 0;

  mVec_mRadicalPointer.push_back(mCurrentStep);

  mVec_mRadicalPointer[mCurrentStep].resize(mDim, fast);

  for (m = 0; m < fast; m++)
    for (i = 0; i < mDim; i++)
      mVec_mRadicalPointer[mCurrentStep][i][m] = std::numeric_limits<C_FLOAT64>::quiet_NaN();

  mVec_mFastReactionPointer.push_back(mCurrentStep);

  mVec_mFastReactionPointer[mCurrentStep].resize(reacs_size, fast);

  for (r = 0; r < reacs_size; r++)
    for (i = 0; i < fast; i++)
      mVec_mFastReactionPointer[mCurrentStep][r][i] = std::numeric_limits<C_FLOAT64>::quiet_NaN();

  mVec_mFastReactionPointerNormed.push_back(mCurrentStep);

  mVec_mFastReactionPointerNormed[mCurrentStep].resize(reacs_size, fast);

  for (r = 0; r < reacs_size; r++)
    for (i = 0; i < fast; i++)
      mVec_mFastReactionPointerNormed[mCurrentStep][r][i] = std::numeric_limits<C_FLOAT64>::quiet_NaN();;

  mVec_mParticipationIndex.push_back(mCurrentStep);
  mVec_mParticipationIndex[mCurrentStep].resize(reacs_size, mDim);

  for (r = 0; r < reacs_size; r++)
    for (i = 0; i < fast; i++)
      mVec_mParticipationIndex[mCurrentStep][r][i] = std::numeric_limits<C_FLOAT64>::quiet_NaN();

  mVec_mFastParticipationIndex.push_back(mCurrentStep);
  mVec_mFastParticipationIndex[mCurrentStep].resize(reacs_size, 1);

  for (i = 0; i < reacs_size; i++)
    mVec_mFastParticipationIndex[mCurrentStep][i][0] = std::numeric_limits<C_FLOAT64>::quiet_NaN();

  mVec_mSlowParticipationIndex.push_back(mCurrentStep);
  mVec_mSlowParticipationIndex[mCurrentStep].resize(reacs_size, 1);

  for (i = 0; i < reacs_size; i++)
    mVec_mSlowParticipationIndex[mCurrentStep][i][0] = std::numeric_limits<C_FLOAT64>::quiet_NaN();

  mVec_mParticipationIndexNormedColumn.push_back(mCurrentStep);
  mVec_mParticipationIndexNormedColumn[mCurrentStep].resize(reacs_size, mDim);

  for (r = 0; r < reacs_size; r++)
    for (i = 0; i < fast; i++)
      mVec_mParticipationIndexNormedColumn[mCurrentStep][r][i] = std::numeric_limits<C_FLOAT64>::quiet_NaN();

  mVec_mParticipationIndexNormedRow.push_back(mCurrentStep);
  mVec_mParticipationIndexNormedRow[mCurrentStep].resize(reacs_size, mDim);

  for (r = 0; r < reacs_size; r++)
    for (i = 0; i < fast; i++)
      mVec_mParticipationIndexNormedRow[mCurrentStep][r][i] = std::numeric_limits<C_FLOAT64>::quiet_NaN();

  mVec_mImportanceIndex.push_back(mCurrentStep);
  mVec_mImportanceIndex[mCurrentStep].resize(reacs_size, mDim);

  for (r = 0; r < reacs_size; r++)
    for (i = 0; i < mDim; i++)
      mVec_mImportanceIndex[mCurrentStep][r][i] = std::numeric_limits<C_FLOAT64>::quiet_NaN();

  mVec_mImportanceIndexNormedRow.push_back(mCurrentStep);
  mVec_mImportanceIndexNormedRow[mCurrentStep].resize(reacs_size, mDim);

  for (r = 0; r < reacs_size; r++)
    for (i = 0; i < mDim; i++)
      mVec_mImportanceIndexNormedRow[mCurrentStep][r][i] = std::numeric_limits<C_FLOAT64>::quiet_NaN();

  mCurrentTime.push_back(mCurrentStep);
  mCurrentTime[mCurrentStep] = *mpContainerStateTime;
}
/**
 *upgrade all vectors with values from actually calculation for current step
 **/
void CCSPMethod::setVectors(int fast)
{

  mVec_TimeScale.push_back(mCurrentStep);
  mVec_TimeScale[mCurrentStep].resize(mDim);
  C_INT i, r, m;
  C_INT reacs_size = (C_INT) mpContainer->getReactions().size();

  for (i = 0; i < mDim; i++)
    mVec_TimeScale[mCurrentStep][i] = -1 / mR(i, i);

  mVec_SlowModes.push_back(mCurrentStep);
  mVec_SlowModes[mCurrentStep] = fast;

#if 0
  mVec_mAmplitude.push_back(mCurrentStep);
  //mVec_mAmplitude[mCurrentStep].resize(mAmplitude.size(), 1);
  mVec_mAmplitude[mCurrentStep].resize(fast, 1);

  for (i = 0; i < fast; i++)
    mVec_mAmplitude[mCurrentStep][i][0] = mAmplitude[i];

#endif

  mVec_mRadicalPointer.push_back(mCurrentStep);
  //mVec_mRadicalPointer[mCurrentStep].resize(mRadicalPointer.numCols(), mRadicalPointer.numRows());
  //mVec_mRadicalPointer[mCurrentStep] = mRadicalPointer;

  mVec_mRadicalPointer[mCurrentStep].resize(mDim, fast);

  for (m = 0; m < fast; m++)
    for (i = 0; i < mDim; i++)
      mVec_mRadicalPointer[mCurrentStep][i][m] = mRadicalPointer(i, m);

  mVec_mFastReactionPointer.push_back(mCurrentStep);
  //mVec_mFastReactionPointer[mCurrentStep].resize(mFastReactionPointer.numCols(), mFastReactionPointer.numRows());
  //mVec_mFastReactionPointer[mCurrentStep] = mFastReactionPointer;

  mVec_mFastReactionPointer[mCurrentStep].resize(reacs_size, fast);

  for (r = 0; r < reacs_size; r++)
    for (i = 0; i < fast; i++)
      mVec_mFastReactionPointer[mCurrentStep][r][i] = mFastReactionPointer(r, i);

  mVec_mFastReactionPointerNormed.push_back(mCurrentStep);
  //mVec_mFastReactionPointerNormed[mCurrentStep].resize(mFastReactionPointerNormed.numCols(), mFastReactionPointerNormed.numRows());
  //mVec_mFastReactionPointerNormed[mCurrentStep] = mFastReactionPointerNormed;

  mVec_mFastReactionPointerNormed[mCurrentStep].resize(reacs_size, fast);

  for (r = 0; r < reacs_size; r++)
    for (i = 0; i < fast; i++)
      mVec_mFastReactionPointerNormed[mCurrentStep][r][i] = mFastReactionPointerNormed(r, i);

  mVec_mParticipationIndex.push_back(mCurrentStep);
  mVec_mParticipationIndex[mCurrentStep].resize(mParticipationIndex.numCols(), mParticipationIndex.numRows());
  mVec_mParticipationIndex[mCurrentStep] = mParticipationIndex;

  mVec_mFastParticipationIndex.push_back(mCurrentStep);
  mVec_mFastParticipationIndex[mCurrentStep].resize(mFastParticipationIndex.size(), 1);

  for (i = 0; i < reacs_size; i++)
    mVec_mFastParticipationIndex[mCurrentStep][i][0] = mFastParticipationIndex[i];

  mVec_mSlowParticipationIndex.push_back(mCurrentStep);
  mVec_mSlowParticipationIndex[mCurrentStep].resize(mSlowParticipationIndex.size(), 1);

  for (i = 0; i < reacs_size; i++)
    mVec_mSlowParticipationIndex[mCurrentStep][i][0] = mSlowParticipationIndex[i];

  mVec_mParticipationIndexNormedColumn.push_back(mCurrentStep);
  mVec_mParticipationIndexNormedColumn[mCurrentStep].resize(mParticipationIndexNormedColumn.numCols(), mParticipationIndexNormedColumn.numRows());
  mVec_mParticipationIndexNormedColumn[mCurrentStep] = mParticipationIndexNormedColumn;

  mVec_mParticipationIndexNormedRow.push_back(mCurrentStep);
  mVec_mParticipationIndexNormedRow[mCurrentStep].resize(mParticipationIndexNormedRow.numCols(), mParticipationIndexNormedRow.numRows());
  mVec_mParticipationIndexNormedRow[mCurrentStep] = mParticipationIndexNormedRow;

  mVec_mImportanceIndex.push_back(mCurrentStep);
  mVec_mImportanceIndex[mCurrentStep].resize(mImportanceIndex.numCols(), mImportanceIndex.numRows());
  mVec_mImportanceIndex[mCurrentStep] = mImportanceIndex;

  mVec_mImportanceIndexNormedRow.push_back(mCurrentStep);
  mVec_mImportanceIndexNormedRow[mCurrentStep].resize(mImportanceIndexNormedRow.numCols(), mImportanceIndexNormedRow.numRows());
  mVec_mImportanceIndexNormedRow[mCurrentStep] = mImportanceIndexNormedRow;

  mCurrentTime.push_back(mCurrentStep);
  mCurrentTime[mCurrentStep] = *mpContainerStateTime;
}

/**
 * Empty every vector to be able to fill them with new values for a new calculation.
 * Also nullify the step counter.
 **/
void CCSPMethod::emptyVectors()
{

  mCurrentStep = 0;
  mVec_TimeScale.erase(mVec_TimeScale.begin(), mVec_TimeScale.end());
  mVec_SlowModes.erase(mVec_SlowModes.begin(), mVec_SlowModes.end());

#if 0
  mVec_mAmplitude.erase(mVec_mAmplitude.begin(), mVec_mAmplitude.end());
#endif

  mVec_mRadicalPointer.erase(mVec_mRadicalPointer.begin(), mVec_mRadicalPointer.end());
  mVec_mFastReactionPointer.erase(mVec_mFastReactionPointer.begin(), mVec_mFastReactionPointer.end());
  mVec_mFastReactionPointerNormed.erase(mVec_mFastReactionPointerNormed.begin(), mVec_mFastReactionPointerNormed.end());
  mVec_mParticipationIndexNormedRow.erase(mVec_mParticipationIndexNormedRow.begin(), mVec_mParticipationIndexNormedRow.end());
  mVec_mParticipationIndexNormedColumn.erase(mVec_mParticipationIndexNormedColumn.begin(), mVec_mParticipationIndexNormedColumn.end());
  mVec_mParticipationIndex.erase(mVec_mParticipationIndex.begin(), mVec_mParticipationIndex.end());
  mVec_mFastParticipationIndex.erase(mVec_mFastParticipationIndex.begin(), mVec_mFastParticipationIndex.end());
  mVec_mSlowParticipationIndex.erase(mVec_mSlowParticipationIndex.begin(), mVec_mSlowParticipationIndex.end());
  mVec_mImportanceIndex.erase(mVec_mImportanceIndex.begin(), mVec_mImportanceIndex.end());
  mVec_mImportanceIndexNormedRow.erase(mVec_mImportanceIndexNormedRow.begin(), mVec_mImportanceIndexNormedRow.end());
}

void CCSPMethod::printResult(std::ostream * ostream) const
{
  const CModel & Model = mpContainer->getModel();

  std::ostream & os = *ostream;
  C_INT M, i, m, r, istep = 0;

  C_INT32 stepNumber;
  //double timeScale;

  assert(CCopasiRootContainer::getDatamodelList()->size() > 0);
  CTSSATask* pTask =
    dynamic_cast<CTSSATask *>(&CCopasiRootContainer::getDatamodelList()->operator[](0).getTaskList()->operator[]("Time Scale Separation Analysis"));

  CTSSAProblem* pProblem = dynamic_cast<CTSSAProblem*>(pTask->getProblem());

  stepNumber = (int)mVec_SlowModes.size();

  this->print(&os);

  const CCopasiVector< CReaction > & reacs = Model.getReactions();

  os << std::endl;
  os << " Radical Pointer: whenever is not a small number, species k is said to be CSP radical" << std::endl;
  os << std::endl;

  os << " Fast Reaction Pointer of the m-th reaction  mode : whenever is not a small number, " << std::endl;
  os << " the r-th reaction is said to be a fast reaction  " << std::endl;
  os << std::endl;

  os << " Participation Index : is a measure of participation of the r-th elementary reaction " << std::endl;
  os << " to the balancing act of the i-th mode " << std::endl;
  os << std::endl;

  os << " Importance Index: is a measure of relative importance of the contribution of r-th elementary " << std::endl;
  os << " reaction to the current reaction rate of i-th species   " << std::endl;
  os << std::endl;

  os << " Species : " << std::endl;

  for (i = 0; i < mDim; i++)
    os << Model.getStateTemplate().beginIndependent()[i]->getObjectName() << std::endl;

  os << std::endl;

  os << " Reactions : " << std::endl;

  for (r = 0; r < (C_INT) reacs.size(); r++)
    os << reacs[r].getObjectName() << std::endl;

  os << std::endl;

  os << "%%% Number of fast modes:  " << std::endl;

  for (istep = 0; istep < stepNumber; istep++)
    {

      M = mVec_SlowModes[istep];

      os << std::endl;
      os << "%%%  Time step " << istep + 1  << std::endl;
      os << std::endl;

      os << M  << std::endl;

      os << std::endl;
    }

  os << std::endl;
  os << "%%% Time scales:  " << std::endl;

  for (istep = 0; istep < stepNumber; istep++)
    {

      M = mVec_SlowModes[istep];

      os << std::endl;
      os << "%%%  Time step " << istep + 1  << std::endl;
      os << std::endl;

      for (i = 0; i < mDim; i++)
        {
          os  << mVec_TimeScale[istep][i] << " ";
        }

      os << std::endl;
    }

  os << std::endl;
  os << "% Radical Pointer   " << std::endl;

  CMatrix<C_FLOAT64> RP;
  RP.resize(mDim, mDim);

  for (istep = 0; istep < stepNumber; istep++)
    {

      M = mVec_SlowModes[istep];

      os << std::endl;
      os << "%%%  Time step " << istep + 1  << std::endl;
      os << std::endl;

      for (m = 0; m < M; m++)
        {
          for (i = 0; i < mDim; i++)
            RP(i, m) = mVec_mRadicalPointer[istep][i][m];
        }

      for (m = M; m < mDim; m++)
        {
          for (i = 0; i < mDim; i++)
            RP(i, m) = 0;
        }

      os << std::endl;

      for (i = 0; i <  mDim; i++)
        {
          for (m = 0; m < mDim; m++)
            os << RP(i, m) << " ";

          os << std::endl;
        }

      os << std::endl;
    }

  os << std::endl;
  os << "%%%% Participation Index : " << std::endl;

  for (istep = 0; istep < stepNumber; istep++)
    {

      M = mVec_SlowModes[istep];

      os << std::endl;
      os << "%%%  Time step " << istep + 1  << std::endl;
      os << std::endl;

      //  os <<  istep + 1 << " ";

      for (r = 0; r < (C_INT) reacs.size(); r++)
        {

          for (i = 0; i < mDim; i++)
            os << mVec_mParticipationIndex[istep][r][i] << "   ";

          os << std::endl;
        }

      os << std::endl;
    }

  os << std::endl;
  os << "%%% Importance Index " << std::endl;

  for (istep = 0; istep < stepNumber; istep++)
    {

      M = mVec_SlowModes[istep];

      os << std::endl;
      os << "%%%  Time step " << istep + 1  << std::endl;
      os << std::endl;

      for (r = 0; r < (C_INT) reacs.size(); r++)
        {

          for (i = 0; i < mDim; i++)
            os << mVec_mImportanceIndex[istep][r][i] << "   ";

          os << std::endl;
        }
    }

  return;
}

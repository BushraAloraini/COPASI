/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/utilities/CluX.h,v $
   $Revision: 1.11 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2005/08/30 15:40:58 $
   End CVS Header */

#ifndef COPASI_CluX
#define COPASI_CluX

// Solve system of linear equations Ax = b.
//
//  Typical usage:
//
//    Matrix(double) A;
//    Vector(Subscript) ipiv;
//    Vector(double) b;
//
//    1)  LU_Factor(A,ipiv);
//    2)  LU_Solve(A,ipiv,b);
//
//   Now b has the solution x.  Note that both A and b
//   are overwritten.  If these values need to be preserved,
//   one can make temporary copies, as in
//
//    O)  Matrix(double) T = A;
//    1)  LU_Factor(T,ipiv);
//    1a) Vector(double) x=b;
//    2)  LU_Solve(T,ipiv,x);
//
//   See details below.
//

// right-looking LU factorization algorithm (unblocked)
//
//   Factors matrix A into lower and upper  triangular matrices
//   (L and U respectively) in solving the linear equation Ax=b.
//
//
// Args:
//
// A        (input/output) Matrix(1:M, 1:N)  In input, matrix to be
//                  factored.  On output, overwritten with lower and
//                  upper triangular factors.
//
// row      (output) Vector(1:M)    Pivot vector. Describes how
//                  the rows of A were reordered to increase
//                  numerical stability.
//
// col      (output) Vector(1:N)    Pivot vector. Describes how
//                  the colums of A were reordered to avoid zero pivots.
//
// Return value:
//
// int      (0 if successful, 1 otherwise)
//
//
#include "CProcessReport.h"

template <class Matrix, class Subscript>
      int LUfactor(Matrix &A, std::vector< Subscript > & row, std::vector< Subscript > & col, CProcessReport * cb = NULL)
  {
    Subscript M = A.numRows();
    Subscript N = A.numCols();

    if (M == 0 || N == 0)
      return 0;
    if (row.size() != M)
      row.resize(M);
    if (col.size() != N)
      col.resize(N);

    Subscript i = 0, j = 0, k = 0;
    Subscript jp = 0;
    Subscript jl = N - 1;

    for (i = 0; i < M; i++)
      row[i] = i;
    for (i = 0; i < N; i++)
      col[i] = i;

    typename Matrix::elementType t;

  Subscript minMN = (M < N ? M : N);        // min(M,N);

    unsigned C_INT32 hProcess;
    if (cb)
      hProcess = cb->addItem("LU decomposition...",
                             CCopasiParameter::UINT,
                             &j,
                             &minMN);

    for (j = 0; j < minMN; j++)
      {
        if (cb && !cb->progress(hProcess)) return 1;

        // find pivot in column j and  test for singularity.
        while (true)
          {
            jp = j;
            t = fabs(A(j, j));
            for (i = j + 1; i < M; i++)
              if (fabs(A(i, j)) > t)
                {
                  jp = i;
                  t = fabs(A(i, j));
                }

            // jp now has the index of maximum element
            // of column j, below the diagonal

            if (A(jp, j) == 0) // now we have to swap colums to find a pivot
              {
                // Instead of blindly swapping with the last available
                // column we first check whether it is suitable. If not
                // we proceed with the lat but one ...
                while (j < jl)
                  {
                    jp = j;
                    t = fabs(A(j, jl));

                    for (i = j + 1; i < M; i++)
                      if (fabs(A(i, jl)) > t)
                        {
                          jp = i;
                          t = fabs(A(i, jl));
                        }

                    if (t > 0.0) break; // Found a suitable column
                    // and row

                    jl--; // proceed with previous column
                  }

                if (j == jl)
                  return 1; // we are done

                for (k = 0; k < M; k++) // swap columns jl and j
                  {
                    t = A(k, jl);
                    A(k, jl) = A(k, j);
                    A(k, j) = t;
                  }
                col[jl] = j;
                jl--;
              }

            break;
          }

        if (jp != j)      // swap rows j and jp
          for (k = 0; k < N; k++)
            {
              t = A(j, k);
              A(j, k) = A(jp, k);
              A(jp, k) = t;
            }
        row[j] = jp;

        if (j < M - 1)    // compute elements j+1 <= k < M of jth column
          {
            // note A(j,j), was A(jp,p) previously which was
            // guarranteed not to be zero (Label #1)
            //
            typename Matrix::elementType recp = 1.0 / A(j, j);

            for (k = j + 1; k < M; k++)
              A(k, j) *= recp;
          }

        if (j < minMN - 1)
          {
            // rank-1 update to trailing submatrix:   E = E - x*y;
            //
            // E is the region A(j+1:M, j+1:N)
            // x is the column vector A(j+1:M,j)
            // y is row vector A(j,j+1:N)

            Subscript ii, jj;

            for (ii = j + 1; ii < M; ii++)
              for (jj = j + 1; jj < N; jj++)
                A(ii, jj) -= A(ii, j) * A(j, jj);
          }
      }

    if (cb) cb->finish(hProcess);

    return 0;
  }

#endif // COPASI_CluX

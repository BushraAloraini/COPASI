// Begin CVS Header 
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/bindings/swig/CMatrix.i,v $ 
//   $Revision: 1.7 $ 
//   $Name:  $ 
//   $Author: shoops $ 
//   $Date: 2009/01/07 18:51:30 $ 
// End CVS Header 

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual 
// Properties, Inc., EML Research, gGmbH, University of Heidelberg, 
// and The University of Manchester. 
// All rights reserved. 

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual 
// Properties, Inc. and EML Research, gGmbH. 
// All rights reserved. 

%{

#include "utilities/CMatrix.h"

%}

#ifdef SWIGJAVA

// ignore some operators to get rid of the warnings
%ignore CMatrix::operator =;
%ignore CMatrix::operator [];
%ignore CMatrix::operator ();

#endif // SWIGJAVA

%include "utilities/CMatrix.h"


%extend CMatrix
{
    virtual elementType& get(const unsigned C_INT32& row,const unsigned C_INT32& col)
    {
        return (*self)(row,col);
    }
}

%template(FloatMatrix) CMatrix<C_FLOAT64>;

typedef CMatrix<C_FLOAT64> FloatMatrix;




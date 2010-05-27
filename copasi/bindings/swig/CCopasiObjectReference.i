// Begin CVS Header 
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/bindings/swig/CCopasiObjectReference.i,v $ 
//   $Revision: 1.1.2.2 $ 
//   $Name:  $ 
//   $Author: gauges $ 
//   $Date: 2010/05/27 18:59:22 $ 
// End CVS Header 

// Copyright (C) 2010 by Pedro Mendes, Virginia Tech Intellectual 
// Properties, Inc., University of Heidelberg, and The University 
// of Manchester. 
// All rights reserved. 

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual 
// Properties, Inc., EML Research, gGmbH, University of Heidelberg, 
// and The University of Manchester. 
// All rights reserved. 

%{

#include "report/CCopasiObjectReference.h"

%}

%ignore referenceType;
%ignore CCopasiObjectReference::getValuePointer;
%ignore CCopasiObjectReference::setReference;
%ignore CCopasiObjectReference::CCopasiObjectReference;
%ignore CCopasiObjectReference::print;

%include "report/CCopasiObjectReference.h"

typedef CCopasiObjectReference<C_FLOAT64> FloatReference;

%template(FloatReference) CCopasiObjectReference<C_FLOAT64>;




// Begin CVS Header 
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/bindings/swig/CChemEq.i,v $ 
//   $Revision: 1.6 $ 
//   $Name:  $ 
//   $Author: shoops $ 
//   $Date: 2010/07/16 18:56:26 $ 
// End CVS Header 

// Copyright (C) 2010 by Pedro Mendes, Virginia Tech Intellectual 
// Properties, Inc., University of Heidelberg, and The University 
// of Manchester. 
// All rights reserved. 

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual 
// Properties, Inc., EML Research, gGmbH, University of Heidelberg, 
// and The University of Manchester. 
// All rights reserved. 

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual 
// Properties, Inc. and EML Research, gGmbH. 
// All rights reserved. 

%{

#include "model/CChemEq.h"

%}

%ignore CChemEq::getSmallestCompartment() const; // this has not been implemented
%ignore CChemEq::reverse(); // this has not been implemented
%catches(CCopasiException) CChemEq::getLargestCompartment() const;

%include "model/CChemEq.h"


%extend CChemEq
{
    CChemEqElement* getSubstrate(unsigned C_INT32 index)
    {
      return self->getSubstrates()[index];
    }

    CChemEqElement* getProduct(unsigned C_INT32 index)
    {
      return self->getProducts()[index];
    }

    CChemEqElement* getModifier(unsigned C_INT32 index)
    {
      return self->getModifiers()[index];
    }

    CChemEqElement const* getBalance(unsigned C_INT32 index)
    {
      return self->getBalances()[index];
    }
}





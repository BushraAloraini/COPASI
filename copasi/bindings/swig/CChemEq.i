// Copyright (C) 2010 - 2016 by Pedro Mendes, Virginia Tech Intellectual 
// Properties, Inc., University of Heidelberg, and The University 
// of Manchester. 
// All rights reserved. 

// Copyright (C) 2008 - 2009 by Pedro Mendes, Virginia Tech Intellectual 
// Properties, Inc., EML Research, gGmbH, University of Heidelberg, 
// and The University of Manchester. 
// All rights reserved. 

// Copyright (C) 2006 - 2007 by Pedro Mendes, Virginia Tech Intellectual 
// Properties, Inc. and EML Research, gGmbH. 
// All rights reserved. 





%{

#include "model/CChemEq.h"
#include "model/CChemEqInterface.h"

%}

%ignore CChemEq::getSmallestCompartment() const; // this has not been implemented
%ignore CChemEq::reverse(); // this has not been implemented
%catches(CCopasiException) CChemEq::getLargestCompartment() const;

%include "model/CChemEq.h"
%include "model/CChemEqInterface.h"


%extend CChemEq
{
    const CChemEqElement* getSubstrate(unsigned C_INT32 index)
    {
	try
	{
      return &self->getSubstrates()[index];
	}
	catch(...)
	{
	return NULL;
	}
    }

    const CChemEqElement* getProduct(unsigned C_INT32 index)
    {
	try
	{
      return &self->getProducts()[index];
	  }
	catch(...)
	{
	return NULL;
	}
    }

    const CChemEqElement* getModifier(unsigned C_INT32 index)
    {
	try
	{
      return &self->getModifiers()[index];
	  }
	catch(...)
	{
	return NULL;
	}
    }

    const CChemEqElement * getBalance(unsigned C_INT32 index)
    {
	try
	{
      return &self->getBalances()[index];
	  }
	catch(...)
	{
	return NULL;
	}
    }
}





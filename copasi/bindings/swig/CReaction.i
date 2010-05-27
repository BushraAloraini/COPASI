// Begin CVS Header 
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/bindings/swig/CReaction.i,v $ 
//   $Revision: 1.13.2.5 $ 
//   $Name:  $ 
//   $Author: gauges $ 
//   $Date: 2010/05/27 18:59:23 $ 
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

#include "model/CReaction.h"

%}
    
%ignore CReaction::load;
%ignore CReaction::getSmallestCompartment() const; // not implemented
%ignore CReaction::getParameterMappings() const; 
%ignore CReaction::setFunctionFromExpressionTree(CEvaluationTree* , std::map<CCopasiObject*, SBase*> & , CFunctionDB* );
%ignore CReaction::calculatePartialDerivative;


#ifdef SWIGJAVA
// remove some const methods to get rid of warnings
%ignore CReaction::getChemEq() const;
%ignore CReaction::getParameters() const;

#endif // SWIGJAVA

%catches(CCopasiException) CReaction::getLargestCompartment() const;

%include "model/CReaction.h"



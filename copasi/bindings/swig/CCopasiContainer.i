// Begin CVS Header 
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/bindings/swig/CCopasiContainer.i,v $ 
//   $Revision: 1.8 $ 
//   $Name:  $ 
//   $Author: gauges $ 
//   $Date: 2009/02/18 20:53:05 $ 
// End CVS Header 

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual 
// Properties, Inc., EML Research, gGmbH, University of Heidelberg, 
// and The University of Manchester. 
// All rights reserved. 

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual 
// Properties, Inc. and EML Research, gGmbH. 
// All rights reserved. 

%{

#include "report/CCopasiContainer.h"

%}



%ignore CCopasiContainer::CCopasiContainer(const std::string& name,
                                           const CCopasiContainer * pParent = NULL,
                                           const std::string & type = "CN",
                                           const unsigned C_INT32 & flag);

%ignore CCopasiContainer::add(CCopasiObject*,const bool&); // needs to be hidden since it conflicts with methods in derived classes 
%ignore CCopasiContainer::remove(CCopasiObject*); // needs to be hidden since it conflicts with methods in derived classes 

%include "report/CCopasiContainer.h"


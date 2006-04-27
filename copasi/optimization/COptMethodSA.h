/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/optimization/COptMethodSA.h,v $
   $Revision: 1.5 $
   $Name:  $
   $Author: shoops $
   $Date: 2006/04/27 01:29:53 $
   End CVS Header */

// Copyright � 2005 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/**
 * COptMethodSA class
 */

#ifndef COPASI_COptMethodSA
#define COPASI_COptMethodSA
#include "utilities/CVector.h"

class CRandom;

// this is an abstract class that contains many virtual functions
// without definitions
//
/** @dia:pos 48.05,34.05 */
/** @dia:route COptMethod; v,46.9608,16.35,33,59.1652,34.05 */

class COptMethodSA : public COptMethod
  {
    friend COptMethod * COptMethod::createMethod(CCopasiMethod::SubType subType);

    // Operations
  private:
    /**
     * Default Constructor
     */
    COptMethodSA();

  public:
    /**
     * Copy Constructor
     * @param const COptMethodSA & src
     */
    COptMethodSA(const COptMethodSA & src);

    /**
     * Destructor
     */
    virtual ~COptMethodSA();

    /**
     * Execute the optimization algorithm calling simulation routine
     * when needed. It is noted that this procedure can give feedback
     * of its progress by the callback function set with SetCallback.
     */
    virtual bool optimise();
  };

#endif  // COPASI_COptMethodSA

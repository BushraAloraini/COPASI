// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/model/CMetab.h,v $
//   $Revision: 1.89 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2009/01/07 19:00:14 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/**
 *  CMetab class.
 *  Derived from Gepasi's cmetab.cpp. (C) Pedro Mendes 1995-2000.
 *  Converted for COPASI by Stefan Hoops 2001
 */

#ifndef COPASI_CMetab
#define COPASI_CMetab

#include <string>
#include <iostream>
#include <set>

#include "model/CModelValue.h"
#include "model/CMoiety.h"

class CCompartment;
class CReadConfig;
//class CWriteConfig;
class CMetabOld;
class CModel;

#define METAB_MOIETY 7

/** @dia:pos 80.8289,51.5961 */
class CMetab : public CModelEntity
  {
    friend void CMoiety::add(C_FLOAT64 value, CMetab * pMetabolite);

  public:

    /**
     * Converts the concentration to number
     * @param const C_FLOAT64 & concentration
     * @param const CCompartment & compartment
     * @param const CModel & model
     * @return C_FLOAT64 number
     */
    static C_FLOAT64 convertToNumber(const C_FLOAT64 & concentration,
                                     const CCompartment & compartment,
                                     const CModel & model);

    /**
     * Converts the number to concentration
     * @param const C_FLOAT64 & number
     * @param const CCompartment & compartment
     * @param const CModel & model
     * @return C_FLOAT64 concentration
     */
    static C_FLOAT64 convertToConcentration(const C_FLOAT64 & number,
                                            const CCompartment & compartment,
                                            const CModel & model);

    // Attributes
  private:
    /**
     *  Concentration of the metabolite as double.
     */
    C_FLOAT64 mConc;

    /**
     *  Initial concentration of the metabolite as double
     */
    C_FLOAT64 mIConc;

    /**
     * The rate of concentration change
     */
    C_FLOAT64 mConcRate;

    /**
     *  Transition time of the metabolite
     */
    C_FLOAT64 mTT;

    /**
     *  pointer to the compartment the metabolite is located in.
     *  The metab needs to know about volumes.
     */
    const CCompartment * mpCompartment;

    /**
     * If dependent the moiety the metabolite is calculated from
     */
    const CMoiety * mpMoiety;

    std::vector< std::pair< C_FLOAT64, const C_FLOAT64 * > > mRateVector;

  protected:
    CCopasiObjectReference<C_FLOAT64> *mpIConcReference;
    CCopasiObjectReference<C_FLOAT64> *mpConcReference;
    CCopasiObjectReference<C_FLOAT64> *mpConcRateReference;
    CCopasiObjectReference<C_FLOAT64> *mpTTReference;

    // Operations
  public:
    /**
     * Default constructor
     * @param const std::string & name (default: "NoName")
     * @param const CCopasiContainer * pParent (default: NULL)
     */
    CMetab(const std::string & name = "NoName",
           const CCopasiContainer * pParent = NULL);

    /**
     * Copy constructor
     * @param const CMetab & src
     * @param const CCopasiContainer * pParent (default: NULL)
     */
    CMetab(const CMetab & src,
           const CCopasiContainer * pParent = NULL);

    /**
     *  Destructor.
     */
    ~CMetab();

    /**
     * Retrieve the units of the child object.
     * @return std::string units
     */
    virtual std::string getChildObjectUnits(const CCopasiObject * pObject) const;

    virtual void * getValuePointer() const;
    //TODO: discuss if it should be concentration or particle number...

    virtual std::string getObjectDisplayName(bool regular = true, bool richtext = false) const;

    /**
     *  Cleanup
     */
    void cleanup();
    void initCompartment(const CCompartment * pCompartment = NULL);

    /**
     *  Assignment operator.
     */
    CMetab & operator=(const CMetabOld & rhs);

    /**
     *  Loads an object with data coming from a CReadConfig object.
     *  (CReadConfig object reads an input stream)
     *  @param pconfigbuffer reference to a CReadConfig object.
     *  @return Fail
     */
    C_INT32 load(CReadConfig & configbuffer);

    /**
     * Sets the parent of the metabolite;
     * @param const CCopasiContainer * pParent
     * @return bool success
     */
    virtual bool setObjectParent(const CCopasiContainer * pParent);

    /**
     *
     */
    virtual void setStatus(const CModelEntity::Status & status);

    /**
     * Compile the model value.
     * @return bool success
     */
    virtual bool compile();

    /**
     * Compile the initial particle number and initial concentration dependencies.
     * @param const bool & updateConcentration (default: true)
     * @return bool success
     */
    bool compileInitialValueDependencies(const bool & updateConcentration = true);

    /**
     * Check whether changing the initial concentration is allowed
     * @return bool allowed
     */
    bool isInitialConcentrationChangeAllowed();

    /**
     * Calculate the value or the rate depending whether we have an ASIGNMENT or ODE
     */
    virtual void calculate();

    /**
     * Retrieve the list of deleted numeric child objects;
     * @return std::set< const CCopasiObject * > deletedObjects
     */
    virtual std::set< const CCopasiObject * > getDeletedObjects() const;

    /**
     *
     */
    void setConcentration(const C_FLOAT64 concentration);

    /**
     *
     */
    const C_FLOAT64 & getConcentration() const;

    /**
     *
     */
    void setInitialConcentration(const C_FLOAT64 & initialConcentration);

    /**
     *
     */
    const C_FLOAT64 & getInitialConcentration() const;

    /**
     * Retrieve object referencing the initial concentration
     * @return CCopasiObject * initialConcentrationReference
     */
    CCopasiObject * getInitialConcentrationReference() const;

    /**
     * Retrieve object referencing the concentration
     * @return CCopasiObject * concentrationReference
     */
    CCopasiObject * getConcentrationReference() const;

    /**
     * Refresh the initial value
     */
    virtual void refreshInitialValue();

    void refreshInitialConcentration();

    void refreshConcentration();

    void refreshNumber();

    /**
     *
     */
    const CCompartment * getCompartment() const;

    /**
     *
     */
    const CModel * getModel() const;

    /**
     *  Set transition time
     *  @param "const C_FLOAT64 &" transitionTime
     */
    void setTransitionTime(const C_FLOAT64 & transitionTime);

    /**
     *  Retrieves the transition time
     *  @return "const C_FLOAT64 &" transitionTime
     */
    const C_FLOAT64 & getTransitionTime() const;

    /**
     * Return rate of production of this metaboLite
     */
    C_FLOAT64 getConcentrationRate() const;

    /**
     * Calculate the concentration rate.
     */
    void refreshConcentrationRate();

    /**
     * Calculate the particle rate.
     */
    void refreshRate();

    /**
     * Calculate the transition time.
     */
    void refreshTransitionTime();

    /**
     * Set whether the metabolite is dependent, i.e., calculated
     * by a moiety
     * @param const CMoiety * pMoiety
     */
    void setDependentOn(const CMoiety * pMoiety);

    /**
     * Retrieve whether the metabolite dependent
     * @return bool dependent
     */
    bool isDependent() const;

    /**
     * ostream operator
     */
    friend std::ostream & operator<<(std::ostream &os, const CMetab & d);

  private:
    /**
     * Initialize the contained CCopasiObjects
     */
    void initObjects();
  };

class CMetabOld : public CCopasiContainer
  {
    friend class CMetab;

    // Attributes

  private:
    /**
     *  Concentration of the metabolite.
     */
    C_FLOAT64 mIConc;

    /**
     *  Status of the metabolite.
     *  One of (METAB_FIXED, METAB_VARIABLE, METAB_DEPENDENT, METAB_MOIETY).
     */
    CMetab::Status mStatus;

    /**
     *  Index of the compartment the metabolite is located in.
     */
    C_INT32 mCompartment;

    // Operations

  public:
    /**
     * Default constructor
     * @param const std::string & name (default: "NoName")
     * @param const CCopasiContainer * pParent (default: NULL)
     */
    CMetabOld(const std::string & name = "NoName",
              const CCopasiContainer * pParent = NULL);

    /**
     * Copy constructor
     * @param const CMetabOld & src
     * @param const CCopasiContainer * pParent (default: NULL)
     */
    CMetabOld(const CMetabOld & src,
              const CCopasiContainer * pParent = NULL);

    /**
     *  Destructor.
     */
    ~CMetabOld();

    /**
     *  cleanup()
     */
    void cleanup();

    /**
     *  Loads an object with data coming from a CReadConfig object.
     *  (CReadConfig object reads an input stream)
     *  @param pconfigbuffer reference to a CReadConfig object.
     *  @return Fail
     */
    C_INT32 load(CReadConfig & configbuffer);

    C_INT32 getIndex() const;
  };

#endif // COPASI_CMetab

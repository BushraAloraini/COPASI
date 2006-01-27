/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/model/CModelValue.h,v $
   $Revision: 1.6.8.1 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2006/01/27 13:49:55 $
   End CVS Header */

#ifndef COPASI_CModelValue
#define COPASI_CModelValue

#include <string>
#include <iostream>

#include "report/CCopasiContainer.h"

class CModel;

/**
 * CModelEntity is a base class for CCompartment, CMetab and CModelValue.
 * These three classes have in common that (in the long run) they can each be model variables 
 * (subject to ODEs), constant, or subject to algebraic assignments.
 * In addition, the CMetab objects can also be subject to reactions, and conservation rules.
 *
 */
class CModelEntity : public CCopasiContainer
  {
  protected:
    /**
     *  Key of the model entity. It is stored here, but constructed in the derived classes.
     */
    std::string mKey;

    /**
     * The id of the corresponding parameter in an SBML file.
     * This value is either set upon importing an SBML file,
     * or when the object is first exported to an SBML file.
     */
    std::string mSBMLId;

  public:
    /**
     *  The valid states for metabolites
     */
    enum Status
    {
      FIXED = 0,      //the entity is constant (for metabs even if they are part of a reaction)
      REACTIONS,      //applies only for metabs, the metab concentration is changed by reactions
      DEPENDENT,      //applies only for metabs, the metab concentration is determined by conservation rules
      UNUSED,
      ODE,            //the entity is changed by an ordinary differential equation
      ASSIGNMENT  //the entity is changed by an assignment rule
    };

    /**
     * String representation of the states
     */
    static const std::string StatusName[];

    /**
     * XML representation of the states
     */
    static const char * XMLStatus[];

    /**
     * Default constructor
     * @param const std::string & name (default: "NoName")
     * @param const CCopasiContainer * pParent (default: NULL)
     */
    CModelEntity(const std::string & name = "NoName",
                 const CCopasiContainer * pParent = NULL,
                 const std::string & type = "ModelEntity",
                 const unsigned C_INT32 & flag = CCopasiObject::Container | CCopasiObject::ValueDbl);

    /**
     * Copy constructor
     * @param const CModelValue & src
     * @param const CCopasiContainer * pParent (default: NULL)
     */
    CModelEntity(const CModelEntity & src,
                 const CCopasiContainer * pParent = NULL);

    /**
     *  Destructor.
     */
    ~CModelEntity();

    /**
     *  Retrieve the key 
     * @return std::string key
     */
    virtual const std::string & getKey() const;

    /**
     *
     */
    const CModelEntity::Status & getStatus() const;

    /**
     *
     */
    const C_FLOAT64 & getValue() const;

    /**
     *
     */
    const C_FLOAT64 & getInitialValue() const;

    /**
     * Return rate of production of this entity
     */
    const C_FLOAT64 & getRate() const;

    /**
     *
     */
    virtual void setStatus(const CModelEntity::Status & status);

    /**
     *
     */
    virtual void setValue(const C_FLOAT64 & value);

    /**
     *
     */
    virtual void setInitialValue(const C_FLOAT64 & initialValue);

    /**
     *  Set the rate (dmValue/dt)
     *  @param "const C_FLOAT64 &" rate 
     */
    void setRate(const C_FLOAT64 & rate);

    virtual void * getReference() const;

    /**
     * Sets the SBMLId.
     */
    void setSBMLId(const std::string& id);

    /**
     * Returns a reference to the SBML Id.
     */
    const std::string& getSBMLId() const;

  protected:
    /**
     *  Concentration of the metabolite as double.
     */
    C_FLOAT64 mValue;

    /**
     *  Initial concentration of the metabolite as double
     */
    C_FLOAT64 mIValue;

    /**
     *  Rate of production of this metabolite
     *  (concentration/time).
     */
    C_FLOAT64 mRate;

    /**
     *  Status of the model entity.  
     */
    Status mStatus;

  private:
    /**
     * Initialize the contained CCopasiObjects
     */
    void initObjects();
  };

/*
Table of possible CModelEntity objects with different Status
 
                        current status        corresponding sbml object
-------------------------------------------------------------------------------------------------
CMetab:                                       Species
 
FIXED                   implemented           constant=true
SUBJECT_TO_REACTION     implemented           constant=false, boundaryCondition=false
DEPENDENT               implemented           constant=false, boundaryCondition=false
ODE                     not implemented       constant=false, boundaryCondition=true, rate rule
ASSIGNMENT              not implemented       constant=false, boundaryCondition=true, assignment rule
 
 
CCompartment:                                 Compartment
 
FIXED                   implemented           constant=true
SUBJECT_TO_REACTION     -
DEPENDENT               -
ODE                     not implemented       constant=false, rate rule
ASSIGNMENT              not implemented       constant=false, assignment rule
  
 
CModelValue:                                  Parameter
 
FIXED                   in progress           constant=true
SUBJECT_TO_REACTION     -
DEPENDENT               -
ODE                     not implemented       constant=false, rate rule
ASSIGNMENT              not implemented       constant=false, rate rule
  
 */

/**
 * CModelValue represents an entity in the model that has a value but is not a concentration (like species) 
 * or a volume (like compartments).
 * It correspondents to global parameters in sbml
 */
class CModelValue : public CModelEntity
  {
  public:
    /**
     * Default constructor
     * @param const std::string & name (default: "NoName")
     * @param const CCopasiContainer * pParent (default: NULL)
     */
    CModelValue(const std::string & name = "NoName",
                const CCopasiContainer * pParent = NULL);

    /**
     * Copy constructor
     * @param const CModelValue & src
     * @param const CCopasiContainer * pParent (default: NULL)
     */
    CModelValue(const CModelValue & src,
                const CCopasiContainer * pParent = NULL);

    /**
     *  Destructor.
     */
    ~CModelValue();

    /**
     * insert operator
     */
    friend std::ostream & operator<<(std::ostream &os, const CModelValue & d);

  private:
    /**
     * Initialize the contained CCopasiObjects
     */
    void initObjects();
  };

#endif

/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/function/CFunctionDB.h,v $
   $Revision: 1.39 $
   $Name:  $
   $Author: gauges $ 
   $Date: 2005/07/19 15:14:39 $
   End CVS Header */

/**
 * CFunctionDB
 * 
 * Created for Copasi by Stefan Hoops
 * (C) Stefan Hoops 2001
 */

#ifndef COPASI_CFunctionDB
#define COPASI_CFunctionDB

#include <string>

#include "CEvaluationTree.h"

#include "report/CCopasiContainer.h"
#include "utilities/CReadConfig.h"
#include "utilities/CCopasiVector.h"

class CFunction;

/** @dia:pos 106.082,17.0878 */
class CFunctionDB : public CCopasiContainer
  {
    // Attributes
  private:
    /**
     *  Filename which contains the function database
     */
    std::string mFilename;

    /**
     *  Vector of the currently loaded functions
     *  @supplierCardinality 0..*
     */
    /** @dia:route 2,0; h,98.6992,12.15,102.581,17.0878,106.082 */
    CCopasiVectorN < CEvaluationTree > mLoadedFunctions;

    // Operations

  public:
    /**
     * Default constructor
     * @param const std::string & name (default: "NoName")
     * @param const CCopasiContainer * pParent (default: RootContainer)
     */
    CFunctionDB(const std::string & name = "FunctionDB",
                const CCopasiContainer * pParent = CCopasiContainer::Root);

    /**
     * Destructor
     */
    ~CFunctionDB();

    /**
     *
     */
    void cleanup();

    virtual void initObjects();

    bool CFunctionDB::load();

    /**
     *  Loads an object with data coming from a CReadConfig object.
     *  (CReadConfig object reads an input stream)
     *  @param pconfigbuffer reference to a CReadConfig object.
     *  @return mFail
     */
    C_INT32 load(CReadConfig & configbuffer);

    /**
     *  Saves the contents of the object to a CWriteConfig object.
     *  (Which usually has a file attached but may also have socket)
     *  @param pconfigbuffer reference to a CWriteConfig object.
     *  @return mFail
     */ 
    //    C_INT32 save(CWriteConfig & configbuffer);

    /**
     *  Saves the contents of the object to a CWriteConfig object.
     *  This function uses the file format of Gepasi 3.21
     *  @param pconfigbuffer reference to a CWriteConfig object.
     *  @return mFail
     */ 
    //    C_INT32 saveOld(CWriteConfig & configbuffer);

    /**
     *
     */
    void setFilename(const std::string & filename);

    /**
     *
     */
    std::string getFilename() const;

#ifdef FFFF
    /**
     *  Load the function functionName from the database
     *  @param "const string" &functionName
     *  @return CEvaluationTree * function (NULL if function can not be loaded)
     */
    CEvaluationTree * dBLoad(const std::string & functionName);
#endif // FFFF

    /**
     * Add the function to the database
     * @param CEvaluationTree * pFunction
     * @param const bool & adopt (default = false)
     * @return bool success
     */
    bool add(CEvaluationTree * pFunction, const bool & adopt);

#ifdef FFFF
    /**
     *  Add the function to the database
     *  @param const std::string & name
     *  @param const CEvaluationTree::Type & type (Default: CEvaluationTree::Base)
     *  @return bool success
     */
    CEvaluationTree* createFunction(const std::string &name, const CEvaluationTree::Type & type = CEvaluationTree::Function);
#endif // FFFF

    bool removeFunction(const std::string &key);

    /**
     *  Delete the function functionName from the database
     *  @param "const string" &functionName
     *  @return C_INT32 Fail
     */ 
    //void dBDelete(const std::string & functionName);

    /**
     *  Search for a function among the loaded functions. If no
     *  function is found NULL is returned
     *  @param "const string" &functionName
     *  @return CEvaluationTree *
     */
    CEvaluationTree * findFunction(const std::string & functionName);

    /**
     *  Search for a function among the loaded functions. If no
     *  function is found the database is searched and the apropriate 
     *  function is loaded.
     *  @param "const string" &functionName
     *  @return CEvaluationTree * function (NULL if function is not found)
     */
    CEvaluationTree * findLoadFunction(const std::string & functionName);

    /**
     *  Retrieves the vector of loaded functions.
     *  @return "CCopasiVectorNS < CKinFunction > &" loadedFunctions
     */
    CCopasiVectorN < CEvaluationTree > & loadedFunctions();

    /**
     *  Retrieves the vector of functions that are suitable for a
     *  number of substrates, products and reversibility status.
     *  Note: The returnes CCopasiVector has to be deleted after use!
     *  @param "const unsigned C_INT32" noSubstrates the number of substrates
     *  @param "const unsigned C_INT32" noProducts the number of products
     *  @param "const TriLogic" reversible the reversibility status
     *  @return "CCopasiVectorN < CFunction > " suitableFunctions
     */
    CCopasiVector <CFunction> *
    suitableFunctions(const unsigned C_INT32 noSubstrates,
                      const unsigned C_INT32 noProducts,
                      const TriLogic reversible);
  };

#endif // COPASI_CFunctionDB

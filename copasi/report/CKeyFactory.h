/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/report/CKeyFactory.h,v $
   $Revision: 1.11 $
   $Name:  $
   $Author: shoops $
   $Date: 2006/04/27 01:31:09 $
   End CVS Header */

// Copyright � 2005 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/**
 * CKeyFactory class.
 * This class is used to create a unique key whithin COPASI. It also allows
 * retreival of the CCopasiObject the key is assigned to.
 *
 * Created for Copasi by Stefan Hoops 2003
 * Copyright Stefan Hoops
 */

#ifndef COPASI_CKeyFactory
#define COPASI_CKeyFactory

#include <string>
#include <map>
#include <stack>

#include "utilities/CVector.h"

class CCopasiObject;

class CKeyFactory
  {
  private:
    class HashTable
      {
      private:
        unsigned C_INT32 mBeyond;
        unsigned C_INT32 mSize;
        CVector< CCopasiObject * > * mpTable;
        std::stack< unsigned C_INT32 > mFree;

      public:
        HashTable();
        HashTable(const HashTable & src);
        ~HashTable();
        unsigned C_INT32 add(CCopasiObject * pObject);
        bool addFix(const unsigned C_INT32 & index, CCopasiObject * pObject);
        CCopasiObject * get(const unsigned C_INT32 & index);
        bool remove(const unsigned C_INT32 & index);
      };

  class CDecisionVector : private CVector< bool >
      {
      private:
        CDecisionVector();

      public:
        CDecisionVector(const std::string & str);

        virtual ~CDecisionVector();

        const bool & operator () (const unsigned char & c) const;
      };

    // Attributes
  protected:
    /**
     * A map of hash tables for the prefixes.
     */
    std::map< std::string, HashTable > mKeyTable;

    /**
     * Fast way to decide whether a character is a digit.
     */
    static CDecisionVector isDigit;

    /**
     * Fast way to decide whether a character is a valid part of the prefix.
     */
    static CDecisionVector isPrefix;

    // Operations
  public:
    /**
     * check whether the key is valid
     * @param const std::string & key
     * @param const std::string & prefix (default: check for [_a-zA-Z]+_\d+)
     * @return bool isValid
     */
    static bool isValidKey(const std::string & key,
                           const std::string & prefix = "");

    /**
     * Default constructor
     */
    CKeyFactory();

    /**
     * Destructor
     */
    ~CKeyFactory();

    /**
     * Add an object with a key generated from the given prefix to the key map.
     * The return value is the actually generated key.
     * @param const std::string & prefix
     * @param CCopasiObject * pObject
     * @return std::string key
     */
    std::string add(const std::string & prefix, CCopasiObject * pObject);

    /**
     * Add an object with a fix given key.
     * The return value indicate whether the key was actually inserted.
     * @param const std::string & key
     * @param CCopasiObject * pObject
     * @return bool success
     */
    bool addFix(const std::string & key, CCopasiObject * pObject);

    /**
     * Remove the key and the related object from the key map.
     * @param const std::string & key
     * @return bool success
     */
    bool remove(const std::string & key);

    /**
     * Retreive the object referred by key from the key map.
     * @param const std::string & key
     * @return CCopasiObject * pObject
     */
    CCopasiObject * get(const std::string & key);
  };

extern CKeyFactory GlobalKeys;

#endif // COPASI_CKeyFactory

/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/function/CFunctionDB.cpp,v $
   $Revision: 1.67 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2005/11/29 17:28:10 $
   End CVS Header */

/**
 * CFunctionDB
 * 
 * Created for Copasi by Stefan Hoops
 * (C) Stefan Hoops 2001
 */

#include "copasi.h"

#include "CFunctionDB.h"
#include "CFunction.h"

#include "utilities/CCopasiException.h"
#include "report/CCopasiObjectReference.h"
#include "xml/CCopasiXML.h"
#include "report/CKeyFactory.h"
#include "FunctionDB.xml.h"

CFunctionDB::CFunctionDB(const std::string & name,
                         const CCopasiContainer * pParent):
    CCopasiContainer(name, pParent, "FunctionDB"),
    mFilename(),
    mLoadedFunctions("Functions", this)
{
  initObjects();
  CONSTRUCTOR_TRACE;
}

CFunctionDB::~CFunctionDB()
{
  cleanup();
  DESTRUCTOR_TRACE;
}

void CFunctionDB::cleanup() {mLoadedFunctions.cleanup();}

void CFunctionDB::initObjects()
{
  addObjectReference("File", mFilename);
}

bool CFunctionDB::load()
{
  CCopasiXML XML;
  std::stringstream DB;
  DB.str(FunctionDBxml);
  //  std::ifstream DB(mFilename.c_str());
  if (DB.fail()) return false;
  if (!XML.load(DB)) return false;

  CCopasiVectorN< CEvaluationTree > * pFunctionList = XML.getFunctionList();

  unsigned C_INT32 i, imax = pFunctionList->size();

  for (i = 0; i < imax; i++)
    mLoadedFunctions.add((*pFunctionList)[i], true);

  XML.freeFunctionList();

  return true;
}

C_INT32 CFunctionDB::load(CReadConfig &configbuffer)
{
  CFunction Function;
  CFunction * pFunction = NULL;

  C_INT32 Size = 0;
  C_INT32 Fail = 0;

  configbuffer.getVariable("TotalUDKinetics", "C_INT32", &Size,
                           CReadConfig::LOOP);

  for (C_INT32 i = 0; i < Size; i++)
    {
      Function.load(configbuffer);

      switch (Function.getType())
        {
        case CEvaluationTree::Function:
          pFunction = new CFunction(Function);
          break;

        case CEvaluationTree::MassAction:
          pFunction = new CMassAction(Function);
          break;

        case CEvaluationTree::PreDefined:
        case CEvaluationTree::UserDefined:
          pFunction = new CKinFunction(Function,
                                       &configbuffer);
          break;

        default:
          fatalError();
        }

      pFunction->compile();

      if (!mLoadedFunctions.add(pFunction, true))
        {
          pdelete(pFunction);
          // We ignore:
          // CCopasiVector (2): Object '%s' allready exists.
          if ((MCCopasiVector + 2) != CCopasiMessage::peekLastMessage().getNumber())
            return Fail = 1;

          // Remove the ignored meesage.
          CCopasiMessage::getLastMessage();
        }
    }

  return Fail;
}

/*C_INT32 CFunctionDB::save(CWriteConfig &configbuffer)
{
  C_INT32 Size = mLoadedFunctions.size();
  C_INT32 Fail = 0;
 
  if ((Fail = configbuffer.setVariable("TotalKinetics", "C_INT32", &Size)))
    return Fail;
 
  mLoadedFunctions.save(configbuffer);
 
  return Fail;
}
 
C_INT32 CFunctionDB::saveOld(CWriteConfig &configbuffer)
{
  C_INT32 Size;
  C_INT32 Fail = 0;
  C_INT32 i, j;
 
  Size = mLoadedFunctions.size();
  for (i = j = 0; i < Size; i++)
    if (mLoadedFunctions[i]->getType() == CFunction::UserDefined)
      j++;
 
  if ((Fail = configbuffer.setVariable("TotalUDKinetics", "C_INT32", &j)))
    return Fail;
 
  // because CCopasiVector does not have saveOld, we will save them one by one
  for (i = 0; i < Size; i++)
    if (mLoadedFunctions[i]->getType() == CFunction::UserDefined)
      mLoadedFunctions[i]->saveOld(configbuffer);
 
  return Fail;
}*/

void CFunctionDB::setFilename(const std::string & filename)
{mFilename = filename;}

std::string CFunctionDB::getFilename() const
  {return mFilename;}

#ifdef FFFF
CFunction * CFunctionDB::dBLoad(const std::string & functionName)
{
  CFunction Function("NoName", &mLoadedFunctions);
  CFunction * pFunction = NULL;

  if (mFilename == "") return NULL;
  CReadConfig inbuf(mFilename);

  while (functionName != Function.getObjectName())
    {
      Function.cleanup();
      Function.load(inbuf);
    }

  switch (Function.getType())
    {
    case CFunction::Base:
      pFunction = new CFunction(Function);
      break;

    case CFunction::MassAction:
      pFunction = new CMassAction(Function.isReversible());
      break;

    case CFunction::PreDefined:

    case CFunction::UserDefined:
      pFunction = new CKinFunction(Function, &inbuf);
      break;

    case CFunction::Expression:
      fatalError(); //disabled
      //pFunction = new CUDFunction(Function);
      break;

    default:
      fatalError();
    }

  if (!mLoadedFunctions.add(pFunction))
    {
      pdelete(pFunction);
      // We ignore:
      // CCopasiVector (2): Object '%s' allready exists.
      if ((MCCopasiVector + 2) != CCopasiMessage::getLastMessage().getNumber())

        pFunction = mLoadedFunctions[Function.getObjectName()];
    }

  return pFunction;
}
#endif // FFFF

#ifdef FFFF
CEvaluationTree * CFunctionDB::createFunction(const std::string & name, const CEvaluationTree::Type & type)
{
  if (mLoadedFunctions.getIndex(name) != C_INVALID_INDEX)
    return NULL;

  //CFunction * pFunction = new CFunction(name);

  CEvaluationTree * pFunction = NULL;
  switch (type)
    {
    case CEvaluationTree::Base:
      pFunction = new CFunction(name);
      break;

    case CEvaluationTree::MassAction:
      pFunction = new CMassAction(name);
      break;

    case CEvaluationTree::PreDefinedKineticLaw:
    case CEvaluationTree::UserDefinedKineticLaw:
      pFunction = new CKinFunction(name);
      break;

    default:
      fatalError();
    }

  if (!mLoadedFunctions.add(pFunction, true))
    {
      delete pFunction;
      return NULL;
    }
  return pFunction;
}
#endif // FFFF

bool CFunctionDB::add(CEvaluationTree * pFunction,
                      const bool & adopt)
{return mLoadedFunctions.add(pFunction, adopt);}

bool CFunctionDB::removeFunction(const std::string &key)
{
  CEvaluationTree* func = dynamic_cast< CEvaluationTree * >(GlobalKeys.get(key));
  if (!func) return false;

  unsigned C_INT32 index =
    mLoadedFunctions.CCopasiVector<CEvaluationTree>::getIndex(func);
  if (index == C_INVALID_INDEX) return false;

  mLoadedFunctions.CCopasiVector<CEvaluationTree>::remove(index);

  return true;
}

// void CFunctionDB::dBDelete(const string & functionName)
// {
//}

CEvaluationTree * CFunctionDB::findFunction(const std::string & functionName)
{
  unsigned C_INT32 index = mLoadedFunctions.getIndex(functionName);

  if (index != C_INVALID_INDEX)
    return mLoadedFunctions[index];
  else
    return NULL;
}

CEvaluationTree * CFunctionDB::findLoadFunction(const std::string & functionName)
{
  unsigned C_INT32 i;

  for (i = 0; i < mLoadedFunctions.size(); i++)
    if (functionName == mLoadedFunctions[i]->getObjectName())
      return mLoadedFunctions[i];

  return NULL;
}

CCopasiVectorN < CEvaluationTree > & CFunctionDB::loadedFunctions()
{return mLoadedFunctions;}

CCopasiVector <CFunction> *
CFunctionDB::suitableFunctions(const unsigned C_INT32 noSubstrates,
                               const unsigned C_INT32 noProducts,
                               const TriLogic reversible)
{
  CCopasiVector< CFunction >* pFunctionVector = new CCopasiVector< CFunction >();
  unsigned C_INT32 i, imax;
  unsigned C_INT32 j, jmax;
  CFunction *pFunction;
  CUsageRange * pRange;

  for (i = 0, imax = mLoadedFunctions.size(); i < imax; i++)
    {
      pFunction = dynamic_cast<CFunction *>(mLoadedFunctions[i]);
      if (!pFunction) continue;

      if (reversible != TriUnspecified &&
          reversible != pFunction->isReversible() &&
          pFunction->isReversible() != TriUnspecified)
        continue;

      for (j = 0, jmax = pFunction->getUsageDescriptions().size();
           j < jmax; j++)
        {
          pRange = pFunction->getUsageDescriptions()[j];

          if (pRange->getUsage() == "SUBSTRATE")
            if (!pRange->isInRange(noSubstrates)) break;

          if (pRange->getUsage() == "PRODUCT")
            if (!pRange->isInRange(noProducts)) break;
        }

      if (j == jmax) pFunctionVector->add(pFunction);
    }

  return pFunctionVector;
}

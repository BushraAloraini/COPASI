/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/sbml/SBMLImporter.cpp,v $
   $Revision: 1.77 $
   $Name:  $
   $Author: gauges $ 
   $Date: 2005/07/21 15:08:40 $
   End CVS Header */

#include "copasi.h"

#include <iostream>
#include <vector>
#include <sstream>
#include <map>
#include <limits>

#include "mathematics.h"

#include "model/CModel.h"
#include "model/CCompartment.h"
#include "model/CMetab.h"
#include "model/CReaction.h"
#include "model/CModelValue.h"
#include "copasi.h"
#include "function/CNodeK.h"
#include "function/CKinFunction.h"
#include "function/CFunctionDB.h"
#include "function/CEvaluationTree.h"
#include "report/CCopasiObjectReference.h"

#include "sbml/SBMLReader.h"
#include "sbml/SBMLDocument.h"
#include "sbml/Compartment.h"
#include "sbml/Species.h"
#include "sbml/SpeciesReference.h"
#include "sbml/ModifierSpeciesReference.h"
#include "sbml/Reaction.h"
#include "sbml/KineticLaw.h"
#include "sbml/math/FormulaFormatter.h"
#include "sbml/Model.h"
#include "sbml/UnitKind.h"
#include "sbml/Unit.h"
#include "sbml/Parameter.h"
#include "sbml/FunctionDefinition.h"
#include "sbml/xml/ParseMessage.h"

#include "SBMLImporter.h"
#include "ConverterASTNode.h"

#include "utilities/CCopasiMessage.h"

/**
 * Creates and returns a Copasi CModel from the SBMLDocument given as argument.
 */
CModel* SBMLImporter::createCModelFromSBMLDocument(SBMLDocument* sbmlDocument, std::map<CCopasiObject*, SBase*>& copasi2sbmlmap)
{
  Model* sbmlModel = sbmlDocument->getModel();

  /* Create an empty model and set the title. */
  this->mpCopasiModel = new CModel();
  copasi2sbmlmap[this->mpCopasiModel] = sbmlModel;
  this->mpCopasiModel->setVolumeUnit(CModel::l);
  this->mpCopasiModel->setTimeUnit(CModel::s);
  this->mpCopasiModel->setQuantityUnit(CModel::Mol);
  /* Set standard units to match the standard units of SBML files. */

  if (sbmlModel->getNumUnitDefinitions() != 0)
    {
      unsigned int counter;
      for (counter = 0; counter < sbmlModel->getNumUnitDefinitions(); counter++)
        {
          UnitDefinition* uDef = sbmlModel->getUnitDefinition(counter);
          std::string unitId = uDef->getId();
          if (unitId == "substance")
            {
              this->mpCopasiModel->setQuantityUnit(this->handleSubstanceUnit(uDef));
            }
          else if (unitId == "time")
            {
              this->mpCopasiModel->setTimeUnit(this->handleTimeUnit(uDef));
            }
          else if (unitId == "volume")
            {
              this->mpCopasiModel->setVolumeUnit(this->handleVolumeUnit(uDef));
            }
          else if ((unitId == "area") || (unitId == "length"))
            {
              /* do nothing, but do not throw an exception either */
            }
          else
            {
              /* Dont' throw an exception any more because individual objects
              ** are tested for unit usage and warning will be created there.
              */ 
              //throw StdException("Error. SBML Units other than \"substance\", \"time\" and \"volume\" not implemented.");
            }
        }
    }

  std::string title, comment;

  comment = sbmlModel->getNotes();

  this->mpCopasiModel->setComments(comment);
  title = sbmlModel->getName();
  if (title == "")
    {
      title = "NoName";
    }
  this->mpCopasiModel->setTitle(title.c_str());

  /* import the functions */
  unsigned int counter;
  CCopasiVectorN< CEvaluationTree >* functions = &(this->functionDB->loadedFunctions());

  unsigned int num = (*functions).size();
  this->sbmlIdMap.clear();
  for (counter = 0; counter < num; ++counter)
    {
      CEvaluationTree* tree = (*functions)[counter];
      if (!tree->getSBMLId().empty())
        {
          this->sbmlIdMap[tree] = tree->getSBMLId();
          tree->setSBMLId("");
        }
    }

  num = sbmlModel->getNumFunctionDefinitions();

  CFunctionDB* pTmpFunctionDB = new CFunctionDB();
  for (counter = 0; counter < num;++counter)
    {
      FunctionDefinition* pSBMLFunDef = sbmlModel->getFunctionDefinition(counter);
      CFunction* pFun = this->createCFunctionFromFunctionDefinition(pSBMLFunDef, pTmpFunctionDB);
      copasi2sbmlmap[pFun] = pSBMLFunDef;
      this->mFunctionNameMapping[pSBMLFunDef->getId()] = pFun->getObjectName();
    }

  // now go through the temporary function db and replace all call nodes with the name of the
  // copasi function.
  functions = &(pTmpFunctionDB->loadedFunctions());
  num = (*functions).size();
  std::set<std::string> usedFunctions;
  for (counter = 0; counter < num; ++counter)
    {
      this->replaceCallNodeNames((*functions)[counter]->getRoot(), usedFunctions);
    }
  // now we can already add the usedFunctions to the global database
  // this will eliminate some name clashed later with functions added by CReaction through
  // setFunctionFromExpression
  std::set<std::string>::const_iterator it = usedFunctions.begin();
  std::set<std::string>::const_iterator endIt = usedFunctions.end();
  while (it != endIt)
    {
      CFunction* pFun = static_cast<CFunction*>(pTmpFunctionDB->findFunction((*it)));
      if (!pFun)
        {
          fatalError();
        }
      this->functionDB->add(pFun, true);
      pTmpFunctionDB->removeFunction(pFun->getKey());
      this->newFunctionKeys.push_back(pFun->getKey());
      ++it;
    }

  std::map<std::string, CCompartment*> compartmentMap;
  /* Create the compartments */
  num = sbmlModel->getNumCompartments();
  for (counter = 0; counter < num; counter++)
    {
      Compartment* sbmlCompartment = sbmlModel->getCompartment(counter);
      if (sbmlCompartment == NULL)
        {
          fatalError();
        }
      CCompartment* copasiCompartment = this->createCCompartmentFromCompartment(sbmlCompartment, this->mpCopasiModel, copasi2sbmlmap);
      std::string key = sbmlCompartment->getId();
      if (mLevel == 1)
        {
          key = sbmlCompartment->getName();
        }
      compartmentMap[key] = copasiCompartment;
    }

  /* Create all species */
  num = sbmlModel->getNumSpecies();

  for (counter = 0; counter < num; ++counter)
    {
      Species* sbmlSpecies = sbmlModel->getSpecies(counter);
      if (sbmlSpecies == NULL)
        {
          fatalError();
        }
      CCompartment* copasiCompartment = compartmentMap[sbmlSpecies->getCompartment()];
      if (copasiCompartment != NULL)
        {
          CMetab* copasiMetabolite = this->createCMetabFromSpecies(sbmlSpecies, this->mpCopasiModel, copasiCompartment, copasi2sbmlmap);
          std::string key;
          if (this->mLevel == 1)
            {
              key = sbmlSpecies->getName();
            }
          else
            {
              key = sbmlSpecies->getId();
            }
          this->speciesMap[key] = copasiMetabolite;
        }
      else
        {
          CCopasiMessage(CCopasiMessage::EXCEPTION, MCSBML + 5 , sbmlSpecies->getCompartment().c_str(), sbmlSpecies->getId().c_str());
        }
    }

  /* Create the global Parameters */
  num = sbmlModel->getNumParameters();
  for (counter = 0; counter < num; ++counter)
    {
      Parameter* sbmlParameter = sbmlModel->getParameter(counter);
      if (sbmlParameter == NULL)
        {
          fatalError();
        }
      this->createCModelValueFromParameter(sbmlParameter, this->mpCopasiModel, copasi2sbmlmap);
    }

  /* Create all reactions */
  num = sbmlModel->getNumReactions();
  for (counter = 0; counter < num; counter++)
    {
      this->createCReactionFromReaction(sbmlModel->getReaction(counter), sbmlModel, this->mpCopasiModel, copasi2sbmlmap, pTmpFunctionDB);
    }
  this->mpCopasiModel->setCompileFlag();
  if (sbmlModel->getNumRules() > 0)
    {
      CCopasiMessage Message(CCopasiMessage::WARNING, MCSBML + 3);
    }
  if (sbmlModel->getNumEvents() > 0)
    {
      CCopasiMessage Message(CCopasiMessage::WARNING, MCSBML + 4);
    }

  // delete the temporary function database and all functions that are still in there.
  functions = &(pTmpFunctionDB->loadedFunctions());
  num = (*functions).size();
  for (counter = 0; counter < num; ++counter)
    {
      CEvaluationTree* pTree = pTmpFunctionDB->findFunction((*functions)[counter]->getObjectName());
      pdelete(pTree);
    }
  delete pTmpFunctionDB;

  return this->mpCopasiModel;
}

CFunction* SBMLImporter::createCFunctionFromFunctionDefinition(const FunctionDefinition* sbmlFunction, CFunctionDB* pTmpFunctionDB)
{
  CFunction* pTmpFunction = this->createCFunctionFromFunctionTree(sbmlFunction);
  if (pTmpFunction)
    {
      pTmpFunction->setSBMLId(sbmlFunction->getId());
      std::string functionName = sbmlFunction->getName();
      if (functionName == "")
        {
          functionName = sbmlFunction->getId();
        }
      unsigned int counter = 2;
      std::ostringstream numberStream;
      std::string appendix = "";
      while (this->functionDB->findFunction(functionName + appendix))
        {
          numberStream.str("");
          numberStream << "_" << counter;
          counter++;
          appendix = numberStream.str();
        }
      pTmpFunction->setObjectName(functionName + appendix);
      pTmpFunctionDB->add(pTmpFunction, false);
    }
  else
    {
      CCopasiMessage::CCopasiMessage(CCopasiMessage::EXCEPTION, MCSBML + 14, sbmlFunction->getId().c_str());
    }
  return pTmpFunction;
}

CFunction* SBMLImporter::createCFunctionFromFunctionTree(const FunctionDefinition* pSBMLFunction)
{
  CFunction* pFun = NULL;
  if (pSBMLFunction->isSetMath())
    {
      const ASTNode* root = pSBMLFunction->getMath();
      if (root->getType() == AST_LAMBDA)
        {
          // get the number of children.
          // the first n-1 children are the parameters for the function
          // the last child is the actual function
          pFun = new CFunction();
          pFun->setTree(*root->getRightChild());
          if (pFun->getRoot() == NULL)
            {
              delete pFun;
              CCopasiMessage::CCopasiMessage(CCopasiMessage::EXCEPTION, MCSBML + 13, pSBMLFunction->getId().c_str());
            }
          unsigned int i, iMax = root->getNumChildren() - 1;
          for (i = 0; i < iMax;++i)
            {
              ASTNode* pVarNode = root->getChild(i);
              if (pVarNode->getType() != AST_NAME)
                {
                  delete pFun;
                  CCopasiMessage::CCopasiMessage(CCopasiMessage::EXCEPTION, MCSBML + 12, pSBMLFunction->getId().c_str());
                }
              pFun->addVariable(pVarNode->getName());
            }
        }
      else
        {
          CCopasiMessage::CCopasiMessage(CCopasiMessage::EXCEPTION, MCSBML + 11, pSBMLFunction->getId().c_str());
        }
    }
  return pFun;
}

/**
 * Creates and returns a Copasi CCompartment from the SBML Compartment
 * given as argument.
 */
CCompartment*
SBMLImporter::createCCompartmentFromCompartment(const Compartment* sbmlCompartment, CModel* copasiModel, std::map<CCopasiObject*, SBase*>& copasi2sbmlmap)
{
  if (sbmlCompartment->isSetUnits())
    {
      std::string cU = sbmlCompartment->getUnits();
      if (cU != "volume" && cU != "area" && cU != "length")
        {
          fatalError();
        }
      else if (cU == "area" || cU == "length")
        {
          /* !!!! create a warning that the units will be ignored. */
        }
    }
  std::string name = sbmlCompartment->getName();
  if (name == "")
    {
      name = sbmlCompartment->getId();
    }
  std::string appendix = "";
  unsigned int counter = 2;
  std::ostringstream numberStream;
  while (copasiModel->getCompartments().getIndex(name + appendix) != static_cast < unsigned C_INT32
         > (-1))
    {
      numberStream.str("");
      numberStream << "_" << counter;
      counter++;
      appendix = numberStream.str();
    }
  double value;
  if (sbmlCompartment->isSetVolume())
    {
      value = sbmlCompartment->getVolume();
    }
  else
    {
      // Set value to NaN and create a warning if it is the first time
      // this happend
      value = std::numeric_limits<C_FLOAT64>::quiet_NaN();
      if (!this->mIncompleteModel)
        {
          this->mIncompleteModel = true;
          CCopasiMessage Message(CCopasiMessage::WARNING, MCSBML + 7);
        }
    }

  CCompartment* copasiCompartment = copasiModel->createCompartment(name + appendix, value);
  if (this->mLevel == 1)
    {
      copasiCompartment->setSBMLId(sbmlCompartment->getName());
    }
  else
    {
      copasiCompartment->setSBMLId(sbmlCompartment->getId());
    }
  //DebugFile << "Created Compartment: " << copasiCompartment->getObjectName() << std::endl;
  copasi2sbmlmap[copasiCompartment] = const_cast<Compartment*>(sbmlCompartment);
  return copasiCompartment;
}

/**
 * Creates and returns a Copasi CMetab from the given SBML Species object.
 */
CMetab*
SBMLImporter::createCMetabFromSpecies(const Species* sbmlSpecies, CModel* copasiModel, CCompartment* copasiCompartment, std::map<CCopasiObject*, SBase*>& copasi2sbmlmap)
{
  if (sbmlSpecies->isSetSubstanceUnits())
    {
      std::string cU = sbmlSpecies->getSubstanceUnits();
      if (cU != "substance")
        {
          fatalError();
        }
    }
  if (sbmlSpecies->isSetSpatialSizeUnits())
    {
      const std::string szU = sbmlSpecies->getSpatialSizeUnits();
      if (szU == "volume")
        {
          /* !!!! create a warning that the spatialSizeUnits will be ignored */
        }
    }
  std::string name = sbmlSpecies->getName();
  if (name == "")
    {
      name = sbmlSpecies->getId();
    }
  std::string appendix = "";
  unsigned int counter = 2;
  std::ostringstream numberStream;
  while (copasiCompartment->getMetabolites().getIndex(name + appendix) != static_cast<unsigned C_INT32>(-1))
    {
      numberStream.str("");
      numberStream << "_" << counter;
      counter++;
      appendix = numberStream.str();
    }

  CMetab* copasiMetabolite = copasiModel->createMetabolite(name + appendix, copasiCompartment->getObjectName());
  if (copasiMetabolite == NULL)
    {
      //DebugFile << "Could not create Copasi metabolite." << std::endl;
      fatalError();
    }
  if (sbmlSpecies->getConstant() || sbmlSpecies->getBoundaryCondition())
    {
      copasiMetabolite->setStatus(CModelEntity::FIXED);
    }
  else
    {
      copasiMetabolite->setStatus(CModelEntity::REACTIONS);
    }
  if (sbmlSpecies->isSetInitialAmount())
    {
      copasiMetabolite->setInitialNumber(sbmlSpecies->getInitialAmount()*copasiModel->getQuantity2NumberFactor()); // CHECK UNITS !!!
    }
  else if (sbmlSpecies->isSetInitialConcentration())
    {
      copasiMetabolite->setInitialConcentration(sbmlSpecies->getInitialConcentration());      // CHECK UNITS !!!
    }
  else
    {
      copasiMetabolite->setInitialConcentration(std::numeric_limits<C_FLOAT64>::quiet_NaN());      // CHECK UNITS !!!
      if (!this->mIncompleteModel)
        {
          this->mIncompleteModel = true;
          CCopasiMessage Message(CCopasiMessage::WARNING, MCSBML + 7);
        }
    }
  //DebugFile << "Created metabolite: " << copasiMetabolite->getObjectName() << std::endl;
  copasi2sbmlmap[copasiMetabolite] = const_cast<Species*>(sbmlSpecies);
  if (this->mLevel == 1)
    {
      copasiMetabolite->setSBMLId(sbmlSpecies->getName());
    }
  else
    {
      copasiMetabolite->setSBMLId(sbmlSpecies->getId());
    }
  return copasiMetabolite;
}

/**
 * Creates and returns a Copasi CReaction object from the given SBML
 * Reaction object.
 */
CReaction*
SBMLImporter::createCReactionFromReaction(const Reaction* sbmlReaction, const Model* sbmlModel, CModel* copasiModel, std::map<CCopasiObject*, SBase*>& copasi2sbmlmap, CFunctionDB* pTmpFunctionDB)
{
  if (sbmlReaction == NULL)
    {
      fatalError();
    }
  std::string name = sbmlReaction->getName();
  if (name == "")
    {
      name = sbmlReaction->getId();
    }
  /* Check if the name of the reaction is unique. */
  std::string appendix = "";
  unsigned int counter = 2;
  std::ostringstream numberStream;
  while (copasiModel->getReactions().getIndex(name + appendix) != C_INVALID_INDEX)
    {
      numberStream.str("");
      numberStream << "_" << counter;
      counter++;
      appendix = numberStream.str();
    }

  /* create a new reaction with the unique name */
  CReaction* copasiReaction = copasiModel->createReaction(name + appendix);
  copasiReaction->setReversible(sbmlReaction->getReversible());
  copasi2sbmlmap[copasiReaction] = const_cast<Reaction*>(sbmlReaction);
  if (copasiReaction == NULL)
    {
      fatalError();
    }
  /* Add all substrates to the reaction */
  unsigned int num = sbmlReaction->getNumReactants();
  bool singleCompartment = true;
  const CCompartment* compartment = NULL;
  for (counter = 0; counter < num; counter++)
    {
      SpeciesReference* sr = sbmlReaction->getReactant(counter);
      if (sr == NULL)
        {
          delete copasiReaction;
          fatalError();
        }
      if (sr->isSetStoichiometryMath())
        {
          delete copasiReaction;
          CCopasiMessage::CCopasiMessage(CCopasiMessage::EXCEPTION, MCSBML + 10);
        }
      float stoich = sr->getStoichiometry() / sr->getDenominator();
      std::map<std::string, CMetab*>::iterator pos;
      pos = this->speciesMap.find(sr->getSpecies());
      if (pos == this->speciesMap.end())
        {
          delete copasiReaction;
          fatalError();
        }
      if (compartment == NULL)
        {
          compartment = pos->second->getCompartment();
        }
      else
        {
          if (singleCompartment && compartment != pos->second->getCompartment())
            {
              singleCompartment = false;
            }
        }
      copasiReaction->addSubstrate(pos->second->getKey(), stoich);
    }

  /* Add all products to the reaction */
  num = sbmlReaction->getNumProducts();
  for (counter = 0; counter < num; counter++)
    {
      SpeciesReference* sr = sbmlReaction->getProduct(counter);
      if (sr == NULL)
        {
          delete copasiReaction;
          fatalError();
        }
      if (sr->isSetStoichiometryMath())
        {
          delete copasiReaction;
          CCopasiMessage::CCopasiMessage(CCopasiMessage::EXCEPTION, MCSBML + 10);
        }
      float stoich = sr->getStoichiometry() / sr->getDenominator();
      std::map<std::string, CMetab*>::iterator pos;
      pos = this->speciesMap.find(sr->getSpecies());
      if (pos == this->speciesMap.end())
        {
          delete copasiReaction;
          fatalError();
        }
      if (compartment == NULL)
        {
          compartment = pos->second->getCompartment();
        }
      else
        {
          if (singleCompartment && compartment != pos->second->getCompartment())
            {
              singleCompartment = false;
            }
        }
      copasiReaction->addProduct(pos->second->getKey(), stoich);
    }

  /* Add all modifiers to the reaction */
  num = sbmlReaction->getNumModifiers();
  for (counter = 0; counter < num; counter++)
    {
      ModifierSpeciesReference* sr = sbmlReaction->getModifier(counter);
      if (sr == NULL)
        {
          delete copasiReaction;
          fatalError();
        }
      std::map<std::string, CMetab*>::iterator pos;
      pos = this->speciesMap.find(sr->getSpecies());
      if (pos == this->speciesMap.end())
        {
          delete copasiReaction;
          fatalError();
        }
      if (compartment == NULL)
        {
          compartment = pos->second->getCompartment();
        }
      else
        {
          if (singleCompartment && compartment != pos->second->getCompartment())
            {
              singleCompartment = false;
            }
        }
      copasiReaction->addModifier(pos->second->getKey());
    }

  /* in the newly created CFunction set the types for all parameters and
   * either a mapping or a value 
   */
  KineticLaw* kLaw = sbmlReaction->getKineticLaw();
  if (kLaw != NULL)
    {
      if (kLaw->isSetSubstanceUnits())
        {
          std::string cU = kLaw->getSubstanceUnits();
          if (cU != "substance")
            {
              delete copasiReaction;
              fatalError();
            }
        }
      if (kLaw->isSetTimeUnits())
        {
          std::string cU = kLaw->getTimeUnits();
          if (cU != "time")
            {
              delete copasiReaction;
              fatalError();
            }
        }

      if (!kLaw->isSetMath())
        {
          kLaw->setMathFromFormula();
        }

      for (counter = 0; counter < kLaw->getNumParameters();++counter)
        {
          Parameter* pSBMLParameter = kLaw->getParameter(counter);
          std::string id;
          if (this->mLevel == 1)
            {
              id = pSBMLParameter->getName();
            }
          else
            {
              id = pSBMLParameter->getId();
            }
          copasiReaction->getParameters().addParameter(id, CCopasiParameter::DOUBLE, pSBMLParameter->getValue());
        }

      const ASTNode* kLawMath = kLaw->getMath();
      if (kLawMath == NULL)
        {
          fatalError();
        }
      ASTNode* node = new ConverterASTNode(*kLawMath);

      node = this->replaceUserDefinedFunctions(node, sbmlModel);
      if (node == NULL)
        {
          delete copasiReaction;
          fatalError();
        }

      this->replacePowerFunctionNodes(node);
      this->replaceLog((ConverterASTNode*)node);
      this->replaceRoot((ConverterASTNode*)node);
      /* if it is a single compartment reaction, we have to divide the whole kinetic
      ** equation by the compartment because copasi expects
      ** kinetic laws that specify concentration/time for single compartment
      ** reactions.
      */
      if (singleCompartment)
        {
          if (compartment != NULL)
            {
              ConverterASTNode* tmpNode1 = new ConverterASTNode();
              tmpNode1->setType(AST_DIVIDE);
              tmpNode1->addChild(node);
              ConverterASTNode* tmpNode2 = new ConverterASTNode();
              tmpNode2->setType(AST_NAME);
              tmpNode2->setName(compartment->getSBMLId().c_str());
              tmpNode1->addChild(tmpNode2);
              node = tmpNode1;
            }
          else
            {
              delete copasiReaction;
              fatalError();
            }
        }

      /* Create a new user defined CKinFunction */
      if (!sbmlId2CopasiCN(node, copasi2sbmlmap, copasiReaction->getParameters())) fatalError();
      CEvaluationNode* pExpressionTreeRoot = CEvaluationTree::convertASTNode(*node);
      if (pExpressionTreeRoot)
        {
          CEvaluationTree* pTmpTree = CEvaluationTree::create(CEvaluationTree::Expression);
          std::set<std::string> dummySet;
          this->replaceCallNodeNames(pTmpTree->getRoot(), dummySet);
          pTmpTree->setRoot(pExpressionTreeRoot);
          // check if the root node is a simple function call
          if (this->isSimpleFunctionCall(pExpressionTreeRoot))
            {
              // if yes, we check if it corresponds to an already existing function
              std::string functionName = this->mFunctionNameMapping[pExpressionTreeRoot->getData()];
              CFunction* tree = dynamic_cast<CFunction*>(pTmpFunctionDB->findFunction(functionName));
              if (!tree)
                {
                  tree = dynamic_cast<CFunction*>(this->functionDB->findFunction(functionName));
                }
              assert(tree);
              CFunction* pExistingFunction = this->findCorrespondingFunction(tree, pExpressionTreeRoot, copasiReaction);
              // if it does, we set the existing function for this reaction
              if (pExistingFunction)
                {
                  copasiReaction->setFunction(pExistingFunction);
                  // do the mapping
                  this->doMapping(copasiReaction, dynamic_cast<const CEvaluationNodeCall*>(pExpressionTreeRoot));
                }
              // else we take the function from the pTmpFunctionDB, copy it and set the usage correctly
              else
                {
                  CFunction* pFun = this->copyFunction(tree);
                  // we need a unique name for the function
                  std::string appendix = "";
                  unsigned int counter = 0;
                  std::string functionName = pFun->getObjectName();
                  std::ostringstream numberStream;
                  while (this->functionDB->findFunction(functionName + appendix) || pTmpFunctionDB->findFunction(functionName + appendix))
                    {
                      counter++;
                      numberStream.str("");
                      numberStream << "_" << counter;
                      appendix = numberStream.str();
                    }
                  pFun->setObjectName(functionName + appendix);
                  this->setCorrectUsage(copasiReaction, dynamic_cast<const CEvaluationNodeCall*>(pExpressionTreeRoot));
                  this->doMapping(copasiReaction, dynamic_cast<const CEvaluationNodeCall*>(pExpressionTreeRoot));
                  this->functionDB->add(pFun, true);
                  this->newFunctionKeys.push_back(copasiReaction->getFunction().getKey());
                }
            }
          else
            {
              if (!copasiReaction->setFunctionFromExpressionTree(pTmpTree, copasi2sbmlmap, this->functionDB))
                {
                  // error message
                  CCopasiMessage::CCopasiMessage(CCopasiMessage::EXCEPTION, MCSBML + 9, copasiReaction->getObjectName().c_str());
                }
              else
                {
                  this->newFunctionKeys.push_back(copasiReaction->getFunction().getKey());
                }
            }
          // delete the temporary tree and all the nodes
          delete pTmpTree;
        }
      else
        {
          // error message
          CCopasiMessage::CCopasiMessage(CCopasiMessage::EXCEPTION, MCSBML + 8, copasiReaction->getObjectName().c_str());
        }
    }
  else
    {
      /* if no KineticLaw was defined for the reaction. */
      copasiReaction->setFunction(NULL);
    }
  //DebugFile << "Created reaction: " << copasiReaction->getObjectName() << std::endl;
  if (this->mLevel == 1)
    {
      copasiReaction->setSBMLId(sbmlReaction->getName());
    }
  else
    {
      copasiReaction->setSBMLId(sbmlReaction->getId());
    }
  return copasiReaction;
}

/**
 * Replaces SBML user defined functions with the actual function definition.
 */
ConverterASTNode*
SBMLImporter::replaceUserDefinedFunctions(ASTNode* node, const Model* sbmlModel)
{
  ConverterASTNode* newNode = new ConverterASTNode(*node);
  newNode->setChildren(new List());
  /* make the replacement recursively, depth first */
  unsigned int counter;
  for (counter = 0; counter < node->getNumChildren(); counter++)
    {
      ConverterASTNode* newChild = this->replaceUserDefinedFunctions(node->getChild(counter), sbmlModel);
      if (newChild == NULL)
        {
          //throw StdException("Error. Could not replace user defined functions.");
          fatalError();
        }
      newNode->addChild(newChild);
    }
  /* if the new node if a user defined function */
  if (newNode->getType() == AST_FUNCTION)
    {
      /* see if there is a corresponding user defined function */
      FunctionDefinition* funDef = this->getFunctionDefinitionForName(newNode->getName(), sbmlModel);
      if (funDef == NULL)
        {
          fatalError();
        }
      /* make a map that maps every parameter of the function definition to a
      ** node in the actual function call. */
      std::map<std::string, ASTNode*> map = this->createBVarMap(funDef->getMath()->getRightChild(), newNode);
      /* make a new node that replaces all call parameters with the actual
      ** parameters used in the function call. */
      newNode = this->replaceBvars(funDef->getMath()->getRightChild(), map);
    }
  return newNode;
}

/**
 * Creates a map of each parameter of the function definition and its
 * corresponding parameter in the function call.
 */
std::map<std::string, ASTNode*>
SBMLImporter::createBVarMap(const ASTNode* uDefFunction, const ASTNode* function)
{
  /* the first n-1 children, where n is the number of children, of a function definition ASTnode are the
   * arguments to the function. These correspond to the m=n-1 children of the
   * function call.
   */
  if (uDefFunction->getNumChildren() != function->getNumChildren() + 1)
    {
      std::string functionName = uDefFunction->getName();
      fatalError();
    }
  std::map<std::string, ASTNode*> varMap;
  unsigned int counter;
  for (counter = 0; counter < uDefFunction->getNumChildren() - 1; counter++)
    {
      varMap[uDefFunction->getChild(counter)->getName()] = function->getChild(counter);
    }
  return varMap;
}

/**
 * Returns the user defined SBML function definition that belongs to the given
 * name, or NULL if none can be found.
 */
FunctionDefinition*
SBMLImporter::getFunctionDefinitionForName(const std::string name, const Model* sbmlModel)
{
  FunctionDefinition* fDef = NULL;
  unsigned int counter;
  for (counter = 0; counter < sbmlModel->getNumFunctionDefinitions(); counter++)
    {
      std::string functionName = sbmlModel->getFunctionDefinition(counter)->getName();
      if (sbmlModel->getFunctionDefinition(counter)->isSetId())
        {
          functionName = sbmlModel->getFunctionDefinition(counter)->getId();
        }
      if (functionName == name)
        {
          fDef = sbmlModel->getFunctionDefinition(counter);
          break;
        }
    }
  return fDef;
}

/**
 * Replaces the variables in a function definition with the actual function
 * parameters that were used when the function was called. The function returns
 * a pointer to the ConverterAST node with the replaced variables.
 */
ConverterASTNode*
SBMLImporter::replaceBvars(const ASTNode* node, std::map<std::string, ASTNode*> bvarMap)
{
  ConverterASTNode* newNode = NULL;
  if (node->isName())
    {
      /* check if name matches any in bvarMap */
      /* if yes, replace node with node in bvarMap */
      /* node needs to be set to be a deep copy of the replacement */
      if (bvarMap.find(node->getName()) != bvarMap.end())
        {
          newNode = new ConverterASTNode(*bvarMap[node->getName()]);
        }
    }
  else
    {
      newNode = new ConverterASTNode(*node);
      newNode->setChildren(new List());
      unsigned int counter;
      for (counter = 0; counter < node->getNumChildren(); counter++)
        {
          newNode->addChild(this->replaceBvars(node->getChild(counter), bvarMap));
        }
    }
  return newNode;
}

/**
 * Constructor that initializes speciesMap and the FunctionDB object
 */
SBMLImporter::SBMLImporter()
{
  this->speciesMap = std::map<std::string, CMetab*>();
  this->functionDB = NULL;
  this->mIncompleteModel = false;
}

/**
 * Destructor that does nothing. 
 */
SBMLImporter::~SBMLImporter()
{}

/**
 * This function replaces the AST_FUNCTION_POWER ASTNodes in a ASTNode tree
 * with the AST_POWER node.
 */
void SBMLImporter::replacePowerFunctionNodes(ASTNode* node)
{
  if (node != NULL)
    {
      if (node->getType() == AST_FUNCTION_POWER)
        {
          //node->setType(AST_POWER);
          node->setCharacter('^');
        }
      unsigned int counter;
      for (counter = 0; counter < node->getNumChildren(); counter++)
        {
          this->replacePowerFunctionNodes(node->getChild(counter));
        }
    }
}

/**
 * Function reads an SBML file with libsbml and converts it to a Copasi CModel
 * object which is returned. Deletion of the returned pointer is up to the
 * caller.
 */
CModel*
SBMLImporter::readSBML(std::string filename, CFunctionDB* funDB, SBMLDocument* pSBMLDocument, std::map<CCopasiObject*, SBase*>& copasi2sbmlmap)
{
  this->mpCopasiModel = NULL;
  if (funDB != NULL)
    {
      this->functionDB = funDB;
      SBMLReader* reader = new SBMLReader(XML_SCHEMA_VALIDATION_NONE);
      SBMLDocument* sbmlDoc = reader->readSBML(filename);
      sbmlDoc->validate();
      if (sbmlDoc->getNumFatals() > 0)
        {
          ParseMessage * pSBMLMessage = sbmlDoc->getFatal(0);

          CCopasiMessage Message(CCopasiMessage::RAW, MCXML + 2,
                                 pSBMLMessage->getLine(),
                                 pSBMLMessage->getColumn(),
                                 pSBMLMessage->getMessage().c_str());

          return NULL;
        }
      else if (sbmlDoc->getNumErrors() > 0)
        {
          ParseMessage * pSBMLMessage = sbmlDoc->getError(0);
          /* some level 1 files have an annotation in the wrong place
           * This is considered an error by libsbml, but
           * it does not really affect the model, so we try to
           * read it anyway.
           */
          if ((sbmlDoc->getNumErrors() > 1) ||
              (strncmp(pSBMLMessage->getMessage().c_str(),
                       "The <sbml> element cannot contain an <annotation>.  Use the <model> element instead."
                       , 85) != 0))
            {
              CCopasiMessage Message(CCopasiMessage::RAW, MCXML + 2,
                                     pSBMLMessage->getLine(),
                                     pSBMLMessage->getColumn(),
                                     pSBMLMessage->getMessage().c_str());

              return NULL;
            }
          else
            {
              CCopasiMessage Message(CCopasiMessage::WARNING, MCSBML + 6);
            }
        }
      if (sbmlDoc->getModel() == NULL)
        {
          CCopasiMessage Message(CCopasiMessage::ERROR, MCSBML + 2);
          return NULL;
        }

      delete reader;
      //DebugFile << "Number of Compartments: " << sbmlDoc->getModel()->getNumCompartments() << std::endl;
      //DebugFile << "Number of Metabolites: "  << sbmlDoc->getModel()->getNumSpecies() << std::endl;
      //DebugFile << "Number of Reactions: "    << sbmlDoc->getModel()->getNumReactions()  << std::endl;
      pSBMLDocument = sbmlDoc;
      this->mLevel = pSBMLDocument->getLevel();
      if (mLevel == 1)
        {
          pSBMLDocument->setLevel(2);
          mLevel = pSBMLDocument->getLevel();
        }

      this->mpCopasiModel = this->createCModelFromSBMLDocument(sbmlDoc, copasi2sbmlmap);
    }
  else
    {
      //throw StdException("Error. readSBML needs a valid CFunctionDB object.");
      fatalError();
    }
  return this->mpCopasiModel;
}

/**
 * Returns the copasi QuantityUnit corresponding to the given SBML
 *  Substance UnitDefinition.
 */
CModel::QuantityUnit
SBMLImporter::handleSubstanceUnit(const UnitDefinition* uDef)
{
  CModel::QuantityUnit qUnit = CModel::Mol;
  if (uDef == NULL)
    {
      //DebugFile << "Argument to handleSubstanceUnit is NULL pointer." << std::endl;
      fatalError();
    }
  if (uDef->getNumUnits() == 1)
    {
      Unit* u = uDef->getUnit(0);
      if (u == NULL)
        {
          //DebugFile << "Expected Unit, got NULL pointer." << std::endl;
          fatalError();
        }
      if ((u->getKind() == UNIT_KIND_MOLE))
        {
          if ((u->getExponent() == 1) && (u->getMultiplier() == 1) && ((u->getScale() % 3) == 0) && (u->getScale() < 1) && (u->getScale() > -16))
            {
              switch (u->getScale())
                {
                case 0:
                  qUnit = CModel::Mol;
                  break;
                case - 3:
                  qUnit = CModel::mMol;
                  break;
                case - 6:
                  qUnit = CModel::microMol;
                  break;
                case - 9:
                  qUnit = CModel::nMol;
                  break;
                case - 12:
                  qUnit = CModel::pMol;
                  break;
                case - 15:
                  qUnit = CModel::fMol;
                  break;
                default:
                  //DebugFile << "Error. This value should never have been reached for the scale of the liter unit." << std::endl;
                  exit(1);
                  break;
                }
            }
          else
            {
              fatalError();
            }
        }
      else if ((u->getKind() == UNIT_KIND_ITEM))
        {
          if ((u->getExponent() == 1) && (u->getMultiplier() == 1) && (u->getScale() == 0))
            {
              qUnit = CModel::number;
            }
          else
            {
              fatalError();
            }
        }
      else
        {
          fatalError();
        }
    }
  else
    {
      fatalError();
    }
  return qUnit;
}

/**
 * Returns the copasi TimeUnit corresponding to the given SBML Time
 *  UnitDefinition.
 */
CModel::TimeUnit
SBMLImporter::handleTimeUnit(const UnitDefinition* uDef)
{
  CModel::TimeUnit tUnit = CModel::s;
  if (uDef == NULL)
    {
      //DebugFile << "Argument to handleTimeUnit is NULL pointer." << std::endl;
      fatalError();
    }
  if (uDef->getNumUnits() == 1)
    {
      Unit* u = uDef->getUnit(0);
      if (u == NULL)
        {
          //DebugFile << "Expected Unit, got NULL pointer." << std::endl;
          fatalError();
        }
      if ((u->getKind() == UNIT_KIND_SECOND))
        {
          if ((u->getExponent() == 1) && ((u->getScale() % 3) == 0) && (u->getScale() < 1) && (u->getScale() > -16))
            {
              if (u->getMultiplier() == 1.0)
                {
                  switch (u->getScale())
                    {
                    case 0:
                      tUnit = CModel::s;
                      break;
                    case - 3:
                      tUnit = CModel::ms;
                      break;
                    case - 6:
                      tUnit = CModel::micros;
                      break;
                    case - 9:
                      tUnit = CModel::ns;
                      break;
                    case - 12:
                      tUnit = CModel::ps;
                      break;
                    case - 15:
                      tUnit = CModel::fs;
                      break;
                    default:
                      //DebugFile << "Error. This value should never have been reached for the scale of the time unit." << std::endl;
                      exit(1);
                      break;
                    }
                }
              else if (u->getMultiplier() == 60.0)
                {
                  tUnit = CModel::m;
                }
              else if (u->getMultiplier() == 3600.0)
                {
                  tUnit = CModel::h;
                }
              else if (u->getMultiplier() == 86400.0)
                {
                  tUnit = CModel::d;
                }
              else
                {
                  fatalError();
                }
            }
          else
            {
              fatalError();
            }
        }
      else
        {
          fatalError();
        }
    }
  else
    {
      fatalError();
    }
  return tUnit;
}

/**
 * Returns the copasi VolumeUnit corresponding to the given SBML Volume
 *  UnitDefinition.
 */
CModel::VolumeUnit
SBMLImporter::handleVolumeUnit(const UnitDefinition* uDef)
{
  CModel::VolumeUnit vUnit = CModel::l;
  if (uDef == NULL)
    {
      //DebugFile << "Argument to handleVolumeUnit is NULL pointer." << std::endl;
      fatalError();
    }
  if (uDef->getNumUnits() == 1)
    {
      Unit* u = uDef->getUnit(0);
      if (u == NULL)
        {
          //DebugFile << "Expected Unit, got NULL pointer." << std::endl;
          fatalError();
        }
      if ((u->getKind() == UNIT_KIND_LITER) || (u->getKind() == UNIT_KIND_LITRE))
        {
          if ((u->getExponent() == 1) && (u->getMultiplier() == 1) && ((u->getScale() % 3) == 0) && (u->getScale() < 1) && (u->getScale() > -16))
            {
              switch (u->getScale())
                {
                case 0:
                  vUnit = CModel::l;
                  break;
                case - 3:
                  vUnit = CModel::ml;
                  break;
                case - 6:
                  vUnit = CModel::microl;
                  break;
                case - 9:
                  vUnit = CModel::nl;
                  break;
                case - 12:
                  vUnit = CModel::pl;
                  break;
                case - 15:
                  vUnit = CModel::fl;
                  break;
                default:
                  //DebugFile << "Error. This value should never have been reached for the scale of the liter unit." << std::endl;
                  exit(1);
                  break;
                }
            }
          else
            {
              fatalError();
            }
        }
      else if ((u->getKind() == UNIT_KIND_METER) || (u->getKind() == UNIT_KIND_METRE))
        {
          if ((u->getExponent() == 3) && (u->getMultiplier() == 1) && (u->getScale() == 0))
            {
              vUnit = CModel::m3;
            }
          else
            {
              fatalError();
            }
        }
      else
        {
          fatalError();
        }
    }
  else
    {
      fatalError();
    }
  return vUnit;
}

void SBMLImporter::replaceLog(ConverterASTNode* sourceNode)
{
  if (sourceNode->getType() == AST_FUNCTION_LOG && sourceNode->getNumChildren() == 2)
    {
      List* l = new List();
      ConverterASTNode* child1 = (ConverterASTNode*)sourceNode->getChild(0);
      ConverterASTNode* child2 = (ConverterASTNode*)sourceNode->getChild(1);
      ConverterASTNode* logNode1 = new ConverterASTNode(AST_FUNCTION_LOG);
      l->add(child1);
      logNode1->setChildren(l);
      ConverterASTNode* logNode2 = new ConverterASTNode(AST_FUNCTION_LOG);
      l = new List();
      l->add(child2);
      logNode2->setChildren(l);
      l = new List();
      l->add(logNode2);
      l->add(logNode1);
      sourceNode->setChildren(l);
      sourceNode->setType(AST_DIVIDE);
      // go down the children and replace logs
      this->replaceLog(child1);
      this->replaceLog(child2);
    }
  else
    {
      // go down to the children and replace logs
      unsigned int i;
      for (i = 0; i < sourceNode->getNumChildren();++i)
        {
          this->replaceLog((ConverterASTNode*)sourceNode->getChild(i));
        }
    }
}

void SBMLImporter::replaceRoot(ConverterASTNode* sourceNode)
{
  if (sourceNode->getType() == AST_FUNCTION_ROOT && sourceNode->getNumChildren() == 2)
    {
      ConverterASTNode* child1 = (ConverterASTNode*)sourceNode->getChild(0);
      ConverterASTNode* child2 = (ConverterASTNode*)sourceNode->getChild(1);
      ConverterASTNode* divideNode = new ConverterASTNode(AST_DIVIDE);
      ConverterASTNode* oneNode = new ConverterASTNode(AST_REAL);
      oneNode->setValue(1.0);
      List* l = new List();
      l->add(divideNode);
      divideNode->addChild(oneNode);
      divideNode->addChild(child1);

      List* l2 = new List();
      l2->add(child2);
      l2->add(divideNode);

      sourceNode->setChildren(l2);
      sourceNode->setType(AST_POWER);
      // go down the children and replace root functions
      this->replaceRoot(child1);
      this->replaceRoot(child2);
    }
  else
    {
      // go down to the children and replace root functions
      unsigned int i;
      for (i = 0; i < sourceNode->getNumChildren();++i)
        {
          this->replaceRoot((ConverterASTNode*)sourceNode->getChild(i));
        }
    }
}

CModelValue* SBMLImporter::createCModelValueFromParameter(const Parameter* sbmlParameter, CModel* copasiModel, std::map<CCopasiObject*, SBase*>& copasi2sbmlmap)
{
  if (sbmlParameter->isSetUnits())
    {
      std::string cU = sbmlParameter->getUnits();
      if (cU != "volume" && cU != "area" && cU != "length")
        {
          fatalError();
        }
      else if (cU == "area" || cU == "length")
        {
          /* !!!! create a warning that the units will be ignored. */
        }
    }
  std::string name = sbmlParameter->getName();
  if (!sbmlParameter->isSetName())
    {
      name = sbmlParameter->getId();
    }
  std::string appendix = "";
  unsigned int counter = 2;
  std::ostringstream numberStream;
  while (copasiModel->getModelValues().getIndex(name + appendix) != static_cast < unsigned C_INT32
         > (-1))
    {
      numberStream.str("");
      numberStream << "_" << counter;
      counter++;
      appendix = numberStream.str();
    }
  double value;
  if (sbmlParameter->isSetValue())
    {
      value = sbmlParameter->getValue();
    }
  else
    {
      // Set value to NaN and create a warning if it is the first time
      // this happend
      value = std::numeric_limits<C_FLOAT64>::quiet_NaN();
      if (!this->mIncompleteModel)
        {
          this->mIncompleteModel = true;
          CCopasiMessage Message(CCopasiMessage::WARNING, MCSBML + 7);
        }
    }
  CModelValue* pMV = copasiModel->createModelValue(name + appendix, value);
  copasi2sbmlmap[pMV] = const_cast<Parameter*>(sbmlParameter);
  if (this->mLevel == 1)
    {
      pMV->setSBMLId(sbmlParameter->getName());
    }
  else
    {
      pMV->setSBMLId(sbmlParameter->getId());
    }
  return pMV;
}

bool SBMLImporter::sbmlId2CopasiCN(ASTNode* pNode, std::map<CCopasiObject*, SBase*>& copasi2sbmlmap, CCopasiParameterGroup& pParamGroup)
{
  bool success = true;
  unsigned int i, iMax = pNode->getNumChildren();
  if (pNode->getType() == AST_NAME)
    {
      Compartment* pSBMLCompartment = NULL;
      Species* pSBMLSpecies = NULL;
      Reaction* pSBMLReaction = NULL;
      Parameter* pSBMLParameter = NULL;
      std::string sbmlId;
      CCopasiParameter* pParam = pParamGroup.getParameter(pNode->getName());
      if (pParam)
        {
          pNode->setName(pParam->getCN().c_str());
        }
      else
        {
          std::map<CCopasiObject*, SBase*>::iterator it = copasi2sbmlmap.begin();
          std::map<CCopasiObject*, SBase*>::iterator endIt = copasi2sbmlmap.end();
          while (it != endIt)
            {
              SBMLTypeCode_t type = it->second->getTypeCode();
              switch (type)
                {
                case SBML_COMPARTMENT:
                  pSBMLCompartment = dynamic_cast<Compartment*>(it->second);
                  if (this->mLevel == 1)
                    {
                      sbmlId = pSBMLCompartment->getName();
                    }
                  else
                    {
                      sbmlId = pSBMLCompartment->getId();
                    }
                  if (sbmlId == pNode->getName())
                    {
                      pNode->setName(dynamic_cast<CCompartment*>(it->first)->getObject(CCopasiObjectName("Reference=InitialVolume"))->getCN().c_str());
                    }
                  break;
                case SBML_SPECIES:
                  pSBMLSpecies = dynamic_cast<Species*>(it->second);
                  if (this->mLevel == 1)
                    {
                      sbmlId = pSBMLSpecies->getName();
                    }
                  else
                    {
                      sbmlId = pSBMLSpecies->getId();
                    }
                  if (sbmlId == pNode->getName())
                    {
                      pNode->setName(dynamic_cast<CMetab*>(it->first)->getObject(CCopasiObjectName("Reference=InitialConcentration"))->getCN().c_str());
                    }
                  break;
                case SBML_REACTION:
                  pSBMLReaction = dynamic_cast<Reaction*>(it->second);
                  if (this->mLevel == 1)
                    {
                      sbmlId = pSBMLReaction->getName();
                    }
                  else
                    {
                      sbmlId = pSBMLReaction->getId();
                    }
                  if (sbmlId == pNode->getName())
                    {
                      pNode->setName(dynamic_cast<CReaction*>(it->first)->getObject(CCopasiObjectName("Reference=ParticleFlux"))->getCN().c_str());
                    }

                  break;
                case SBML_PARAMETER:
                  pSBMLParameter = dynamic_cast<Parameter*>(it->second);
                  if (this->mLevel == 1)
                    {
                      sbmlId = pSBMLParameter->getName();
                    }
                  else
                    {
                      sbmlId = pSBMLParameter->getId();
                    }
                  if (sbmlId == pNode->getName())
                    {
                      pNode->setName(dynamic_cast<CModelValue*>(it->first)->getObject(CCopasiObjectName("Reference=Value"))->getCN().c_str());
                    }
                  break;
                default:
                  break;
                }
              ++it;
            }
        }
    }
  for (i = 0; i < iMax;++i)
    {
      if (!this->sbmlId2CopasiCN(pNode->getChild(i), copasi2sbmlmap, pParamGroup))
        {
          success = false;
          break;
        }
    }
  return success;
}

void SBMLImporter::printMap(const std::map<CCopasiObject*, SBase*>& map)
{
  std::map<CCopasiObject*, SBase*>::const_iterator it = map.begin();
  std::map<CCopasiObject*, SBase*>::const_iterator end = map.end();
  std::cout << "Number of elements: " << map.size() << std::endl;
  while (it != end)
    {
      std::cout << "(@" << it->first << ")" << it->first->getObjectName() << " : " << "(@" << it->second << ")" << it->second->getTypeCode() << std::endl;
      ++it;
    }
  std::cout << std::endl;
}

void SBMLImporter::restoreFunctionDB()
{
  // set all the old sbml ids
  std::map<CEvaluationTree*, std::string>::iterator it = this->sbmlIdMap.begin();
  std::map<CEvaluationTree*, std::string>::iterator endIt = this->sbmlIdMap.end();
  while (it != endIt)
    {
      it->first->setSBMLId(it->second);
      ++it;
    }

  // remove all the functions that were added during import
  std::vector<std::string>::iterator it2 = this->newFunctionKeys.begin();
  std::vector<std::string>::iterator endIt2 = this->newFunctionKeys.end();
  while (it2 != endIt2)
    {
      this->functionDB->removeFunction(*it2);
      ++it2;
    }
}

void SBMLImporter::replaceCallNodeNames(CEvaluationNode* node, std::set<std::string>& usedFunctions)
{
  if (node)
    {
      CEvaluationNodeCall* pCallNode = dynamic_cast<CEvaluationNodeCall*>(node);
      if (pCallNode)
        {
          std::map<std::string, std::string>::const_iterator pos = this->mFunctionNameMapping.find(pCallNode->getData());

          if (pos == this->mFunctionNameMapping.end())
            {
              fatalError();
            }
          std::string newName = pos->second;
          pCallNode->setData(newName);
          usedFunctions.insert(newName);
        }
      CEvaluationNode* pChildNode = static_cast<CEvaluationNode*>(node->getChild());
      while (pChildNode)
        {
          this->replaceCallNodeNames(pChildNode, usedFunctions);
          pChildNode = static_cast<CEvaluationNode*>(pChildNode->getSibling());
        }
    }
}

/**
 * The methods gets a function where all the parameters have a usage of "PARAMETER".
 * In addition it get the root node of a call to that function which is an expression
 * and contains the acutal objects with which the function is called in a certain reaction.
 * From this expression we can determine if there already is a function in the database
 * that does the same. Or we can find out if this function is a Mass Action kinetic.
 */
CFunction* SBMLImporter::findCorrespondingFunction(const CFunction* tree, const CEvaluationNode* pRootNode, const CReaction* pCopasiReaction)
{
  CFunction* pCorrespondingFunction = NULL;
  if (this->isMassAction(pRootNode, &pCopasiReaction->getChemEq().getSubstrates(), pCopasiReaction->isReversible(), &pCopasiReaction->getChemEq().getProducts()))
    {
      if (pCopasiReaction->isReversible())
        {
          pCorrespondingFunction = static_cast<CFunction*>(this->functionDB->findFunction("Mass action (reversible)"));
        }
      else
        {
          pCorrespondingFunction = static_cast<CFunction*>(this->functionDB->findFunction("Mass action (irreversible)"));
        }
      if (!pCorrespondingFunction)
        {
          fatalError();
        }
    }
  else
    {
      CCopasiVector<CFunction>* functions = this->functionDB->suitableFunctions(pCopasiReaction->getChemEq().getSubstrates().size(), pCopasiReaction->getChemEq().getProducts().size(), (TriLogic)pCopasiReaction->isReversible());
      unsigned int i, iMax = functions->size();
      for (i = 0; i < iMax;++i)
        {
          CFunction* pFun = ((*functions)[i]);
          // make sure the function is not compared to itself since it can already
          // be in the database if it has been used a call in another function
          // don't compare the mass action kinetics
          if ((pFun != tree) && (!dynamic_cast<CMassAction*>(pFun)) && this->areEqualFunctions(pFun, tree))
            {
              pCorrespondingFunction = pFun;
              break;
            }
        }
      delete functions;
    }
  return pCorrespondingFunction;
}

bool SBMLImporter::areEqualFunctions(const CFunction* pFun, const CFunction* pFun2)
{
  bool result = true;
  const CFunctionParameters& funParams1 = pFun->getVariables();
  const CFunctionParameters& funParams2 = pFun2->getVariables();
  if (funParams1.size() == funParams2.size())
    {
      unsigned int i, iMax = funParams1.size();
      for (i = 0; i < iMax;++i)
        {
          const CFunctionParameter* pFunParam1 = funParams1[i];
          const CFunctionParameter* pFunParam2 = funParams2[i];
          if (pFunParam1->getObjectName() != pFunParam2->getObjectName())
            {
              result = false;
              break;
            }
        }
      if (result == true)
        {
          const CEvaluationNode* pNodeFun1 = static_cast<const CEvaluationNode*>(pFun->getRoot());
          const CEvaluationNode* pNodeFun2 = static_cast<const CEvaluationNode*>(pFun2->getRoot());
          while (pNodeFun1 || pNodeFun2)
            {
              if ((!pNodeFun1) || (!pNodeFun2) || (pNodeFun1->getType() != pNodeFun2->getType()) || (pNodeFun1->getData() != pNodeFun2->getData()))
                {
                  result = false;
                  break;
                }
              if (pNodeFun1->getSibling() || pNodeFun2->getSibling())
                {
                  pNodeFun1 = static_cast<const CEvaluationNode*>(pNodeFun1->getSibling());
                  pNodeFun2 = static_cast<const CEvaluationNode*>(pNodeFun2->getSibling());
                }
              else
                {
                  pNodeFun1 = static_cast<const CEvaluationNode*>(pNodeFun1->getParent()->getSibling());
                  pNodeFun2 = static_cast<const CEvaluationNode*>(pNodeFun2->getParent()->getSibling());
                }
            }
        }
    }
  else
    {
      result = false;
    }
  return result;
}

bool SBMLImporter::isMassAction(const CEvaluationNode* pRootNode, const CCopasiVector<CChemEqElement>* substrates, bool reversible, const CCopasiVector<CChemEqElement>* products)
{
  bool result = true;
  if (reversible)
    {
      const CEvaluationNodeOperator* op = dynamic_cast<const CEvaluationNodeOperator*>(pRootNode);
      if (op && (((CEvaluationNodeOperator::SubType)CEvaluationNode::subType(op->getType())) == CEvaluationNodeOperator::MINUS))
        {
          if (!products) fatalError();
          result = this->isMassAction(static_cast<const CEvaluationNode*>(pRootNode->getChild()), substrates);
          if (result)
            {
              result = this->isMassAction(static_cast<const CEvaluationNode*>(pRootNode->getChild()->getSibling()), products);
            }
        }
    }
  else
    {
      std::vector<const CEvaluationNode*> arguments = std::vector<const CEvaluationNode*>();
      this->separateProductArguments(pRootNode, arguments);
      if (arguments.empty())
        {
          result = false;
        }
      else
        {
          // the number of arguments must be one more then the number of
          // products since it is a product of a constant and each product
          if (arguments.size() == substrates->size() + 1)
            {
              std::vector<CChemEqElement*> v = std::vector<CChemEqElement*>();
              unsigned int k, kMax = substrates->size();
              for (k = 0; k < kMax;++k)
                {
                  v.push_back((*substrates)[k]);
                }
              unsigned int numParameters = 0;
              unsigned int i, iMax = arguments.size();
              for (i = 0;(i < iMax) && (result == true);++i)
                {
                  // metabolites, model values and local parameters are object nodes
                  const CEvaluationNode* argument = arguments[i];
                  if (dynamic_cast<const CEvaluationNodeObject*>(argument))
                    {
                      std::vector<CCopasiContainer*> listOfContainers = std::vector<CCopasiContainer*>();
                      listOfContainers.push_back(this->mpCopasiModel);
                      const CCopasiObject* object = CCopasiContainer::ObjectFromName(listOfContainers, argument->getData().substr(1, argument->getData().length() - 2));
                      if (!object)
                        {
                          fatalError();
                        }
                      CCopasiVector<CChemEqElement>::iterator it = v.begin();
                      CCopasiVector<CChemEqElement>::iterator endIt = v.end();
                      if (dynamic_cast<const CCopasiObjectReference<C_FLOAT64>*>(object))
                        {
                          object = object->getObjectParent();
                          if (dynamic_cast<const CMetab*>(object))
                            {
                              const CMetab* metab = dynamic_cast<const CMetab*>(object);
                              while (it != endIt)
                                {
                                  CChemEqElement* element = *it;
                                  if ((&element->getMetabolite()) == metab)
                                    {
                                      v.erase(it);
                                      if (element->getMultiplicity() != 1.0)
                                        {
                                          result = false;
                                        }
                                      break;
                                    }
                                  ++it;
                                }
                              if (it == endIt)
                                {
                                  result = false;
                                }
                            }
                          else if (dynamic_cast<const CModelValue*>(object))
                            {
                              numParameters++;
                              if (numParameters > 1)
                                {
                                  result = false;
                                  break;
                                }
                            }
                          else
                            {
                              result = false;
                              break;
                            }
                        }
                      else
                        {
                          if (dynamic_cast<const CCopasiParameter*>(object))
                            {
                              numParameters++;
                              if (numParameters > 1)
                                {
                                  result = false;
                                  break;
                                }
                            }
                          else
                            {
                              result = false;
                              break;
                            }
                        }
                    }
                  // it could be the power operator
                  else if (dynamic_cast<const CEvaluationNodeOperator*>(argument) && ((CEvaluationNodeOperator::SubType)CEvaluationNode::subType(argument->getType()) == CEvaluationNodeOperator::POWER))
                    {
                      // so the arguments to this must be a metabolite
                      // and its stoichiometry
                      const CEvaluationNodeObject* pObjectNode = dynamic_cast<const CEvaluationNodeObject*>(argument->getChild());
                      const CEvaluationNodeNumber* pNumberNode = dynamic_cast<const CEvaluationNodeNumber*>(argument->getChild()->getSibling());
                      if ((!pObjectNode) || (!pNumberNode))
                        {
                          result = false;
                          break;
                        }
                      std::vector<CCopasiContainer*> listOfContainers = std::vector<CCopasiContainer*>();
                      listOfContainers.push_back(this->mpCopasiModel);
                      const CCopasiObject* object = CCopasiContainer::ObjectFromName(listOfContainers, pObjectNode->getData().substr(1, pObjectNode->getData().length() - 2));
                      if (!object)
                        {
                          fatalError();
                        }
                      const CMetab* metab = dynamic_cast<const CMetab*>(object);
                      if (!metab)
                        {
                          result = false;
                          break;
                        }
                      CCopasiVector<CChemEqElement>::iterator it = v.begin();
                      CCopasiVector<CChemEqElement>::iterator endIt = v.end();

                      while (it != endIt)
                        {
                          CChemEqElement* element = *it;
                          if ((&element->getMetabolite()) == metab)
                            {
                              v.erase(it);
                              if (element->getMultiplicity() != (*pNumberNode->getValuePointer()))
                                {
                                  result = false;
                                }
                              break;
                            }
                          ++it;
                        }
                      if (it == endIt)
                        {
                          result = false;
                        }
                    }
                }
              if ((!v.empty()) || (numParameters != 1))
                {
                  result = false;
                }
            }
          else
            {
              result = false;
            }
        }
    }
  return result;
}

void SBMLImporter::separateProductArguments(const CEvaluationNode* pRootNode, std::vector<const CEvaluationNode*>& arguments)
{
  const CEvaluationNodeOperator* pMultiplyNode = dynamic_cast<const CEvaluationNodeOperator*>(pRootNode);
  if (pMultiplyNode && (((CEvaluationNodeOperator::SubType)CEvaluationNode::subType(pMultiplyNode->getType())) == CEvaluationNodeOperator::MULTIPLY))
    {
      // check if one if the children is an object node or a power operator, if so,
      // add the node to the vector
      // the nodes not one of those two are passed to this function recursively.
      const CEvaluationNode* pChildNode = static_cast<const CEvaluationNode*>(pMultiplyNode->getChild());
      while (pChildNode)
        {
          const CEvaluationNodeObject* pObjectNode = dynamic_cast<const CEvaluationNodeObject*>(pChildNode);
          const CEvaluationNodeOperator* pOperatorNode = dynamic_cast<const CEvaluationNodeOperator*>(pChildNode);
          if (pObjectNode || (pOperatorNode && (((CEvaluationNodeOperator::SubType)CEvaluationNode::subType(pOperatorNode->getType())) == CEvaluationNodeOperator::POWER)))
            {
              arguments.push_back(pObjectNode);
            }
          else
            {
              this->separateProductArguments(pChildNode, arguments);
              if (arguments.empty())
                {
                  // it is not a mass action kinetic, so we can stop here
                  break;
                }
            }
          pChildNode = static_cast<const CEvaluationNode*>(pChildNode->getSibling());
        }
    }
  else
    {
      arguments.clear();
    }
}

void SBMLImporter::setCorrectUsage(CReaction* pCopasiReaction, const CEvaluationNodeCall* pCallNode)
{
  // find out what type each argument in the call node has.
  // it can be a local parameter, a global parameter, a compartment or a metabolite
  // if it is a metabolite, try to find out if it is a substrate, product, or modifier
  if (!pCallNode)
    {
      fatalError();
    }
  const CEvaluationNode* pChildNode = static_cast<const CEvaluationNode*>(pCallNode->getChild());
  std::vector<CCopasiContainer*> listOfContainers = std::vector<CCopasiContainer*>();
  listOfContainers.push_back(this->mpCopasiModel);
  CChemEq& pChemEq = pCopasiReaction->getChemEq();
  std::map<const CChemEqElement*, CChemEq::MetaboliteRole> v;
  const CCopasiVector<CChemEqElement>* pV = &pChemEq.getSubstrates();
  unsigned int i, iMax = pV->size();
  for (i = 0; i < iMax;++i)
    {
      v[(*pV)[i]] = CChemEq::SUBSTRATE;
    }
  pV = &pChemEq.getProducts();
  iMax = pV->size();
  for (i = 0; i < iMax;++i)
    {
      v[(*pV)[i]] = CChemEq::PRODUCT;
    }
  pV = &pChemEq.getModifiers();
  iMax = pV->size();
  for (i = 0; i < iMax;++i)
    {
      v[(*pV)[i]] = CChemEq::MODIFIER;
    }
  unsigned int parameterIndex = 0;
  while (pChildNode)
    {
      const CEvaluationNodeObject* pObjectNode = dynamic_cast<const CEvaluationNodeObject*>(pChildNode);
      if (!pObjectNode)
        {
          fatalError();
        }
      const CCopasiObject* object = CCopasiContainer::ObjectFromName(listOfContainers, pObjectNode->getData().substr(1, pObjectNode->getData().length() - 2));
      if (!object)
        {
          fatalError();
        }
      CFunctionParameter* pFunParam = const_cast<CFunctionParameter*>(pCopasiReaction->getFunction().getVariables()[parameterIndex]);

      if (dynamic_cast<const CCopasiObjectReference<C_FLOAT64>*>(object))
        {
          object = object->getObjectParent();
          if (dynamic_cast<const CMetab*>(object))
            {
              std::map<const CChemEqElement*, CChemEq::MetaboliteRole>::iterator it = v.begin();
              std::map<const CChemEqElement*, CChemEq::MetaboliteRole>::iterator endIt = v.end();
              while (it != endIt)
                {
                  if (it->first == object)
                    {
                      // get the role of the metabolite
                      switch (it->second)
                        {
                        case CChemEq::SUBSTRATE:
                          // it is a substrate
                          pFunParam->setUsage("SUBSTRATE");
                          break;
                        case CChemEq::PRODUCT:
                          // it is a product
                          pFunParam->setUsage("PRODUCT");
                          break;
                        case CChemEq::MODIFIER:
                          // it is a modifier
                          pFunParam->setUsage("MODIFIER");
                          break;
                        default:
                          fatalError();
                          break;
                        }
                    }
                  ++it;
                }
              if (it == endIt)
                {
                  fatalError();
                }
            }
          else if (dynamic_cast<const CModelValue*>(object))
            {
              // it is a global parameter
              pFunParam->setUsage("VARIABLE");
            }
          else if (dynamic_cast<const CCompartment*>(object))
            {
              // it is a volume
              pFunParam->setUsage("VOLUME");
            }
          else
            {
              fatalError()
            }
        }
      else
        {
          // it is a local parameter
          pFunParam->setUsage("PARAMETER");
        }
      pChildNode = static_cast<const CEvaluationNode*>(pChildNode->getSibling());
      ++parameterIndex;
    }
}

void SBMLImporter::doMapping(CReaction* pCopasiReaction, const CEvaluationNodeCall* pCallNode)
{
  // map the first argument of the call node to the first variable of the function of the reaction
  // and so on
  if (!pCallNode)
    {
      fatalError();
    }
  unsigned int i, iMax = pCopasiReaction->getFunction().getVariables().size();
  const CEvaluationNodeObject* pChild = dynamic_cast<const CEvaluationNodeObject*>(pCallNode->getChild());
  for (i = 0; i < iMax;++i)
    {
      if (!pChild)
        {
          fatalError();
        }
      std::string objectCN = pChild->getData();
      objectCN = objectCN.substr(1, objectCN.length() - 2);
      std::vector<CCopasiContainer*> listOfContainers;
      listOfContainers.push_back(this->mpCopasiModel);
      CCopasiObject* pObject = CCopasiContainer::ObjectFromName(listOfContainers, objectCN);
      assert(pObject);
      const std::string& objectKey = pObject->getKey();
      pCopasiReaction->setParameterMapping(i, objectKey);
      pChild = dynamic_cast<const CEvaluationNodeObject*>(pChild->getSibling());
    }
}

bool SBMLImporter::isSimpleFunctionCall(const CEvaluationNode* pRootNode)
{
  // it is a simple function call if it is a CEvaluationNodeCall object and all
  // its arguments are object nodes.
  bool result = true;
  if (dynamic_cast<const CEvaluationNodeCall*>(pRootNode))
    {
      const CEvaluationNode* pChildNode = static_cast<const CEvaluationNode*>(pRootNode->getChild());
      // I guess it must have at least one child to qualify.
      if (!pChildNode)
        {
          result = false;
        }
      while (pChildNode)
        {
          if (!dynamic_cast<const CEvaluationNodeObject*>(pChildNode))
            {
              result = false;
              break;
            }
          pChildNode = static_cast<const CEvaluationNode*>(pChildNode->getSibling());
        }
    }
  else
    {
      result = false;
    }
  return result;
}

CFunction* SBMLImporter::copyFunction(const CFunction* pFun)
{
  CFunction* pNewFun = new CFunction(*pFun);
  const CEvaluationNode* pRootNode = pFun->getRoot();
  CEvaluationNode* pNewRoot = CEvaluationNode::create(pRootNode->getType(), pRootNode->getData());
  const CEvaluationNode* pChildNode = static_cast<const CEvaluationNode*>(pRootNode->getChild());
  while (pChildNode)
    {
      pNewRoot->addChild(CEvaluationNode::create(pChildNode->getType(), pChildNode->getData()));
    }
  pNewFun->setRoot(pNewRoot);
  return pNewFun;
}

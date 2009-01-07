// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/xml/CCopasiXML.cpp,v $
//   $Revision: 1.110 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2009/01/07 19:40:34 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/*!
 \file CCopasiXML.cpp
 \brief Implementation file of class CCopasiXML.
 */

/**
 * CCopasiXML class.
 * This class implements a CCopasiXMLInterface to the COPASI XML specified in
 * http://www.copasi.org/schema/copasi.xsd
 *
 * Created for COPASI by Stefan Hoops 2003
 * Copyright Stefan Hoops
 */
#include <iostream>
#include <map>
#include <locale>

#include "copasi.h"

#include "CCopasiXML.h"
#include "CCopasiXMLParser.h"
#include "CCopasiXMLVersion.h"
#include "CFixLocalReactionParameters.h"

#include "CopasiDataModel/CCopasiDataModel.h"
#include "utilities/CCopasiVector.h"
#include "utilities/CSlider.h"
#include "model/CModel.h"
#include "model/CState.h"
#include "function/CFunctionDB.h"
#include "function/CEvaluationTree.h"
#include "report/CReportDefinitionVector.h"
#include "utilities/CCopasiTask.h"
#include "utilities/CCopasiMethod.h"
#include "utilities/CCopasiProblem.h"
#include "utilities/CDirEntry.h"
#include "utilities/utility.h"
#include "plot/COutputDefinitionVector.h"
#include "plot/CPlotItem.h"
#include "layout/CListOfLayouts.h"

// class CCopasiTask;
// class CCopasiReport;

CCopasiXML::CCopasiXML():
    CCopasiXMLInterface(),
    mpModel(NULL),
    mpFunctionList(NULL),
    mpTaskList(NULL),
    mpReportList(NULL),
    mpPlotList(NULL),
    mpGUI(NULL),
    mpLayoutList(NULL)
{
  mVersion.setVersion(COPASI_XML_VERSION_MAJOR,
                      COPASI_XML_VERSION_MINOR,
                      COPASI_XML_VERSION_BUILD,
                      COPASI_XML_VERSION_COMMENT);
}

CCopasiXML::~CCopasiXML() {}

bool CCopasiXML::save(std::ostream & os,
                      const std::string & relativeTo)
{
  mFilename = relativeTo;

  os.imbue(std::locale::classic());
  os.precision(16);

  mpOstream = &os;
  bool success = true;

  *mpOstream << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
  << std::endl;

  *mpOstream << "<!-- generated with COPASI "
  << CCopasiDataModel::Global->getVersion()->getVersion()
  << " (http://www.copasi.org) at "
  << UTCTimeStamp()
  << " UTC -->"
  << std::endl;

  CXMLAttributeList Attributes;
  Attributes.add("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
  Attributes.add("xsi:noNamespaceSchemaLocation",
                 "http://www.copasi.org/static/schema.xsd");
  Attributes.add("versionMajor", mVersion.getVersionMajor());
  Attributes.add("versionMinor", mVersion.getVersionMinor());
  Attributes.add("versionDevel", mVersion.getVersionDevel());

  startSaveElement("COPASI", Attributes);

  if (haveModel() && !haveFunctionList())
    {
      if (!buildFunctionList()) success = false;
      if (!saveFunctionList()) success = false;
      if (!freeFunctionList()) success = false;
    }
  else
    if (!saveFunctionList()) success = false;

  if (!saveModel()) success = false;
  if (!saveTaskList()) success = false;
  if (!saveReportList()) success = false;
  if (!savePlotList()) success = false;
  if (!saveGUI()) success = false;
  if (!saveLayoutList()) success = false;
  if (!saveSBMLReference()) success = false;

  endSaveElement("COPASI");

  return success;
}

bool CCopasiXML::load(std::istream & is,
                      const std::string & relativeTo)
{
  mFilename = relativeTo;

  is.imbue(std::locale::classic());
  is.precision(16);

  mpIstream = &is;
  bool success = true;
  bool done = false;

  CVersion FileVersion;
  CCopasiXMLParser Parser(FileVersion);

  Parser.setFunctionList(mpFunctionList);
  Parser.setModel(mpModel);
  Parser.setReportList(mpReportList);
  Parser.setTaskList(mpTaskList);
  Parser.setPlotList(mpPlotList);
  Parser.setGUI(mpGUI);
  Parser.setLayoutList(mpLayoutList);

#define BUFFER_SIZE 0xfffe
  char * pBuffer = new char[BUFFER_SIZE + 1];

  while (!done)
    {
      mpIstream->get(pBuffer, BUFFER_SIZE, 0);

      if (mpIstream->eof()) done = true;
      if (mpIstream->fail() && !done) fatalError();

      if (!Parser.parse(pBuffer, -1, done))
        {
          CCopasiMessage Message(CCopasiMessage::RAW, MCXML + 2,
                                 Parser.getCurrentLineNumber(),
                                 Parser.getCurrentColumnNumber(),
                                 Parser.getErrorString());
          done = true;
          success = false;
        }
    }
  delete [] pBuffer;
#undef BUFFER_SIZE

  if (success)
    {
      mpFunctionList = Parser.getFunctionList();
      mpModel = Parser.getModel();
      mpReportList = Parser.getReportList();
      mpTaskList = Parser.getTaskList();
      mpPlotList = Parser.getPlotList();
      mpLayoutList = Parser.getLayoutList();

      CFixLocalReactionParameters FixLocalReactionParameters;
      FixLocalReactionParameters.fixModel(mpModel);
    }

  if (FileVersion.getVersionDevel() > mVersion.getVersionDevel())
    CCopasiMessage(CCopasiMessage::WARNING, MCXML + 9,
                   mFilename.c_str(), FileVersion.getVersion().c_str());

  return success;
}

const CVersion & CCopasiXML::getVersion() const
{return mVersion;}

bool CCopasiXML::setModel(CModel * pModel)
{
  mpModel = pModel;
  return true;
}

CModel * CCopasiXML::getModel() const {return mpModel;}

bool CCopasiXML::haveModel() const {return mpModel != NULL;}

bool CCopasiXML::freeModel()
{
  pdelete(mpModel);
  return true;
}

bool CCopasiXML::setFunctionList(CCopasiVectorN< CEvaluationTree > *pFunctionList)
{
  mpFunctionList = pFunctionList;
  return true;
}

CCopasiVectorN< CEvaluationTree > * CCopasiXML::getFunctionList() const
  {return mpFunctionList;}

bool CCopasiXML::haveFunctionList() const
  {return mpFunctionList != NULL;}

bool CCopasiXML::freeFunctionList()
{
  pdelete(mpFunctionList);
  return true;
}

bool CCopasiXML::setTaskList(CCopasiVectorN< CCopasiTask > * pTaskList)
{
  mpTaskList = pTaskList;
  return true;
}

CCopasiVectorN< CCopasiTask > * CCopasiXML::getTaskList() const
  {return mpTaskList;}

bool CCopasiXML::haveTaskList() const
  {return mpTaskList != NULL;}

bool CCopasiXML::freeTaskList()
{
  pdelete(mpTaskList);
  return true;
}

//************

bool CCopasiXML::setPlotList(COutputDefinitionVector * pPlotList)
{
  mpPlotList = pPlotList;
  return true;
}

COutputDefinitionVector * CCopasiXML::getPlotList() const
  {return mpPlotList;}

bool CCopasiXML::havePlotList() const
  {return mpPlotList != NULL;}

bool CCopasiXML::freePlotList()
{
  pdelete(mpPlotList);
  return true;
}

//************

bool CCopasiXML::setReportList(CReportDefinitionVector * pReportList)
{
  mpReportList = pReportList;
  return true;
}

CReportDefinitionVector * CCopasiXML::getReportList() const
  {return mpReportList;}

bool CCopasiXML::haveReportList() const
  {return mpReportList != NULL;}

bool CCopasiXML::freeReportList()
{
  pdelete(mpReportList);
  return true;
}

//************

bool CCopasiXML::setGUI(SCopasiXMLGUI * pGUI)
{
  mpGUI = pGUI;
  return true;
}

SCopasiXMLGUI * CCopasiXML::getGUI() const {return mpGUI;}

bool CCopasiXML::haveGUI() const {return mpGUI != NULL;}

bool CCopasiXML::freeGUI()
{
  pdelete(mpGUI);
  return true;
}

//************

bool CCopasiXML::setLayoutList(const CListOfLayouts & layoutList)
{
  mpLayoutList = const_cast<CListOfLayouts *>(&layoutList);
  return true;
}

CListOfLayouts * CCopasiXML::getLayoutList() const
  {return mpLayoutList;}

bool CCopasiXML::haveLayoutList() const
  {return mpLayoutList != NULL;}

bool CCopasiXML::freeLayoutList()
{
  pdelete(mpLayoutList);
  return true;
}

bool CCopasiXML::saveModel()
{
  bool success = true;
  if (!haveModel()) return success;

  CXMLAttributeList Attributes;
  Attributes.add("key", mpModel->getKey());
  Attributes.add("name", mpModel->getObjectName());
  Attributes.add("timeUnit", mpModel->getTimeUnitName());
  Attributes.add("volumeUnit", mpModel->getVolumeUnitName());
  Attributes.add("quantityUnit", mpModel->getQuantityUnitName());
  Attributes.add("type", CModel::ModelTypeNames[mpModel->getModelType()]);

  // This is now optional
  // Attributes.add("stateVariable",
  //                mpModel->getStateTemplate().getKey(mpModel->getKey()));

  startSaveElement("Model", Attributes);

  if (mpModel->getMiriamAnnotation() != "")
    {
      startSaveElement("MiriamAnnotation");
      *mpOstream << mpModel->getMiriamAnnotation() << std::endl;
      endSaveElement("MiriamAnnotation");
    }

  if (mpModel->getInitialExpression() != "")
    {
      startSaveElement("InitialExpression");
      saveData(mpModel->getInitialExpression());
      endSaveElement("InitialExpression");
    }

  startSaveElement("Comment");
  saveXhtml(mpModel->getComments());
  endSaveElement("Comment");

  unsigned C_INT32 i, imax;

  // Compartment
  if ((imax = mpModel->getCompartments().size()) > 0)
    {
      startSaveElement("ListOfCompartments");

      Attributes.erase();
      Attributes.add("key", "");
      Attributes.add("name", "");
      Attributes.add("simulationType", "");

      unsigned C_INT32 i, imax = mpModel->getCompartments().size();
      for (i = 0; i < imax; i++)
        {
          CCompartment * pComp = mpModel->getCompartments()[i];

          Attributes.setValue(0, pComp->getKey());
          Attributes.setValue(1, pComp->getObjectName());
          CModelEntity::Status SimulationType = pComp->getStatus();
          Attributes.setValue(2, CModelEntity::XMLStatus[SimulationType]);

          startSaveElement("Compartment", Attributes);

          if (pComp->getMiriamAnnotation() != "")
            {
              startSaveElement("MiriamAnnotation");
              *mpOstream << pComp->getMiriamAnnotation() << std::endl;
              endSaveElement("MiriamAnnotation");
            }

          if (SimulationType != CModelEntity::FIXED &&
              pComp->getExpression() != "")
            {
              startSaveElement("Expression");
              saveData(pComp->getExpression());
              endSaveElement("Expression");
            }

          if (pComp->getInitialExpression() != "")
            {
              startSaveElement("InitialExpression");
              saveData(pComp->getInitialExpression());
              endSaveElement("InitialExpression");
            }

          endSaveElement("Compartment");
          if (pComp->getSBMLId() != "")
            mSBMLReference[pComp->getSBMLId()] = pComp->getKey();
        }

      endSaveElement("ListOfCompartments");
    }

  // Metabolites (aka. Species)
  if ((imax = mpModel->getMetabolites().size()) > 0)
    {
      startSaveElement("ListOfMetabolites");

      Attributes.erase();
      Attributes.add("key", "");
      Attributes.add("name", "");
      Attributes.add("simulationType", "");
      Attributes.add("compartment", "");

      for (i = 0; i < imax; i++)
        {
          CMetab * pMetab = mpModel->getMetabolites()[i];

          Attributes.setValue(0, pMetab->getKey());
          Attributes.setValue(1, pMetab->getObjectName());
          CModelEntity::Status SimulationType = pMetab->getStatus();
          Attributes.setValue(2, CModelEntity::XMLStatus[SimulationType]);
          Attributes.setValue(3, pMetab->getCompartment()->getKey());

          startSaveElement("Metabolite", Attributes);

          if (pMetab->getMiriamAnnotation() != "")
            {
              startSaveElement("MiriamAnnotation");
              *mpOstream << pMetab->getMiriamAnnotation() << std::endl;
              endSaveElement("MiriamAnnotation");
            }

          if (SimulationType != CModelEntity::FIXED &&
              SimulationType != CModelEntity::REACTIONS &&
              pMetab->getExpression() != "")
            {
              startSaveElement("Expression");
              saveData(pMetab->getExpression());
              endSaveElement("Expression");
            }

          if (pMetab->getInitialExpression() != "")
            {
              startSaveElement("InitialExpression");
              saveData(pMetab->getInitialExpression());
              endSaveElement("InitialExpression");
            }

          endSaveElement("Metabolite");
          if (pMetab->getSBMLId() != "")
            mSBMLReference[pMetab->getSBMLId()] = pMetab->getKey();
        }

      endSaveElement("ListOfMetabolites");
    }

  // Model Values (aka. Global Quantities)
  if ((imax = mpModel->getModelValues().size()) > 0)
    {
      startSaveElement("ListOfModelValues");

      Attributes.erase();
      Attributes.add("key", "");
      Attributes.add("name", "");
      Attributes.add("simulationType", "");

      for (i = 0; i < imax; i++)
        {
          CModelValue * pMV = mpModel->getModelValues()[i];

          Attributes.setValue(0, pMV->getKey());
          Attributes.setValue(1, pMV->getObjectName());
          CModelEntity::Status SimulationType = pMV->getStatus();
          Attributes.setValue(2, CModelEntity::XMLStatus[SimulationType]);

          startSaveElement("ModelValue", Attributes);

          if (pMV->getMiriamAnnotation() != "")
            {
              startSaveElement("MiriamAnnotation");
              *mpOstream << pMV->getMiriamAnnotation() << std::endl;
              endSaveElement("MiriamAnnotation");
            }

          if (SimulationType != CModelEntity::FIXED &&
              pMV->getExpression() != "")
            {
              startSaveElement("Expression");
              saveData(pMV->getExpression());
              endSaveElement("Expression");
            }

          if (pMV->getInitialExpression() != "")
            {
              startSaveElement("InitialExpression");
              saveData(pMV->getInitialExpression());
              endSaveElement("InitialExpression");
            }

          endSaveElement("ModelValue");
          if (pMV->getSBMLId() != "")
            mSBMLReference[pMV->getSBMLId()] = pMV->getKey();
        }

      endSaveElement("ListOfModelValues");
    }

  // Reactions
  if ((imax = mpModel->getReactions().size()) > 0)
    {
      startSaveElement("ListOfReactions");

      CXMLAttributeList Attr;
      const CCopasiVector< CChemEqElement > * pReactantList;
      unsigned C_INT32 j, jmax;

      std::vector< const CCopasiObject * > ObjectList;
      unsigned C_INT32 k, kmax;

      Attributes.erase();
      Attributes.add("key", "");
      Attributes.add("name", "");
      Attributes.add("compartment", ""); //TODO necessary?
      Attributes.add("reversible", "");

      for (i = 0; i < imax; i++)
        {
          CReaction * pReaction = mpModel->getReactions()[i];

          Attributes.setValue(0, pReaction->getKey());
          Attributes.setValue(1, pReaction->getObjectName());
          //if (pReaction->getCompartment())
          //  Attributes.setValue(2, pReaction->getCompartment()->getKey());
          //else
          Attributes.skip(2);
          Attributes.setValue(3, pReaction->isReversible() ? "true" : "false");
          if (pReaction->getSBMLId() != "")
            mSBMLReference[pReaction->getSBMLId()] = pReaction->getKey();

          startSaveElement("Reaction", Attributes);

          if (pReaction->getMiriamAnnotation() != "")
            {
              startSaveElement("MiriamAnnotation");
              *mpOstream << pReaction->getMiriamAnnotation() << std::endl;
              endSaveElement("MiriamAnnotation");
            }

          Attr.erase();
          Attr.add("metabolite", "");
          Attr.add("stoichiometry", "");

          pReactantList = & pReaction->getChemEq().getSubstrates();
          if ((jmax = pReactantList->size()) > 0)
            {
              startSaveElement("ListOfSubstrates");

              for (j = 0; j < jmax; j++)
                {
                  Attr.setValue(0, (*pReactantList)[j]->getMetaboliteKey());
                  Attr.setValue(1, (*pReactantList)[j]->getMultiplicity());

                  saveElement("Substrate", Attr);
                }

              endSaveElement("ListOfSubstrates");
            }
          //startSaveElement("ListOfProducts"); // this seems to belong further down

          pReactantList = & pReaction->getChemEq().getProducts();
          if ((jmax = pReactantList->size()) > 0)
            {
              startSaveElement("ListOfProducts"); //this seems to belong here

              for (j = 0; j < jmax; j++)
                {
                  Attr.setValue(0, (*pReactantList)[j]->getMetaboliteKey());
                  Attr.setValue(1, (*pReactantList)[j]->getMultiplicity());

                  saveElement("Product", Attr);
                }

              endSaveElement("ListOfProducts");
            }

          pReactantList = & pReaction->getChemEq().getModifiers();
          if ((jmax = pReactantList->size()) > 0)
            {
              startSaveElement("ListOfModifiers");

              for (j = 0, jmax = pReactantList->size(); j < jmax; j++)
                {
                  Attr.setValue(0, (*pReactantList)[j]->getMetaboliteKey());
                  Attr.setValue(1, (*pReactantList)[j]->getMultiplicity());

                  saveElement("Modifier", Attr);
                }

              endSaveElement("ListOfModifiers");
            }

          const CCopasiParameterGroup * pParamList;
          pParamList = & pReaction->getParameters();
          if ((jmax = pParamList->size()) > 0)
            {
              startSaveElement("ListOfConstants");

              Attr.erase();
              Attr.add("key", "");
              Attr.add("name", "");
              Attr.add("value", "");

              for (j = 0; j < jmax; j++)
                {
                  Attr.setValue(0, pParamList->getKey(j));
                  Attr.setValue(1, pParamList->getName(j));
                  Attr.setValue(2, * pParamList->getValue(j).pDOUBLE);

                  saveElement("Constant", Attr);
                }

              endSaveElement("ListOfConstants");
            }

          if (pReaction->getFunction() !=
              dynamic_cast<CFunction *>(GlobalKeys.get("UndefinedFunction_0")))
            {
              Attr.erase();
              Attr.add("function", pReaction->getFunction()->getKey());
              startSaveElement("KineticLaw", Attr);
              if ((jmax = pReaction->getFunctionParameters().size()))
                {
                  startSaveElement("ListOfCallParameters");
                  const std::vector< std::vector<std::string> > & rMap =
                    pReaction->getParameterMappings();

                  for (j = 0; j < jmax; j++)
                    {
                      Attr.erase();
                      Attr.add("functionParameter",
                               pReaction->
                               getFunction()->getVariables()[j]->getKey());

                      startSaveElement("CallParameter", Attr);

                      Attr.erase();
                      Attr.add("reference", "");

                      for (k = 0, kmax = rMap[j].size(); k < kmax; k++)
                        {
                          Attr.setValue(0, rMap[j][k]);
                          saveElement("SourceParameter", Attr);
                        }

                      endSaveElement("CallParameter");
                    }
                  endSaveElement("ListOfCallParameters");
                }
              endSaveElement("KineticLaw");
            }
          endSaveElement("Reaction");
        }
      endSaveElement("ListOfReactions");
    }

  // Events (added 07.04.08)
  if ((imax = mpModel->getEvents().size()) > 0)
    {
      startSaveElement("ListOfEvents");

      Attributes.erase();
      Attributes.add("key", "");
      Attributes.add("name", "");
      //      Attributes.add("delay", "");

      for (i = 0; i < imax; i++)
        {
          CEvent * pEvent = mpModel->getEvents()[i];

          Attributes.setValue(0, pEvent->getKey());
          Attributes.setValue(1, pEvent->getObjectName());

          startSaveElement("Event", Attributes);
          /*
                    if (pEvent->getMiriamAnnotation() != "")
                      {
                        startSaveElement("MiriamAnnotation");
                        *mpOstream << pEvent->getMiriamAnnotation() << std::endl;
                        endSaveElement("MiriamAnnotation");
                      }
          */
          if (pEvent->getExpressionTrigger() != "")
            {
              startSaveElement("TriggerExpression");
              saveData(pEvent->getExpressionTrigger());
              endSaveElement("TriggerExpression");
            }

          if (pEvent->getExpressionDelay() != "")
            {
              startSaveElement("DelayExpression");
              saveData(pEvent->getExpressionDelay());
              endSaveElement("DelayExpression");
            }

          CXMLAttributeList Attr;
          if (pEvent->getNumAssignments())
            //          if (pEvent->getExpressionEA() != "")
            {
              startSaveElement("ListOfAssignments");

              unsigned C_INT32 idxAct = 0;
              for (; idxAct < pEvent->getNumAssignments(); idxAct++)
                {
                  Attr.erase();
                  Attr.add("targetkey", "");
                  Attr.setValue(0, pEvent->getAssignmentObjectKey(idxAct));

                  startSaveElement("Assignment", Attr);
                  startSaveElement("Expression");
                  saveData(pEvent->getAssignmentExpressionStr(idxAct));
                  //saveData(pEvent->getExpressionEA());
                  endSaveElement("Expression");
                  endSaveElement("Assignment");
                }

              endSaveElement("ListOfAssignments");
            }

          endSaveElement("Event");
          if (pEvent->getSBMLId() != "")
            mSBMLReference[pEvent->getSBMLId()] = pEvent->getKey();
        }

      endSaveElement("ListOfEvents");
    }

  startSaveElement("StateTemplate");

  Attributes.erase();
  // This is now optional.
  // Attributes.add("key", "");
  Attributes.add("objectReference", "");
  std::pair< std::string, std::string > Variable;

  CModelEntity *const* Entity = mpModel->getStateTemplate().getEntities();
  for (i = 0, imax = mpModel->getStateTemplate().size(); i < imax; i++, ++Entity)
    {
      Attributes.setValue(0, (*Entity)->getKey());

      saveElement("StateTemplateVariable", Attributes);
    }

  endSaveElement("StateTemplate");

  Attributes.erase();
  Attributes.add("type", "initialState");
  startSaveElement("InitialState", Attributes);
  CState InitialState = mpModel->getInitialState();

  *mpOstream << mIndent;

  *mpOstream << (DBL) InitialState.getTime();
  C_FLOAT64 * it = InitialState.beginIndependent();
  C_FLOAT64 * end = InitialState.endFixed();

  for (;it != end; ++it)
    *mpOstream << " " << (DBL) *it;

  *mpOstream << std::endl;

  endSaveElement("InitialState");

  endSaveElement("Model");

  return success;
}

bool CCopasiXML::saveFunctionList()
{
  bool success = true;

  if (!haveFunctionList()) return success;
  unsigned C_INT32 i, imax = mpFunctionList->size();

  if (!imax) return success;

  CXMLAttributeList Attributes;
  CEvaluationTree * pEvaluationTree = NULL;
  CFunction * pFunction = NULL;

  startSaveElement("ListOfFunctions");

  for (i = 0; i < imax; i++)
    {
      pEvaluationTree = (*mpFunctionList)[i];
      pFunction = dynamic_cast<CFunction *>(pEvaluationTree);

      Attributes.erase();
      Attributes.add("key", pEvaluationTree->getKey());
      Attributes.add("name", pEvaluationTree->getObjectName());
      Attributes.add("type", CEvaluationTree::XMLType[pEvaluationTree->getType()]);
      if (pFunction)
        {
          switch (pFunction->isReversible())
            {
            case TriUnspecified:
              Attributes.add("reversible", "unspecified");
              break;

            case TriFalse:
              Attributes.add("reversible", "false");
              break;

            case TriTrue:
              Attributes.add("reversible", "true");
              break;
            }

          if (pFunction->getSBMLId() != "")
            mSBMLReference[pFunction->getSBMLId()] = pFunction->getKey();
        }

      startSaveElement("Function", Attributes);

      if (pEvaluationTree->getMiriamAnnotation() != "")
        {
          startSaveElement("MiriamAnnotation");
          *mpOstream << pEvaluationTree->getMiriamAnnotation() << std::endl;
          endSaveElement("MiriamAnnotation");
        }

      startSaveElement("Expression");
      saveData(pEvaluationTree->getInfix());
      endSaveElement("Expression");

      if (pFunction)
        {
          startSaveElement("ListOfParameterDescriptions");

          unsigned C_INT32 j, jmax = pFunction->getVariables().size();
          CFunctionParameter * pParameter;

          Attributes.erase();
          Attributes.add("key", "");
          Attributes.add("name", "");
          Attributes.add("order", "");
          Attributes.add("role", "");
          Attributes.add("minOccurs", "");
          Attributes.add("maxOccurs", "");

          for (j = 0; j < jmax; j++)
            {
              pParameter = pFunction->getVariables()[j];
              Attributes.setValue(0, pParameter->getKey());
              Attributes.setValue(1, pParameter->getObjectName());
              Attributes.setValue(2, j);
              Attributes.setValue(3, CFunctionParameter::RoleNameXML[pParameter->getUsage()]);

              if (true /*pParameter->getType() < CFunctionParameter::VINT32*/)
                {
                  Attributes.skip(4);
                  Attributes.skip(5);
                }
              else
                {
                  /*
                  CUsageRange * pUsageRange =
                    pFunction->getVariables().getUsageRanges()[pParameter->getUsage()];
                  Attributes.setValue(4, pUsageRange->getLow());
                  if (pUsageRange->getHigh() == (unsigned C_INT32) CRange::Infinity)
                    Attributes.setValue(5, "unbounded");
                  else
                    Attributes.setValue(5, pUsageRange->getHigh());
                  */
                }

              saveElement("ParameterDescription", Attributes);
            }

          endSaveElement("ListOfParameterDescriptions");
        }

      endSaveElement("Function");
    }

  endSaveElement("ListOfFunctions");

  return success;
}

bool CCopasiXML::savePlotList()
{
  //std::cerr << "Saving plot list. " << std::endl;
  bool success = true;
  if (!havePlotList())
    {
      //std::cerr << "No plot list defined." << std::endl;
      return success;
    }

  unsigned C_INT32 i, imax = mpPlotList->size();
  //std::cerr << "Saving " << imax << " plots." << std::endl;
  if (!imax) return success;

  CXMLAttributeList Attributes;

  startSaveElement("ListOfPlots");

  for (i = 0; i < imax; i++)
    {
      const CPlotSpecification* pPlot = (*mpPlotList)[i];

      Attributes.erase();
      Attributes.add("name", pPlot->getObjectName());
      Attributes.add("type", CPlotSpecification::XMLType[pPlot->getType()]);
      Attributes.add("active", pPlot->isActive());
      startSaveElement("PlotSpecification", Attributes);
      saveParameterGroup(* pPlot->CCopasiParameter::getValue().pGROUP);
      startSaveElement("ListOfPlotItems");
      unsigned C_INT32 j, jmax = pPlot->getItems().size();
      //std::cerr << "Saving " << jmax << "PlotItems." << std::endl;
      for (j = 0; j < jmax; j++)
        {
          const CPlotItem* pPlotItem = pPlot->getItems()[j];
          Attributes.erase();
          Attributes.add("name", pPlotItem->getObjectName());
          Attributes.add("type", CPlotItem::XMLType[pPlotItem->getType()]);
          startSaveElement("PlotItem", Attributes);
          saveParameterGroup(* pPlotItem->CCopasiParameter::getValue().pGROUP);
          startSaveElement("ListOfChannels");
          unsigned C_INT32 k, kmax = pPlotItem->getNumChannels();
          //std::cerr << "Saving " << kmax << " Channels." << std::endl;
          for (k = 0; k < kmax; k++)
            {
              const CPlotDataChannelSpec pDataChannelSpec = pPlotItem->getChannels()[k];
              Attributes.erase();
              Attributes.add("cn", pDataChannelSpec);
              if (!pDataChannelSpec.minAutoscale)
                {
                  Attributes.add("min", pDataChannelSpec.min);
                }
              if (!pDataChannelSpec.maxAutoscale)
                {
                  Attributes.add("max", pDataChannelSpec.max);
                }
              saveElement("ChannelSpec", Attributes);
            }
          endSaveElement("ListOfChannels");
          endSaveElement("PlotItem");
        }
      endSaveElement("ListOfPlotItems");
      endSaveElement("PlotSpecification");
    }
  endSaveElement("ListOfPlots");
  return success;
}

//Mrinmayee
bool CCopasiXML::saveTaskList()
{
  bool success = true;
  if (!haveTaskList()) return success;

  unsigned C_INT32 i, imax = mpTaskList->size();

  if (!imax) return success;

  CXMLAttributeList Attributes;
  CCopasiTask * pTask = NULL;

  startSaveElement("ListOfTasks");

  for (i = 0; i < imax; i++)
    {
      pTask = (*mpTaskList)[i];

      Attributes.erase();
      Attributes.add("key", pTask->getKey());
      Attributes.add("name", pTask->getObjectName());
      Attributes.add("type", CCopasiTask::XMLType[pTask->getType()]);
      Attributes.add("scheduled", pTask->isScheduled() ? "true" : "false");
      Attributes.add("updateModel", pTask->isUpdateModel() ? "true" : "false");

      startSaveElement("Task", Attributes);

      // Report Element
      CReport & tReport = pTask->getReport();
      if (tReport.getReportDefinition())
        {
          Attributes.erase();
          Attributes.add("reference", tReport.getReportDefinition()->getKey());

          std::string Target = tReport.getTarget();
          if (!CDirEntry::isRelativePath(Target) &&
              !CDirEntry::makePathRelative(Target, mFilename))
            Target = CDirEntry::fileName(Target);

          Attributes.add("target", Target);
          Attributes.add("append", tReport.append());
          saveElement("Report", Attributes);
        }

      //Problem Element
      CCopasiProblem *tProblem = pTask->getProblem();

      Attributes.erase();
      startSaveElement("Problem");
      saveParameterGroup(* tProblem->CCopasiParameter::getValue().pGROUP);
      endSaveElement("Problem");

      // Method Element
      CCopasiMethod *tMethod = pTask->getMethod();

      Attributes.erase();
      Attributes.add("name", tMethod->CCopasiParameter::getObjectName());
      Attributes.add("type", CCopasiMethod::XMLSubType[tMethod->getSubType()]);
      startSaveElement("Method", Attributes);
      saveParameterGroup(* tMethod->CCopasiParameter::getValue().pGROUP);
      endSaveElement("Method");

      endSaveElement("Task");
    }

  endSaveElement("ListOfTasks");

  return success;
}

//Mrinmayee
bool CCopasiXML::saveReportSection(const std::string & name,
                                   const std::vector <CRegisteredObjectName> & section)
{
  CXMLAttributeList Attributes;
  Attributes.add("NoName", "");

  unsigned C_INT32 j, jmax = section.size();

  if (jmax)
    {
      startSaveElement(name);
      for (j = 0; j < jmax; j++)
        {
          if (section[j].getObjectType() == "html")
            {
              //Write in Text
              startSaveElement("html");
              Attributes.set(0, "xmlns", "http://www.w3.org/1999/xhtml");
              startSaveElement("body", Attributes);
              saveData(section[j].getObjectName()); //TODO check
              endSaveElement("body");
              endSaveElement("html");
            }
          else
            {
              //Write in Object
              Attributes.set(0, "cn", section[j]);
              saveElement("Object", Attributes);
            }
        }
      endSaveElement(name);
    }

  return true;
}

bool CCopasiXML::saveReportList()
{
  bool success = true;
  if (!haveReportList()) return success;

  unsigned C_INT32 i, imax = mpReportList->size();
  if (!imax) return success;

  CXMLAttributeList Attributes;
  CReportDefinition * pReport = NULL;

  startSaveElement("ListOfReports");

  for (i = 0; i < imax; i++)
    {
      pReport = (*mpReportList)[i];

      Attributes.erase();
      Attributes.add("key", pReport->getKey());
      Attributes.add("name", pReport->getObjectName());
      Attributes.add("taskType", CCopasiTask::XMLType[pReport->getTaskType()]);
      Attributes.add("separator", pReport->getSeparator().getStaticString());
      Attributes.add("precision", pReport->getPrecision());

      startSaveElement("Report", Attributes);

      startSaveElement("Comment");
      saveXhtml(pReport->getComment());
      endSaveElement("Comment");

      if (pReport->isTable())
        {
          Attributes.erase();
          Attributes.add("printTitle", pReport->getTitle());
          startSaveElement("Table", Attributes);

          const std::vector <CRegisteredObjectName> & Table = * pReport->getTableAddr();
          unsigned C_INT32 j, jmax = Table.size();

          Attributes.erase();
          Attributes.add("cn", "");

          for (j = 0; j < jmax; j++)
            {
              Attributes.setValue(0, Table[j]);
              saveElement("Object", Attributes);
            }

          endSaveElement("Table");
        }
      else
        {
          saveReportSection("Header", * pReport->getHeaderAddr());
          saveReportSection("Body", * pReport->getBodyAddr());
          saveReportSection("Footer", * pReport->getFooterAddr());
        }

      endSaveElement("Report");
    }

  endSaveElement("ListOfReports");

  return success;
}

void CCopasiXML::savePosition(const CLPoint& p, const std::string & tag)
{
  CXMLAttributeList Attributes;
  Attributes.erase();
  Attributes.add("x", p.getX());
  Attributes.add("y", p.getY());
  saveElement(tag, Attributes);
}

void CCopasiXML::saveDimensions(const CLDimensions& d)
{
  CXMLAttributeList Attributes;
  Attributes.erase();
  Attributes.add("width", d.getWidth());
  Attributes.add("height", d.getHeight());
  saveElement("Dimensions", Attributes);
}

void CCopasiXML::saveBoundingBox(const CLBoundingBox& bb)
{
  startSaveElement("BoundingBox");
  savePosition(bb.getPosition());
  saveDimensions(bb.getDimensions());
  endSaveElement("BoundingBox");
}

void CCopasiXML::saveCurve(const CLCurve& c)
{
  CXMLAttributeList Attributes;
  startSaveElement("Curve");
  if (c.getNumCurveSegments() > 0)
    {
      startSaveElement("ListOfCurveSegments");
      unsigned C_INT32 i, imax = c.getNumCurveSegments();
      for (i = 0; i < imax; ++i)
        {
          const CLLineSegment & cs = c.getCurveSegments()[i];

          Attributes.erase();
          if (cs.isBezier())
            Attributes.add("xsi:type", "CubicBezier");
          else
            Attributes.add("xsi:type", "LineSegment");
          startSaveElement("CurveSegment", Attributes);

          savePosition(cs.getStart(), "Start");
          savePosition(cs.getEnd(), "End");

          if (cs.isBezier())
            {
              savePosition(cs.getBase1(), "BasePoint1");
              savePosition(cs.getBase2(), "BasePoint2");
            }
          endSaveElement("CurveSegment");
        }
      endSaveElement("ListOfCurveSegments");
    }
  endSaveElement("Curve");
}

bool CCopasiXML::saveLayoutList()
{
  bool success = true;
  if (!haveLayoutList()) return success;

  unsigned C_INT32 i, imax = mpLayoutList->size();
  if (!imax) return success;

  CXMLAttributeList Attributes;
  CLayout * pLayout = NULL;

  startSaveElement("ListOfLayouts");

  for (i = 0; i < imax; i++)
    {
      pLayout = (*mpLayoutList)[i];

      Attributes.erase();
      Attributes.add("key", pLayout->getKey());
      Attributes.add("name", pLayout->getObjectName());
      startSaveElement("Layout", Attributes);

      Attributes.erase();
      Attributes.add("width", pLayout->getDimensions().getWidth());
      Attributes.add("height", pLayout->getDimensions().getHeight());
      saveElement("Dimensions", Attributes);

      unsigned C_INT32 j, jmax;

      //compartment glyphs
      if (pLayout->getListOfCompartmentGlyphs().size() > 0)
        {
          startSaveElement("ListOfCompartmentGlyphs");

          jmax = pLayout->getListOfCompartmentGlyphs().size();
          for (j = 0; j < jmax; ++j)
            {
              CLCompartmentGlyph* cg = pLayout->getListOfCompartmentGlyphs()[j];
              Attributes.erase();
              Attributes.add("key", cg->getKey());
              Attributes.add("name", cg->getObjectName());
              Attributes.add("compartment", cg->getModelObjectKey());
              startSaveElement("CompartmentGlyph", Attributes);

              saveBoundingBox(cg->getBoundingBox());

              endSaveElement("CompartmentGlyph");
            }
          endSaveElement("ListOfCompartmentGlyphs");
        }

      //species glyphs
      if (pLayout->getListOfMetaboliteGlyphs().size() > 0)
        {
          startSaveElement("ListOfMetabGlyphs");

          jmax = pLayout->getListOfMetaboliteGlyphs().size();
          for (j = 0; j < jmax; ++j)
            {
              CLMetabGlyph* cg = pLayout->getListOfMetaboliteGlyphs()[j];
              Attributes.erase();
              Attributes.add("key", cg->getKey());
              Attributes.add("name", cg->getObjectName());
              Attributes.add("metabolite", cg->getModelObjectKey());
              startSaveElement("MetaboliteGlyph", Attributes);

              saveBoundingBox(cg->getBoundingBox());

              endSaveElement("MetaboliteGlyph");
            }
          endSaveElement("ListOfMetabGlyphs");
        }

      //reaction glyphs
      if (pLayout->getListOfReactionGlyphs().size() > 0)
        {
          startSaveElement("ListOfReactionGlyphs");

          jmax = pLayout->getListOfReactionGlyphs().size();
          for (j = 0; j < jmax; ++j)
            {
              CLReactionGlyph* cg = pLayout->getListOfReactionGlyphs()[j];
              Attributes.erase();
              Attributes.add("key", cg->getKey());
              Attributes.add("name", cg->getObjectName());
              Attributes.add("reaction", cg->getModelObjectKey());
              startSaveElement("ReactionGlyph", Attributes);

              if (cg->getCurve().getNumCurveSegments() == 0)
                saveBoundingBox(cg->getBoundingBox());
              else
                saveCurve(cg->getCurve());

              // metab reference glyphs
              startSaveElement("ListOfMetaboliteReferenceGlyphs");
              unsigned C_INT32 k, kmax = cg->getListOfMetabReferenceGlyphs().size();
              for (k = 0; k < kmax; ++k)
                {
                  CLMetabReferenceGlyph * mrg = cg->getListOfMetabReferenceGlyphs()[k];
                  Attributes.erase();
                  Attributes.add("key", mrg->getKey());
                  Attributes.add("name", mrg->getObjectName());
                  Attributes.add("metaboliteGlyph", mrg->getMetabGlyphKey());
                  //Attributes.add("metaboliteReference", mrg->getXXX());
                  Attributes.add("role", CLMetabReferenceGlyph::XMLRole[mrg->getRole()]);
                  startSaveElement("MetaboliteReferenceGlyph", Attributes);

                  if (mrg->getCurve().getNumCurveSegments() == 0)
                    saveBoundingBox(mrg->getBoundingBox());
                  else
                    saveCurve(mrg->getCurve());

                  endSaveElement("MetaboliteReferenceGlyph");
                }
              endSaveElement("ListOfMetaboliteReferenceGlyphs");

              endSaveElement("ReactionGlyph");
            }
          endSaveElement("ListOfReactionGlyphs");
        }

      //text Glyphs
      if (pLayout->getListOfTextGlyphs().size() > 0)
        {
          startSaveElement("ListOfTextGlyphs");

          jmax = pLayout->getListOfTextGlyphs().size();
          for (j = 0; j < jmax; ++j)
            {
              CLTextGlyph* cg = pLayout->getListOfTextGlyphs()[j];
              Attributes.erase();
              Attributes.add("key", cg->getKey());
              Attributes.add("name", cg->getObjectName());
              Attributes.add("graphicalObject", cg->getGraphicalObjectKey());
              if (cg->isTextSet())
                Attributes.add("text", cg->getText());
              else
                Attributes.add("originOfText", cg->getModelObjectKey());

              startSaveElement("TextGlyph", Attributes);

              saveBoundingBox(cg->getBoundingBox());

              endSaveElement("TextGlyph");
            }
          endSaveElement("ListOfTextGlyphs");
        }

      //additional graphical objects
      if (pLayout->getListOfGraphicalObjects().size() > 0)
        {
          startSaveElement("ListOfAdditionalGraphicalObjects");

          jmax = pLayout->getListOfGraphicalObjects().size();
          for (j = 0; j < jmax; ++j)
            {
              CLGraphicalObject* cg = pLayout->getListOfGraphicalObjects()[j];
              Attributes.erase();
              Attributes.add("key", cg->getKey());
              Attributes.add("name", cg->getObjectName());
              startSaveElement("AdditionalGraphicalObject", Attributes);

              saveBoundingBox(cg->getBoundingBox());

              endSaveElement("AdditionalGraphicalObject");
            }
          endSaveElement("ListOfAdditionalGraphicalObjects");
        }

      endSaveElement("Layout");
    }

  endSaveElement("ListOfLayouts");

  return success;
}

bool CCopasiXML::saveGUI()
{
  bool success = true;
  if (!haveGUI()) return success;

  startSaveElement("GUI");

  if (mpGUI->pSliderList && mpGUI->pSliderList->size())
    {
      startSaveElement("ListOfSliders");

      CSlider * pSlider;
      CXMLAttributeList Attributes;

      Attributes.add("key", "");
      Attributes.add("associatedEntityKey", "");
      Attributes.add("objectCN", "");
      Attributes.add("objectType", "");
      Attributes.add("objectValue", "");
      Attributes.add("minValue", "");
      Attributes.add("maxValue", "");
      Attributes.add("tickNumber", "");
      Attributes.add("tickFactor", "");
      Attributes.add("scaling", "");

      unsigned C_INT32 i, imax = mpGUI->pSliderList->size();
      for (i = 0; i < imax; i++)
        {
          pSlider = (*mpGUI->pSliderList)[i];
          Attributes.setValue(0, pSlider->getKey());
          Attributes.setValue(1, pSlider->getAssociatedEntityKey());
          Attributes.setValue(2, pSlider->getSliderObjectCN());
          Attributes.setValue(3, CSlider::TypeName[pSlider->getSliderType()]);
          Attributes.setValue(4, pSlider->getSliderValue());
          Attributes.setValue(5, pSlider->getMinValue());
          Attributes.setValue(6, pSlider->getMaxValue());
          Attributes.setValue(7, pSlider->getTickNumber());
          Attributes.setValue(8, pSlider->getTickFactor());
          Attributes.setValue(9, pSlider->convertScaleToScaleName(pSlider->getScaling()));
          saveElement("Slider", Attributes);
        }

      endSaveElement("ListOfSliders");
    }

  endSaveElement("GUI");

  return success;
}

bool CCopasiXML::saveSBMLReference()
{
  if (!CCopasiDataModel::Global) return false;

  if (CCopasiDataModel::Global->getSBMLFileName() == "" ||
      mSBMLReference.size() == 0)
    return true;

  CXMLAttributeList Attributes;

  std::string SBMLFile = CCopasiDataModel::Global->getSBMLFileName();
  if (!CDirEntry::isRelativePath(SBMLFile) &&
      !CDirEntry::makePathRelative(SBMLFile, mFilename))
    SBMLFile = CDirEntry::fileName(SBMLFile);

  Attributes.add("file", SBMLFile);

  startSaveElement("SBMLReference", Attributes);
  Attributes.erase();
  Attributes.add("SBMLid", "");
  Attributes.add("COPASIkey", "");

  std::map<std::string, std::string>::const_iterator it = mSBMLReference.begin();
  std::map<std::string, std::string>::const_iterator end = mSBMLReference.end();

  for (; it != end; ++it)
    {
      Attributes.setValue(0, it->first);
      Attributes.setValue(1, it->second);

      saveElement("SBMLMap", Attributes);
    }

  endSaveElement("SBMLReference");

  return true;
}

bool CCopasiXML::buildFunctionList()
{
  bool success = true;

  CCopasiVectorN< CEvaluationTree > * pFunctionList
  = new CCopasiVectorN< CEvaluationTree >;

  *pFunctionList = CCopasiDataModel::Global->getFunctionList()->getUsedFunctions();

  if (!setFunctionList(pFunctionList)) success = false;

  return success;
}

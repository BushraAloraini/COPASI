// Begin CVS Header 
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/bindings/swig/copasi.i,v $ 
//   $Revision: 1.32.2.1 $ 
//   $Name:  $ 
//   $Author: gauges $ 
//   $Date: 2011/04/27 20:30:36 $ 
// End CVS Header 

// Copyright (C) 2011 - 2010 by Pedro Mendes, Virginia Tech Intellectual 
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

%module COPASI



%{

#define COPASI_MAIN
#include "copasi.h"
#undef COPASI_MAIN

#include "local.cpp"

%}

%ignore DebugFile;

%include "copasi.h"

%include std_string.i
%include std_vector.i


%include "CVector.i"
%include "CRandom.i"
%include "CCopasiMessage.i"
%include "messages.i"
%include "CKeyFactory.i"
%include "CCopasiException.i"
%include "CCopasiObjectName.i"
%include "CCopasiObject.i"
%include "CCopasiObjectReference.i"
%include "CCopasiContainer.i"
%include "CCopasiArray.i"
%include "CAnnotatedMatrix.i"
%include "CCopasiVector.i"
%include "CEigen.i"
%include "CFunctionParameter.i"
%include "CFunctionParameters.i"
%include "CCallParameters.i"
%include "CAnnotation.i"
%include "CEvaluationTree.i"
%include "CExpression.i"
%include "CFunction.i"
%include "CFunctionDB.i"
%include "CCopasiParameter.i"
%include "CCopasiParameterGroup.i"
%include "CCopasiStaticString.i"
%include "CMoiety.i"
%include "CModelValue.i"
%include "CMetab.i"
%include "CCompartment.i"
%include "CMatrix.i"
%include "CState.i"
%include "CChemEqElement.i"
%include "CChemEq.i"
%include "CReaction.i"
%include "CModel.i"
%include "CVersion.i"
%include "CCopasiMethod.i"
%include "CCopasiProblem.i"
%include "COutputHandler.i"
%include "CReport.i"
%include "CCopasiTask.i"
%include "CReportDefinition.i"
%include "CReportDefinitionVector.i"
%include "CCopasiDataModel.i"
%include "CTimeSeries.i"
%include "CTrajectoryProblem.i"
%include "CTrajectoryMethod.i"
%include "CTrajectoryTask.i"
%include "COutputAssistant.i"
%include "CSteadyStateProblem.i"
%include "CSteadyStateMethod.i"
%include "CSteadyStateTask.i"
%include "CNewtonMethod.i"
%include "CScanMethod.i"
%include "CScanProblem.i"
%include "CScanTask.i"
%include "CLyapMethod.i"
%include "CLyapProblem.i"
%include "CLyapTask.i"
%include "COptItem.i"
%include "COptMethod.i"
%include "COptProblem.i"
%include "COptTask.i"
%include "CExperimentFileInfo.i"
%include "CExperiment.i"
%include "CExperimentSet.i"
%include "CExperimentObjectMap.i"
%include "CFitItem.i"
%include "CFitMethod.i"
%include "CFitProblem.i"
%include "CFitTask.i"
%include "compare_utilities.i"
%include "CCopasiRootContainer.i"
%include "CEvent.i"
%include "CLBase.i"
%include "CLGraphicalObject.i"
%include "CLGlyphs.i"
%include "CLCurve.i"
%include "CLReactionGlyph.i"
%include "CLayout.i"
%include "CListOfLayouts.i"
%include "CBiologicalDescription.i"
%include "CReference.i"
%include "CCreator.i"
%include "CModified.i"
%include "CModelMIRIAMInfo.i"


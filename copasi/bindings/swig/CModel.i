// Begin CVS Header 
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/bindings/swig/CModel.i,v $ 
//   $Revision: 1.16 $ 
//   $Name:  $ 
//   $Author: gauges $ 
//   $Date: 2009/09/01 14:08:16 $ 
// End CVS Header 

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual 
// Properties, Inc., EML Research, gGmbH, University of Heidelberg, 
// and The University of Manchester. 
// All rights reserved. 

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual 
// Properties, Inc. and EML Research, gGmbH. 
// All rights reserved. 

%{

#include "model/CModel.h"

%}

%template(ObjectStdVector) std::vector<CCopasiObject*>;
typedef std::vector<CCopasiObject*> ObjectStdVector;

%ignore CModel::compileIfNecessary(CProcessReport* pProcessReport);
%ignore CModel::forceCompile(CProcessReport* pProcessReport);
%ignore _cpp_min;
%ignore CModel::VolumeUnitNames;
%ignore CModel::AreaUnitNames;
%ignore CModel::LengthUnitNames;
%ignore CModel::TimeUnitNames;
%ignore CModel::QuantityUnitOldXMLNames;
%ignore CModel::QuantityUnitNames;
%ignore CModel::ModelTypeNames;

#ifdef SWIGJAVA

// ignore some methods for the Java wrappers to get rid of the warnings
%ignore CModel::getMetabolites() const;
%ignore CModel::getMetabolitesX() const;
%ignore CModel::getModelValues() const;
%ignore CModel::getReactions() const;
%ignore CModel::getEvents() const;
%ignore CModel::getCompartments() const;
%ignore CModel::getStateTemplate() const;

#endif // SWIGJAVA

%include "model/CModel.h"

%extend CModel
{
    /**
     *  Get the number of compartments 
     *  @return C_INT32 getCompartments().size()
     */
    unsigned C_INT32 getNumCompartments() const
    {
      return self->getCompartments().size();
    }

    /**
     *  Get the number of reactions
     *  @return C_INT32 getReactions().size()
     */
    unsigned C_INT32 getNumReactions() const
    {
      return self->getReactions().size();
    }

    CReaction* getReaction(unsigned C_INT32 index)
    {
        return self->getReactions()[index];
    }

    CCompartment* getCompartment(unsigned C_INT32 index)
    {
        return self->getCompartments()[index];
    }

    CMetab* getMetabolite(unsigned C_INT32 index)
    {
        return self->getMetabolites()[index];
    }

    CModelValue* getModelValue(unsigned C_INT32 index)
    {
        return self->getModelValues()[index];
    }

    CMoiety* getMoiety(unsigned C_INT32 index)
    {
        return self->getMoieties()[index];
    }

    bool forceCompile()
    {
        return $self->forceCompile(NULL);
    };

    bool compileIfNecessary()
    {
        return $self->compileIfNecessary(NULL);
    };

    void applyInitialValues()
    {
        $self->compileIfNecessary(NULL);
        $self->applyInitialValues();
        $self->updateNonSimulatedValues();
    }

    void updateInitialValues(const std::vector<CCopasiObject*>& v)
    {
        std::set<const CCopasiObject*> changedObjects;
        changedObjects.insert(v.begin(),v.end());
        std::vector<Refresh*> refreshes=$self->buildInitialRefreshSequence(changedObjects);
        std::vector<Refresh*>::iterator refreshIt = refreshes.begin(), refreshEndit = refreshes.end();
        while (refreshIt != refreshEndit)
            (**refreshIt++)();
    };

    CModelValue* getModelValue(const std::string& name)
    {
        return $self->getModelValues()[name];
    }

    // for backwards compatibility
   unsigned C_INT32 getNumIndependentMetabs() const 
   {
        return $self->getNumIndependentReactionMetabs();
   }

    // for backwards compatibility
   unsigned C_INT32 getNumDependentMetabs() const 
   {
        return $self->getNumDependentReactionMetabs();
   }
}




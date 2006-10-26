%{

#include "report/CCopasiObject.h"
%}

%nodefaultctor CCopasiObject;
%nodefaultdtor CCopasiObject;

class CCopasiObject
{
  public:
    bool setObjectName(const std::string& name);
    const std::string& getObjectName() const;
    virtual std::string getObjectDisplayName(bool regular = true, bool richtext = false) const;
    const std::string& getObjectType() const;
    CCopasiContainer* getObjectParent() const;
    virtual CCopasiObjectName getCN() const;

    bool isContainer() const; 
    bool isVector() const; 
    bool isMatrix() const; 
    bool isNameVector() const; 
    bool isReference() const; 
    bool isValueBool() const; 
    bool isValueInt() const; 
    bool isValueDbl() const; 
    bool isNonUniqueName() const; 
    bool isStaticString() const; 
    bool isValueString() const; 
    bool isSeparator() const; 

    virtual const std::string& getKey() const;
};




/**
 * CFunction
 * 
 * Created for Copasi by Stefan Hoops
 * (C) Stefan Hoops 2001
 */

#define COPASI_TRACE_CONSTRUCTION
#include "copasi.h"
#include "CFunction.h"

CFunction::CFunction()
{
  CONSTRUCTOR_TRACE;
  mReversible = TriUnspecified;
}

CFunction::CFunction(const CFunction & src)
{
  CONSTRUCTOR_TRACE;
  mType = src.mType;
  mName = src.mName;
  mDescription = src.mDescription;
  mReversible = src.mReversible;
  mUsageDescriptions = CCopasiVectorNS < CUsageRange > (src.mUsageDescriptions);
  mParameters = CFunctionParameters(src.mParameters);
}
CFunction::~CFunction() {cleanup(); DESTRUCTOR_TRACE; }

void CFunction::cleanup()
{
  mUsageDescriptions.cleanup();
  mParameters.cleanup();
}

void CFunction::load(CReadConfig & configBuffer,
                     CReadConfig::Mode mode)
{
  cleanup();

  if (configBuffer.getVersion() < "4")
    {
      C_INT32 Type;
      CUsageRange UsageDescription;

      mode = CReadConfig::SEARCH;
      configBuffer.getVariable("User-defined", "C_INT32", &Type, mode);

      switch (Type)
        {
        case 1:
          mType = CFunction::UserDefined;
          break;

        default:
          fatalError();
        }

      configBuffer.getVariable("Reversible", "C_INT32", &mReversible);

      configBuffer.getVariable("Substrates", "C_INT32", &Type);
      UsageDescription.setUsage("Substrates");
      UsageDescription.setLow(Type);
      mUsageDescriptions.add(UsageDescription);

      configBuffer.getVariable("Products", "C_INT32", &Type);
      UsageDescription.setUsage("Products");
      UsageDescription.setLow(Type);
      mUsageDescriptions.add(UsageDescription);

      mode = CReadConfig::SEARCH;
    }
  else
    {
      configBuffer.getVariable("FunctionType", "C_INT32", &mType, mode);
      mode = CReadConfig::NEXT;
    }

  configBuffer.getVariable("FunctionName", "string", &mName, mode);
  configBuffer.getVariable("Description", "string", &mDescription);

  if (configBuffer.getVersion() >= "4")
    {
      unsigned C_INT32 Size;
      configBuffer.getVariable("Reversible", "C_INT32", &mReversible);
      configBuffer.getVariable("UsageDescriptionSize", "C_INT32", &Size);
      mUsageDescriptions.load(configBuffer, Size);
      mParameters.load(configBuffer);
    }

  // For older file version the parameters have to be build from information
  // dependend on the function type. Luckilly, only user defined functions are
  // the only ones occuring in those files.
}

void CFunction::save(CWriteConfig & configBuffer)
{
  configBuffer.setVariable("FunctionType", "C_INT32", &mType);
  configBuffer.setVariable("FunctionName", "string", &mName);
  configBuffer.setVariable("Description", "string", &mDescription);
  configBuffer.setVariable("Reversible", "C_INT32", &mReversible);

  unsigned C_INT32 Size = mUsageDescriptions.size();
  configBuffer.setVariable("UsageDescriptionSize", "C_INT32", &Size);
  mUsageDescriptions.save(configBuffer);
  mParameters.save(configBuffer);
}
void CFunction::setName(const string& name){mName = name; }
const string & CFunction::getName() const { return mName; }

void CFunction::setDescription(const string & description)
{mDescription = description; }
const string & CFunction::getDescription() const{ return mDescription; }
void CFunction::setType(const CFunction::Type & type){mType = type; }
const CFunction::Type & CFunction::getType() const { return mType; }

void CFunction::setReversible(const TriLogic & reversible)
{mReversible = reversible; }
const TriLogic & CFunction::isReversible() const { return mReversible; }
CFunctionParameters & CFunction::getParameters() { return mParameters; }

unsigned C_INT32 CFunction::getParameterPosition(const string & name)
{ return mParameters[0] - mParameters[name]; }

C_FLOAT64 CFunction::calcValue(const CCallParameters & callParameters) const
  { return 0.0; }

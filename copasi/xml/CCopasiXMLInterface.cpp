/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/xml/CCopasiXMLInterface.cpp,v $
   $Revision: 1.39 $
   $Name:  $
   $Author: shoops $
   $Date: 2006/04/27 01:33:05 $
   End CVS Header */

// Copyright � 2005 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/**
 * CCopasiXMLInterface class.
 * The class CCopasiXMLInterface is the interface to various XML document
 * containing Copasi relevant informtion.
 *
 * Created for Copasi by Stefan Hoops 2003
 */

#include <fstream>
#include <limits>

#ifdef SunOS
# include <ieeefp.h>
#else
# include <float.h>
#endif

#include "copasi.h"
#include "CCopasiXMLInterface.h"
#include "model/CModel.h"
#include "report/CReportDefinition.h"
#include "report/CReportDefinitionVector.h"
#include "plot/COutputDefinitionVector.h"
#include "plot/CPlotSpecification.h"
#include "CCopasiXMLParser.h"
#include "utilities/CCopasiVector.h"
#include "utilities/CSlider.h"

SCopasiXMLGUI::SCopasiXMLGUI():
    pSliderList(new CCopasiVector<CSlider>)
{}

SCopasiXMLGUI::~SCopasiXMLGUI()
{
  pdelete(pSliderList);
}

void encodeNONE(const char & chr, std::ostringstream & xml)
{
  xml << chr;
  return;
}

void encodeSTD(const char & chr, std::ostringstream & xml)
{
  switch (chr)
    {
    case '&':
      xml << "&amp;";
      break;

    case '\'':
      xml << "&apos;";
      break;

    case '<':
      xml << "&lt;";
      break;

    case '>':
      xml << "&gt;";
      break;

    case '\"':
      xml << "&quot;";
      break;

    default:
      xml << chr;
      break;
    }
  return;
}

void encodeATTRIBUTE(const char & chr, std::ostringstream & xml)
{
  switch (chr)
    {
    case '&':
      xml << "&amp;";
      break;

    case '<':
      xml << "&lt;";
      break;

    case '\"':
      xml << "&quot;";
      break;

    case '\t':            // Without this <tab> is converted to <space>
      xml << "&#x09;";
      break;

    case '\n':            // Without this linebreak is converted to <space>
      xml << "&#x0a;";
      break;

    default:
      xml << chr;
      break;
    }
  return;
}

void encodeCHARACTER(const char & chr, std::ostringstream & xml)
{
  switch (chr)
    {
    case '&':
      xml << "&amp;";
      break;

    case '<':
      xml << "&lt;";
      break;

    default:
      xml << chr;
      break;
    }
  return;
}

std::string CCopasiXMLInterface::encode(const std::string & str, const EncodingType & type)
{
  /* All COPASI std::strings and char are already UTF-8 encoded.*/
  std::string tmp = str;
  std::ostringstream xml;

  void (*encode)(const char & chr, std::ostringstream & xml);
  std::string::const_iterator it = str.begin();
  std::string::const_iterator end = str.end();

  switch (type)
    {
    case none:
      encode = encodeNONE;
      break;

    case std:
      encode = encodeSTD;
      break;

    case attribute:
      encode = encodeATTRIBUTE;
      break;

    case character:
      encode = encodeCHARACTER;
      break;
    }

  for (; it != end; ++it)
    encode(*it, xml);

  return xml.str();
}

/*
std::string CCopasiXMLInterface::encodeDBL(const C_FLOAT64 & dbl)
{
  std::ostringstream value;


  if (isnan(dbl))
    value << "NaN";
  else if (finite(dbl))
    value << dbl;
  else if (dbl > 0.0)
    value << "INF";
  else if (dbl < 0.0)
    value << "-INF";


  return value.str();
}
 */

CCopasiXMLInterface::DBL::DBL(const C_FLOAT64 & value):
    mValue(value)
{}

#ifdef WIN32
// warning C4056: overflow in floating-point constant arithmetic
// warning C4756: overflow in constant arithmetic
# pragma warning (disable: 4056 4756)
#endif

CCopasiXMLInterface::DBL::DBL(const char * value):
    mValue(std::numeric_limits<C_FLOAT64>::quiet_NaN())
{
  if (!value || !*value) return;

  char * Tail;
  mValue = strtod(value, & Tail);

  if (!*Tail) return;

  if (!strcmp(value, "INF"))
    mValue = DBL_MAX * 2;
  else if (!strcmp(value, "-INF"))
    mValue = - DBL_MAX * 2;
  else mValue = std::numeric_limits<C_FLOAT64>::quiet_NaN();
}

#ifdef WIN32
# pragma warning (default: 4056 4756)
#endif

CCopasiXMLInterface::DBL::~DBL() {}

CCopasiXMLInterface::DBL::operator const C_FLOAT64 & () const
  {return mValue;}

std::ostream & operator << (std::ostream & os, const CCopasiXMLInterface::DBL & dbl)
{
  if (isnan(dbl.mValue))
    os << "NaN";
  else if (finite(dbl.mValue))
    os << dbl.mValue;
  else if (dbl.mValue > 0.0)
    os << "INF";
  else if (dbl.mValue < 0.0)
    os << "-INF";

  return os;
}

std::string CCopasiXMLInterface::utf8(const std::string & str)
{
  return str;

  std::ostringstream utf8;

  /* Based on RFC 2279.
     Since every string whithin COPASI is treated as latin1 and input
     is only optained through QT and Expat which will provide latin1
     encoded strings the below should suffice. */
  unsigned C_INT32 i, imax;
  for (i = 0, imax = str.length(); i < imax; i++)
    {
      if ((unsigned char) str[i] < 0x80) utf8 << str[i];
      else
        {
          utf8 << 0xc0 + ((str[i] >> 6) & 0x03);
          utf8 << 0x80 + (str[i] & 0x3f);
        }
    }
  return utf8.str();
}

CCopasiXMLInterface::CCopasiXMLInterface():
    mpModel(NULL),
    mpFunctionList(NULL),
    mpTaskList(NULL),
    mpReportList(NULL),
    mpPlotList(NULL),
    mpGUI(NULL),
    mpIstream(NULL),
    mpOstream(NULL),
    mIndent()
{}

CCopasiXMLInterface::~CCopasiXMLInterface() {}

bool CCopasiXMLInterface::load(const std::string & fileName)
{
  mFilename = fileName;
  std::ifstream is(fileName.c_str());

  if (is.fail()) return false;

  return load(is);
}

bool CCopasiXMLInterface::save(const std::string & fileName)
{
  mFilename = fileName;

  std::ostringstream tmp;
  if (!save(tmp)) return false;

  std::ofstream os(mFilename.c_str());
  if (os.fail()) return false;

  os << tmp.str();
  if (os.fail()) return false;

  return true;
}

bool CCopasiXMLInterface::setModel(const CModel & model)
{
  mpModel = const_cast<CModel *>(&model);
  return true;
}

CModel * CCopasiXMLInterface::getModel() const {return mpModel;}

bool CCopasiXMLInterface::haveModel() const {return mpModel != NULL;}

bool CCopasiXMLInterface::freeModel()
{
  pdelete(mpModel);
  return true;
}

bool CCopasiXMLInterface::setFunctionList(const CCopasiVectorN< CEvaluationTree > & functionList)
{
  mpFunctionList = const_cast<CCopasiVectorN< CEvaluationTree > *>(&functionList);
  return true;
}

CCopasiVectorN< CEvaluationTree > * CCopasiXMLInterface::getFunctionList() const
  {return mpFunctionList;}

bool CCopasiXMLInterface::haveFunctionList() const
  {return mpFunctionList != NULL;}

bool CCopasiXMLInterface::freeFunctionList()
{
  pdelete(mpFunctionList);
  return true;
}

bool CCopasiXMLInterface::setTaskList(const CCopasiVectorN< CCopasiTask > & taskList)
{
  mpTaskList = const_cast<CCopasiVectorN< CCopasiTask > *>(&taskList);
  return true;
}

CCopasiVectorN< CCopasiTask > * CCopasiXMLInterface::getTaskList() const
  {return mpTaskList;}

bool CCopasiXMLInterface::haveTaskList() const
  {return mpTaskList != NULL;}

bool CCopasiXMLInterface::freeTaskList()
{
  pdelete(mpTaskList);
  return true;
}

bool CCopasiXMLInterface::setPlotList(const COutputDefinitionVector & plotList)
{
  mpPlotList = const_cast<COutputDefinitionVector *>(&plotList);
  return true;
}

COutputDefinitionVector * CCopasiXMLInterface::getPlotList() const
  {return mpPlotList;}

bool CCopasiXMLInterface::havePlotList() const
  {return mpPlotList != NULL;}

bool CCopasiXMLInterface::freePlotList()
{
  pdelete(mpPlotList);
  return true;
}

bool CCopasiXMLInterface::setReportList(const CReportDefinitionVector & reportList)
{
  mpReportList = const_cast<CReportDefinitionVector *>(&reportList);
  return true;
}

CReportDefinitionVector * CCopasiXMLInterface::getReportList() const
  {return mpReportList;}

bool CCopasiXMLInterface::haveReportList() const
  {return mpReportList != NULL;}

bool CCopasiXMLInterface::freeReportList()
{
  pdelete(mpReportList);
  return true;
}

bool CCopasiXMLInterface::setGUI(const SCopasiXMLGUI & GUI)
{
  mpGUI = const_cast<SCopasiXMLGUI *>(&GUI);
  return true;
}

SCopasiXMLGUI * CCopasiXMLInterface::getGUI() const {return mpGUI;}

bool CCopasiXMLInterface::haveGUI() const {return mpGUI != NULL;}

bool CCopasiXMLInterface::freeGUI()
{
  pdelete(mpGUI);
  return true;
}

bool CCopasiXMLInterface::saveData(const std::string & data)
{
  *mpOstream << mIndent << CCopasiXMLInterface::encode(data) << std::endl;

  return true;
}

bool CCopasiXMLInterface::saveXhtml(const std::string & xhtml)
{
  if (xhtml[0] == '<')
    {
      std::string::size_type pos = xhtml.find('>');
      std::string FirstElement = xhtml.substr(0, pos);

      if (FirstElement.find("xmlns=\"http://www.w3.org/1999/xhtml\"") == std::string::npos)
        FirstElement += " xmlns=\"http://www.w3.org/1999/xhtml\"";

      *mpOstream << mIndent << FirstElement << xhtml.substr(pos) << std::endl;
    }
  else
    {
      CXMLAttributeList Attributes;
      Attributes.add("xmlns", "http://www.w3.org/1999/xhtml");

      startSaveElement("body", Attributes);

      saveData(xhtml);

      endSaveElement("body");
    }

  return true;
}

bool CCopasiXMLInterface::saveElement(const std::string & name,
                                      CXMLAttributeList & attributeList)
{
  *mpOstream << mIndent << "<" << name;

  unsigned C_INT32 i, imax;

  for (i = 0, imax = attributeList.size(); i < imax; i++)
    *mpOstream << attributeList.getAttribute(i);

  *mpOstream << "/>" << std::endl;

  return true;
}

bool CCopasiXMLInterface::startSaveElement(const std::string & name)
{
  *mpOstream << mIndent << "<" << name << ">" << std::endl;

  mIndent += "  ";
  return true;
}

bool CCopasiXMLInterface::startSaveElement(const std::string & name,
    CXMLAttributeList & attributeList)
{
  *mpOstream << mIndent << "<" << name;

  unsigned C_INT32 i, imax;
  for (i = 0, imax = attributeList.size(); i < imax; i++)
    *mpOstream << attributeList.getAttribute(i);

  *mpOstream << ">" << std::endl;

  mIndent += "  ";
  return true;
}

bool CCopasiXMLInterface::endSaveElement(const std::string & name)
{
  mIndent = mIndent.substr(0, mIndent.length() - 2);
  *mpOstream << mIndent << "</" << name << ">" << std::endl;

  return true;
}

CXMLAttributeList::CXMLAttributeList():
    mAttributeList(),
    mSaveList()
{}

CXMLAttributeList::CXMLAttributeList(const CXMLAttributeList & src):
    mAttributeList(src.mAttributeList),
    mSaveList(src.mSaveList)
{}

CXMLAttributeList::~CXMLAttributeList() {}

bool CXMLAttributeList::erase()
{
  mAttributeList.clear();
  mSaveList.clear();

  return true;
}

unsigned C_INT32 CXMLAttributeList::size() {return mAttributeList.size() / 2;}

bool CXMLAttributeList::add(const std::string & name, const C_FLOAT64 & value)
{
  return add(name,
             (CCopasiXMLInterface::DBL) value,
             CCopasiXMLInterface::attribute);
}

bool CXMLAttributeList::setName(const unsigned C_INT32 & index,
                                const std::string & name)
{
  mAttributeList[2 * index] = name;
  return true;
}

const std::string & CXMLAttributeList::getName(const unsigned C_INT32 & index) const
  {return mAttributeList[2 * index];}

const std::string & CXMLAttributeList::getValue(const unsigned C_INT32 & index) const
  {return mAttributeList[2 * index + 1];}

bool CXMLAttributeList::skip(const unsigned C_INT32 & index)
{
  mSaveList[index] = false;
  return true;
}

std::string CXMLAttributeList::getAttribute(const unsigned C_INT32 & index) const
  {
    if (mSaveList[index])
      return " " + mAttributeList[2 * index] + "=\"" + mAttributeList[2 * index + 1] + "\"";
    else
      return "";
  }

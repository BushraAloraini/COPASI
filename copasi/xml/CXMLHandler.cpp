/**
 * CXMLHandler classes.
 * These classes are the base class of all XML event handlers.
 *
 * Created for Copasi by Stefan Hoops 2003
 * Copyright Stefan Hoops
 */

#include "copasi.h"

#include "CXMLHandler.h"

CXMLElementHandler::CXMLElementHandler(std::stack< CXMLElementHandler * > & stack):
    mCurrentElement(-1),
    mpCurrentHandler(NULL),
    mStack(stack)
{}

CXMLElementHandler::~CXMLElementHandler() {}

void CXMLElementHandler::start(const XML_Char *pszName,
                               const XML_Char **papszAttrs) {}

void CXMLElementHandler::end(const XML_Char *pszName) {}

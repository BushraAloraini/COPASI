// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/sbml/unittests/test000074.cpp,v $
//   $Revision: 1.3 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2009/02/19 19:52:25 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

#include "test000074.h"

#include <sstream>
#include <string>
#include "copasi/CopasiDataModel/CCopasiDataModel.h"

#include "copasi/report/CCopasiRootContainer.h"

/**
 * Make sure importing a model with COPASI MIRIAM annotation and exporting it
 * again does not CRASH
 */
CCopasiDataModel* test000074::pCOPASIDATAMODEL = NULL;

void test000074::setUp()
{
  // Create the root container.
  CCopasiRootContainer::init(false, 0, NULL);
  // Create the global data model.
  pCOPASIDATAMODEL = CCopasiRootContainer::addDatamodel();
}

void test000074::tearDown()
{
  delete CCopasiRootContainer::Root;
  CCopasiRootContainer::Root = NULL;
}

void test000074::test_bug1088()
{
  CCopasiDataModel* pDataModel = pCOPASIDATAMODEL;
  CPPUNIT_ASSERT(pDataModel->importSBMLFromString(MODEL_STRING1));
  std::string s = pDataModel->exportSBMLToString(NULL, 2, 3);
  CPPUNIT_ASSERT(!s.empty());
}

const char* test000074::MODEL_STRING1 =
  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
  "<sbml xmlns=\"http://www.sbml.org/sbml/level2/version3\" metaid=\"metaid_0000001\" level=\"2\" version=\"3\">\n"
  "  <model metaid=\"metaid_0000002\" id=\"model_1\" name=\"model_1\">\n"
  "    <annotation>\n"
  "      <COPASI xmlns=\"http://www.copasi.org/static/sbml\">\n"
  "        <rdf:RDF xmlns:CopasiMT=\"http://www.copasi.org/RDF/MiriamTerms#\" xmlns:dcterms=\"http://purl.org/dc/terms/\" xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\" xmlns:vCard=\"http://www.w3.org/2001/vcard-rdf/3.0#\">\n"
  "          <rdf:Description rdf:about=\"#metaid_0000002\">\n"
  "            <dcterms:bibliographicCitation>\n"
  "              <rdf:Description>\n"
  "                <CopasiMT:isDescribedBy rdf:resource=\"urn:miriam:pubmed:14556891\"/>\n"
  "              </rdf:Description>\n"
  "            </dcterms:bibliographicCitation>\n"
  "            <dcterms:created>\n"
  "              <rdf:Description>\n"
  "                <dcterms:W3CDTF>2007-03-22T10:28:32Z</dcterms:W3CDTF>\n"
  "              </rdf:Description>\n"
  "            </dcterms:created>\n"
  "            <dcterms:creator>\n"
  "              <rdf:Description>\n"
  "                <rdf:Bag/>\n"
  "              </rdf:Description>\n"
  "            </dcterms:creator>\n"
  "            <dcterms:modified>\n"
  "              <rdf:Description>\n"
  "                <dcterms:W3CDTF>2007-09-25T11:11:02Z</dcterms:W3CDTF>\n"
  "              </rdf:Description>\n"
  "            </dcterms:modified>\n"
  "            <CopasiMT:hasPart>\n"
  "              <rdf:Bag>\n"
  "                <rdf:li rdf:resource=\"urn:miriam:kegg.pathway:hsa04020\"/>\n"
  "                <rdf:li rdf:resource=\"urn:miriam:obo.go:GO%3A0008607\"/>\n"
  "              </rdf:Bag>\n"
  "            </CopasiMT:hasPart>\n"
  "            <CopasiMT:is rdf:resource=\"urn:miriam:biomodels.db:BIOMD0000000100\"/>\n"
  "            <CopasiMT:isVersionOf>\n"
  "              <rdf:Bag>\n"
  "                <rdf:li rdf:resource=\"urn:miriam:obo.go:GO%3A0019722\"/>\n"
  "                <rdf:li rdf:resource=\"urn:miriam:obo.go:GO%3A0048016\"/>\n"
  "              </rdf:Bag>\n"
  "            </CopasiMT:isVersionOf>\n"
  "          </rdf:Description>\n"
  "        </rdf:RDF>\n"
  "      </COPASI>\n"
  "    </annotation>\n"
  "  </model>\n"
  "</sbml>\n";

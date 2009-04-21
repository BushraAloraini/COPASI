// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/MIRIAMUI/Attic/CQRDFListViewWidget.ui.h,v $
//   $Revision: 1.19 $
//   $Name:  $
//   $Creator: aekamal $
//   $Date: 2009/04/21 16:17:35 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

#include "CRDFListViewItem.h"

#include "UI/CQMessageBox.h"
#include "UI/qtUtilities.h"

#include "MIRIAM/CRDFGraph.h"
#include "MIRIAM/CRDFParser.h"
#include "MIRIAM/CRDFWriter.h"

#include "CopasiDataModel/CCopasiDataModel.h"
#include "report/CKeyFactory.h"
#include "model/CModel.h"
#include "model/CModelValue.h"
#include "utilities/CCopasiMessage.h"
#include "copasi/report/CCopasiRootContainer.h"

#include "MIRIAM/CRDFSubject.h"

#define COL_SUBJECT    0
#define COL_PREDICATE  1
#define COL_OBJECT     2

void CQRDFListViewWidget::init()
{
  mpGraph = NULL;
}

bool CQRDFListViewWidget::enter(const std::string & key)
{
  mKey = key;
  load();

  return true;
}

bool CQRDFListViewWidget::leave()
{
  save();

  return true;
}

void CQRDFListViewWidget::load()
{
  Ui::CQRDFListViewWidget::mpListView->clear();
  pdelete(mpGraph);

  CCopasiObject *pObject = dynamic_cast< CCopasiObject * >(CCopasiRootContainer::getKeyFactory()->get(mKey));

  if (pObject != NULL)
    {
      CModelEntity * pEntity = NULL;
      CReaction * pReaction = NULL;
      CFunction * pFunction = NULL;
      const std::string * pMiriamAnnotation = NULL;

      if ((pEntity = dynamic_cast< CModelEntity * >(pObject)) != NULL)
        pMiriamAnnotation = &pEntity->getMiriamAnnotation();
      else if ((pReaction = dynamic_cast< CReaction * >(pObject)) != NULL)
        pMiriamAnnotation = &pReaction->getMiriamAnnotation();
      else if ((pFunction = dynamic_cast< CFunction * >(pObject)) != NULL)
        pMiriamAnnotation = &pFunction->getMiriamAnnotation();

      if (pMiriamAnnotation && *pMiriamAnnotation != "")
        mpGraph = CRDFParser::graphFromXml(*pMiriamAnnotation);
    }

  CCopasiMessage::clearDeque();

  if (CCopasiMessage::size() != 0)
    {
      QString Message = FROM_UTF8(CCopasiMessage::getAllMessageText());
      CQMessageBox::warning(this, QString("RDF Warning"), Message,
                            QMessageBox::Ok, QMessageBox::Ok);
    }

  if (mpGraph == NULL)
    mpGraph = new CRDFGraph;

  // We make sure that we always have an about node.
  mpGraph->createAboutNode(mKey);

  // We iterate of all triplets
  std::set< CRDFTriplet >::const_iterator it = mpGraph->getTriplets().begin();
  std::set< CRDFTriplet >::const_iterator end = mpGraph->getTriplets().end();

  for (; it != end; ++it)
    {
      CRDFListViewItem * pSubjectItem = Ui::CQRDFListViewWidget::mpListView->find(it->pSubject);

      if (pSubjectItem == NULL)
        {
          pSubjectItem = new CRDFListViewItem(Ui::CQRDFListViewWidget::mpListView, NULL);
          Ui::CQRDFListViewWidget::mpListView->insert(it->pSubject, pSubjectItem);
          // Display the subject information

          const CRDFSubject & Subject = it->pSubject->getSubject();

          switch (Subject.getType())
            {
              case CRDFSubject::RESOURCE:
                pSubjectItem->setText(COL_SUBJECT, FROM_UTF8(Subject.getResource()));
                break;

              case CRDFSubject::BLANK_NODE:
                pSubjectItem->setText(COL_SUBJECT, FROM_UTF8(Subject.getBlankNodeID()));
                break;
            }
        }

      CRDFListViewItem * pObjectItem = NULL;

      if (it->Predicate.getURI() == "http://www.w3.org/1999/02/22-rdf-syntax-ns#subject")
        pObjectItem = new CRDFListViewItem(pSubjectItem, NULL);
      else
        pObjectItem = Ui::CQRDFListViewWidget::mpListView->find(it->pObject);

      if (pObjectItem == NULL)
        {
          pObjectItem = new CRDFListViewItem(pSubjectItem, NULL);
          Ui::CQRDFListViewWidget::mpListView->insert(it->pObject, pObjectItem);
        }
      else
        {
          Q3ListViewItem * pParent = pObjectItem->parent();

          if (pParent == NULL)
            {
              Ui::CQRDFListViewWidget::mpListView->takeItem(pObjectItem);
              pSubjectItem->insertItem(pObjectItem);
            }
          else
            {
              pParent->takeItem(pObjectItem);
              pSubjectItem->insertItem(pObjectItem);
            }
        }

      pObjectItem->setTriplet(*it);
    }
}

void CQRDFListViewWidget::save()
{
  pdelete(mpGraph);
}

void CQRDFListViewWidget::destroy()
{}

bool CQRDFListViewWidget::update(ListViews::ObjectType objectType, ListViews::Action, const std::string & key)
{
  if (objectType != ListViews::MIRIAM)
    return true;

  if (key != mKey)
    return true;

  load();

  return true;
}

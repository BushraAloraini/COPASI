/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/Attic/CopasiDefaultWidget.cpp,v $
   $Revision: 1.5 $
   $Name:  $
   $Author: shoops $
   $Date: 2006/04/27 01:27:43 $
   End CVS Header */

// Copyright � 2005 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/*******************************************************************
 **  $ CopasiUI/CopasiDefaultWidget.cpp
 **  $ Author  : Mudita Singhal
 **
 ** This file is used to create the GUI FrontPage for the  information
 ** obtained from the data model about the Moiety----It is
 ** the Second level of Moieties.
 ********************************************************************/
#include <qlabel.h>
//#include <qlistbox.h>
//#include <qlineedit.h>
//#include <qpushbutton.h>
#include <qlayout.h>
//#include <qtoolbar.h>
#include <qwidget.h>
//#include <qframe.h>
#include <qpixmap.h>

#include "copasi.h"
//#include "utilities/CCopasiVector.h"
#include "CopasiDefaultWidget.h"
//#include "model/CModel.h"
//#include "model/CMoiety.h"
//#include "listviews.h"
//#include "DataModelGUI.h"
#include "qtUtilities.h"
//#include "report/CKeyFactory.h"

#include "./icons/copasi_rc.xpm"

/*
 *  Constructs a MoietyWidget which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */

CopasiDefaultWidget::CopasiDefaultWidget(QWidget *parent, const char * name, WFlags f)
    : CopasiWidget(parent, name, f)

{
  if (!name)
    setName("CopasiDefaultWidget");
  //setCaption("CopasiDefaultWidget");
  CopasiDefaultWidgetLayout = new QGridLayout(this, 3, 3, 11, 6, "CopasiDefaultWidgetLayout");

  QPixmap * pLogo = new QPixmap((const char**)copasi_rc_xpm);

  TextLabel1 = new QLabel(this, "TextLabel1");
  TextLabel1->setText("empty");
  TextLabel1->setPixmap(*pLogo);

  CopasiDefaultWidgetLayout->addWidget(TextLabel1, 1, 1);

  // signals and slots connections
}

CopasiDefaultWidget::~CopasiDefaultWidget()
{}

bool CopasiDefaultWidget::update(ListViews::ObjectType C_UNUSED(objectType),
                                 ListViews::Action C_UNUSED(action), const std::string & C_UNUSED(key))
{
  return true;
}

bool CopasiDefaultWidget::leave()
{
  return true;
}

bool CopasiDefaultWidget::enter(const std::string & C_UNUSED(key))
{
  return true;
}

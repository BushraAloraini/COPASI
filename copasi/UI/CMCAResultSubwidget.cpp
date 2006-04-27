/* Begin CVS Header
   $Source: /home/cvs/copasi_dev/copasi/CopasiUI/CMCAResultSubwidget.cpp,v $
   $Revision: 1.6 $
   $Name:  $
   $Author: shoops $
   $Date: 2006/03/02 02:21:43 $
   End CVS Header */

// Copyright � 2005 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/****************************************************************************
 ** Form implementation generated from reading ui file 'CMCAResultSubwidget.ui'
 **
 ** Created: Thu Feb 23 13:33:29 2006
 **      by: The User Interface Compiler ($Id: CMCAResultSubwidget.cpp,v 1.6 2006/03/02 02:21:43 shoops Exp $)
 **
 ** WARNING! All changes made in this file will be lost!
 ****************************************************************************/

#include "CMCAResultSubwidget.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qtabwidget.h>
#include <qtable.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include "CMCAResultSubwidget.ui.h"

/*
 *  Constructs a CMCAResultSubwidget as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
CMCAResultSubwidget::CMCAResultSubwidget(QWidget* parent, const char* name, WFlags fl)
    : QWidget(parent, name, fl)
{
  if (!name)
    setName("CMCAResultSubwidget");
  CMCAResultSubwidgetLayout = new QVBoxLayout(this, 11, 6, "CMCAResultSubwidgetLayout");

  layout3 = new QHBoxLayout(0, 0, 6, "layout3");

  mTopLabel = new QLabel(this, "mTopLabel");
  layout3->addWidget(mTopLabel);

  mComboScale = new QComboBox(FALSE, this, "mComboScale");
  mComboScale->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, mComboScale->sizePolicy().hasHeightForWidth()));
  layout3->addWidget(mComboScale);

  mSaveButton = new QPushButton(this, "mSaveButton");
  mSaveButton->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, mSaveButton->sizePolicy().hasHeightForWidth()));
  layout3->addWidget(mSaveButton);
  CMCAResultSubwidgetLayout->addLayout(layout3);

  mTabWidget = new QTabWidget(this, "mTabWidget");

  tab = new QWidget(mTabWidget, "tab");
  tabLayout = new QVBoxLayout(tab, 11, 6, "tabLayout");

  mTableElasticities = new QTable(tab, "mTableElasticities");
  mTableElasticities->setNumRows(0);
  mTableElasticities->setNumCols(0);
  mTableElasticities->setReadOnly(TRUE);
  tabLayout->addWidget(mTableElasticities);
  mTabWidget->insertTab(tab, QString(""));

  tab_2 = new QWidget(mTabWidget, "tab_2");
  tabLayout_2 = new QVBoxLayout(tab_2, 11, 6, "tabLayout_2");

  mTableFCC = new QTable(tab_2, "mTableFCC");
  mTableFCC->setNumRows(0);
  mTableFCC->setNumCols(0);
  mTableFCC->setReadOnly(TRUE);
  tabLayout_2->addWidget(mTableFCC);
  mTabWidget->insertTab(tab_2, QString(""));

  TabPage = new QWidget(mTabWidget, "TabPage");
  TabPageLayout = new QVBoxLayout(TabPage, 11, 6, "TabPageLayout");

  mTableCCC = new QTable(TabPage, "mTableCCC");
  mTableCCC->setNumRows(0);
  mTableCCC->setNumCols(0);
  mTableCCC->setReadOnly(TRUE);
  TabPageLayout->addWidget(mTableCCC);
  mTabWidget->insertTab(TabPage, QString(""));
  CMCAResultSubwidgetLayout->addWidget(mTabWidget);
  languageChange();
  resize(QSize(580, 422).expandedTo(minimumSizeHint()));
  clearWState(WState_Polished);

  // signals and slots connections
  connect(mComboScale, SIGNAL(activated(int)), this, SLOT(slotScaled()));
  connect(mSaveButton, SIGNAL(clicked()), this, SLOT(slotSave()));
  init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
CMCAResultSubwidget::~CMCAResultSubwidget()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void CMCAResultSubwidget::languageChange()
{
  setCaption(tr("Form1"));
  mTopLabel->setText(QString::null);
  mComboScale->clear();
  mComboScale->insertItem(tr("scaled"));
  mComboScale->insertItem(tr("unscaled"));
  mSaveButton->setText(tr("Save data to file"));
  mTabWidget->changeTab(tab, tr("Elasticities"));
  mTabWidget->changeTab(tab_2, tr("Flux Control Coefficients"));
  mTabWidget->changeTab(TabPage, tr("Concentration Control Coefficients"));
}

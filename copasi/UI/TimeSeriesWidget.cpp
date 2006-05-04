/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/Attic/TimeSeriesWidget.cpp,v $
   $Revision: 1.10 $
   $Name:  $
   $Author: shoops $
   $Date: 2006/05/04 19:13:10 $
   End CVS Header */

// Copyright � 2005 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

//#include <qpushbutton.h>
#include <qlayout.h>
#include "copasi.h"

#include "TimeSeriesWidget.h"
#include "TimeSeriesSubwidget.h"
#include "CopasiDataModel/CCopasiDataModel.h"
#include "utilities/CCopasiVector.h"
#include "trajectory/CTrajectoryTask.h"
#include "CTimeSeriesTable.h"

//#include "report/CKeyFactory.h"
#include "qtUtilities.h"

/*
 *  Constructs a TimeSeriesWidget which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
TimeSeriesWidget::TimeSeriesWidget(QWidget* parent, const char* name, WFlags fl)
    : CopasiWidget(parent, name, fl),
    objKey("")
{
  if (!name)
    setName("TimeSeriesWidget");
  setCaption(trUtf8("TimeSeriesWidget"));

  mWidgetLayout = new QGridLayout(this, 1, 1, 0, -1, "Layout");

  mCentralWidget = new TimeSeriesSubWidget(this, "TimeSeriesSubwidget");
  mWidgetLayout->addWidget(mCentralWidget, 0, 0);

  /*commitChanges = new QPushButton(this, "commitChanges");
  commitChanges->setText(trUtf8("Commit"));
  Layout5->addWidget(commitChanges);

  cancelChanges = new QPushButton(this, "cancelChanges");
  cancelChanges->setText(trUtf8("Revert"));
  Layout5->addWidget(cancelChanges);*/

  // signals and slots connections
  //connect(commitChanges, SIGNAL(clicked()), this, SLOT(slotBtnOKClicked()));
  //connect(cancelChanges, SIGNAL(clicked()), this, SLOT(slotBtnCancelClicked()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
TimeSeriesWidget::~TimeSeriesWidget()
{}

bool TimeSeriesWidget::loadFromBackend()
{
  //  mCentralWidget->table()->setTimeSeries(CCopasiDataModel::Global->getTrajectoryTask()->getTimeSeries());
  mCentralWidget->displayOptimizationTab(false);
  mCentralWidget->table()->setTimeSeries(dynamic_cast<CTrajectoryTask *>((*CCopasiDataModel::Global->getTaskList())["Time-Course"])->getTimeSeries());
  return true;
}

bool TimeSeriesWidget::saveToBackend()
{
  return true;
}

/*void TimeSeriesWidget::slotBtnCancelClicked()
{
  enter(objKey); // reload
}*/

/*void TimeSeriesWidget::slotBtnOKClicked()
{
  saveToCompartment();
}*/

bool TimeSeriesWidget::update(ListViews::ObjectType C_UNUSED(objectType), ListViews::Action C_UNUSED(action), const std::string & C_UNUSED(key))
{
  if (this->isShown())
    return loadFromBackend();
  else
    return true;
}

bool TimeSeriesWidget::leave()
{
  //return saveToCompartment();
  return true;
}

bool TimeSeriesWidget::enter(const std::string & C_UNUSED(key))
{
  return loadFromBackend();
  /*objKey = key;
  CCompartment* comp = dynamic_cast< CCompartment * >(GlobalKeys.get(key));

  if (comp) return loadFromCompartment(comp);
  else return false;*/
}

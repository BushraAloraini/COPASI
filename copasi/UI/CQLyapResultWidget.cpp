/* Begin CVS Header
$Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/CQLyapResultWidget.cpp,v $
$Revision: 1.6 $
$Name:  $
$Author: shoops $
$Date: 2009/02/19 19:53:06 $
End CVS Header */

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

//#include <qpushbutton.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <q3table.h>
//Added by qt3to4:
#include <Q3GridLayout>

#include "copasi.h"

#include "CQLyapResultWidget.h"
//#include "StateSubwidget.h"
//#include "CTimeSeriesTable.h"
#include "qtUtilities.h"

#include "CopasiDataModel/CCopasiDataModel.h"
#include "report/CCopasiRootContainer.h"
#include "report/CKeyFactory.h"
#include "model/CModel.h"
#include "lyap/CLyapTask.h"
#include "lyap/CLyapProblem.h"

/*
 *  Constructs a CQLyapResultWidget which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
CQLyapResultWidget::CQLyapResultWidget(QWidget* parent, const char* name, Qt::WFlags fl)
    : CopasiWidget(parent, name, fl)
{
  if (!name)
    setName("CQLyapResultWidget");
  setCaption("CQLyapResultWidget");

  mWidgetLayout = new Q3GridLayout(this, 1, 1, 11, 6, "LyapResultWidgetLayout");

  // **********  Exponents **************
  mLabelExponents = new QLabel(this, "ExponentsLabel");
  mLabelExponents->setText(trUtf8("Lyapunov Exponents"));
  mLabelExponents->setAlignment(int(Qt::AlignVCenter
                                    | Qt::AlignRight));
  mWidgetLayout->addWidget(mLabelExponents, 4, 0);

  QSpacerItem* spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  mWidgetLayout->addItem(spacer, 5, 0);

  mTableExponents = new Q3Table(this, "mTableExponents");
  mWidgetLayout->addMultiCellWidget(mTableExponents, 4, 5, 1, 2);
  mTableExponents->setNumRows(0);
  mTableExponents->setNumCols(1);
  Q3Header *colHeader = mTableExponents->horizontalHeader();
  colHeader->setLabel(0, tr("Exponent"));
  mTableExponents->setColumnStretchable(0, true);
  mTableExponents->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

  // ********** Sum *********************
  mLabelSum = new QLabel(this, "SumLabel");
  mLabelSum->setText(trUtf8("Sum of Lyapunov Exponents"));
  mLabelSum->setAlignment(int(Qt::AlignVCenter
                              | Qt::AlignRight));
  mWidgetLayout->addWidget(mLabelSum, 6, 0);

  mLineEditSum = new QLineEdit(this, "mLineEditSum");
  mLineEditSum->setReadOnly(true);
  mWidgetLayout->addWidget(mLineEditSum, 6, 1);

  // ************* Divergence **************
  mLabelDivergence = new QLabel(this, "mLabelDivergence");
  mLabelDivergence->setText(trUtf8("Divergence"));
  mLabelDivergence->setAlignment(int(Qt::AlignVCenter
                                     | Qt::AlignRight));
  mWidgetLayout->addWidget(mLabelDivergence, 7, 0);

  mLineEditDivergence = new QLineEdit(this, "mLineEditDivergence");
  mLineEditDivergence->setReadOnly(true);
  mWidgetLayout->addWidget(mLineEditDivergence, 7, 1);

  // ************* comment ******************
  mLabelComment = new QLabel(this, "mLabelComment");
  mLabelComment->setAlignment(int(Qt::WordBreak));
  mLabelComment->setText("");

  mWidgetLayout->addWidget(mLabelComment, 8, 1);

  spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
  mWidgetLayout->addItem(spacer, 9, 0);
}

/*
 *  Destroys the object and frees any allocated resources
 */
CQLyapResultWidget::~CQLyapResultWidget()
{}

/* This function loads the optimization result widget when its name is
  clicked in the tree   */
bool CQLyapResultWidget::loadFromBackend()
{
  //bool success = true;
  //std::ostringstream os;

  assert(CCopasiRootContainer::getDatamodelList()->size() > 0);
  CLyapTask * pTask = dynamic_cast<CLyapTask*>((*(*CCopasiRootContainer::getDatamodelList())[0]->getTaskList())["Lyapunov Exponents"]);
  if (!pTask) return false;

  CLyapProblem * pProblem = dynamic_cast< CLyapProblem * >(pTask->getProblem());
  if (!pProblem) return false;

  if (!pTask->resultAvailable())
    {
      mTableExponents->setEnabled(false);
      mTableExponents->setNumRows(0);
      mLineEditSum->setEnabled(false);
      mLineEditSum->setText("");
      mLineEditDivergence->setEnabled(false);
      mLineEditDivergence->setText("");
      mLabelComment->setText("No result available.");
      return false;
    }

  mTableExponents->setEnabled(true);
  mLineEditSum->setEnabled(true);
  mLineEditDivergence->setEnabled(true);

  unsigned C_INT32 i, imax = pProblem->getExponentNumber();

  mTableExponents->setNumRows(imax);
  for (i = 0; i < imax; ++i)
    mTableExponents->setText(i, 0, QString::number(pTask->exponents()[i]));

  mLineEditSum->setText(QString::number(pTask->sumOfExponents()));

  if (pTask->resultHasDivergence())
    {
      mLineEditDivergence->setEnabled(true);
      mLineEditDivergence->setText(QString::number(pTask->averageDivergence()));
    }
  else
    {
      mLineEditDivergence->setEnabled(false);
      mLineEditDivergence->setText("");
    }

  //comment
  mLabelComment->setText("");
  if (pTask->resultHasDivergence()
      && (pTask->modelVariablesInResult() == pTask->numberOfExponentsCalculated()))
    {
      if ((pTask->sumOfExponents() < 0.0) && (pTask->averageDivergence() < 0.0))
        {
          C_FLOAT64 factor = pTask->averageDivergence() / pTask->sumOfExponents();
          if (factor > 1.01)
            mLabelComment->setText("Warning: Divergence differs from sum of exponents. This may indicate that the strongly negative exponents are calculated inaccuratly.");
        }
    }

  return true;
}

bool CQLyapResultWidget::update(ListViews::ObjectType C_UNUSED(objectType), ListViews::Action
                                C_UNUSED(action), const std::string & C_UNUSED(key))
{
  if (this->isShown())
    return loadFromBackend();
  else
    return true;
}

bool CQLyapResultWidget::leave()
{
  return true;
}

bool CQLyapResultWidget::enter(const std::string & C_UNUSED(key))
{
  return loadFromBackend();
}

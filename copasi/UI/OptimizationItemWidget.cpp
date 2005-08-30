/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/Attic/OptimizationItemWidget.cpp,v $
   $Revision: 1.36 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2005/08/30 15:39:51 $
   End CVS Header */

/********************************************************
Author: Liang Xu
Version : 1.xx  <first>
Description: 
Date: 10/01
Comment : OptimizationItemWidget for embeded widget for limit of the optimization function
Contact: Please contact lixu1@vt.edu.
 *********************************************************/
#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qmessagebox.h>

#include "OptimizationWidget.h"
#include "OptimizationItemWidget.h"
#include "CopasiDataModel/CCopasiDataModel.h" 
//#include "ScanItemWidget.h"
#include "FunctionItemWidget.h"
#include "report/CKeyFactory.h"
#include "function/CFunction.h"
#include "function/CFunctionDB.h"
#include "function/CKinFunction.h"
#include "optimization/COptItem.h"

#include "utilities/CCopasiException.h"
#include "utilities/utility.h"
#include "qtUtilities.h"
#include "ObjectBrowserItem.h"
#include "CCopasiSelectionDialog.h"

/*
 *  Constructs a OptimizationItemWidget as a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 */
OptimizationItemWidget::OptimizationItemWidget(QWidget* parent, const char* name, WFlags fl)
    : QWidget(parent, name, fl),
    mpParent(parent)
{
  if (!name)
    setName("OptimizationItemWidget");

  textLabel2 = new QLabel(this, "textLabel2");
  textLabel2->setGeometry(QRect(190, 10, 48, 20));

  comboBoxLowerOp = new QLabel(this, "comboBoxLowerOp");
  comboBoxLowerOp->setGeometry(QRect(130, 30, 43, 20));

  textLabel4 = new QLabel(this, "textLabel4");
  textLabel4->setGeometry(QRect(130, 10, 30, 16));

  lineLower = new QLineEdit(this, "lineLower");
  lineLower->setGeometry(QRect(10, 30, 80, 20));
  lineLower->setFrameShape(QLineEdit::LineEditPanel);
  lineLower->setFrameShadow(QLineEdit::Sunken);

  ObjectName = new QLineEdit(this, "ObjectName");
  ObjectName->setEnabled(FALSE);
  ObjectName->setGeometry(QRect(190, 30, 80, 20));
  ObjectName->setFrameShape(QLineEdit::LineEditPanel);
  ObjectName->setFrameShadow(QLineEdit::Sunken);

  buttonParamEdit = new QPushButton(this, "buttonParamEdit");
  buttonParamEdit->setGeometry(QRect(280, 30, 16, 20));

  buttonLowerEdit = new QPushButton(this, "buttonLowerEdit");
  buttonLowerEdit->setGeometry(QRect(100, 30, 16, 20));

  line3 = new QFrame(this, "line3");
  line3->setGeometry(QRect(170, 20, 20, 40));
  line3->setFrameShape(QFrame::VLine);
  line3->setFrameShadow(QFrame::Sunken);
  line3->setFrameShape(QFrame::VLine);

  comboBoxUpperOp = new QLabel(this, "comboBoxUpperOp");
  comboBoxUpperOp->setGeometry(QRect(320, 30, 43, 20));

  line3_2 = new QFrame(this, "line3_2");
  line3_2->setGeometry(QRect(300, 20, 16, 40));
  line3_2->setFrameShape(QFrame::VLine);
  line3_2->setFrameShadow(QFrame::Sunken);
  line3_2->setFrameShape(QFrame::VLine);

  textLabel3 = new QLabel(this, "textLabel3");
  textLabel3->setGeometry(QRect(320, 10, 30, 20));

  lineUpper = new QLineEdit(this, "lineUpper");
  lineUpper->setGeometry(QRect(370, 30, 80, 20));

  buttonUpperEdit = new QPushButton(this, "buttonUpperEdit");
  buttonUpperEdit->setGeometry(QRect(460, 30, 16, 20));

  checkUpperInf = new QCheckBox(this, "checkUpperInf");
  checkUpperInf->setGeometry(QRect(370, 60, 50, 16));

  checkLowerInf = new QCheckBox(this, "checkLowerInf");
  checkLowerInf->setGeometry(QRect(10, 60, 50, 16));
  languageChange();
  resize(QSize(494, 78).expandedTo(minimumSizeHint()));
  clearWState(WState_Polished);

  // signals and slots connections
  connect(checkUpperInf, SIGNAL(clicked()), this, SLOT(slotPosInfClicked()));
  connect(buttonUpperEdit, SIGNAL(clicked()), this, SLOT(slotUpperEdit()));
  connect(buttonLowerEdit, SIGNAL(clicked()), this, SLOT(slotLowerEdit()));
  connect(checkLowerInf, SIGNAL(clicked()), this, SLOT(slotNegInfClicked()));
  connect(buttonParamEdit, SIGNAL(clicked()), this, SLOT(slotParamEdit()));

  // tab order
  setTabOrder(ObjectName, comboBoxUpperOp);
  setTabOrder(comboBoxUpperOp, lineUpper);
  setTabOrder(lineUpper, buttonUpperEdit);
  setTabOrder(buttonUpperEdit, checkUpperInf);
  setTabOrder(checkUpperInf, comboBoxLowerOp);
  setTabOrder(comboBoxLowerOp, lineLower);
  setTabOrder(lineLower, buttonLowerEdit);
  setTabOrder(buttonLowerEdit, checkLowerInf);

  checkUpperInf->setChecked(true);
  checkLowerInf->setChecked(true);

  //  buttonUpperEdit->setEnabled(false);
  //  buttonLowerEdit->setEnabled(false);

  lineUpper->setEnabled(false);
  lineLower->setEnabled(false);
  ObjectName->setEnabled(true);

  paramObjectCN = NULL;

  mpLowerObject = NULL;
  mpUpperObject = NULL;
}

/*
 *  Destroys the object and frees any allocated resources
 */
OptimizationItemWidget::~OptimizationItemWidget()
{
  // no need to delete child widgets, Qt does it all for us
}

void OptimizationItemWidget::languageChange()
{
  setCaption(tr("Optimization Item"));
  textLabel2->setText(tr("Parameter"));
  textLabel4->setText(tr("Lower"));

  comboBoxLowerOp->setText(tr("<="));

  comboBoxUpperOp->setText(tr("<="));

  textLabel3->setText(tr("Upper"));
  checkUpperInf->setText(tr("+ Inf"));
  checkLowerInf->setText(tr("- Inf"));
  buttonUpperEdit->setText(tr("..."));
  buttonLowerEdit->setText(tr("..."));
  buttonParamEdit->setText(tr("..."));
}

void OptimizationItemWidget::slotPosInfClicked()
{
  lineUpper->setEnabled(!checkUpperInf->isChecked());
  //buttonUpperEdit->setEnabled(!checkUpperInf->isChecked());
}

void OptimizationItemWidget::slotLowerEdit()
{
  checkLowerInf->setChecked(false);
  lineLower->setEnabled(true);
  CCopasiSelectionDialog* browseDialog = new CCopasiSelectionDialog(this);
  browseDialog->setModel(CCopasiDataModel::Global->getModel());
  browseDialog->setSingleSelection(true);
  std::vector<CCopasiObject*>* selection = new std::vector<CCopasiObject*>();
  browseDialog->setOutputVector(selection);

  if (browseDialog->exec () == QDialog::Accepted && selection->size() != 0)
    {
      mpLowerObject = selection->at(0);
      lineLower->setText(FROM_UTF8(mpLowerObject->getObjectDisplayName()));
    }
}

void OptimizationItemWidget::slotNegInfClicked()
{
  lineLower->setEnabled(!checkLowerInf->isChecked());
  //buttonLowerEdit->setEnabled(!checkLowerInf->isChecked());
}

void OptimizationItemWidget::slotUpperEdit()
{
  checkUpperInf->setChecked(false);
  lineUpper->setEnabled(true);
  CCopasiSelectionDialog* browseDialog = new CCopasiSelectionDialog(this);
  browseDialog->setModel(CCopasiDataModel::Global->getModel());
  browseDialog->setSingleSelection(true);
  std::vector<CCopasiObject*>* selection = new std::vector<CCopasiObject*>();
  browseDialog->setOutputVector(selection);

  if (browseDialog->exec () == QDialog::Accepted && selection->size() != 0)
    {
      mpUpperObject = selection->at(0);
      lineUpper->setText(FROM_UTF8(mpUpperObject->getObjectDisplayName()));
    }
}

std::string OptimizationItemWidget::getItemUpperLimit() const
  {
    if (checkUpperInf->isChecked())
      return "inf";
    else if (isNumber((const char*) lineUpper->text().utf8()))
      return (const char*) lineUpper->text().utf8();
    else if (mpUpperObject)
      return mpUpperObject->getCN();
    else
      return "";
  }

std::string OptimizationItemWidget::getItemLowerLimit() const
  {
    if (checkLowerInf->isChecked())
      return "-inf";
    else if (isNumber((const char*) lineLower->text().utf8()))
      return (const char*) lineLower->text().utf8();
    else if (mpLowerObject)
      return mpLowerObject->getCN();
    else
      return "";
  }

CCopasiObject* OptimizationItemWidget::getCopasiObject()
{
  return mpObject;
}

void OptimizationItemWidget::setCopasiObjectPtr (CCopasiObject* sourceObject)
{
  if (!sourceObject) // NULL pointer
    return;
  mpObject = sourceObject;
  ObjectName->setText(FROM_UTF8(mpObject->getObjectUniqueName()));
}

void OptimizationItemWidget::setItemUpperLimit(std::string strUpperLimit)
{
  if (strUpperLimit == "inf")
    {
      checkUpperInf->setChecked(true);
      //buttonUpperEdit->setEnabled(false);
      lineUpper->setEnabled(false);
      lineUpper->setText("");
    }
  else if (isNumber(strUpperLimit))
    {
      checkUpperInf->setChecked(false);
      //buttonUpperEdit->setEnabled(true);
      lineUpper->setEnabled(true);
      lineUpper->setText(FROM_UTF8(strUpperLimit));
    }
  else
    {
      checkUpperInf->setChecked(false);
      //buttonUpperEdit->setEnabled(true);
      lineUpper->setEnabled(true);

      mpUpperObject = RootContainer.getObject(strUpperLimit);
      if (mpUpperObject)
        lineUpper->setText(FROM_UTF8(mpUpperObject->getObjectDisplayName()));
      else
        lineUpper->setText("");
    }
}

void OptimizationItemWidget::setItemLowerLimit(std::string strLowerLimit)
{
  mpLowerObject = NULL;
  if (strLowerLimit == "-inf")
    {
      checkLowerInf->setChecked(true);
      //buttonLowerEdit->setEnabled(false);
      lineLower->setEnabled(false);
      lineLower->setText("");
    }
  else if (isNumber(strLowerLimit))
    {
      checkLowerInf->setChecked(false);
      //buttonLowerEdit->setEnabled(true);
      lineLower->setEnabled(true);
      lineLower->setText(FROM_UTF8(strLowerLimit));
    }
  else
    {
      checkLowerInf->setChecked(false);
      lineLower->setEnabled(true);

      mpLowerObject = RootContainer.getObject(strLowerLimit);
      if (mpLowerObject)
        lineLower->setText(FROM_UTF8(mpLowerObject->getObjectDisplayName()));
      else
        lineLower->setText("");
    }
}

std::string OptimizationItemWidget::getItemUpperOper() const
  {
    return (const char*)comboBoxUpperOp->text().utf8();
  }

std::string OptimizationItemWidget::getItemLowerOper() const
  {
    return (const char*)comboBoxLowerOp->text().utf8();
  }

void OptimizationItemWidget::setItemUpperOper(std::string oper)
{
  comboBoxUpperOp->setText("<=");
}

void OptimizationItemWidget::setItemLowerOper(std::string oper)
{
  comboBoxLowerOp->setText("<=");
}

void OptimizationItemWidget::slotParamEdit()
{
  ObjectName->setEnabled(true);
  CCopasiSelectionDialog* browseDialog = new CCopasiSelectionDialog(this);
  browseDialog->setModel(CCopasiDataModel::Global->getModel());
  browseDialog->setSingleSelection(true);
  std::vector<CCopasiObject*>* selection = new std::vector<CCopasiObject*>();
  browseDialog->setOutputVector(selection);

  if (browseDialog->exec () == QDialog::Accepted && selection->size() != 0)
    {
      paramObjectCN = new CCopasiObjectName();
      *paramObjectCN = (selection->at(0))->getCN();
      ObjectName->setText(FROM_UTF8((selection->at(0))->getObjectDisplayName()));
    }
}

bool OptimizationItemWidget::saveToOptItem(COptProblem * pg) const
  {
    if (paramObjectCN != NULL)
      {
        COptItem & pTmp = (pg->addOptItem(*paramObjectCN));
        pTmp.setLowerBound(getItemLowerLimit());
        pTmp.setLowerRelation(getItemLowerOper());
        pTmp.setUpperBound(getItemUpperLimit());
        pTmp.setUpperRelation(getItemUpperOper());
        return true;
      }

    else
      return false;
  }

bool OptimizationItemWidget::initFromOptItem(const COptItem& item)
{
  if (paramObjectCN == NULL) paramObjectCN = new CCopasiObjectName();

  *paramObjectCN = item.getObjectCN();
  ObjectName->setText(FROM_UTF8(RootContainer.getObject(*paramObjectCN)->getObjectDisplayName()));

  setItemUpperLimit(item.getUpperBound());
  setItemLowerLimit(item.getLowerBound());

  setItemUpperOper(item.getUpperRelation());
  setItemLowerOper(item.getLowerRelation());

  return true;
}

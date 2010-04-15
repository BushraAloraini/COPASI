// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/Attic/ModelWidget.cpp,v $
//   $Revision: 1.61 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2010/04/15 15:57:53 $
// End CVS Header

// Copyright (C) 2010 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/*******************************************************************
 **  $ CopasiUI/ModelWidget.cpp
 **  $ Author  : Mudita Singhal
 **
 ** This file is used to create the GUI FrontPage for the
 ** information obtained from the data model about the Model
 ********************************************************************/
#include <qlabel.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <q3toolbar.h>
#include <qwidget.h>
#include <q3frame.h>
#include <q3listbox.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <Q3GridLayout>
#include <Q3VBoxLayout>

#include "copasi.h"
#include "CopasiDataModel/CCopasiDataModel.h"
#include "report/CCopasiRootContainer.h"
#include "ModelWidget.h"
#include "listviews.h"
#include "model/CReactionInterface.h"
#include "model/CModel.h"
#include "report/CKeyFactory.h"
#include "qtUtilities.h"
#include "xml/CCopasiXMLInterface.h"

/*
 *  Constructs a ModelWidget which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
ModelWidget::ModelWidget(QWidget* parent, const char* name, Qt::WFlags fl)
    : CopasiWidget(parent, name, fl)
{
  if (!name)
    setName("ModelWidget");

  setCaption(trUtf8("ModelWidget"));
  ModelWidgetLayout = new Q3GridLayout(this, 1, 1, 11, 4, "ModelWidgetLayout");

  TextLabel1 = new QLabel(this, "TextLabel1");
  TextLabel1->setText(trUtf8("Model name"));
  TextLabel1->setAlignment(int(Qt::AlignVCenter
                               | Qt::AlignRight));
  ModelWidgetLayout->addWidget(TextLabel1, 0, 0);
  LineEdit = new QLineEdit(this, "LineEdit");
  ModelWidgetLayout->addMultiCellWidget(LineEdit, 0, 0, 1, 3);

  QSpacerItem* spacer2 = new QSpacerItem(0, 6, QSizePolicy::Minimum, QSizePolicy::Fixed);
  ModelWidgetLayout->addItem(spacer2, 1, 1);

  TextLabel2 = new QLabel(this, "TextLabel2");
  TextLabel2->setText(trUtf8("Time unit"));
  TextLabel2->setAlignment(int(Qt::AlignVCenter
                               | Qt::AlignRight));
  ModelWidgetLayout->addWidget(TextLabel2, 2, 0);
  ComboBox1 = new QComboBox(false, this, "ComboBox1");
  ComboBox1->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, ComboBox1->sizePolicy().hasHeightForWidth()));
  ModelWidgetLayout->addWidget(ComboBox1, 2, 1);

  //Volume Units
  TextLabel3 = new QLabel(this, "TextLabel3");
  TextLabel3->setText(trUtf8("Volume unit"));
  TextLabel3->setAlignment(int(Qt::AlignVCenter
                               | Qt::AlignRight));
  ModelWidgetLayout->addWidget(TextLabel3, 3, 0);
  ComboBox2 = new QComboBox(false, this, "ComboBox2");
  ComboBox2->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, ComboBox2->sizePolicy().hasHeightForWidth()));
  ModelWidgetLayout->addWidget(ComboBox2, 3, 1);

#ifdef COPASI_EXTUNIT
  //Area units
  TextLabelArea = new QLabel(this, "TextLabelArea");
  TextLabelArea->setText(trUtf8("Area unit"));
  TextLabelArea->setAlignment(int(Qt::AlignVCenter
                                  | Qt::AlignRight));
  ModelWidgetLayout->addWidget(TextLabelArea, 4, 0);
  ComboBoxArea = new QComboBox(false, this, "ComboBoxArea");
  ComboBoxArea->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, ComboBoxArea->sizePolicy().hasHeightForWidth()));
  ModelWidgetLayout->addWidget(ComboBoxArea, 4, 1);

  //Length units
  TextLabelLength = new QLabel(this, "TextLabelLength");
  TextLabelLength->setText(trUtf8("Length unit"));
  TextLabelLength->setAlignment(int(Qt::AlignVCenter
                                    | Qt::AlignRight));
  ModelWidgetLayout->addWidget(TextLabelLength, 5, 0);
  ComboBoxLength = new QComboBox(false, this, "ComboBoxLength");
  ComboBoxLength->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, ComboBoxLength->sizePolicy().hasHeightForWidth()));
  ModelWidgetLayout->addWidget(ComboBoxLength, 5, 1);
#endif

  //Quantity units
  TextLabel4 = new QLabel(this, "TextLabel4");
  TextLabel4->setText(trUtf8("Quantity unit"));
  TextLabel4->setAlignment(int(Qt::AlignVCenter
                               | Qt::AlignRight));
  ModelWidgetLayout->addWidget(TextLabel4, 6, 0);
  ComboBox3 = new QComboBox(false, this, "ComboBox3");
  ComboBox3->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, ComboBox3->sizePolicy().hasHeightForWidth()));
  ModelWidgetLayout->addWidget(ComboBox3, 6, 1);

  mpLblModelType = new QLabel(this, "mpLblModelType");
  mpLblModelType->setText(trUtf8("Rate Law Interpretation"));
  mpLblModelType->setAlignment(int(Qt::AlignVCenter
                                   | Qt::AlignRight));
  ModelWidgetLayout->addWidget(mpLblModelType, 2, 2);
  mpBoxModelType = new QComboBox(false, this, "mpBoxModelType");
  mpBoxModelType->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)3, (QSizePolicy::SizeType)0, 0, 0, mpBoxModelType->sizePolicy().hasHeightForWidth()));
  ModelWidgetLayout->addWidget(mpBoxModelType, 2, 3);

  mpLblInitial = new QLabel(this, "mpLblInitial");
  mpLblInitial->setText("Initial");
  mpLblInitial->setAlignment(int(Qt::AlignVCenter | Qt::AlignLeft));
  ModelWidgetLayout->addWidget(mpLblInitial, 7, 1);

  mpLblCurrent = new QLabel(this, "mpLblCurrent");
  mpLblCurrent->setText("Current");
  mpLblCurrent->setAlignment(int(Qt::AlignVCenter | Qt::AlignLeft));
  ModelWidgetLayout->addWidget(mpLblCurrent, 7, 2);

  mpLblTime = new QLabel(this, "mpLblTime");
  mpLblTime->setText("Time (s)");
  mpLblTime->setAlignment(int(Qt::AlignVCenter | Qt::AlignRight));
  ModelWidgetLayout->addWidget(mpLblTime, 8, 0);

  mpInitialTime = new QLineEdit(this, "mpInitialTime");
  ModelWidgetLayout->addWidget(mpInitialTime, 8, 1);

  mpCurrentTime = new QLineEdit(this, "mpCurrentTime");
  mpCurrentTime->setEnabled(false);
  ModelWidgetLayout->addMultiCellWidget(mpCurrentTime, 8, 8, 2, 3);

  Q3Frame * Line1 = new Q3Frame(this, "Line1");
  Line1->setFrameShape(Q3Frame::HLine);
  ModelWidgetLayout->addMultiCellWidget(Line1, 9, 9, 0, 3);

  // textBrowser = new QTextBrowser (this, "Text Browser");
  // ModelWidgetLayout->addMultiCellWidget(textBrowser, 8, 8, 1, 2);

  mpEditComment = new Q3TextEdit(this, "Edit Comment");
  mpEditComment->setTextFormat(Qt::RichText);
  ModelWidgetLayout->addMultiCellWidget(mpEditComment, 10, 10, 1, 3);
  mpEditComment->setText("");
  // editComments->hide();

  mpToggleMarkup = new QPushButton(this, "ToggleMarkup");
  mpToggleMarkup->setText(trUtf8("Show Markup"));

  //********* buttons ***********

  showMarkupLayout = new Q3VBoxLayout(0, 0, 6, "MarkupLayout");
  spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Minimum);
  showMarkupLayout->addWidget(mpToggleMarkup);
  showMarkupLayout->addItem(spacer);
  ModelWidgetLayout->addLayout(showMarkupLayout, 10, 0);

  Layout5 = new Q3HBoxLayout(0, 0, 6, "Layout5");

  commitChanges = new QPushButton(this, "commitChanges");
  commitChanges->setText(trUtf8("Commit"));
  Layout5->addWidget(commitChanges);

  cancelChanges = new QPushButton(this, "cancelChanges");
  cancelChanges->setText(trUtf8("Revert"));
  Layout5->addWidget(cancelChanges);

  // preliminary

  ModelWidgetLayout->addMultiCellLayout(Layout5, 12, 12, 0, 3);

  // signals and slots connections
  connect(mpToggleMarkup, SIGNAL(clicked()), this, SLOT(toggleEditorBox()));
  connect(commitChanges, SIGNAL(clicked()), this, SLOT(slotBtnOKClicked()));
  connect(cancelChanges, SIGNAL(clicked()), this, SLOT(slotBtnCancelClicked()));
}

void ModelWidget::toggleEditorBox()
{
  QString Comment = mpEditComment->text();

  if (mpToggleMarkup->text() == "Show Markup")
    {
      mpToggleMarkup->setText("Hide Markup");
      mpEditComment->setTextFormat(Qt::PlainText);
    }
  else
    {
      mpToggleMarkup->setText("Show Markup");
      mpEditComment->setTextFormat(Qt::RichText);
    }

  mpEditComment->setText(Comment);
}

/*
 *  Destroys the object and frees any allocated resources
 */
ModelWidget::~ModelWidget()
{
  // no need to delete child widgets, Qt does it all for us
}

/*This function is to load the model for the compartments*/
bool ModelWidget::loadModel(CModel *model)
{
  bool ret = true;

  LineEdit->setText(FROM_UTF8(model->getObjectName()));
  // textBrowser->setText(FROM_UTF8(model->getComments()));

  if (mpToggleMarkup->text() != "Show Markup")
    toggleEditorBox();

  mpEditComment->setText(FROM_UTF8(model->getComments()));
  // We save the current comment so that we can see whether
  // it has changed.
  mOldComment = mpEditComment->text();

  // textBrowser->setReadOnly(false);
  ComboBox1->clear();
  ComboBox2->clear();
  ComboBox3->clear();
  mpBoxModelType->clear();
  QStringList comboEntries;

  unsigned int temp1;

  for (temp1 = 0; model->TimeUnitNames[temp1] /*!= ""*/; temp1++)
    {
      comboEntries.push_front(QString::fromUtf8(model->TimeUnitNames[temp1]));
    }

  ComboBox1->insertStringList(comboEntries, -1);
  ComboBox1->setCurrentText(FROM_UTF8(model->getTimeUnitName()));

  mpLblTime->setText("Time (" + ComboBox1->currentText() + ")");

  QStringList comboEntries1;

  for (temp1 = 0; CModel::VolumeUnitNames[temp1]  /*!= ""*/; temp1++)
    {
      comboEntries1.push_front(QString::fromUtf8(CModel::VolumeUnitNames[temp1]));
    }

  ComboBox2->insertStringList(comboEntries1, -1);
  ComboBox2->setCurrentText(FROM_UTF8(model->getVolumeUnitName()));

  QStringList comboEntries2;

  for (temp1 = 0; CModel::QuantityUnitNames[temp1] /*!= ""*/; temp1++)
    {
      comboEntries2.push_front(QString::fromUtf8(CModel::QuantityUnitNames[temp1]));
    }

  ComboBox3->insertStringList(comboEntries2, -1);
  ComboBox3->setCurrentText(FROM_UTF8(model->getQuantityUnitName()));

#ifdef COPASI_EXTUNIT
  QStringList comboEntriesArea;

  for (temp1 = 0; CModel::AreaUnitNames[temp1]  /*!= ""*/; temp1++)
    {
      comboEntriesArea.push_front(QString::fromUtf8(CModel::AreaUnitNames[temp1]));
    }

  ComboBoxArea->clear();
  ComboBoxArea->insertStringList(comboEntriesArea, -1);
  ComboBoxArea->setCurrentText(FROM_UTF8(model->getAreaUnitName()));

  QStringList comboEntriesLength;

  for (temp1 = 0; CModel::LengthUnitNames[temp1]  /*!= ""*/; temp1++)
    {
      comboEntriesLength.push_front(QString::fromUtf8(CModel::LengthUnitNames[temp1]));
    }

  ComboBoxLength->clear();
  ComboBoxLength->insertStringList(comboEntriesLength, -1);
  ComboBoxLength->setCurrentText(FROM_UTF8(model->getLengthUnitName()));
#endif

  QStringList ModelTypes;

  for (temp1 = 0; CModel::ModelTypeNames[temp1] /*!= ""*/; temp1++)
    {
      ModelTypes.push_back(QString::fromUtf8(CModel::ModelTypeNames[temp1]));
    }

  mpBoxModelType->insertStringList(ModelTypes, -1);
  mpBoxModelType->setCurrentItem(model->getModelType());

  mpInitialTime->setText(QString::number(model->getInitialTime()));
  mpInitialTime->setReadOnly(model->isAutonomous());
  mpCurrentTime->setText(QString::number(model->getTime()));

  return ret;
}

bool ModelWidget::saveToModel()
{
  CModel* model = dynamic_cast< CModel * >(CCopasiRootContainer::getKeyFactory()->get(mKey));

  if (!model) return false;

  bool changed = false;

  bool success = true;

  if (TO_UTF8(LineEdit->text()) != model->getObjectName())
    {
      model->setTitle(TO_UTF8(LineEdit->text()));
      changed = true;
    }

  if (mOldComment != mpEditComment->text())
    {
      std::string Richtext = TO_UTF8(mpEditComment->text());
      std::string::size_type pos = 0;

      // We do not need a html document we need only the xhtml element.
      if (Richtext.find("<!DOCTYPE", 0) != std::string::npos)
        {
          pos = Richtext.find('>', 0);
          Richtext.erase(0, pos + 1);
        }

      // remove leading white spaces
      pos = Richtext.find_first_not_of("\x0a\x0d\t ");

      if (pos != 0) Richtext.erase(0, pos);

      // remove trailing white space
      pos = Richtext.find_last_not_of("\x0a\x0d\t ");

      if (pos < Richtext.length())
        Richtext = Richtext.substr(0, pos + 1);

      // Fix <hr> to <hr /> to have proper xhtml.
      pos = 0;

      while ((pos = Richtext.find("<hr>", pos)) != std::string::npos)
        {
          pos += 3;
          Richtext.insert(pos, " /");
        }

      std::ostringstream xhtml;

      if (Richtext == "")
        {
          xhtml << "<body xmlns=\"http://www.w3.org/1999/xhtml\" />";
        }
      else if (Richtext[0] == '<')
        {
          std::string::size_type pos = Richtext.find('>');
          std::string FirstElement = Richtext.substr(0, pos);

          if (FirstElement.find("xmlns=\"http://www.w3.org/1999/xhtml\"") == std::string::npos)
            FirstElement += " xmlns=\"http://www.w3.org/1999/xhtml\"";

          xhtml << FirstElement << Richtext.substr(pos);
        }
      else
        {
          xhtml << "<body xmlns=\"http://www.w3.org/1999/xhtml\">";
          xhtml << CCopasiXMLInterface::encode(Richtext);
          xhtml << "</body>";
        }

      model->setComments(xhtml.str());

      changed = true;
    }

  if (TO_UTF8(ComboBox1->currentText()) != model->getTimeUnitName())
    {
      model->setTimeUnit(TO_UTF8(ComboBox1->currentText()));
      changed = true;
    }

  if (TO_UTF8(ComboBox2->currentText()) != model->getVolumeUnitName())
    {
      model->setVolumeUnit(TO_UTF8(ComboBox2->currentText()));
      changed = true;
    }

#ifdef COPASI_EXTUNIT

  if (TO_UTF8(ComboBoxArea->currentText()) != model->getAreaUnitName())
    {
      model->setAreaUnit(TO_UTF8(ComboBoxArea->currentText()));
      changed = true;
    }

  if (TO_UTF8(ComboBoxLength->currentText()) != model->getLengthUnitName())
    {
      model->setLengthUnit(TO_UTF8(ComboBoxLength->currentText()));
      changed = true;
    }

#endif

  if (TO_UTF8(ComboBox3->currentText()) != model->getQuantityUnitName())
    {
      model->setQuantityUnit(TO_UTF8(ComboBox3->currentText()));
      changed = true;
    }

  if ((CModel::ModelType) mpBoxModelType->currentItem() != model->getModelType())
    {
      model->setModelType((CModel::ModelType) mpBoxModelType->currentItem());
      changed = true;
    }

  if (mpInitialTime->text() != QString::number(model->getInitialTime()))
    {
      model->setInitialTime(mpInitialTime->text().toDouble());
      changed = true;
    }

  if (changed)
    {
      protectedNotify(ListViews::MODEL, ListViews::CHANGE, mKey);
      assert(CCopasiRootContainer::getDatamodelList()->size() > 0);
      (*CCopasiRootContainer::getDatamodelList())[0]->changed(true);
    }

  return success;
}

void ModelWidget::slotBtnCancelClicked()
{
  //TOD: let the user confirm
  enter(mKey); // reload
}

void ModelWidget::slotBtnOKClicked()
{
  //let the user confirm?
  saveToModel();
  loadModel(dynamic_cast< CModel * >(CCopasiRootContainer::getKeyFactory()->get(mKey)));
}

bool ModelWidget::update(ListViews::ObjectType objectType,
                         ListViews::Action C_UNUSED(action), const std::string & key)
{
  if (mIgnoreUpdates) return true;

  switch (objectType)
    {
      case ListViews::MODEL:
        enter(key);
        break;

      default:
        break;
    }

  return true;
}

bool ModelWidget::leave()
{
  //let the user confirm?
  return saveToModel();
}

bool ModelWidget::enterProtected()
{
  CModel* model = dynamic_cast< CModel * >(mpObject);

  if (model) return loadModel(model);
  else return false;
}

// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/wizard/Attic/wizard.ui.h,v $
//   $Revision: 1.17 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2009/01/08 16:07:11 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/****************************************************************************
 ** ui.h extension file, included from the uic-generated form implementation.
 **
 ** If you want to add, delete, or rename functions or slots, use
 ** Qt Designer to update this file, preserving your code.
 **
 ** You should not define a constructor or destructor in this file.
 ** Instead, write your code in functions called init() and destroy().
 ** These will automatically be called by the form's constructor and
 ** destructor.
 *****************************************************************************/

#include "commandline/COptions.h"
#include "UI/CQMessageBox.h"
const char* WizardDialog::texts[6] = {"TutWiz-Step1.html", "TutWiz-Step2.html", "TutWiz-Step3.html", "TutWiz-Step4.html", "TutWiz-Step5.html", "TutWiz-Step6.html"};

const char* WizardDialog::Error = "<html><head><title>Error</title></head><body><h1>Could not find help files.<br><br>Please set the COPASI_HELP_PATH environment variable to the absolute path that contains the html help files!</h1></body></html>";

QString WizardDialog::helpPath = "";

QWidget* copasiMainWindow = NULL;

void WizardDialog::buttonGroup2_clicked(int)
{
  int selectedId = this->buttonGroup->selectedId();
  if (selectedId == 0)
    {
      this->backButton->setEnabled(false);
      this->forwardButton->setEnabled(true);
    }
  else if (selectedId == 5)
    {
      this->forwardButton->setEnabled(false);
      this->backButton->setEnabled(true);
    }
  else
    {
      this->forwardButton->setEnabled(true);
      this->backButton->setEnabled(true);
    }
  if (WizardDialog::helpPath != "")
    {
      QString source = WizardDialog::helpPath + "/" + WizardDialog::texts[this->buttonGroup->selectedId()];
      this->textBrowser->setSource(source);
    }
}

void WizardDialog::init()
{
  Qt::WindowFlags f = this->windowFlags();
  f = (f | Qt::WStyle_Minimize | Qt::WDestructiveClose);
  this->setWindowFlags(f);

  std::string WizardDir;
  COptions::getValue("WizardDir", WizardDir);

  if (!WizardDir.empty())
    {
      // the next line will hopefully ensure that this works under windows as well.
      WizardDialog::helpPath = QDir(WizardDir.c_str()).absolutePath();
      QString source = WizardDialog::helpPath + "/" + WizardDialog::texts[0];
      this->textBrowser->setSource(source);
    }
  else
    {
      this->textBrowser->setText(WizardDialog::Error);;
    }

  this->button1->setStyle("motif");
  this->button2->setStyle("motif");
  this->button3->setStyle("motif");
  this->button4->setStyle("motif");
  this->button5->setStyle("motif");
  this->button6->setStyle("motif");
  this->button1->setBackgroundOrigin(QWidget::WindowOrigin);
  this->button2->setBackgroundOrigin(QWidget::WindowOrigin);
  this->button3->setBackgroundOrigin(QWidget::WindowOrigin);
  this->button4->setBackgroundOrigin(QWidget::WindowOrigin);
  this->button5->setBackgroundOrigin(QWidget::WindowOrigin);
  this->button6->setBackgroundOrigin(QWidget::WindowOrigin);
}

void WizardDialog::forwardButton_clicked()
{
  this->buttonGroup->setButton(this->buttonGroup->selectedId() + 1);
  this->buttonGroup2_clicked(this->buttonGroup->selectedId());
}

void WizardDialog::backButton_clicked()
{
  this->buttonGroup->setButton(this->buttonGroup->selectedId() - 1);
  this->buttonGroup2_clicked(this->buttonGroup->selectedId());
}

void WizardDialog::textBrowser_anchorClicked(const QString &name, const QString & C_UNUSED(link))
{
  if (name == "Model Settings")
    {
      ListViews::switchAllListViewsToWidget(1, "");
    }
  else if (name == "Reaction Overview")
    {
      ListViews::switchAllListViewsToWidget(114, "");
    }
  else if (name == "Report Definition")
    {
      ListViews::switchAllListViewsToWidget(43, "");
    }
  else if (name == "Step 1")
    {
      this->buttonGroup->setButton(0);
      this->buttonGroup2_clicked(this->buttonGroup->selectedId());
    }
  else if (name == "Time Course")
    {
      ListViews::switchAllListViewsToWidget(23, "");
    }
  else if (name == "Plot Definition Overview")
    {
      ListViews::switchAllListViewsToWidget(42, "");
    }
  else
    {
      CQMessageBox::information(this, "Error", "Link not found",
                                QMessageBox::Ok | QMessageBox::Default, Qt::NoButton);
    }
}

void WizardDialog::setCopasiMainWindow(QWidget * copasiMainWindow)
{
  this->copasiMainWindow = copasiMainWindow;
}

/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/Attic/CScanWidgetBreak.cpp,v $
   $Revision: 1.3 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2006/03/28 23:09:04 $
   End CVS Header */

/****************************************************************************
 ** Form implementation generated from reading ui file 'CScanWidgetBreak.ui'
 **
 ** Created: Tue Mar 28 14:23:46 2006
 **      by: The User Interface Compiler ($Id: CScanWidgetBreak.cpp,v 1.3 2006/03/28 23:09:04 shoops Exp $)
 **
 ** WARNING! All changes made in this file will be lost!
 ****************************************************************************/

#include "CScanWidgetBreak.h"

#include <qvariant.h>
 #include <qframe.h>
 #include <qcombobox.h>
 #include <qlabel.h>
 #include <qlayout.h>
 #include <qtooltip.h>
 #include <qwhatsthis.h>
 #include "CScanWidgetBreak.ui.h"

/*
 *  Constructs a CScanWidgetBreak as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
CScanWidgetBreak::CScanWidgetBreak(QWidget* parent, const char* name, WFlags fl)
    : QWidget(parent, name, fl)
{
  if (!name)
    setName("CScanWidgetBreak");
  CScanWidgetBreakLayout = new QVBoxLayout(this, 5, 6, "CScanWidgetBreakLayout");

  frame = new QFrame(this, "frame");
  frame->setPaletteBackgroundColor(QColor(253, 255, 197));
  frame->setFrameShape(QFrame::StyledPanel);
  frame->setFrameShadow(QFrame::Raised);
  frameLayout = new QGridLayout(frame, 1, 1, 6, 6, "frameLayout");

  comboBoxReport = new QComboBox(FALSE, frame, "comboBoxReport");
  comboBoxReport->setEnabled(FALSE);
  comboBoxReport->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, comboBoxReport->sizePolicy().hasHeightForWidth()));

  frameLayout->addWidget(comboBoxReport, 0, 3);

  textLabelReport = new QLabel(frame, "textLabelReport");
  textLabelReport->setEnabled(FALSE);
  textLabelReport->setAlignment(int(QLabel::AlignVCenter | QLabel::AlignRight));

  frameLayout->addWidget(textLabelReport, 0, 2);

  comboBoxPlot = new QComboBox(FALSE, frame, "comboBoxPlot");
  comboBoxPlot->setEnabled(FALSE);
  comboBoxPlot->setSizePolicy(QSizePolicy((QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, comboBoxPlot->sizePolicy().hasHeightForWidth()));

  frameLayout->addWidget(comboBoxPlot, 1, 3);

  labelTitle = new QLabel(frame, "labelTitle");

  frameLayout->addMultiCellWidget(labelTitle, 0, 1, 0, 0);

  textLabelPlot = new QLabel(frame, "textLabelPlot");
  textLabelPlot->setEnabled(FALSE);
  textLabelPlot->setAlignment(int(QLabel::AlignVCenter | QLabel::AlignRight));

  frameLayout->addWidget(textLabelPlot, 1, 2);
  spacer1 = new QSpacerItem(30, 20, QSizePolicy::Maximum, QSizePolicy::Minimum);
  frameLayout->addMultiCell(spacer1, 0, 1, 1, 1);
  CScanWidgetBreakLayout->addWidget(frame);
  languageChange();
  resize(QSize(420, 74).expandedTo(minimumSizeHint()));
  clearWState(WState_Polished);
  init();
}

/*
 *  Destroys the object and frees any allocated resources
 */
CScanWidgetBreak::~CScanWidgetBreak()
{
  // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void CScanWidgetBreak::languageChange()
{
  setCaption(tr("Form1"));
  comboBoxReport->clear();
  comboBoxReport->insertItem(tr("empty line"));
  comboBoxReport->insertItem(tr("page break"));
  comboBoxReport->insertItem(tr("new file"));
  comboBoxReport->insertItem(tr("no break"));
  textLabelReport->setText(tr("for reports:"));
  comboBoxPlot->clear();
  comboBoxPlot->insertItem(tr("new curve"));
  comboBoxPlot->insertItem(tr("new plot window"));
  comboBoxPlot->insertItem(tr("no break"));
  labelTitle->setText(tr("<h2>Output separator</h2>"));
  textLabelPlot->setText(tr("for plots:"));
}

/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/plot/Attic/HistoWidget.ui.h,v $
   $Revision: 1.5 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2006/04/15 16:23:30 $
   End CVS Header */

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

#include "report/CCopasiContainer.h"
#include "CopasiUI/CCopasiSelectionDialog.h"
#include "CopasiUI/qtUtilities.h"

void HistoWidget::buttonPressedX()
{
  if (!mpModel) return;
  CCopasiSelectionDialog* browseDialog = new CCopasiSelectionDialog(this);
  browseDialog->setModel(mpModel);
  browseDialog->setSingleSelection(true);
  std::vector<CCopasiObject*>* selection = new std::vector<CCopasiObject*>();
  if (mpObjectX)
    selection->push_back(mpObjectX);
  browseDialog->setOutputVector(selection);

  if (browseDialog->exec() == QDialog::Accepted && selection->size() != 0)
    {
      if (mpObjectX == selection->at(0)) return; //nothing to be done
      mpObjectX = selection->at(0);

      if (mpObjectX)
        {
          lineEditXName->setText(FROM_UTF8(mpObjectX->getObjectDisplayName()));
          lineEditTitle->setText("Histogram: " + FROM_UTF8(mpObjectX->getObjectDisplayName()));
        }
      else
        {
          lineEditXName->setText("");
          lineEditTitle->setText("Histogram");
        }

      //TODO update tab title
    }
}

bool HistoWidget::LoadFromCurveSpec(const CPlotItem * curve)
{
  if (!curve) return false;

  if (curve->getType() != CPlotItem::histoItem1d) return false;
  if (curve->getChannels().size() != 1) return false;

  //title
  lineEditTitle->setText(FROM_UTF8(curve->getTitle()));

  //variable
  mpObjectX = NULL;

  if (curve->getChannels().size() >= 1)
    mpObjectX = CCopasiContainer::ObjectFromName(curve->getChannels()[0]);

  if (mpObjectX)
    lineEditXName->setText(FROM_UTF8(mpObjectX->getObjectDisplayName()));

  //other parameters:
  const void* tmp;
  if (!(tmp = curve->getValue("increment").pVOID)) return false;
  lineEditInc->setText(QString::number(*(const C_FLOAT64*)tmp));

  mpCheckBefore->setChecked(curve->getActivity() & CPlotItem::BEFORE);
  mpCheckDuring->setChecked(curve->getActivity() & CPlotItem::DURING);
  mpCheckAfter->setChecked(curve->getActivity() & CPlotItem::AFTER);

  return true; //TODO
}

bool HistoWidget::SaveToCurveSpec(CPlotItem * curve) const
  {
    //title
    curve->setTitle((const char*)lineEditTitle->text().utf8());

    //channels
    curve->getChannels().clear();
    curve->getChannels().push_back(CPlotDataChannelSpec(mpObjectX ? mpObjectX->getCN() : CCopasiObjectName("")));

    //other parameters: TODO
    curve->setValue("increment", lineEditInc->text().toDouble());

    C_INT32 Activity = 0;
    if (mpCheckBefore->isChecked()) Activity += CPlotItem::BEFORE;
    if (mpCheckDuring->isChecked()) Activity += CPlotItem::DURING;
    if (mpCheckAfter->isChecked()) Activity += CPlotItem::AFTER;
    curve->setActivity((CPlotItem::RecordingActivity) Activity);

    return true;
  }

void HistoWidget::setModel(const CModel * model)
{
  mpModel = model;
}

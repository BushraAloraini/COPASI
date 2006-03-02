/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/Attic/CTimeSeriesTable.cpp,v $
   $Revision: 1.5 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2006/03/02 02:21:43 $
   End CVS Header */

#include "CTimeSeriesTable.h"
#include "qtUtilities.h"

void CTimeSeriesTable::setTimeSeries(const CTimeSeries & ts)
{
  mTS = &ts;
  setNumRows(mTS->getNumSteps());
  setNumCols(mTS->getNumVariables());

  QHeader *tableHeader = horizontalHeader();
  C_INT32 i, imax = mTS->getNumVariables();
  for (i = 0; i < imax; ++i)
    tableHeader->setLabel(i, FROM_UTF8(mTS->getTitle(i)));

  // :TODO: save old current cell and set afterwards
}

QString CTimeSeriesTable::textForCell(unsigned int row, unsigned int col)
{
  if (row > mTS->getNumSteps()) return "";
  if (col > mTS->getNumVariables()) return "";

  if (mFlagConc)
    return QString::number(mTS->getConcentrationData(row, col));
  else
    return QString::number(mTS->getData(row, col));
}

void CTimeSeriesTable::showConcentrations(bool conc)
{
  mFlagConc = conc;
  hide(); show(); //TODO: ugly!
}

const CTimeSeries* CTimeSeriesTable::getTimeSeries()
{
  return mTS;
}

bool CTimeSeriesTable::doShowConcentrations() const
  {
    return this->mFlagConc;
  }

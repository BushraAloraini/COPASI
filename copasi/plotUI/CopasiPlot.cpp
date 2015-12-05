// Copyright (C) 2010 - 2015 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 - 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2003 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#include <QtCore/QString>
#include <QtGui/QColor>   //might need to go to the header file
#include <QtGui/QCursor>
#include <QtCore/QMutexLocker>

#include <qwt_symbol.h>
#include <qwt_legend.h>
#if QWT_VERSION > 0x060000
#include <qwt_legend_data.h>
#include <qwt_legend_label.h>
#include <qwt_plot_canvas.h>
#else
#include <qwt_legend_item.h>
#endif

#include <qwt_scale_engine.h>
#include <qwt_color_map.h>
#include <qwt_scale_widget.h>

#include <limits>
#include <algorithm>
#include <cmath>

#include "scrollzoomer.h"

#include "copasi.h"
#include "CopasiPlot.h"
#include "CQPlotColors.h"
#include "plot/CPlotSpecification.h"
#include "UI/qtUtilities.h"
#include "report/CCopasiRootContainer.h"
#include "model/CModel.h"
#include "commandline/CLocaleString.h"


#include <QApplication>

#define ActivitySize 8

//********************  data  *********************************************
C2DCurveData::C2DCurveData():
#if QWT_VERSION > 0x060000
  QwtSeriesData<QPointF>(),
#else
  QwtData(),
#endif

  mpX(NULL),
  mpY(NULL),
  mSize(0),
  mMaxSize(0),
  mLastRectangle(0),
  mMinX(std::numeric_limits<double>::quiet_NaN()),
  mMaxX(std::numeric_limits<double>::quiet_NaN()),
  mMinY(std::numeric_limits<double>::quiet_NaN()),
  mMaxY(std::numeric_limits<double>::quiet_NaN())
{}

C2DCurveData::C2DCurveData(const CVector< C_FLOAT64 > & x, const CVector< C_FLOAT64 > & y, size_t size):
#if QWT_VERSION > 0x060000
  QwtSeriesData<QPointF>(),
#else
  QwtData(),
#endif
  mpX(x.array()),
  mpY(y.array()),
  mSize(size),
  mMaxSize(x.size()),
  mLastRectangle(0),
  mMinX(std::numeric_limits<double>::quiet_NaN()),
  mMaxX(std::numeric_limits<double>::quiet_NaN()),
  mMinY(std::numeric_limits<double>::quiet_NaN()),
  mMaxY(std::numeric_limits<double>::quiet_NaN())
{
  assert(x.size() == y.size());
  assert(mSize <= mMaxSize);
}

C2DCurveData::~C2DCurveData()
{}

#if QWT_VERSION > 0x060000
QwtSeriesData<QPointF> * C2DCurveData::copy() const
{
  C2DCurveData * pCopy = new C2DCurveData();

  *pCopy = *this;

  return pCopy;
}

QPointF C2DCurveData::sample(size_t i) const
{
  return QPointF(x(i), y(i));
}

#else
QwtData * C2DCurveData::copy() const
{
  C2DCurveData * pCopy = new C2DCurveData();

  *pCopy = *this;

  return pCopy;
}
#endif

size_t C2DCurveData::size() const
{
  return mSize;
}

double C2DCurveData::x(size_t i) const
{
  return *(mpX + i);
}

double C2DCurveData::y(size_t i) const
{
  return *(mpY + i);
}

QwtDoubleRect C2DCurveData::boundingRect() const
{
  if (mSize <= 0)
    return QwtDoubleRect(1.0, 1.0, -2.0, -2.0); // invalid

  if (mLastRectangle == mSize)
    return QwtDoubleRect(mMinX, mMinY, mMaxX - mMinX, mMaxY - mMinY);

  const double *xIt = mpX + mLastRectangle;
  const double *yIt = mpY + mLastRectangle;
  const double *end = mpX + mSize;

  mLastRectangle = mSize;

  // We have to remember whether we have an initial NaN
  bool MinXisNaN = isnan(mMinX);
  bool MaxXisNaN = isnan(mMaxX);
  bool MinYisNaN = isnan(mMinY);
  bool MaxYisNaN = isnan(mMaxY);

  while (xIt < end)
    {
      const double xv = *xIt++;

      if (!isnan(xv))
        {
          if ((xv < mMinX || MinXisNaN) && xv > -std::numeric_limits< double >::infinity())
            {
              mMinX = xv;
              MinXisNaN = false;
            }

          if ((xv > mMaxX || MaxXisNaN) && xv < std::numeric_limits< double >::infinity())
            {
              mMaxX = xv;
              MaxXisNaN = false;
            }
        }

      const double yv = *yIt++;

      if (!isnan(yv))
        {
          if ((yv < mMinY || MinYisNaN) && yv > -std::numeric_limits< double >::infinity())
            {
              mMinY = yv;
              MinYisNaN = false;
            }

          if ((yv > mMaxY  || MaxYisNaN) && yv < std::numeric_limits< double >::infinity())
            {
              mMaxY = yv;
              MaxYisNaN = false;
            }
        }
    }

  if (isnan(mMinX + mMaxX + mMinY + mMaxY))
    return QwtDoubleRect(1.0, 1.0, -2.0, -2.0); // invalid

  // We need to avoid very small data ranges (absolute and relative)
  C_FLOAT64 minRange = fabs(mMinX + mMaxX) * 5.e-5 + std::numeric_limits< C_FLOAT64 >::min() * 100.0;

  if (mMaxX - mMinX < minRange)
    {
      mMinX = mMinX - minRange * 0.5;
      mMaxX = mMaxX + minRange * 0.5;
    }

  minRange = fabs(mMinY + mMaxY) * 5e-5 + std::numeric_limits< C_FLOAT64 >::min() * 100.0;

  if (mMaxY - mMinY < minRange)
    {
      mMinY = mMinY - minRange * 0.5;
      mMaxY = mMaxY + minRange * 0.5;
    }

  return QwtDoubleRect(mMinX, mMinY, mMaxX - mMinX, mMaxY - mMinY);
}

void C2DCurveData::setSize(const size_t & size)
{
  mSize = size;
  assert(mSize <= mMaxSize);
}

void C2DCurveData::reallocated(const CVector< double > * pX, const CVector< double > *pY)
{
  mpX = pX->array();
  mpY = pY->array();
  mMaxSize = pX->size();

  assert(mSize <= mMaxSize);
}

C2DCurveData & C2DCurveData::operator = (const C2DCurveData & rhs)
{
  mpX = rhs.mpX;
  mpY = rhs.mpY;
  mSize = rhs.mSize;
  mMaxSize = rhs.mMaxSize;
  mLastRectangle = rhs.mLastRectangle;
  mMinX = rhs.mMinX;
  mMaxX = rhs.mMaxX;
  mMinY = rhs.mMinY;
  mMaxY = rhs.mMaxY;

  return * this;
}

//********************  CBandedGraphData  *********************************
CBandedGraphData::CBandedGraphData():
#if QWT_VERSION > 0x060000
  QwtSeriesData<QPointF>(),
#else
  QwtData(),
#endif
  mpX(NULL),
  mpY1(NULL),
  mpY2(NULL),
  mSize(0),
  mMaxSize(0),
  mLastRectangle(0),
  mMinX(std::numeric_limits<double>::quiet_NaN()),
  mMaxX(std::numeric_limits<double>::quiet_NaN()),
  mMinY(std::numeric_limits<double>::quiet_NaN()),
  mMaxY(std::numeric_limits<double>::quiet_NaN())
{}

CBandedGraphData::CBandedGraphData(const CVector< double > & x,
                                   const CVector< double > & y1,
                                   const CVector< double > & y2,
                                   size_t size):
#if QWT_VERSION > 0x060000
  QwtSeriesData<QPointF>(),
#else
  QwtData(),
#endif
  mpX(x.array()),
  mpY1(y1.array()),
  mpY2(y2.array()),
  mSize(size),
  mMaxSize(x.size()),
  mLastRectangle(0),
  mMinX(std::numeric_limits<double>::quiet_NaN()),
  mMaxX(std::numeric_limits<double>::quiet_NaN()),
  mMinY(std::numeric_limits<double>::quiet_NaN()),
  mMaxY(std::numeric_limits<double>::quiet_NaN())
{
  assert(x.size() == y1.size());
  assert(x.size() == y2.size());
  assert(mSize <= mMaxSize);
}

CBandedGraphData::~CBandedGraphData()
{}

#if QWT_VERSION > 0x060000
QwtSeriesData<QPointF> *
CBandedGraphData::copy() const
{
  CBandedGraphData * pCopy = new CBandedGraphData();

  *pCopy = *this;

  return pCopy;
}

QPointF CBandedGraphData::sample(size_t i) const
{
  return QPointF(x(i), y(i));
}

#else
QwtData *
CBandedGraphData::copy() const
{
  CBandedGraphData * pCopy = new CBandedGraphData();

  *pCopy = *this;

  return pCopy;
}
#endif

size_t
CBandedGraphData::size() const
{return 2 * mSize;}

double CBandedGraphData::x(size_t i) const
{
  // to make a polygon out of the two data curves
  // one curve goes in order, the other backwards
  if (i >= mSize)
    i = (2 * mSize - i - 1);

  return *(mpX + i);
}

double CBandedGraphData::y(size_t i) const
{
  double ret;

  // to make a polygon out of the two data curves
  // one curve goes in order, the other backwards
  if (i < mSize)
    ret = *(mpY1 + i);
  else
    ret = *(mpY2 + (2 * mSize - i - 1));

  return ret;
}

double CBandedGraphData::y1(size_t i) const
{
  return *(mpY1 + i);
}

double CBandedGraphData::y2(size_t i) const
{
  return *(mpY2 + i);
}

QwtDoubleRect
CBandedGraphData::boundingRect() const
{
  if (mSize <= 0)
    return QwtDoubleRect(1.0, 1.0, -2.0, -2.0); // invalid

  if (mLastRectangle == mSize)
    return QwtDoubleRect(mMinX, mMinY, mMaxX - mMinX, mMaxY - mMinY);

  const double *xIt = mpX + mLastRectangle;
  const double *yIt1 = mpY1 + mLastRectangle;
  const double *yIt2 = mpY2 + mLastRectangle;
  const double *end = mpX + mSize;

  mLastRectangle = mSize;

  // We have to remember whether we have an initial NaN
  bool MinXisNaN = isnan(mMinX);
  bool MaxXisNaN = isnan(mMaxX);
  bool MinYisNaN = isnan(mMinY);
  bool MaxYisNaN = isnan(mMaxY);

  while (xIt < end)
    {
      const double xv = *xIt++;

      if (!isnan(xv))
        {
          if (xv < mMinX || MinXisNaN)
            {
              mMinX = xv;
              MinXisNaN = false;
            }

          if (xv > mMaxX || MaxXisNaN)
            {
              mMaxX = xv;
              MaxXisNaN = false;
            }
        }

      double yv1 = *yIt1++;
      double yv2 = *yIt2++;

      if (isnan(yv1) || isnan(yv2))
        {
          yv1 = isnan(yv1) ? yv2 : yv1;
          yv2 = yv1;
        }
      else if (yv1 > yv2)
        {
          double tmp = yv1;
          yv1 = yv2;
          yv2 = tmp;
        } // now: yv1 <= yv2

      if (!isnan(yv1))
        {
          if (yv1 < mMinY || MinYisNaN)
            {
              mMinY = yv1;
              MinYisNaN = false;
            }

          if (yv2 > mMaxY || MaxYisNaN)
            {
              mMaxY = yv2;
              MaxYisNaN = false;
            }
        }
    }

  if (isnan(mMinX + mMaxX + mMinY + mMaxY))
    return QwtDoubleRect(1.0, 1.0, -2.0, -2.0); // invalid

  // We need to avoid very small data ranges (absolute and relative)
  C_FLOAT64 minRange = fabs(mMinX + mMaxX) * 5.e-5 + std::numeric_limits< C_FLOAT64 >::min() * 100.0;

  if (mMaxX - mMinX < minRange)
    {
      mMinX = mMinX - minRange * 0.5;
      mMaxX = mMaxX + minRange * 0.5;
    }

  minRange = fabs(mMinY + mMaxY) * 5e-5 + std::numeric_limits< C_FLOAT64 >::min() * 100.0;

  if (mMaxY - mMinY < minRange)
    {
      mMinY = mMinY - minRange * 0.5;
      mMaxY = mMaxY + minRange * 0.5;
    }

  return QwtDoubleRect(mMinX, mMinY, mMaxX - mMinX, mMaxY - mMinY);
}

void
CBandedGraphData::setSize(const size_t & size)
{
  mSize = size;
  assert(mSize <= mMaxSize);
}

void
CBandedGraphData::reallocated(const CVector< double > * pX, const CVector< double > *pY1, const CVector< double > *pY2)
{
  mpX = pX->array();
  mpY1 = pY1->array();
  mpY2 = pY2->array();
  mMaxSize = pX->size();

  assert(mSize <= mMaxSize);
}

CBandedGraphData & CBandedGraphData::operator = (const CBandedGraphData & rhs)
{
  if (&rhs == this) return *this;

  mpX = rhs.mpX;
  mpY1 = rhs.mpY1;
  mpY2 = rhs.mpY2;
  mSize = rhs.mSize;
  mMaxSize = rhs.mMaxSize;
  mLastRectangle = rhs.mLastRectangle;
  mMinX = rhs.mMinX;
  mMaxX = rhs.mMaxX;
  mMinY = rhs.mMinY;
  mMaxY = rhs.mMaxY;

  return * this;
}

//********************  data  *********************************************
CHistoCurveData::CHistoCurveData():
#if QWT_VERSION > 0x060000
  QwtSeriesData<QPointF>(),
#else
  QwtData(),
#endif
  mpX(NULL),
  mSize(0),
  mMaxSize(0),
  mLastRectangle(0),
  mMinX(std::numeric_limits<double>::quiet_NaN()),
  mMaxX(std::numeric_limits<double>::quiet_NaN()),
  mMinY(std::numeric_limits<double>::quiet_NaN()),
  mMaxY(std::numeric_limits<double>::quiet_NaN()),
  mIncrement(1.0),
  mMap(),
  mHistoX(0),
  mHistoY(0)
{}

CHistoCurveData::CHistoCurveData(const CVector< C_FLOAT64 > & x,
                                 size_t size,
                                 const C_FLOAT64 & increment):
#if QWT_VERSION > 0x060000
  QwtSeriesData<QPointF>(),
#else
  QwtData(),
#endif
  mpX(x.array()),
  mSize(size),
  mMaxSize(x.size()),
  mLastRectangle(0),
  mMinX(std::numeric_limits<double>::quiet_NaN()),
  mMaxX(std::numeric_limits<double>::quiet_NaN()),
  mMinY(std::numeric_limits<double>::quiet_NaN()),
  mMaxY(std::numeric_limits<double>::quiet_NaN()),
  mIncrement(increment),
  mMap(),
  mHistoX(0),
  mHistoY(0)
{
  assert(mSize <= mMaxSize);
}

CHistoCurveData::~CHistoCurveData()
{}

#if QWT_VERSION > 0x060000
QwtSeriesData<QPointF>* CHistoCurveData::copy() const
{
  CHistoCurveData * pCopy = new CHistoCurveData();

  *pCopy = *this;

  return pCopy;
}

QPointF CHistoCurveData::sample(size_t i) const
{
  return QPointF(x(i), y(i));
}

#else
QwtData* CHistoCurveData::copy() const
{
  CHistoCurveData * pCopy = new CHistoCurveData();

  *pCopy = *this;

  return pCopy;
}
#endif

size_t CHistoCurveData::size() const
{
  return mHistoX.size();
}

double CHistoCurveData::x(size_t i) const
{
  return mHistoX[i];
}

double CHistoCurveData::y(size_t i) const
{
  return mHistoY[i];
}

QwtDoubleRect CHistoCurveData::boundingRect() const
{
  if (mSize <= 0)
    return QwtDoubleRect(1.0, 1.0, -2.0, -2.0); // invalid

  if (mLastRectangle == mSize)
    return QwtDoubleRect(mMinX, mMinY, mMaxX - mMinX, mMaxY - mMinY);

  const double *xIt = mpX + mLastRectangle;
  const double *end = mpX + mSize;

  mLastRectangle = mSize;

  double InvIncrement = 1.0 / mIncrement;

  for (; xIt != end; ++xIt)
    {
      //just ignore breaks. Later we perhaps want to start a new histogram...
      if (isnan(*xIt)) //NaN
        continue;

      if (-std::numeric_limits< C_FLOAT64 >::infinity() < *xIt &&
          *xIt < std::numeric_limits< C_FLOAT64 >::infinity())
        {
          mMap[(C_INT32) floor(*xIt * InvIncrement)]++;
          mMap[(C_INT32) floor(*xIt * InvIncrement) + 1];
        }
    }

  //construct arrays
  mHistoX.resize(mMap.size() + 1);
  mHistoY.resize(mMap.size() + 1);

  double *pX = mHistoX.array();
  double *pY = mHistoY.array();

  //add one bin to the left
  if (mMap.size() > 0)
    {
      *pX = (mMap.begin()->first - 1) * mIncrement;
    }
  else
    {
      *pX = 0.0;
    }

  *pY = 0.0;

  mMinX = mMaxX = *pX++;
  mMinY = mMaxY = *pY++;

  C_FLOAT64 tmpFactor = 1.0 / (mSize * mIncrement);
  std::map<C_INT32, C_INT32>::const_iterator it = mMap.begin();
  std::map<C_INT32, C_INT32>::const_iterator itEnd = mMap.end();

  for (; it != itEnd; ++it, ++pX, ++pY)
    {
      //TODO use pointer increments instead of [...]
      *pX = it->first * mIncrement;
      *pY = (double)it->second * 100.0 / (double)mSize;

      if (*pX < mMinX)
        mMinX = *pX;

      if (*pX > mMaxX)
        mMaxX = *pX;

      if (*pY < mMinY)
        mMinY = *pY;

      if (*pY > mMaxY)
        mMaxY = *pY;
    }

  return QwtDoubleRect(mMinX, mMinY, mMaxX - mMinX, mMaxY - mMinY);
}

void CHistoCurveData::setSize(const size_t & size)
{
  mSize = size;
  assert(mSize <= mMaxSize);
}

void CHistoCurveData::reallocated(const CVector< double > * pX)
{
  mpX = pX->array();
  mMaxSize = pX->size();

  assert(mSize <= mMaxSize);
}

CHistoCurveData & CHistoCurveData::operator = (const CHistoCurveData & rhs)
{
  mpX = rhs.mpX;
  mSize = rhs.mSize;
  mMaxSize = rhs.mMaxSize;

  mLastRectangle = rhs.mLastRectangle;
  mMinX = rhs.mMinX;
  mMaxX = rhs.mMaxX;
  mMinY = rhs.mMinY;
  mMaxY = rhs.mMaxY;

  mIncrement = rhs.mIncrement;
  mMap = rhs.mMap;
  mHistoX = rhs.mHistoX;
  mHistoY = rhs.mHistoY;

  return * this;
}

//********************  curve  ********************************************

C2DPlotCurve::C2DPlotCurve(QMutex *pMutex, const CPlotItem::Type &type, const COutputInterface::Activity &activity, const QString &title):
  QwtPlotCurve(title),
  mpMutex(pMutex),
  mCurveType(type),
  mIncrement(1.0),
  mActivity(activity)
{
  assert(mpMutex != NULL);
}

void C2DPlotCurve::setDataSize(const size_t & size)
{
#if QWT_VERSION > 0x060000

  switch (mCurveType)
    {
      case CPlotItem::curve2d:
        static_cast< C2DCurveData * >(data())->setSize(size);
        break;

      case CPlotItem::bandedGraph:
        static_cast< CBandedGraphData * >(data())->setSize(size);
        break;

      case CPlotItem::histoItem1d:
        static_cast< CHistoCurveData * >(data())->setSize(size);
        break;

      default:
        fatalError();
        break;
    }

#else

  switch (mCurveType)
    {
      case CPlotItem::curve2d:
        static_cast< C2DCurveData * >(&data())->setSize(size);
        break;

      case CPlotItem::bandedGraph:
        static_cast< CBandedGraphData * >(&data())->setSize(size);
        break;

      case CPlotItem::histoItem1d:
        static_cast< CHistoCurveData * >(&data())->setSize(size);
        break;

      default:
        fatalError();
        break;
    }

#endif
}

void C2DPlotCurve::reallocatedData(const CVector< double > * pX, const CVector< double > * pY, const CVector< double > * pY2)
{
#if QWT_VERSION > 0x060000

  switch (mCurveType)
    {
      case CPlotItem::curve2d:
        static_cast< C2DCurveData * >(data())->reallocated(pX, pY);
        break;

      case CPlotItem::bandedGraph:
        static_cast< CBandedGraphData * >(data())->reallocated(pX, pY, pY2);
        break;

      case CPlotItem::histoItem1d:
        static_cast< CHistoCurveData * >(data())->reallocated(pX);
        break;

      default:
        fatalError();
        break;
    }

#else

  switch (mCurveType)
    {
      case CPlotItem::curve2d:
        static_cast< C2DCurveData * >(&data())->reallocated(pX, pY);
        break;

      case CPlotItem::bandedGraph:
        static_cast< CBandedGraphData * >(&data())->reallocated(pX, pY, pY2);
        break;

      case CPlotItem::histoItem1d:
        static_cast< CHistoCurveData * >(&data())->reallocated(pX);
        break;

      default:
        fatalError();
        break;
    }

#endif
}

void C2DPlotCurve::setIncrement(const C_FLOAT64 & increment)
{
  mIncrement = increment;
}

const C_FLOAT64 & C2DPlotCurve::getIncrement() const
{
  return mIncrement;
}

const CPlotItem::Type & C2DPlotCurve::getType() const
{
  return mCurveType;
}

const COutputInterface::Activity & C2DPlotCurve::getActivity() const
{
  return mActivity;
}

#if QWT_VERSION > 0x060000

//draw the several curves, separated by NaNs.
void C2DPlotCurve::myDrawLines(QPainter *painter,
                               const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                               const QRectF &canvasRect, int from, int to) const
{
  int to2;

  do
    {
      int i;

      for (i = from; i <= to; ++i)
        if (isnan(this->sample(i).x()) || isnan(sample(i).y())) //NaN
          break;

      if (i == from)
        {
          ++from;
          continue;
        }

      to2 = i - 1;

      QwtPlotCurve::drawLines(painter, xMap, yMap, canvasRect, from, to2);

      from = to2 + 2;
    }
  while (from < to);
}

//virtual
void C2DPlotCurve::drawCurve(QPainter *painter, int style,
                             const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                             const QRectF &canvasRect, int from, int to) const
{
  QMutexLocker Locker(mpMutex);

  if (style == Lines)
    {
      myDrawLines(painter, xMap, yMap, canvasRect, from, to);
    }
  else
    {
      QwtPlotCurve::drawCurve(painter, style, xMap, yMap, canvasRect, from, to);
    }
}

void C2DPlotCurve::drawSymbols(QPainter *painter, const QwtSymbol &symbol,
                               const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                               const QRectF &canvasRect, int from, int to) const
{

  int from2 = from;
  int to2;

  for (;;)
    {

      //find the next not-NaN point
      while (isnan(sample(from2).x()) || isnan(sample(from2).y()))
        {
          ++from2;

          if (from2 >= to)
            return;
        }

      //find the nex NaN point (or the end of data)
      to2 = from2;

      do
        {
          ++to2;

          if (to2 > to)
            break;
        }
      while (!(isnan(sample(to2).x()) || isnan(sample(to2).y())));

      --to2;

      QwtPlotCurve::drawSymbols(painter, symbol, xMap, yMap, canvasRect, from2, to2);

      //are we done?
      if (to2 >= to)
        return;

      //continue with the next data point
      from2 = to2 + 1;
    }
}

#else

//draw the several curves, separated by NaNs.
void C2DPlotCurve::myDrawLines(QPainter *painter,
                               const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                               int from, int to) const
{
  int to2;

  do
    {
      int i;

      for (i = from; i <= to; ++i)
        if (isnan(x(i)) || isnan(y(i))) //NaN
          break;

      if (i == from)
        {
          ++from;
          continue;
        }

      to2 = i - 1;

      QwtPlotCurve::drawLines(painter, xMap, yMap, from, to2);

      from = to2 + 2;
    }
  while (from < to);
}

//virtual
void C2DPlotCurve::drawCurve(QPainter *painter, int style,
                             const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                             int from, int to) const
{
  QMutexLocker Locker(mpMutex);

  if (style == Lines)
    {
      myDrawLines(painter, xMap, yMap, from, to);
    }
  else
    {
      QwtPlotCurve::drawCurve(painter, style, xMap, yMap, from, to);
    }
}

void C2DPlotCurve::drawSymbols(QPainter *painter, const QwtSymbol &symbol,
                               const QwtScaleMap &xMap, const QwtScaleMap &yMap,
                               int from, int to) const
{

  int from2 = from;
  int to2;

  for (;;)
    {

      //find the next not-NaN point
      while (isnan(x(from2)) || isnan(y(from2)))
        {
          ++from2;

          if (from2 >= to)
            return;
        }

      //find the nex NaN point (or the end of data)
      to2 = from2;

      do
        {
          ++to2;

          if (to2 > to)
            break;
        }
      while (!(isnan(x(to2)) || isnan(y(to2))));

      --to2;

      QwtPlotCurve::drawSymbols(painter, symbol, xMap, yMap, from2, to2);

      //are we done?
      if (to2 >= to)
        return;

      //continue with the next data point
      from2 = to2 + 1;
    }
}

#endif

//************************************
C_FLOAT64 CopasiPlot::MissingValue = std::numeric_limits<C_FLOAT64>::quiet_NaN();

CopasiPlot::CopasiPlot(QWidget* parent):
  QwtPlot(parent),
  mCurves(0),
  mCurveMap(),
  mSpectograms(0),
  mSpectogramMap(),
  mDataBefore(0),
  mDataDuring(0),
  mDataAfter(0),
  mHaveBefore(false),
  mHaveDuring(false),
  mHaveAfter(false),
  mpPlotSpecification(NULL),
  mNextPlotTime(),
  mIgnoreUpdate(false),
  mpZoomer(NULL),
  mReplotFinished(false)
{}

CopasiPlot::CopasiPlot(const CPlotSpecification* plotspec, QWidget* parent):
  QwtPlot(parent),
  mCurves(0),
  mCurveMap(),
  mSpectograms(0),
  mSpectogramMap(),
  mDataBefore(0),
  mDataDuring(0),
  mDataAfter(0),
  mHaveBefore(false),
  mHaveDuring(false),
  mHaveAfter(false),
  mpPlotSpecification(NULL),
  mNextPlotTime(),
  mIgnoreUpdate(false),
  mpZoomer(NULL),
  mReplotFinished(false)
{
  QwtLegend *legend = new QwtLegend(this);

#if QWT_VERSION > 0x060000
  legend->setDefaultItemMode(QwtLegendData::Checkable);
  ((QwtPlotCanvas*)canvas())->setPaintAttribute(QwtPlotCanvas::Opaque, true);

  connect(legend, SIGNAL(checked(const QVariant &, bool, int)),
          SLOT(legendChecked(const QVariant &, bool)));

#else
  legend->setItemMode(QwtLegend::CheckableItem);
  setCanvasLineWidth(0);
  canvas()->setPaintAttribute(QwtPlotCanvas::PaintPacked, true);
#endif

  // whole legend can not be displayed at bottom on DARWIN
  // maybe a Qwt bug ?!?
#ifdef Darwin
  insertLegend(legend, QwtPlot::TopLegend);
#else
  insertLegend(legend, QwtPlot::BottomLegend);
#endif

  // Set up the zoom facility
  mpZoomer = new ScrollZoomer(canvas());
  mpZoomer->setRubberBandPen(QColor(Qt::black));
  mpZoomer->setTrackerPen(QColor(Qt::black));
  mpZoomer->setTrackerMode(QwtPicker::AlwaysOn);
  mpZoomer->setTrackerFont(this->font());

  // white background better for printing...
  setCanvasBackground(Qt::white);

  //  setTitle(FROM_UTF8(plotspec->getTitle()));

  connect(this, SIGNAL(legendChecked(QwtPlotItem *, bool)),
          SLOT(showCurve(QwtPlotItem *, bool)));

  // Size the vectors to be able to store information for all activities.
  mData.resize(ActivitySize);
  mObjectValues.resize(ActivitySize);
  mObjectInteger.resize(ActivitySize);
  mDataSize.resize(ActivitySize);
  mDataIndex.clear();

  // Initialize from the plot specification
  initFromSpec(plotspec);
  connect(this, SIGNAL(replotSignal()), this, SLOT(replot()));
}

#if QWT_VERSION > 0x060000
void CopasiPlot::legendChecked(const QVariant &itemInfo, bool on)
{
  QwtPlotItem *plotItem = infoToItem(itemInfo);

  if (plotItem)
    showCurve(plotItem, on);
}
#endif

CPlotSpectogram *CopasiPlot::createSpectogram(CPlotItem *plotItem)
{
  QString strLimitZ = FROM_UTF8(plotItem->getValue<std::string>("maxZ"));
  bool flag;
  double limitZ = strLimitZ.toDouble(&flag);

  if (!flag)
    limitZ = std::numeric_limits<double>::quiet_NaN();

  bool logZ = plotItem->getValue<bool>("logZ");

  CPlotSpectogram *pSpectogram = new CPlotSpectogram(
    &mMutex,
    plotItem->getType(),
    plotItem->getActivity(),
    FROM_UTF8(plotItem->getTitle()),
    logZ,
    limitZ,
    plotItem->getValue<bool>("bilinear"));

  pSpectogram->attach(this);
  pSpectogram->setRenderHint(QwtPlotItem::RenderAntialiased);
  pSpectogram->setDefaultContourPen(QPen(0.5));

  std::string colorMap = *plotItem->assertParameter("colorMap", CCopasiParameter::STRING, std::string("Default"));

#if QWT_VERSION > 0x060000
  pSpectogram->setRenderThreadCount ( 0 );

  if (colorMap == "Grayscale")
    {
      QwtLinearColorMap *colorMap = new QwtLinearColorMap(Qt::white, Qt::black);
      pSpectogram->setColorMap(colorMap);

    }
  else if (colorMap == "Yellow-Red")
    {
      QwtLinearColorMap *colorMap = new QwtLinearColorMap(Qt::yellow, Qt::red);
      pSpectogram->setColorMap(colorMap);
    }
  else
    {
      QwtLinearColorMap *colorMap = new QwtLinearColorMap(Qt::darkCyan, Qt::red);
      colorMap->addColorStop(0.1, Qt::cyan);
      colorMap->addColorStop(0.6, Qt::green);
      colorMap->addColorStop(0.95, Qt::yellow);

      pSpectogram->setColorMap(colorMap);
    }
#else
  if (colorMap == "Grayscale")
    {
      QwtLinearColorMap colorMap(Qt::white, Qt::black);
      pSpectogram->setColorMap(colorMap);

    }
  else if (colorMap == "Yellow-Red")
    {
      QwtLinearColorMap colorMap(Qt::yellow, Qt::red);
      pSpectogram->setColorMap(colorMap);
    }
  else
    {
      QwtLinearColorMap colorMap(Qt::darkCyan, Qt::red);
      colorMap.addColorStop(0.1, Qt::cyan);
      colorMap.addColorStop(0.6, Qt::green);
      colorMap.addColorStop(0.95, Qt::yellow);

      pSpectogram->setColorMap(colorMap);
    }
#endif

  QString contours = FROM_UTF8(* plotItem->assertParameter("contours", CCopasiParameter::STRING, std::string("")));

  int levels = contours.toInt(&flag);

  if (flag)
    {
      // have only a certain number of levels, applying them here
      QwtValueList contourLevels;

      for (double level = 0.5; level < 10.0; level += 1.0)
        contourLevels += level;

      pSpectogram->setContourLevels(contourLevels);
      pSpectogram->setDisplayMode(QwtPlotSpectrogram::ContourMode, true);

    }
  else
    {
      // have explicit list of numbers to plot
      QStringList list = contours.split(QRegExp(",| |;"), QString::SkipEmptyParts);
      QwtValueList contourLevels;
      foreach(const QString & level, list)
      {
        contourLevels += level.toDouble();
      }
      pSpectogram->setContourLevels(contourLevels);
      pSpectogram->setDisplayMode(QwtPlotSpectrogram::ContourMode, true);
    }

  CCopasiDataModel* dataModel = (*CCopasiRootContainer::getDatamodelList())[0];

  setAxisTitle(xBottom, FROM_UTF8(dataModel->getObject((plotItem->getChannels()[0]))->getObjectDisplayName()));
  enableAxis(xBottom);

  setAxisTitle(yLeft, FROM_UTF8(dataModel->getObject((plotItem->getChannels()[1]))->getObjectDisplayName()));
  enableAxis(yLeft);

#if QWT_VERSION > 0x060000
  // TODO
#else
  setAxisScaleEngine(xTop,
                     logZ ? (QwtScaleEngine *)new QwtLog10ScaleEngine() : (QwtScaleEngine *)new QwtLinearScaleEngine());
#endif

  setAxisTitle(xTop, FROM_UTF8(dataModel->getObject((plotItem->getChannels()[2]))->getObjectDisplayName()));

  QwtScaleWidget *topAxis = axisWidget(QwtPlot::xTop);
  topAxis->setColorBarEnabled(true);

  enableAxis(xTop);



  return pSpectogram;
}

bool CopasiPlot::initFromSpec(const CPlotSpecification* plotspec)
{
  mIgnoreUpdate = true;
  mpPlotSpecification = plotspec;

  if (mpZoomer) mpZoomer->setEnabled(false);

  // size_t k, kmax = mpPlotSpecification->getItems().size();

  setTitle(FROM_UTF8(mpPlotSpecification->getTitle()));

  mCurves.resize(mpPlotSpecification->getItems().size());
  mCurves = NULL;

  mSpectograms.resize(mpPlotSpecification->getItems().size());
  mSpectograms = NULL;


  std::map< std::string, C2DPlotCurve * >::iterator found;

  CCopasiVector< CPlotItem >::const_iterator itPlotItem = mpPlotSpecification->getItems().begin();
  CCopasiVector< CPlotItem >::const_iterator endPlotItem = mpPlotSpecification->getItems().end();

  CVector< bool > Visible(mpPlotSpecification->getItems().size());
  Visible = true;
  bool * pVisible = Visible.array();

  for (; itPlotItem != endPlotItem; ++itPlotItem, ++pVisible)
    {
      // Qwt does not like it to reuse the curve as this may lead to access
      // violation. We therefore delete the curves but remember their visibility.
      if ((found = mCurveMap.find((*itPlotItem)->CCopasiParameter::getKey())) != mCurveMap.end())
        {
          *pVisible = found->second->isVisible();
        }
    }

  // Remove unused curves if definition has changed
  std::map< std::string, C2DPlotCurve * >::iterator it = mCurveMap.begin();
  std::map< std::string, C2DPlotCurve * >::iterator end = mCurveMap.end();

  for (; it != end; ++it)
    pdelete(it->second);

  mCurveMap.clear();

  std::map< std::string, CPlotSpectogram * >::iterator it2 = mSpectogramMap.begin();
  std::map< std::string, CPlotSpectogram * >::iterator end2 = mSpectogramMap.end();

  for (; it2 != end2; ++it2)
    pdelete(it2->second);

  mSpectogramMap.clear();


  itPlotItem = mpPlotSpecification->getItems().begin();
  pVisible = Visible.array();
  C2DPlotCurve ** ppCurve = mCurves.array();
  CPlotSpectogram** ppSpectogram = mSpectograms.array();
  unsigned long int k = 0;
  bool needLeft = false;
  bool needRight = false;

  for (; itPlotItem != endPlotItem; ++itPlotItem, ++pVisible, ++ppCurve, ++ppSpectogram, ++k)
    {
      if ((*itPlotItem)->getType() == CPlotItem::spectogram)
        {
          CPlotSpectogram* pSpectogram = createSpectogram(*itPlotItem);

          *ppSpectogram = pSpectogram;
          mSpectogramMap[(*itPlotItem)->CCopasiParameter::getKey()] = pSpectogram;

          showCurve(pSpectogram, *pVisible);

          needLeft = true;

          continue;
        }

      // set up the curve
      C2DPlotCurve * pCurve = new C2DPlotCurve(&mMutex,
          (*itPlotItem)->getType(),
          (*itPlotItem)->getActivity(),
          FROM_UTF8((*itPlotItem)->getTitle()));
      *ppCurve = pCurve;

      mCurveMap[(*itPlotItem)->CCopasiParameter::getKey()] = pCurve;

      //color handling should be similar for different curve types
      QColor color;

      if (pCurve->getType() == CPlotItem::curve2d
          || pCurve->getType() == CPlotItem::histoItem1d
          || pCurve->getType() == CPlotItem::bandedGraph)
        {
          std::string colorstr = (*itPlotItem)->getValue< std::string >("Color");
          color = CQPlotColors::getColor(colorstr, k);
        }

      pCurve->setPen(color);
      pCurve->attach(this);

      showCurve(pCurve, *pVisible);

      if (pCurve->getType() == CPlotItem::curve2d
          || pCurve->getType() == CPlotItem::bandedGraph)
        {
          needLeft = true;
          pCurve->setRenderHint(QwtPlotItem::RenderAntialiased);

          unsigned C_INT32 linetype = (*itPlotItem)->getValue< unsigned C_INT32 >("Line type");

          if (linetype == 0      //line
              || linetype == 3)  //line+symbols
            {
              pCurve->setStyle(QwtPlotCurve::Lines);
              unsigned C_INT32 linesubtype = (*itPlotItem)->getValue< unsigned C_INT32 >("Line subtype");
              C_FLOAT64 width = (*itPlotItem)->getValue< C_FLOAT64 >("Line width");

              switch (linesubtype) //symbol type
                {
                  case 1:
                    pCurve->setPen(QPen(QBrush(color), width, Qt::DotLine, Qt::FlatCap));
                    break;

                  case 2:
                    pCurve->setPen(QPen(QBrush(color), width, Qt::DashLine));
                    break;

                  case 3:
                    pCurve->setPen(QPen(QBrush(color), width, Qt::DashDotLine));
                    break;

                  case 4:
                    pCurve->setPen(QPen(QBrush(color), width, Qt::DashDotDotLine));
                    break;

                  case 0:
                  default:
                    pCurve->setPen(QPen(QBrush(color), width, Qt::SolidLine));
                    break;
                }
            }

          if (linetype == 1) //points
            {
              C_FLOAT64 width = (*itPlotItem)->getValue< C_FLOAT64 >("Line width");
              pCurve->setPen(QPen(color, width, Qt::SolidLine, Qt::RoundCap));
              pCurve->setStyle(QwtPlotCurve::Dots);
            }

          if (linetype == 2) //only symbols
            {
              pCurve->setStyle(QwtPlotCurve::NoCurve);
            }

          if (linetype == 2      //symbols
              || linetype == 3)  //line+symbols
            {
              unsigned C_INT32 symbolsubtype = (*itPlotItem)->getValue< unsigned C_INT32 >("Symbol subtype");

              switch (symbolsubtype) //symbol type
                {
                  case 1:
#if QWT_VERSION > 0x060000
                    pCurve->setSymbol(new QwtSymbol(QwtSymbol::Cross, QBrush(), QPen(QBrush(color), 2), QSize(7, 7)));
#else
                    pCurve->setSymbol(QwtSymbol(QwtSymbol::Cross, QBrush(), QPen(QBrush(color), 2), QSize(7, 7)));
#endif
                    break;

                  case 2:
#if QWT_VERSION > 0x060000
                    pCurve->setSymbol(new QwtSymbol(QwtSymbol::Ellipse, QBrush(), QPen(QBrush(color), 1), QSize(8, 8)));
#else
                    pCurve->setSymbol(QwtSymbol(QwtSymbol::Ellipse, QBrush(), QPen(QBrush(color), 1), QSize(8, 8)));
#endif
                    break;

                  case 0:
                  default:
#if QWT_VERSION > 0x060000
                    pCurve->setSymbol(new QwtSymbol(QwtSymbol::Cross, QBrush(color), QPen(QBrush(color), 1), QSize(5, 5)));
#else
                    pCurve->setSymbol(QwtSymbol(QwtSymbol::Cross, QBrush(color), QPen(QBrush(color), 1), QSize(5, 5)));
#endif

                    break;
                }
            }
        } //2d curves and banded graphs

      if (pCurve->getType() == CPlotItem::bandedGraph)
        {
          //set fill color
          QColor c = color;
          c.setAlpha(64);
          pCurve->setBrush(c);
        }

      if (pCurve->getType() == CPlotItem::histoItem1d)
        {
          pCurve->setIncrement((*itPlotItem)->getValue< C_FLOAT64 >("increment"));

          pCurve->setStyle(QwtPlotCurve::Steps);
          pCurve->setYAxis(QwtPlot::yRight);
          pCurve->setCurveAttribute(QwtPlotCurve::Inverted);

          needRight = true;
        }
    }

  if (plotspec->isLogX())
#if QWT_VERSION > 0x060000
    setAxisScaleEngine(xBottom, new QwtLogScaleEngine());

#else
    setAxisScaleEngine(xBottom, new QwtLog10ScaleEngine());
#endif
  else
    setAxisScaleEngine(xBottom, new QwtLinearScaleEngine());

  setAxisAutoScale(xBottom);

  if (plotspec->isLogY())
#if QWT_VERSION > 0x060000
    setAxisScaleEngine(yLeft, new QwtLogScaleEngine());

#else
    setAxisScaleEngine(yLeft, new QwtLog10ScaleEngine());
#endif
  else
    setAxisScaleEngine(yLeft, new QwtLinearScaleEngine());

  setAxisAutoScale(yLeft);

  enableAxis(yLeft, needLeft);

  if (needRight)
    {
      setAxisScaleEngine(yRight, new QwtLinearScaleEngine());
      setAxisTitle(yRight, "Percent %");
      enableAxis(yRight);
    }

  mIgnoreUpdate = false;

  return true; //TODO really check!
}

bool CopasiPlot::compile(CObjectInterface::ContainerList listOfContainer)
{
  clearBuffers();

  size_t i, imax;
  size_t j, jmax;

  std::pair< std::set< const CObjectInterface * >::iterator, bool > Inserted;
  std::pair< Activity, size_t > DataIndex;
  std::vector< std::set < const CObjectInterface * > > ActivityObjects;

  ActivityObjects.resize(ActivitySize);

  // Loop over all curves.
  imax = mpPlotSpecification->getItems().size();
  mDataIndex.resize(imax);

  std::vector< std::vector < const CObjectInterface * > >::iterator itX;

  for (i = 0; i < imax; ++i)
    {
      CPlotItem * pItem = mpPlotSpecification->getItems()[i];
      Activity ItemActivity = pItem->getActivity();
      DataIndex.first = ItemActivity;

      // Loop over all channels
      jmax = pItem->getNumChannels();
      mDataIndex[i].resize(jmax);

      for (j = 0; j < jmax; ++j)
        {
          const CObjectInterface * pObj = CObjectInterface::GetObjectFromCN(listOfContainer, pItem->getChannels()[j]);

          if (pObj)
            mObjects.insert(pObj);
          else
            CCopasiMessage(CCopasiMessage::WARNING, MCCopasiTask + 6,
                           pItem->getChannels()[j].c_str());

          // Remember the actual order for saving the data.
          // Note, we are currently only dealing with 2D curves and histograms.
          // In addition the data is not normalized. The same data column may appear
          // multiple times, e.g. as X value and as Y value for another curve.
          if (j == 0)
            {
              // We have an X value
              for (itX = mSaveCurveObjects.begin(); itX != mSaveCurveObjects.end(); ++itX)
                if (*itX->begin() == pObj) break;

              if (itX == mSaveCurveObjects.end())
                {
                  std::vector < const CObjectInterface * > NewX;
                  NewX.push_back(pObj);

                  mSaveCurveObjects.push_back(NewX);
                  itX = mSaveCurveObjects.end() - 1;

                  setAxisUnits(xBottom, pObj);
                }

              if (pItem->getType() == CPlotItem::histoItem1d)
                mSaveHistogramObjects.push_back(pObj);
            }
          else
            {
              itX->push_back(pObj);
              setAxisUnits(yLeft, pObj);
            }

          Inserted = ActivityObjects[ItemActivity].insert(pObj);

          if (Inserted.second)
            {
              if (ItemActivity & COutputInterface::BEFORE) mHaveBefore = true;

              if (ItemActivity & COutputInterface::DURING) mHaveDuring = true;

              if (ItemActivity & COutputInterface::AFTER) mHaveAfter = true;

              // The insert was successful
              DataIndex.second = ActivityObjects[ItemActivity].size() - 1;

              // Allocate the data buffer
              mData[ItemActivity].push_back(new CVector<double>(1000));

              // Store the pointer to the current object value. (Only if it has a double or integer value
              // and the value pointer actually exists. If not, use a dummy value.)
              void * tmp;

              const CCopasiObject * pDataObject = CObjectInterface::DataObject(pObj);

              if (pDataObject != NULL &&
                  (tmp = pObj->getValuePointer()) != NULL &&
                  (pDataObject->isValueInt() || pDataObject->isValueDbl()))
                {
                  mObjectValues[ItemActivity].push_back((C_FLOAT64 *) tmp); //pObj->getValuePointer());
                  mObjectInteger[ItemActivity].push_back(pDataObject->isValueInt());
                }
              else
                {
                  mObjectValues[ItemActivity].push_back(&MissingValue);
                  mObjectInteger[ItemActivity].push_back(false);
                }

              // Store [curve][channel] to data index
              mDataIndex[i][j] = DataIndex;

              // Store the [Activity][object] to data index.
              mObjectIndex[ItemActivity][pObj] = DataIndex.second;
            }
          else
            {
              // The object already existed we only need to
              // store [curve][channel] to data index.
              DataIndex.second = mObjectIndex[ItemActivity][pObj];
              mDataIndex[i][j] = DataIndex;
            }
        }
    }

  // We need to set the curve data here!
  size_t k = 0;
  C2DPlotCurve ** itCurves = mCurves.array();
  C2DPlotCurve ** endCurves = itCurves + mCurves.size();

  for (; itCurves != endCurves; ++itCurves, ++k)
    {
      if (*itCurves == NULL) continue;

      std::vector< CVector< double > * > & data = mData[(*itCurves)->getActivity()];

      switch ((*itCurves)->getType())
        {
          case CPlotItem::curve2d:
#if QWT_VERSION > 0x060000
            (*itCurves)->setData(new C2DCurveData(*data[mDataIndex[k][0].second],
                                                  *data[mDataIndex[k][1].second],
                                                  0));
#else
            (*itCurves)->setData(C2DCurveData(*data[mDataIndex[k][0].second],
                                              *data[mDataIndex[k][1].second],
                                              0));
#endif
            break;

          case CPlotItem::bandedGraph:
#if QWT_VERSION > 0x060000
            (*itCurves)->setData(new CBandedGraphData(*data[mDataIndex[k][0].second],
                                 *data[mDataIndex[k][1].second],
                                 *data[mDataIndex[k][2].second],
                                 0));
#else
            (*itCurves)->setData(CBandedGraphData(*data[mDataIndex[k][0].second],
                                                  *data[mDataIndex[k][1].second],
                                                  *data[mDataIndex[k][2].second],
                                                  0));
#endif
            break;

          case CPlotItem::histoItem1d:
#if QWT_VERSION > 0x060000
            (*itCurves)->setData(new CHistoCurveData(*data[mDataIndex[k][0].second],
                                 0,
                                 mCurves[k]->getIncrement()));
#else
            (*itCurves)->setData(CHistoCurveData(*data[mDataIndex[k][0].second],
                                                 0,
                                                 mCurves[k]->getIncrement()));
#endif
            break;

          default:
            fatalError();
            break;
        }
    }

  k = 0;
  CPlotSpectogram ** itSpectrograms = mSpectograms.array();
  CPlotSpectogram ** endSpectrograms = itSpectrograms + mSpectograms.size();

  for (; itSpectrograms != endSpectrograms; ++itSpectrograms, ++k)
    {
      if (*itSpectrograms == NULL) continue;

      std::vector< CVector< double > * > & data = mData[(*itSpectrograms)->getActivity()];

      switch ((*itSpectrograms)->getType())
        {
          case CPlotItem::spectogram:
#if QWT_VERSION > 0x060000
            (*itSpectrograms)->setData(
              new CSpectorgramData(
                *data[mDataIndex[k][0].second],
                *data[mDataIndex[k][1].second],
                *data[mDataIndex[k][2].second],
                0,
                (*itSpectrograms)->getLogZ(),
                (*itSpectrograms)->getLimitZ(),
                (*itSpectrograms)->getBilinear()
              )
            );
#else
            (*itSpectrograms)->setData(
              CSpectorgramData(
                *data[mDataIndex[k][0].second],
                *data[mDataIndex[k][1].second],
                *data[mDataIndex[k][2].second],
                0,
                (*itSpectrograms)->getLogZ(),
                (*itSpectrograms)->getLimitZ(),
                (*itSpectrograms)->getBilinear()
              ));
#endif
            break;


          default:
            fatalError();
            break;
        }
    }

  mNextPlotTime = CCopasiTimeVariable::getCurrentWallTime();
  mReplotFinished = true;

  return true;
}

void CopasiPlot::output(const Activity & activity)
{
  size_t i, imax;
  C_INT32 ItemActivity;

  if (mHaveBefore && (activity == COutputInterface::BEFORE)) mDataBefore++;
  else if (mHaveDuring && (activity == COutputInterface::DURING)) mDataDuring++;
  else if (mHaveAfter && (activity == COutputInterface::AFTER)) mDataAfter++;

  for (ItemActivity = 0; ItemActivity < ActivitySize; ItemActivity++)
    if ((ItemActivity & activity) && mData[ItemActivity].size())
      {
        std::vector< CVector< double > * > & data = mData[ItemActivity];
        size_t & ndata = mDataSize[ItemActivity];

        if ((imax = data.size()) != 0)
          {
            if (ndata >= data[0]->size())
              {
                resizeCurveData(ItemActivity);
              }

            //the data that needs to be stored internally:
            for (i = 0; i < imax; ++i)
              if (mObjectInteger[ItemActivity][i])
                (*data[i])[ndata] = *(C_INT32 *)mObjectValues[ItemActivity][i];
              else
                (*data[i])[ndata] = *mObjectValues[ItemActivity][i];

            ++ndata;
          }
      }

  updatePlot();
}

void CopasiPlot::separate(const Activity & activity)
{
  size_t i, imax;
  C_INT32 ItemActivity;

  if (mHaveBefore && (activity == COutputInterface::BEFORE)) mDataBefore++;

  if (mHaveDuring && (activity == COutputInterface::DURING)) mDataDuring++;

  if (mHaveAfter && (activity == COutputInterface::AFTER)) mDataAfter++;

  for (ItemActivity = 0; ItemActivity < ActivitySize; ItemActivity++)
    if ((ItemActivity & activity) && mData[ItemActivity].size())
      {
        std::vector< CVector< double > * > & data = mData[ItemActivity];
        size_t & ndata = mDataSize[ItemActivity];

        if ((imax = data.size()) != 0)
          {
            if (ndata >= data[0]->size())
              {
                resizeCurveData(ItemActivity);
              }

            //the data that needs to be stored internally:
            for (i = 0; i < imax; ++i)
              (*data[i])[ndata] = MissingValue;

            ++ndata;
          }
      }

  updatePlot();

  return;
}

void CopasiPlot::finish()
{


  // We need to force a replot, i.e., the next mNextPlotTime should be in the past.
  mNextPlotTime = 0;
  replot();


  if (mpZoomer)
    {
      mpZoomer->setEnabled(true);
      mpZoomer->setZoomBase();
    }
}


void CopasiPlot::updateCurves(const size_t & activity)
{
  if (activity == C_INVALID_INDEX)
    {
      C_INT32 ItemActivity;

      for (ItemActivity = 0; ItemActivity < ActivitySize; ItemActivity++)
        updateCurves(ItemActivity);

      return;
    }

  size_t k = 0;
  C2DPlotCurve ** itCurves = mCurves.array();
  C2DPlotCurve ** endCurves = itCurves + mCurves.size();

  for (; itCurves != endCurves; ++itCurves, ++k)
    {
      if (*itCurves == NULL) continue;

      if ((size_t)(*itCurves)->getActivity() == activity)
        {
          (*itCurves)->setDataSize(mDataSize[activity]);
        }
    }

  // skip rendering when shift is pressed
  Qt::KeyboardModifiers mods = QApplication::keyboardModifiers();

  if (((int)mods & (int)Qt::ShiftModifier) == (int)Qt::ShiftModifier)
    return;

  k = 0;
  CPlotSpectogram ** itSpectograms = mSpectograms.array();
  CPlotSpectogram ** endSpectograms = itSpectograms + mSpectograms.size();

  for (; itSpectograms != endSpectograms; ++itSpectograms, ++k)
    {
      if (*itSpectograms == NULL) continue;

      if ((size_t)(*itSpectograms)->getActivity() == activity)
        {
          (*itSpectograms)->setDataSize(mDataSize[activity]);
#if QWT_VERSION > 0x060000
          QwtScaleWidget *topAxis = axisWidget(QwtPlot::xTop);
          const QwtInterval zInterval = (*itSpectograms)->data()->interval( Qt::ZAxis );
          topAxis->setColorBarEnabled(true);
          topAxis->setColorMap(zInterval,
                               const_cast<QwtColorMap*>((*itSpectograms)->colorMap()));
          setAxisScale(QwtPlot::xTop,
                       zInterval.minValue(),
                       zInterval.maxValue());
#else

          QwtScaleWidget *topAxis = axisWidget(QwtPlot::xTop);
          topAxis->setColorBarEnabled(true);
          topAxis->setColorMap((*itSpectograms)->data().range(),
                               (*itSpectograms)->colorMap());
          setAxisScale(QwtPlot::xTop,
                       (*itSpectograms)->data().range().minValue(),
                       (*itSpectograms)->data().range().maxValue());
#endif
        }
    }
}

void CopasiPlot::resizeCurveData(const size_t & activity)
{
  std::vector< CVector< double > * > & data = mData[activity];
  std::vector< CVector< double > * >::iterator it = data.begin();

  std::vector< CVector< double > * > OldData = data;
  std::vector< CVector< double > * >::iterator itOld = OldData.begin();
  std::vector< CVector< double > * >::iterator endOld = OldData.end();

  size_t oldSize = (*it)->size();
  size_t newSize = 2 * (*it)->size();

  // We must not deallocate the old data since this will create a window of time
  // were the GUI thread may access the old location before it is notified.
  for (; itOld != endOld; ++itOld, ++it)
    {
      *it = new CVector< double >(newSize);
      memcpy((*it)->array(), (*itOld)->array(), oldSize * sizeof(double));
    }

  // Tell the curves that the location of the data has changed
  // otherwise repaint events could crash
  size_t k = 0;
  C2DPlotCurve ** itCurves = mCurves.array();
  C2DPlotCurve ** endCurves = itCurves + mCurves.size();

  for (; itCurves != endCurves; ++itCurves, ++k)
    {
      if (*itCurves == NULL) continue;

      if ((size_t)(*itCurves)->getActivity() == activity)
        {
          std::vector< CVector< double > * > & data = mData[activity];

          switch ((*itCurves)->getType())
            {
              case CPlotItem::curve2d:
                (*itCurves)->reallocatedData(data[mDataIndex[k][0].second],
                                             data[mDataIndex[k][1].second]);
                break;

              case CPlotItem::bandedGraph:
                (*itCurves)->reallocatedData(data[mDataIndex[k][0].second],
                                             data[mDataIndex[k][1].second],
                                             data[mDataIndex[k][2].second]);
                break;

              case CPlotItem::histoItem1d:
                (*itCurves)->reallocatedData(data[mDataIndex[k][0].second],
                                             NULL);
                break;

              default:
                fatalError();
                break;
            }
        }
    }

  k = 0;
  CPlotSpectogram ** itSpectograms = mSpectograms.array();
  CPlotSpectogram ** endSpectograms = itSpectograms + mSpectograms.size();

  for (; itSpectograms != endSpectograms; ++itSpectograms, ++k)
    {
      if (*itSpectograms == NULL) continue;

      if ((size_t)(*itSpectograms)->getActivity() == activity)
        {
          std::vector< CVector< double > * > & data = mData[activity];

          switch ((*itSpectograms)->getType())
            {
              case CPlotItem::spectogram:
                (*itSpectograms)->reallocatedData(data[mDataIndex[k][0].second],
                                                  data[mDataIndex[k][1].second],
                                                  data[mDataIndex[k][2].second]);
                break;

              default:
                fatalError();
                break;
            }
        }
    }

  // It is now save to delete the old data since the GUI thread has been notified.
  for (itOld = OldData.begin(); itOld != endOld; ++itOld)
    {
      // pdelete(*itOld);
    }
}

void CopasiPlot::updatePlot()
{
  if (mReplotFinished)
    {
      mReplotFinished = false;
      emit replotSignal();
    }
}

//-----------------------------------------------------------------------------

/*void CopasiPlot::enableZoom(bool enabled)
{
  zoomOn = enabled;
}*/

//-----------------------------------------------------------------------------

CopasiPlot::~CopasiPlot()
{
  clearBuffers();
}

bool CopasiPlot::saveData(const std::string & filename)
{
  // No objects.
  if (!mObjects.size()) return true;

  // Find out whether we have any data.
  C_INT32 ItemActivity;

  for (ItemActivity = 0; ItemActivity < ActivitySize; ItemActivity++)
    if (mDataSize[ItemActivity] != 0) break;

  // No data
  if (ItemActivity == ActivitySize) return true;

  std::ofstream fs(CLocaleString::fromUtf8(filename).c_str());

  if (!fs.good()) return false;

  // Write the table header
  fs << "# ";

  std::vector< std::vector < const CObjectInterface  * > >::const_iterator itX;
  std::vector< std::vector < const CObjectInterface * > >::const_iterator endX =
    mSaveCurveObjects.end();

  std::vector < const CObjectInterface * >::const_iterator it;
  std::vector < const CObjectInterface * >::const_iterator end;

  for (itX = mSaveCurveObjects.begin(); itX != endX; ++itX)
    for (it = itX->begin(), end = itX->end(); it != end; ++it)
      if (CObjectInterface::DataObject(*it) != NULL)
        fs << CObjectInterface::DataObject(*it)->getObjectDisplayName() << "\t";
      else
        fs << "Not found\t";

  fs << "\n";

  size_t i, imax = mObjects.size();
  std::vector< CVector< double > * > Data;
  Data.resize(imax);

  std::vector< CVector< double > * >::const_iterator itData;
  std::vector< CVector< double > * >::const_iterator endData = Data.end();

  std::vector< size_t > Offset;
  std::vector< size_t >::const_iterator itOffset;

  Offset.resize(imax);

  std::map< Activity, std::map< const CObjectInterface *, size_t > >::iterator itActivity;
  std::map< const CObjectInterface *, size_t >::iterator itObject;

  if (mDataBefore)
    {
      for (itX = mSaveCurveObjects.begin(), i = 0; itX != endX; ++itX)
        for (it = itX->begin(), end = itX->end(); it != end; ++it, ++i)
          {
            if ((itActivity = mObjectIndex.find(COutputInterface::BEFORE)) != mObjectIndex.end() &&
                (itObject = itActivity->second.find(*it)) != itActivity->second.end())
              {
                Data[i] = mData[COutputInterface::BEFORE][itObject->second];
                continue;
              }

            if ((itActivity = mObjectIndex.find((COutputInterface::Activity)(COutputInterface::BEFORE | COutputInterface::DURING))) != mObjectIndex.end() &&
                (itObject = itActivity->second.find(*it)) != itActivity->second.end())
              {
                Data[i] = mData[COutputInterface::BEFORE | COutputInterface::DURING][itObject->second];
                continue;
              }

            if ((itActivity = mObjectIndex.find((COutputInterface::Activity)(COutputInterface::BEFORE | COutputInterface::AFTER))) != mObjectIndex.end() &&
                (itObject = itActivity->second.find(*it)) != itActivity->second.end())
              {
                Data[i] = mData[COutputInterface::BEFORE | COutputInterface::AFTER][itObject->second];
                continue;
              }

            if ((itActivity = mObjectIndex.find((COutputInterface::Activity)(COutputInterface::BEFORE | COutputInterface::DURING | COutputInterface::AFTER))) != mObjectIndex.end() &&
                (itObject = itActivity->second.find(*it)) != itActivity->second.end())
              {
                Data[i] = mData[COutputInterface::BEFORE | COutputInterface::DURING | COutputInterface::AFTER][itObject->second];
                continue;
              }

            Data[i] = NULL;
          }

      for (i = 0; i < mDataBefore; i++)
        {
          for (itData = Data.begin(); itData != endData; ++itData)
            {
              if (*itData) fs << (**itData)[i];
              else fs << MissingValue;

              fs << "\t";
            }

          fs << std::endl;
        }
    }

  if (mDataDuring)
    {
      for (itX = mSaveCurveObjects.begin(), i = 0; itX != endX; ++itX)
        for (it = itX->begin(), end = itX->end(); it != end; ++it, ++i)
          {
            if ((itActivity = mObjectIndex.find(COutputInterface::DURING)) != mObjectIndex.end() &&
                (itObject = itActivity->second.find(*it)) != itActivity->second.end())
              {
                Data[i] = mData[COutputInterface::DURING][itObject->second];
                Offset[i] = 0;
                continue;
              }

            if ((itActivity = mObjectIndex.find((COutputInterface::Activity)(COutputInterface::BEFORE | COutputInterface::DURING))) != mObjectIndex.end() &&
                (itObject = itActivity->second.find(*it)) != itActivity->second.end())
              {
                Data[i] = mData[COutputInterface::BEFORE | COutputInterface::DURING][itObject->second];
                Offset[i] = mDataBefore;
                continue;
              }

            if ((itActivity = mObjectIndex.find((COutputInterface::Activity)(COutputInterface::DURING | COutputInterface::AFTER))) != mObjectIndex.end() &&
                (itObject = itActivity->second.find(*it)) != itActivity->second.end())
              {
                Data[i] = mData[COutputInterface::DURING | COutputInterface::AFTER][itObject->second];
                Offset[i] = 0;
                continue;
              }

            if ((itActivity = mObjectIndex.find((COutputInterface::Activity)(COutputInterface::BEFORE | COutputInterface::DURING | COutputInterface::AFTER))) != mObjectIndex.end() &&
                (itObject = itActivity->second.find(*it)) != itActivity->second.end())
              {
                Data[i] = mData[COutputInterface::BEFORE | COutputInterface::DURING | COutputInterface::AFTER][itObject->second];
                Offset[i] = mDataBefore;
                continue;
              }

            Data[i] = NULL;
          }

      for (i = 0; i < mDataDuring; i++)
        {
          for (itData = Data.begin(), itOffset = Offset.begin(); itData != endData; ++itData)
            {
              if (*itData) fs << (**itData)[i + *itOffset];
              else fs << MissingValue;

              fs << "\t";
            }

          fs << std::endl;
        }
    }

  if (mDataAfter)
    {
      for (itX = mSaveCurveObjects.begin(), i = 0; itX != endX; ++itX)
        for (it = itX->begin(), end = itX->end(); it != end; ++it, ++i)
          {
            if ((itActivity = mObjectIndex.find(COutputInterface::AFTER)) != mObjectIndex.end() &&
                (itObject = itActivity->second.find(*it)) != itActivity->second.end())
              {
                Data[i] = mData[COutputInterface::AFTER][itObject->second];
                Offset[i] = 0;
                continue;
              }

            if ((itActivity = mObjectIndex.find((COutputInterface::Activity)(COutputInterface::BEFORE | COutputInterface::AFTER))) != mObjectIndex.end() &&
                (itObject = itActivity->second.find(*it)) != itActivity->second.end())
              {
                Data[i] = mData[COutputInterface::BEFORE | COutputInterface::AFTER][itObject->second];
                Offset[i] = mDataBefore;
                continue;
              }

            if ((itActivity = mObjectIndex.find((COutputInterface::Activity)(COutputInterface::DURING | COutputInterface::AFTER))) != mObjectIndex.end() &&
                (itObject = itActivity->second.find(*it)) != itActivity->second.end())
              {
                Data[i] = mData[COutputInterface::DURING | COutputInterface::AFTER][itObject->second];
                Offset[i] = mDataDuring;
                continue;
              }

            if ((itActivity = mObjectIndex.find((COutputInterface::Activity)(COutputInterface::BEFORE | COutputInterface::DURING | COutputInterface::AFTER))) != mObjectIndex.end() &&
                (itObject = itActivity->second.find(*it)) != itActivity->second.end())
              {
                Data[i] = mData[COutputInterface::BEFORE | COutputInterface::DURING | COutputInterface::AFTER][itObject->second];
                Offset[i] = mDataBefore + mDataDuring;
                continue;
              }

            Data[i] = NULL;
          }

      for (i = 0; i < mDataAfter; i++)
        {
          for (itData = Data.begin(), itOffset = Offset.begin(); itData != endData; ++itData)
            {
              if (*itData) fs << (**itData)[i + *itOffset];
              else fs << MissingValue;

              fs << "\t";
            }

          fs << std::endl;
        }
    }

  bool FirstHistogram = true;
  size_t HistogramIndex = 0;

  C2DPlotCurve ** itCurves = mCurves.array();
  C2DPlotCurve ** endCurves = itCurves + mCurves.size();

  for (; itCurves != endCurves; ++itCurves)
    {
      if (*itCurves == NULL) continue;

      if ((*itCurves)->getType() == CPlotItem::histoItem1d)
        {
          if (FirstHistogram)
            {
              fs << "\n# The histograms: \n";
              FirstHistogram = false;
            }

          if (CObjectInterface::DataObject(mSaveHistogramObjects[HistogramIndex]) != NULL)
            fs << CObjectInterface::DataObject(mSaveHistogramObjects[HistogramIndex])->getObjectDisplayName();
          else
            fs << "Not found";

          fs << std::endl;

#if QWT_VERSION > 0x060000
          CHistoCurveData * pData = static_cast< CHistoCurveData * >((*itCurves)->data());
#else
          CHistoCurveData * pData = static_cast< CHistoCurveData * >(&(*itCurves)->data());
#endif
          size_t i, imax = pData->size();

          for (i = 0; i < imax; ++i)
            {
              fs << pData->x(i) << "\t" << pData->y(i) << "\n";
            }
        }
    }

  fs.close();

  if (!fs.good()) return false;

  return true;
}

void CopasiPlot::showCurve(QwtPlotItem *item, bool on)
{
  item->setVisible(on);
  item->setItemAttribute(QwtPlotItem::AutoScale, on);
#if QWT_VERSION > 0x060000
  QwtLegend *lgd = qobject_cast<QwtLegend *>(legend());
  QList<QWidget *> legendWidgets =
    lgd->legendWidgets(itemToInfo(item));

  if (legendWidgets.size() == 1)
    {
      QwtLegendLabel *legendLabel =
        qobject_cast<QwtLegendLabel *>(legendWidgets[0]);

      if (legendLabel)
        legendLabel->setChecked(on);
    }

#else
  QWidget *w = legend()->find(item);

  if (w && w->inherits("QwtLegendItem"))
    static_cast< QwtLegendItem * >(w)->setChecked(on);

#endif

  if (!mIgnoreUpdate)
    replot();
}

void CopasiPlot::setCurvesVisibility(const bool & visibility)
{
  std::map< std::string, C2DPlotCurve * >::iterator it = mCurveMap.begin();
  std::map< std::string, C2DPlotCurve * >::iterator end = mCurveMap.end();

  for (; it != end; ++it)
    {
      it->second->setVisible(visibility);
      it->second->setItemAttribute(QwtPlotItem::AutoScale, visibility);
#if QWT_VERSION > 0x060000
      QwtLegend *lgd = qobject_cast<QwtLegend *>(legend());
      QList<QWidget *> legendWidgets =
        lgd->legendWidgets(itemToInfo(it->second));

      if (legendWidgets.size() == 1)
        {
          QwtLegendLabel *legendLabel =
            qobject_cast<QwtLegendLabel *>(legendWidgets[0]);

          if (legendLabel)
            legendLabel->setChecked(visibility);
        }

#else
      QWidget *w = legend()->find(it->second);

      if (w && w->inherits("QwtLegendItem"))
        static_cast< QwtLegendItem * >(w)->setChecked(visibility);

#endif
    }

  if (!mIgnoreUpdate)
    replot();
}

void CopasiPlot::clearBuffers()
{
  mObjects.clear();

  size_t Activity;
  size_t i, imax;

  for (Activity = 0; Activity < ActivitySize; Activity++)
    {
      std::vector< CVector< double > * > & data = mData[Activity];

      // Delete each QMemArray
      for (i = 0, imax = data.size(); i < imax; i++)
        if (data[i] != NULL) delete data[i];

      data.clear();

      mObjectValues[Activity].clear();
      mObjectInteger[Activity].clear();
      mDataSize[Activity] = 0;
    }

  mDataIndex.clear();
  mObjectIndex.clear();

  mSaveCurveObjects.clear();
  mSaveHistogramObjects.clear();

  mDataBefore = 0;
  mDataDuring = 0;
  mDataAfter = 0;

  mHaveBefore = false;
  mHaveDuring = false;
  mHaveAfter = false;
}

void CopasiPlot::setAxisUnits(const C_INT32 & index,
                              const CObjectInterface * pObjectInterface)
{
  const CCopasiObject * pObject = CObjectInterface::DataObject(pObjectInterface);

  if (pObject == NULL) return;

  std::string Units = pObject->getUnits();

  if (Units != "")
    setAxisTitle(index, FROM_UTF8(Units));

  return;
}

// virtual
void CopasiPlot::replot()
{
  if (mNextPlotTime < CCopasiTimeVariable::getCurrentWallTime())
    {
      CCopasiTimeVariable Delta = CCopasiTimeVariable::getCurrentWallTime();

      {
        QMutexLocker Locker(&mMutex);
        updateCurves(C_INVALID_INDEX);
      }

      QwtPlot::replot();

      Delta = CCopasiTimeVariable::getCurrentWallTime() - Delta;
      mNextPlotTime = CCopasiTimeVariable::getCurrentWallTime() + 3 * Delta.getMicroSeconds();
    }

  mReplotFinished = true;
}

CSpectorgramData::CSpectorgramData()
  : QwtRasterData()
  , mpX(NULL)
  , mpY(NULL)
  , mpZ(NULL)
  , mSize(0)
  , mMaxSize(0)
  , mLastRectangle(0)
  , mMinX(std::numeric_limits<double>::quiet_NaN())
  , mMaxX(std::numeric_limits<double>::quiet_NaN())
  , mMinY(std::numeric_limits<double>::quiet_NaN())
  , mMaxY(std::numeric_limits<double>::quiet_NaN())
  , mMinZ(std::numeric_limits<double>::quiet_NaN())
  , mMaxZ(std::numeric_limits<double>::quiet_NaN())
  , mpMatrix(NULL)
  , mLogZ(false)
  , mLimitZ(std::numeric_limits<double>::quiet_NaN())
  , mBilinear(true)
{

}

CSpectorgramData::CSpectorgramData(const CVector<double> &x,
                                   const CVector<double> &y,
                                   const CVector<double> &z,
                                   size_t size,
                                   bool logZ,
                                   double limitZ,
                                   bool bilinear)
#if QWT_VERSION > 0x060000
  : QwtRasterData()
#else
  : QwtRasterData(QwtDoubleRect(0, 0, 100, 100))
#endif
  , mpX(x.array())
  , mpY(y.array())
  , mpZ(z.array())
  , mSize(size)
  , mMaxSize(x.size())
  , mLastRectangle(0)
  , mMinX(std::numeric_limits<double>::quiet_NaN())
  , mMaxX(std::numeric_limits<double>::quiet_NaN())
  , mMinY(std::numeric_limits<double>::quiet_NaN())
  , mMaxY(std::numeric_limits<double>::quiet_NaN())
  , mMinZ(std::numeric_limits<double>::quiet_NaN())
  , mMaxZ(std::numeric_limits<double>::quiet_NaN())
  , mpMatrix(NULL)
  , mLogZ(logZ)
  , mLimitZ(limitZ)
  , mBilinear(bilinear)

{

}


CSpectorgramData::CSpectorgramData(const CSpectorgramData& other)
#if QWT_VERSION > 0x060000
  : QwtRasterData()
#else
  : QwtRasterData(other)
#endif
  , mpX(other.mpX)
  , mpY(other.mpY)
  , mpZ(other.mpZ)
  , mSize(other.mSize)
  , mMaxSize(other.mMaxSize)
  , mLastRectangle(other.mLastRectangle)
  , mMinX(other.mMinX)
  , mMaxX(other.mMaxX)
  , mMinY(other.mMinY)
  , mMaxY(other.mMaxY)
  , mMinZ(other.mMinZ)
  , mMaxZ(other.mMaxZ)
  , mpMatrix(NULL)
  , mLogZ(other.mLogZ)
  , mLimitZ(other.mLimitZ)
  , mBilinear(other.mBilinear)
{

}

CSpectorgramData &CSpectorgramData::operator =(const CSpectorgramData &rhs)
{
  if (&rhs == this) return *this;

  mpX = rhs.mpX;
  mpY = rhs.mpY;
  mpZ = rhs.mpZ;

  mSize = rhs.mSize;
  mMaxSize = rhs.mMaxSize;
  mLastRectangle = rhs.mLastRectangle;

  mMinX = rhs.mMinX;
  mMaxX = rhs.mMaxX;
  mMinY = rhs.mMinY;
  mMaxY = rhs.mMaxY;
  mMinZ = rhs.mMinZ;
  mMaxZ = rhs.mMaxZ;

  mpMatrix = NULL;

  mLogZ = rhs.mLogZ;
  mLimitZ = rhs.mLimitZ;
  mBilinear = rhs.mBilinear;

  return *this;
}


CSpectorgramData::~CSpectorgramData()
{
  pdelete(mpMatrix);
}

QwtRasterData *CSpectorgramData::copy() const
{
  CSpectorgramData * pCopy = new CSpectorgramData(*this);

  return pCopy;
}

void
CSpectorgramData::calculateExtremes() const
{
  if (mLastRectangle == mSize)
    return;

  const double *xIt = mpX + mLastRectangle;
  const double *yIt = mpY + mLastRectangle;
  const double *zIt = mpZ + mLastRectangle;
  const double *end = mpX + mSize;

  mLastRectangle = mSize;

  // We have to remember whether we have an initial NaN
  bool MinXisNaN = isnan(mMinX);
  bool MaxXisNaN = isnan(mMaxX);
  bool MinYisNaN = isnan(mMinY);
  bool MaxYisNaN = isnan(mMaxY);
  bool MinZisNaN = isnan(mMinZ);
  bool MaxZisNaN = isnan(mMaxZ);

  while (xIt < end)
    {
      const double xv = *xIt++;

      if (!isnan(xv))
        {
          if (xv < mMinX || MinXisNaN)
            {
              mMinX = xv;
              MinXisNaN = false;
            }

          if (xv > mMaxX || MaxXisNaN)
            {
              mMaxX = xv;
              MaxXisNaN = false;
            }
        }

      double yv = *yIt++;

      if (!isnan(yv))
        {
          if (yv < mMinY || MinYisNaN)
            {
              mMinY = yv;
              MinYisNaN = false;
            }

          if (yv > mMaxY || MaxYisNaN)
            {
              mMaxY = yv;
              MaxYisNaN = false;
            }
        }

      double zv = *zIt++;

      if (!isnan(zv))
        {
          if (zv < mMinZ || MinZisNaN)
            {
              mMinZ = zv;
              MinZisNaN = false;
            }

          if (zv > mMaxZ || MaxZisNaN)
            {
              mMaxZ = zv;
              MaxZisNaN = false;
            }
        }
    }

  if (isnan(mMinX + mMaxX + mMinY + mMaxY))
    return;

  // We need to avoid very small data ranges (absolute and relative)
  C_FLOAT64 minRange = fabs(mMinX + mMaxX) * 5.e-5 + std::numeric_limits< C_FLOAT64 >::min() * 100.0;

  if (mMaxX - mMinX < minRange)
    {
      mMinX = mMinX - minRange * 0.5;
      mMaxX = mMaxX + minRange * 0.5;
    }

  minRange = fabs(mMinY + mMaxY) * 5e-5 + std::numeric_limits< C_FLOAT64 >::min() * 100.0;

  if (mMaxY - mMinY < minRange)
    {
      mMinY = mMinY - minRange * 0.5;
      mMaxY = mMaxY + minRange * 0.5;
    }
}


QwtDoubleRect CSpectorgramData::boundingRect() const
{
  if (mSize <= 0)
    return QwtDoubleRect(1.0, 1.0, -0.1, -0.1); // invalid

  calculateExtremes();

  if (isnan(mMinX + mMaxX + mMinY + mMaxY))
    return QwtDoubleRect(1.0, 1.0, -0.1, -0.1); // invalid

  return QwtDoubleRect(mMinX, mMinY, mMaxX - mMinX, mMaxY - mMinY);
}

QwtDoubleInterval CSpectorgramData::range() const
{
  if (mSize == 0)
    return QwtDoubleInterval(0, 1);

  if (mLimitZ == mLimitZ)
    return QwtDoubleInterval(mMinZ, std::min(mMaxZ, mLimitZ));

  return QwtDoubleInterval(mMinZ, mMaxZ);
}

size_t CSpectorgramData::size() const
{
  return 2 * mSize;
}

double
CSpectorgramData::bilinearAround(int xIndex, int yIndex,
                                 double x,
                                 double y) const
{
  int dimX = mValuesX.size();
  int dimY = mValuesY.size();

  double x1 = mValuesX[xIndex];
  double y1 = mValuesY[yIndex];
  double diffXX1 = x - x1;
  double diffYY1 = y - y1;
  int xNeighbor = xIndex +
                  (std::signbit(diffXX1) ? -1 : 1);

  if (xNeighbor >= dimX) xNeighbor = dimX - 1;

  if (xNeighbor < 0) xNeighbor = 0;

  int yNeighbor = yIndex +
                  (std::signbit(diffYY1) ? -1 : 1);

  if (yNeighbor >= dimY) yNeighbor = dimY - 1;

  if (yNeighbor < 0) yNeighbor = 0;

  double x2 = mValuesX[xNeighbor];
  double y2 = mValuesY[yNeighbor];

  double x1y1 = (*mpMatrix)(xIndex, yIndex);

  if (x2 == x1 || y2 == y1) return  x1y1;

  double x2y1 = (*mpMatrix)(xNeighbor, yIndex);
  double x1y2 = (*mpMatrix)(xIndex, yNeighbor);
  double x2y2 = (*mpMatrix)(xNeighbor, yNeighbor);


  return
    ((y2 - y) / (y2 - y1))
    * ((x2 - x) / (x2 - x1) * x1y1 + (x - x1) / (x2 - x1) * x2y1)
    + ((y - y1) / (y2 - y1))
    * ((x2 - x) / (x2 - x1) * x1y2 + (x - x1) / (x2 - x1) * x2y2);

}

bool
CSpectorgramData::getBilinear() const
{
  return mBilinear;
}

void
CSpectorgramData::setBilinear(bool bilinear)
{
  mBilinear = bilinear;
}

double
CSpectorgramData::value(double x, double y) const
{

  if (mpMatrix == NULL || mSize == 0 || mValuesX.size() < 2 || mValuesY.size() < 2)
    return 0;

  double distanceX = fabs(mValuesX[1] - mValuesX[0]);
  double distanceY = fabs(mValuesY[1] - mValuesY[0]);

  int xpos = 0;
  int ypos = 0;
  std::vector<double>::const_iterator curX = mValuesX.begin();

  for (; curX != mEndX; ++curX)
    {
      if (fabs(*curX - x) < distanceX)
        break;

      ++xpos;
    }

  std::vector<double>::const_iterator curY = mValuesY.begin();

  for (; curY != mEndY; ++curY)
    {
      if (fabs(*curY - y) < distanceY)
        break;

      ++ypos;
    }

  if (xpos == mValuesX.size()) --xpos;

  if (ypos == mValuesY.size()) --ypos;

  double value = mBilinear
                 ? bilinearAround(xpos, ypos, x, y)
                 : mpMatrix->operator()(xpos, ypos);

  if (value != value)
    return 0;

  if (mLimitZ == mLimitZ && value > mLimitZ)
    return mLimitZ;

  if (mLogZ && value > 0)
    return log(value);

  return value;

}

void
CSpectorgramData::setSize(const size_t &size)
{
  mSize = size;

  initializeMatrix();

#if QWT_VERSION > 0x060000
  boundingRect();
  setInterval(Qt::XAxis, QwtInterval(mMinX, mMaxX) );
  setInterval(Qt::YAxis, QwtInterval(mMinY, mMaxY) );
  if (mLimitZ == mLimitZ)
    setInterval(Qt::ZAxis, QwtInterval(mMinZ, std::min(mMaxZ, mLimitZ)) );
  else 
    setInterval(Qt::ZAxis, QwtInterval(mMinZ, mMaxZ) );
#else
  setBoundingRect(boundingRect());
#endif

  assert(mSize <= mMaxSize);
}

void
CSpectorgramData::reallocated(const CVector<double> *pX, const CVector<double> *pY, const CVector<double> *pZ)
{
  mpX = pX->array();
  mpY = pY->array();
  mpZ = pZ->array();
  mMaxSize = pX->size();

  assert(mSize <= mMaxSize);

}

void
CSpectorgramData::initializeMatrix()
{
  if (mSize == 0) return;

  pdelete(mpMatrix);

  calculateExtremes();

  const double *xIt = mpX;
  const double *yIt = mpY;
  const double *end = mpX + mSize;
  const double *endY = mpY + mSize;


  mValuesX.clear();
  mValuesY.clear();

  for (; xIt != end; ++xIt)
    {
      double current = *xIt;

      if (current != current)
        continue;

      if (std::find(mValuesX.begin(), mValuesX.end(), current) != mValuesX.end())
        continue;

      mValuesX.push_back(current);
    }

  for (; yIt != endY; ++yIt)
    {
      double current = *yIt;

      if (current != current)
        continue;

      if (std::find(mValuesY.begin(), mValuesY.end(), current) != mValuesY.end())
        continue;

      mValuesY.push_back(current);
    }

  std::sort(mValuesX.begin(), mValuesX.end());
  std::sort(mValuesY.begin(), mValuesY.end());

  mEndX = mValuesX.end();
  mEndY = mValuesY.end();

  mpMatrix = new CMatrix<double>(mValuesX.size(), mValuesY.size());
  *mpMatrix = std::numeric_limits<double>::quiet_NaN();

  xIt = mpX;
  yIt = mpY;
  const double *zIt = mpZ;

  std::vector<double>::const_iterator curX;
  std::vector<double>::const_iterator curY;

  for (; xIt != end; ++xIt, ++yIt, ++zIt)
    {
      double dX = *xIt; double dY = *yIt;
      curX = std::find(mValuesX.begin(), mEndX, *xIt);

      if (curX == mEndX) continue;

      curY = std::find(mValuesY.begin(), mEndY, *yIt);

      if (curY == mEndY) continue;

      int xpos = curX - mValuesX.begin();
      int ypos = curY - mValuesY.begin();


      (*mpMatrix)(xpos, ypos) = *zIt;

    }

}
double
CSpectorgramData::getLimitZ() const
{
  return mLimitZ;
}

void
CSpectorgramData::setLimitZ(double limitZ)
{
  mLimitZ = limitZ;
}


CPlotSpectogram::CPlotSpectogram(QMutex *pMutex,
                                 const CPlotItem::Type &type,
                                 const COutputInterface::Activity &activity,
                                 const QString &title,
                                 bool logZ,
                                 double limitZ,
                                 bool bilinear)
  : QwtPlotSpectrogram(title)
  , mpMutex(pMutex)
  , mType(type)
  , mActivity(activity)
  , mLogZ(logZ)
  , mLimitZ(limitZ)
  , mBilinear(bilinear)
{

}

void CPlotSpectogram::setDataSize(const size_t &size)
{
#if QWT_VERSION > 0x060000

  switch (mType)
    {

      case CPlotItem::spectogram:
	{
          QwtRasterData *pData = const_cast<QwtRasterData *>(data());
          static_cast< CSpectorgramData * >(pData)->setSize(size);
          break;
	}
      default:
        fatalError();
        break;
    }

#else

  switch (mType)
    {

      case CPlotItem::spectogram:
      {
        QwtRasterData *pData = const_cast<QwtRasterData *>(&data());
        static_cast<CSpectorgramData *>(pData)->setSize(size);
        break;
      }

      default:
        fatalError();
        break;
    }

#endif
}

void CPlotSpectogram::reallocatedData(const CVector<double> *pX,
                                      const CVector<double> *pY,
                                      const CVector<double> *pZ)
{
#if QWT_VERSION > 0x060000

  switch (mType)
    {
      case CPlotItem::spectogram:
	{
          QwtRasterData *pData = const_cast<QwtRasterData *>(data());
          static_cast< CSpectorgramData * >(pData)->reallocated(pX, pY, pZ);
          break;
	}
      default:
        fatalError();
        break;
    }

#else

  switch (mType)
    {
      case CPlotItem::spectogram:
      {
        QwtRasterData *pData = const_cast<QwtRasterData *>(&data());
        static_cast<CSpectorgramData *>(pData)->reallocated(pX, pY, pZ);
        break;
      }

      default:
        fatalError();
        break;
    }

#endif
}

const CPlotItem::Type &
CPlotSpectogram::getType() const
{
  return mType;
}

const COutputInterface::Activity &
CPlotSpectogram::getActivity() const
{
  return mActivity;
}

bool
CPlotSpectogram::getLogZ() const
{
  return mLogZ;
}

void
CPlotSpectogram::setLogZ(bool logZ)
{
  mLogZ = logZ;
}

double
CPlotSpectogram::getLimitZ() const
{
  return mLimitZ;
}

void
CPlotSpectogram::setLimitZ(double limitZ)
{
  mLimitZ = limitZ;
}
bool CPlotSpectogram::getBilinear() const
{
  return mBilinear;
}

void CPlotSpectogram::setBilinear(bool bilinear)
{
  mBilinear = bilinear;
}




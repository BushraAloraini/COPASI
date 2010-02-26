// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/plotUI/Attic/CHistogram.cpp,v $
//   $Revision: 1.10.2.1 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2010/02/26 16:47:30 $
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

#include <float.h>
#include <math.h>
#include "CHistogram.h"

//  public:

CHistogram::CHistogram():
    mIncrement(0.1),
    mUptodate(true),
    mXArray(NULL),
    mYArray(NULL),
    mArraySize(0),
    mMap(),
    mCount(0)
{
  mInvIncrement = 1 / mIncrement;
}

CHistogram::CHistogram(C_FLOAT64 incr):
    mIncrement(incr),
    mUptodate(true),
    mXArray(NULL),
    mYArray(NULL),
    mArraySize(0),
    mMap(),
    mCount(0)
{
  mInvIncrement = 1 / mIncrement;
}

CHistogram::~CHistogram()
{
  if (mXArray)
    delete[] mXArray;

  if (mYArray)
    delete[] mYArray;
}

void CHistogram::addValue(const C_FLOAT64 & val)
{
  //just ignore breaks. Later we perhaps want to start a new histogram...
  if (isnan(val)) //NaN
    return;

  if (val >= DBL_MAX || val <= -DBL_MAX)
    return;

  mUptodate = false;

  mMap[(C_INT32)floor(val * mInvIncrement)]++;
  mMap[(((C_INT32)floor(val * mInvIncrement))) + 1];
  ++mCount;
}

const double* CHistogram::getXArray()
{
  if (!mUptodate)
    updateArray();

  return mXArray;
}

const double* CHistogram::getYArray()
{
  if (!mUptodate)
    updateArray();

  return mYArray;
}

int CHistogram::size()
{
  if (!mUptodate)
    updateArray();

  return mArraySize;
}

//private
void CHistogram::updateArray()
{
  if (mUptodate)
    return; //do nothing

  //delete Arrays
  if (mXArray)
    delete[] mXArray;

  if (mYArray)
    delete[] mYArray;

  //construct arrays
  mXArray = new double[mMap.size() + 1];
  mYArray = new double[mMap.size() + 1];
  mArraySize = mMap.size() + 1;

  C_FLOAT64 tmpFactor = 1 / (mCount * mIncrement);

  //add one bin to the left
  if (mMap.size() > 0)
    {
      mXArray[0] = (mMap.begin()->first - 1) * mIncrement;
    }
  else
    {
      mXArray[0] = 0.0;
    }

  mYArray[0] = 0.0;

  C_INT32 i;
  std::map<C_INT32, C_INT32>::const_iterator it, itEnd = mMap.end();

  for (it = mMap.begin(), i = 1; it != itEnd; ++it, ++i)
    {
      //TODO use pointer increments instead of [...]
      mXArray[i] = it->first * mIncrement;
      mYArray[i] = (double)it->second * tmpFactor;
    }

  mUptodate = true;
}

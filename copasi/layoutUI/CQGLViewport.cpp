// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/layoutUI/CQGLViewport.cpp,v $
//   $Revision: 1.6 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2008/12/18 17:41:15 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

#include "CQGLViewport.h"

#include <qgl.h>
#include <qscrollbar.h>
#include <qrect.h>
#include <q3hbox.h>
#include <qlayout.h>
//Added by qt3to4:
#include <QResizeEvent>
#include <Q3VBoxLayout>
#include <Q3Frame>

#include <iostream>

#include "copasi/layout/CLBase.h"
#include "copasi/layout/CLayout.h"
#include "CQGLNetworkPainter.h"

/**
 * Constructor.
 */
CQGLViewport::CQGLViewport(QWidget* pParent, const char* name, Qt::WFlags f): Q3Frame(pParent, name, f)
{
  Q3VBoxLayout* pVBoxLayout = new Q3VBoxLayout(this);
  Q3HBox* pHBox = new Q3HBox(this);
  pVBoxLayout->addWidget(pHBox);
  QGLFormat format;
  format.setDoubleBuffer(TRUE);
  mpNetworkPainter = new CQGLNetworkPainter(format, pHBox, "gl graph");
  mpVerticalScrollbar = new QScrollBar(Qt::Vertical, pHBox, "vertical scrollbar");
  mpVerticalScrollbar->setLineStep(1);
  mpHorizontalScrollbar = new QScrollBar(Qt::Horizontal, this, "horizontal scrollbar");
  mpHorizontalScrollbar->setLineStep(1);
  pVBoxLayout->addWidget(mpHorizontalScrollbar);
  connect(this->mpVerticalScrollbar, SIGNAL(valueChanged(int)), this, SLOT(slotVValueChanged(int)));
  connect(this->mpHorizontalScrollbar, SIGNAL(valueChanged(int)), this, SLOT(slotHValueChanged(int)));
}

/**
 * Destructor.
 */
CQGLViewport::~CQGLViewport()
{}

void CQGLViewport::resizeEvent(QResizeEvent* e)
{
  this->updateScrollbars();
  Q3Frame::resizeEvent(e);
}

const CQGLNetworkPainter* CQGLViewport::getPainter() const
  {
    return this->mpNetworkPainter;
  }

CQGLNetworkPainter* CQGLViewport::getPainter()
{
  return this->mpNetworkPainter;
}

void CQGLViewport::createGraph(CLayout* IP)
{
  this->mpNetworkPainter->createGraph(IP);
  this->updateScrollbars();
}

void CQGLViewport::setZoomFactor(C_FLOAT64 zoom)
{
  // TODO make sure the GL window is only redrawn once
  this->mpNetworkPainter->setZoomFactor(zoom);
  this->updateScrollbars();
  this->mpNetworkPainter->update();
}

void CQGLViewport::updateScrollbars()
{
  // reset the scollbar range
  // TODO check te setting for the scroll range since there seem to be some
  // error messages
  // disconnect the scrollbar listeners and handle the update so that the GL
  // window is only redrawn once
  double zoom = this->mpNetworkPainter->getZoomFactor();
  CLPoint max = this->mpNetworkPainter->getGraphMax();
  CLPoint min = this->mpNetworkPainter->getGraphMin();
  double graphWidth = (max.getX() - min.getX()) * zoom;
  double graphHeight = (max.getY() - min.getY()) * zoom;
  double rectangleHeight = this->contentsRect().height();
  double rectangleWidth = this->contentsRect().width();
  if (graphHeight < rectangleHeight)
    {
      this->mpVerticalScrollbar->hide();
      this->mpVerticalScrollbar->setValue(0);
    }
  else
    {
      this->mpVerticalScrollbar->setPageStep(rectangleHeight);
      this->mpVerticalScrollbar->setRange(0, (unsigned int)(graphHeight - rectangleHeight));
      this->mpVerticalScrollbar->show();
      this->mpNetworkPainter->update();
    }
  if (graphWidth < rectangleWidth)
    {
      this->mpHorizontalScrollbar->hide();
      this->mpHorizontalScrollbar->setValue(0);
    }
  else
    {
      this->mpHorizontalScrollbar->setPageStep(rectangleWidth);
      this->mpHorizontalScrollbar->setRange(0, (unsigned int)(graphWidth - rectangleWidth));
      this->mpHorizontalScrollbar->show();
      this->mpNetworkPainter->update();
    }
}

void CQGLViewport::slotVValueChanged(int value)
{
  CLPoint p = this->mpNetworkPainter->getGraphMin();
  double zoom = this->mpNetworkPainter->getZoomFactor();
  this->mpNetworkPainter->setCurrentPositionY((double)(p.getY() + value / zoom));
}

void CQGLViewport::slotHValueChanged(int value)
{
  CLPoint p = this->mpNetworkPainter->getGraphMin();
  double zoom = this->mpNetworkPainter->getZoomFactor();
  this->mpNetworkPainter->setCurrentPositionX((double)(p.getX() + value / zoom));
}

void CQGLViewport::resetView()
{
  // TODO disconnect the scrollbar listeners
  // so that the display is only redrawn once
  this->mpNetworkPainter->resetView();
  this->updateScrollbars();
}

void CQGLViewport::updateWidget()
{
  this->mpNetworkPainter->update();
}

bool CQGLViewport::isCircleMode() const
  {
    return this->mpNetworkPainter->isCircleMode();
  }

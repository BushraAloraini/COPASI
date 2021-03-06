// Copyright (C) 2017 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and University of
// of Connecticut School of Medicine.
// All rights reserved.

// Copyright (C) 2012 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

#include "CQPlotEditWidget.h"

#include "copasi.h"

#include "model/CModel.h"

CQPlotEditWidget::CQPlotEditWidget(QWidget* parent /*= NULL*/ , Qt::WindowFlags fl /*= 0*/)
  : QWidget(parent, fl)
  , mpModel(NULL)
{
}

CQPlotEditWidget::~CQPlotEditWidget()
{
}

void CQPlotEditWidget::setModel(const CModel * model)
{
  mpModel = model;
}

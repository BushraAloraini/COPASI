// Copyright (C) 2017 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and University of
// of Connecticut School of Medicine.
// All rights reserved.

// Copyright (C) 2010 - 2016 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2008 - 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2004 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#include "copasi/copasi.h"

#include "CPlotSpecification.h"

#include "model/CModel.h"
#include "copasi/core/CDataObjectReference.h"

// static
CPlotSpecification * CPlotSpecification::fromData(const CData & data)
{
  return new CPlotSpecification(data.getProperty(CData::OBJECT_NAME).toString(),
                                NO_PARENT,
                                (CPlotSpecification::Type) data.getProperty(CData::PLOT_TYPE).toUint());
}

// virtual
CData CPlotSpecification::toData() const
{
  CData Data;

  // TODO CRITICAL Implement me!
  fatalError();

  return Data;
}

// virtual
bool CPlotSpecification::applyData(const CData & data)
{
  bool success = true;

  // TODO CRITICAL Implement me!
  fatalError();

  return success;
}

CPlotSpecification::CPlotSpecification(const std::string & name,
                                       const CDataContainer * pParent,
                                       const CPlotSpecification::Type & type):
  CPlotItem(name, pParent, type),
  items("Curves", this),
  mActive(true)
{initObjects();}

CPlotSpecification::CPlotSpecification(const CPlotSpecification & src,
                                       const CDataContainer * pParent):
  CPlotItem(src, pParent),
  items(src.getItems(), this),
  mActive(src.mActive)
{initObjects();}

CPlotSpecification::~CPlotSpecification() {}

void CPlotSpecification::cleanup()
{
  items.cleanup();
  this->CPlotItem::cleanup();
}

void CPlotSpecification::initObjects()
{
  //  CDataContainer::addObjectReference("Active", mActive, CDataObject::ValueBool);
}

//*************************************

void CPlotSpecification::setActive(const bool & active)
{mActive = active;}

const bool & CPlotSpecification::isActive() const
{return mActive;}

bool CPlotSpecification::isLogX() const
{
  return getValue< bool >("log X");
}

bool CPlotSpecification::isLogY() const
{
  return getValue< bool >("log Y");
}

void CPlotSpecification::setLogX(bool l)
{
  setValue("log X", l);
}

void CPlotSpecification::setLogY(bool l)
{
  setValue("log Y", l);
}

//*************************************

CPlotItem* CPlotSpecification::createItem(const std::string & name, CPlotItem::Type type)
{
  CPlotItem * itm = new CPlotItem(name, NULL, type);

  if (!items.add(itm, true))
    {
      delete itm;
      return NULL;
    }

  return itm;
}

bool CPlotSpecification::createDefaultPlot(const CModel* model)
{
  mActive = true;

  //TODO cleanup before?
  //title = "Default Data Plot 2D";

  CPlotItem * plItem;
  std::string itemTitle;
  CPlotDataChannelSpec name2;
  const CDataObject * tmp;

  CPlotDataChannelSpec name1 = model->getValueReference()->getCN();

  size_t i, imax = model->getMetabolites().size();

  for (i = 0; i < imax; ++i)
    {
      tmp = model->getMetabolites()[i].getConcentrationReference();
      name2 = tmp->getCN();
      itemTitle = tmp->getObjectDisplayName();

      plItem = this->createItem(itemTitle, CPlotItem::curve2d);
      plItem->addChannel(name1);
      plItem->addChannel(name2);
    }

  return true; //TODO: really check;
}

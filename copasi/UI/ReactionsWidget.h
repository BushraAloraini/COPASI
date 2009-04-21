/* Begin CVS Header
 $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/Attic/ReactionsWidget.h,v $
 $Revision: 1.29 $
 $Name:  $
 $Author: shoops $
 $Date: 2009/04/21 16:20:31 $
 End CVS Header */

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#ifndef REACTIONS_WIDGET_H
#define REACTIONS_WIDGET_H

#include "UI/CopasiTableWidget.h"

class ReactionsWidget : public CopasiTableWidget
{
  Q_OBJECT

public:
  ReactionsWidget(QWidget *parent, const char * name = 0, Qt::WFlags f = 0)
      : CopasiTableWidget(parent, false, name, f)
  {init();}

  virtual void setFramework(int framework);

protected:

  /**
   * This initializes the widget
   */
  virtual void init();

  /**
   * shows units for the quantities
   */
  virtual void updateHeaderUnits();

  /**
   * returns a list of objects that should be displayed
   */
  virtual std::vector<const CCopasiObject*> getObjects() const;

  /**
   * fills one table row with the data from one object
   */
  virtual void tableLineFromObject(const CCopasiObject* obj, unsigned C_INT32 row);

  /**
   * reads the contents of one row of the table and writes it to the object
   */
  virtual void tableLineToObject(unsigned C_INT32 row, CCopasiObject* obj);

  /**
   * creates a new object
   */
  virtual CCopasiObject* createNewObject(const std::string & name);

  /**
   * deletes objects. Performs all additional tasks, like asking the user, ...
   */
  virtual void deleteObjects(const std::vector<std::string> & keys);

  /**
   * this is used to fill a row of the table when a new object is added to the table.
   * it fills only the data columns, not the name. It should not fill column exc.
   */
  virtual void defaultTableLineContent(unsigned C_INT32 row, unsigned C_INT32 exc);

  /**
   * the prefix that is used to construct new object names
   */
  virtual QString defaultObjectName() const;
};

#endif

// Begin CVS Header
//   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/MIRIAMUI/Attic/CRDFListViewItem.h,v $
//   $Revision: 1.6 $
//   $Name:  $
//   $Author: shoops $
//   $Date: 2008/12/18 18:57:10 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

#ifndef COPASI_CRDFListViewItem
#define COPASI_CRDFListViewItem

#include <q3listview.h>

#include "copasi/MIRIAM/CRDFTriplet.h"

class CRDFListView;
class CRDFNode;
class CRDFEdge;

class CRDFListViewItem: public Q3ListViewItem
  {
    // Operations
  public:
    /**
     * Default Constructor
     * @param CRDFListView * pParent
     * @param CRDFListViewItem * pAfter (default: NULL)
     */
    CRDFListViewItem(CRDFListView * pParent, CRDFListViewItem * pAfter = NULL);

    /**
     * Specific Constructor
     * @param CRDFListViewItem * pParent
     * @param CRDFListViewItem * pAfter (default: NULL)
     */
    CRDFListViewItem(CRDFListViewItem * pParent, CRDFListViewItem * pAfter = NULL);

    /**
     * Destructor
     */
    virtual ~CRDFListViewItem();

    /**
     * Set the RDF triplet represented by this item
     * @param const CRDFTriplet & triplet
     */
    void setTriplet(const CRDFTriplet & triplet);

    // Attributes
  private:
    /**
     * The triplet represneted by this item.
     */
    CRDFTriplet mTriplet;
  };

#endif // COPASI_CRDFListViewItem

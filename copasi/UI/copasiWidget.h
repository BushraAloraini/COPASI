/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/copasiWidget.h,v $
   $Revision: 1.17 $
   $Name:  $
   $Author: shoops $
   $Date: 2006/04/27 01:27:46 $
   End CVS Header */

// Copyright � 2005 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/********************************************************
 Author: Liang Xu
 Version : 1.xx  <first>
 Description:
 Date: 04/03
 Comment : CopasiWidget: Provide base class for all widget: for resizable/minimize
 Contact: Please contact lixu1@vt.edu.
 *********************************************************/

#if !defined COPASI_WIDGET_H
#define COPASI_WIDGET_H
#include <qwidget.h>
#include "listviews.h"

class DataModelGUI;
class QResizeEvent;

class CopasiWidget : public QWidget
  {
  public:
    CopasiWidget(QWidget * parent = 0, const char * name = 0, WFlags f = 0);
    //virtual void resizeEvent (QResizeEvent *);
    virtual bool update(ListViews::ObjectType objectType, ListViews::Action action, const std::string & key);
    virtual bool leave();
    virtual bool enter(const std::string & key = "");

  public slots:
    //virtual void resize (int w, int h);

  private:
    //static int realMinWidth;
    //static int realMinHeight;
    //bool bInitialized;
    //bool bSetMinSize;

  protected:
    bool protectedNotify(ListViews::ObjectType objectType, ListViews::Action action, const std::string & key = "");
    bool mIgnoreUpdates;

    static DataModelGUI* dataModel;
    ListViews* pListView;
  };

class CopasiParametersWidget : public CopasiWidget
  {
  public:
    CopasiParametersWidget(QWidget * parent = 0, const char * name = 0, WFlags f = 0);
    virtual void viewMousePressEvent(QMouseEvent * e) = 0;
  };

#endif // !defined(COPASI_WIDGET_H)

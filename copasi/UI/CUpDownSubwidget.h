/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/UI/CUpDownSubwidget.h,v $
   $Revision: 1.5 $
   $Name:  $
   $Author: shoops $ 
   $Date: 2005/10/07 16:40:39 $
   End CVS Header */

/****************************************************************************
 ** Form interface generated from reading ui file 'CUpDownSubwidget.ui'
 **
 ** Created: Fri Oct 7 11:58:54 2005
 **      by: The User Interface Compiler ($Id: CUpDownSubwidget.h,v 1.5 2005/10/07 16:40:39 shoops Exp $)
 **
 ** WARNING! All changes made in this file will be lost!
 ****************************************************************************/

#ifndef CUPDOWNSUBWIDGET_H
 #define CUPDOWNSUBWIDGET_H

#include <qvariant.h>
 #include <qpixmap.h>
 #include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QToolButton;

class CUpDownSubwidget : public QWidget
  {
    Q_OBJECT

  public:
    CUpDownSubwidget(QWidget* parent = 0, const char* name = 0, WFlags fl = 0);
    ~CUpDownSubwidget();

    QToolButton* toolButtonUp;
    QToolButton* toolButtonDel;
    QToolButton* toolButtonDown;

    virtual int getIndex() const;

  public slots:
    virtual void setIndex(int index, bool isFirst, bool isLast);

  signals:
    void up(int);
    void down(int);
    void del(int);

  protected:
    int mIndex;

    QVBoxLayout* CUpDownSubwidgetLayout;
    QSpacerItem* spacer;
    QGridLayout* layout3;

  protected slots:
    virtual void languageChange();

    virtual void slotUp();
    virtual void slotDown();
    virtual void slotDel();

  private:
    QPixmap image0;
    QPixmap image1;
    QPixmap image2;

    void init();
  };

#endif // CUPDOWNSUBWIDGET_H

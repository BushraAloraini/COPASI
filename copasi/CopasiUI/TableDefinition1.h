/* Begin CVS Header
   $Source: /Volumes/Home/Users/shoops/cvs/copasi_dev/copasi/CopasiUI/Attic/TableDefinition1.h,v $
   $Revision: 1.13 $
   $Name:  $
   $Author: shoops $
   $Date: 2006/04/27 01:27:46 $
   End CVS Header */

// Copyright � 2005 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

/****************************************************************************
 **
 ** Created: Wed Aug 6 22:43:06 2003
 **      by: Liang Xu
($Id: TableDefinition1.h,v 1.13 2006/04/27 01:27:46 shoops Exp $)
 **
 ** WARNING! All changes made in this file will be lost!
 ****************************************************************************/

#ifndef TableDefinition1_H
#define TableDefinition1_H

#include "copasi.h"
#include <string>
#include <qvariant.h>
#include <qwidget.h>
#include <qlistbox.h>
#include "copasiWidget.h"

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QLabel;
class QCheckBox;
class QLineEdit;
class QFrame;
class QPushButton;
class QRadioButton;
class QComboBox;
class CModel;
class CReport;
class QString;

//this is a listBoxItem that stores an additional string (to allow a display name that
//is different from the CN)
class MyListBoxItem : public QListBoxText
  {
  private:
    std::string mCN;

  public:
    MyListBoxItem(QListBox * listbox, const QString & text, const std::string & cn)
        : QListBoxText(listbox, text)
    {mCN = cn;}

    const std::string & getCN() const {return mCN;};
    void setCN(const std::string & cn) {mCN = cn;};
  };

class TableDefinition1 : public CopasiWidget
  {
    Q_OBJECT
  protected slots:
    //virtual void nameTextChanged(const QString&);
    virtual void commentTextChanged(const QString&);
    virtual void separatorTextChanged(const QString&);
    virtual void tabButtonClicked();
    virtual void titleButtonClicked();

    virtual void languageChange();
    virtual void slotBtnCancelClicked();
    virtual void slotBtnConfirmClicked();

    virtual void addButtonClicked();
    virtual void deleteButtonClicked();
    virtual void upButtonClicked();
    virtual void downButtonClicked();

    virtual void newReportClicked();
    virtual void delReportClicked();

    virtual void comboTaskChanged(const QString & string);

  public:
    TableDefinition1(QWidget* parent = 0, const char* name = 0, WFlags fl = 0);
    ~TableDefinition1();

  private:
    CModel *mModel;
    inline void setModel(CModel* pNewModel)
    {mModel = pNewModel;}

    CReport *mReport;

    std::string reportKey;
    inline void setKey(const std::string pNewKey)
    {reportKey = pNewKey;}

    QListBox* itemsTable;

    void loadTableDefinition1();

  public:
    virtual bool update(ListViews::ObjectType objectType, ListViews::Action action, const std::string & key);
    virtual bool leave();
    virtual bool enter(const std::string & key = "");

  private:
    QFrame* bodyField;
    QPushButton* confirmButton;
    QPushButton* cancelButton;
    QPushButton* newReportButton;
    QPushButton* delReportButton;
    QLabel* itemsLabel;
    QPushButton* upButton;
    QPushButton* downButton;
    QPushButton* deleteButton;
    QPushButton* addButton;
    //    QTable* itemsTable;
    QFrame* frame4;
    QCheckBox* tabChecked;
    QLineEdit* separatorEdit;
    QLabel* separatorLabel;
    QComboBox* comboTask;
    QFrame* line4;
    QLabel* taskLabel;
    QFrame* frame5;
    QLineEdit* commentEdit;
    QLabel* commentLabel;
    QLabel* targetLabel;
    QLineEdit* nameEdit;
    QCheckBox* titleChecked;

  protected:
    QGridLayout* TableDefinitionLayout;
    QHBoxLayout* layout14;
    QHBoxLayout* layout7;
    QVBoxLayout* layout6;
    QGridLayout* layout5;
    QGridLayout* frame4Layout;
    QGridLayout* frame5Layout;
    QWidget* pParent;
    bool bUpdated;
  };

#endif // TableDefinition1_H

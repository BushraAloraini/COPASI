// Copyright (C) 2010 - 2013 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., University of Heidelberg, and The University
// of Manchester.
// All rights reserved.

// Copyright (C) 2009 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

#include <QComboBox>
#include <QtGui/QSortFilterProxyModel>

#include "CQComboDelegate.h"

#include "copasi.h"

CQComboDelegate::CQComboDelegate(const QStringList *pComboItems, QObject *parent):
  QItemDelegate(parent),
  mpComboItems(pComboItems),
  mEditorToIndex(),
  mRowToItems()
{}

CQComboDelegate::~CQComboDelegate()
{}

QWidget *CQComboDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem & C_UNUSED(option),
                                       const QModelIndex & index) const
{
  QModelIndex  SourceIndex = index;
  const QAbstractItemModel *pModel = index.model();

  while (pModel->inherits("QSortFilterProxyModel"))
    {
      SourceIndex = static_cast< const QSortFilterProxyModel *>(pModel)->mapToSource(index);
      pModel = SourceIndex.model();
    }

  QComboBox *pEditor = new QComboBox(parent);

  if (mpComboItems != NULL)
    {
      pEditor->addItems(*mpComboItems);
    }
  else
    {
      QMap< int, const QStringList * >::const_iterator found = mRowToItems.find(SourceIndex.row());

      if (found != mRowToItems.end() && found.value() != NULL)
        {
          pEditor->addItems(*found.value());
        }
    }

  mEditorToIndex[pEditor] = SourceIndex;

  connect(pEditor, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCurrentIndexChanged(int)));
  connect(pEditor, SIGNAL(destroyed(QObject *)), this, SLOT(slotEditorDeleted(QObject *)));

  return pEditor;
}

void CQComboDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
  QString value = index.model()->data(index, Qt::DisplayRole).toString();
  QComboBox *comboBox = static_cast<QComboBox*>(editor);
  comboBox->setCurrentIndex(comboBox->findText(value));
}

void CQComboDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
  QComboBox *comboBox = static_cast<QComboBox*>(editor);
  QVariant value(comboBox->currentText());
  model->setData(index, value, Qt::EditRole);
}

void CQComboDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex & C_UNUSED(index)) const
{
  editor->setGeometry(option.rect);
}

void CQComboDelegate::setItems(int row, const QStringList* pComboItems)
{
  mRowToItems[row] = pComboItems;
}

void CQComboDelegate::slotCurrentIndexChanged(int index)
{
  QComboBox * pEditor = dynamic_cast< QComboBox * >(sender());

  if (pEditor)
    {
      QMap< QWidget * , QModelIndex >::const_iterator found = mEditorToIndex.find(pEditor);

      if (found != mEditorToIndex.end())
        {
          emit currentIndexChanged(found.value().row(), index);
        }
    }
}

void CQComboDelegate::slotEditorDeleted(QObject * pObject)
{
  mEditorToIndex.erase(static_cast< QWidget * >(pObject));
}

CQIndexComboDelegate::CQIndexComboDelegate(const QStringList *pComboItems, QObject *parent)
  : CQComboDelegate(pComboItems, parent)
{}

CQIndexComboDelegate::~CQIndexComboDelegate()
{}

void CQIndexComboDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                        const QModelIndex &index) const
{
  QComboBox *comboBox = static_cast<QComboBox*>(editor);
  QVariant value(comboBox->currentIndex());
  model->setData(index, value, Qt::EditRole);
}


/*
    simplefiletreemodel.cpp

    Provides a simple tree model for files and folders
*/

#include "simplefiletreemodel.h"

#include <QDir>
#include <QFileIconProvider>
#include <QStringList>

SimpleFileTreeModel::SimpleFileTreeModel(const BSA::Folder::Ptr root, QObject* parent)
    : QAbstractItemModel(parent)
{
  QFileIconProvider* provider = new QFileIconProvider();
  m_FolderIcon                = provider->icon(QFileIconProvider::Folder);
  m_FileIcon                  = provider->icon(QFileIconProvider::File);
  delete provider;
  m_RootItem = std::make_unique<SimpleFileTreeItem>(root);
}

int SimpleFileTreeModel::columnCount(const QModelIndex& parent) const
{
  if (parent.isValid())
    return static_cast<SimpleFileTreeItem*>(parent.internalPointer())->columnCount();
  return m_RootItem->columnCount();
}

QVariant SimpleFileTreeModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();

  const auto* item = static_cast<SimpleFileTreeItem*>(index.internalPointer());

  if (role == Qt::DecorationRole) {
    if (item->childCount() > 0) {
      return m_FolderIcon;
    } else {
      return m_FileIcon;
    }
  }

  if (role == Qt::DisplayRole) {
    return item->data(index.column());
  }

  return {};
}

Qt::ItemFlags SimpleFileTreeModel::flags(const QModelIndex& index) const
{
  return index.isValid() ? QAbstractItemModel::flags(index)
                         : Qt::ItemFlags(Qt::NoItemFlags);
}

QVariant SimpleFileTreeModel::headerData(int section, Qt::Orientation orientation,
                                         int role) const
{
  return orientation == Qt::Horizontal && role == Qt::DisplayRole ? "Name" : QVariant{};
}

QModelIndex SimpleFileTreeModel::index(int row, int column,
                                       const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent))
    return {};

  SimpleFileTreeItem* parentItem =
      parent.isValid() ? static_cast<SimpleFileTreeItem*>(parent.internalPointer())
                       : m_RootItem.get();

  if (auto* childItem = parentItem->child(row)) {
    return createIndex(row, column, childItem);
  }

  return {};
}

QModelIndex SimpleFileTreeModel::parent(const QModelIndex& index) const
{
  if (!index.isValid()) {
    return {};
  }

  const auto* childItem = static_cast<SimpleFileTreeItem*>(index.internalPointer());
  const SimpleFileTreeItem* parentItem = childItem->parentItem();

  return parentItem != m_RootItem.get() ? createIndex(parentItem->row(), 0, parentItem)
                                        : QModelIndex{};
}

int SimpleFileTreeModel::rowCount(const QModelIndex& parent) const
{
  if (parent.column() > 0)
    return 0;

  const SimpleFileTreeItem* parentItem =
      parent.isValid()
          ? static_cast<const SimpleFileTreeItem*>(parent.internalPointer())
          : m_RootItem.get();

  return parentItem->childCount();
}

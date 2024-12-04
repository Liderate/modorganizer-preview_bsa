#ifndef SIMPLEFILETREEITEM_H
#define SIMPLEFILETREEITEM_H

#include <QVariant>
#include <QVector>

#include <bsatk.h>

class SimpleFileTreeItem
{
public:
  explicit SimpleFileTreeItem(const BSA::Folder::Ptr folder,
                              SimpleFileTreeItem* parentItem = nullptr);

  explicit SimpleFileTreeItem(const BSA::File::Ptr file,
                              SimpleFileTreeItem* parentItem = nullptr);

  void appendChild(std::unique_ptr<SimpleFileTreeItem>&& child);

  SimpleFileTreeItem* child(int row);
  int childCount() const;
  int columnCount() const;
  QVariant data(int column) const;
  int row() const;
  SimpleFileTreeItem* parentItem() const { return m_parentItem; };

private:
  void populateChildren();

private:
  BSA::Folder::Ptr m_folder = nullptr;
  BSA::File::Ptr m_file     = nullptr;
  std::string m_name;

  std::vector<std::unique_ptr<SimpleFileTreeItem>> m_childItems;
  SimpleFileTreeItem* m_parentItem;
};

#endif  // SIMPLEFILETREEITEM_H

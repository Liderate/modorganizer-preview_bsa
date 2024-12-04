

/*
    simplefiletreeitem.cpp

    A container for items of data supplied by the simple file tree model.
*/

#include "simplefiletreeitem.h"

#include <unordered_set>

SimpleFileTreeItem::SimpleFileTreeItem(const BSA::Folder::Ptr folder,
                                       SimpleFileTreeItem* parent)
    : m_folder(folder), m_name(folder->getName()), m_parentItem(parent)
{
  populateChildren();
}

SimpleFileTreeItem::SimpleFileTreeItem(const BSA::File::Ptr file,
                                       SimpleFileTreeItem* parent)
    : m_file(file), m_name(file->getName()), m_parentItem(parent)
{}

void SimpleFileTreeItem::appendChild(std::unique_ptr<SimpleFileTreeItem>&& child)
{
  m_childItems.push_back(std::move(child));
}

SimpleFileTreeItem* SimpleFileTreeItem::child(int row)
{
  if (row < 0 || row >= childCount())
    return nullptr;
  return m_childItems.at(row).get();
}

int SimpleFileTreeItem::childCount() const
{
  return int(m_childItems.size());
}

int SimpleFileTreeItem::columnCount() const
{
  return 1;
}

QVariant SimpleFileTreeItem::data(int column) const
{
  if (column < 0 || column >= columnCount())
    return QVariant();
  return QString::fromStdString(m_name);
}

int SimpleFileTreeItem::row() const
{
  if (m_parentItem == nullptr) {
    return 0;
  }
  const auto it = std::find_if(
      m_parentItem->m_childItems.cbegin(), m_parentItem->m_childItems.cend(),
      [this](const std::unique_ptr<SimpleFileTreeItem>& treeItem) {
        return treeItem.get() == this;
      });

  if (it != m_parentItem->m_childItems.cend()) {
    return std::distance(m_parentItem->m_childItems.cbegin(), it);
  }

  Q_ASSERT(false);
  return -1;
}

void SimpleFileTreeItem::populateChildren()
{
  // handle files
  const auto fileCount = m_folder->getNumFiles();
  for (unsigned int i = 0; i < fileCount; ++i) {
    appendChild(std::make_unique<SimpleFileTreeItem>(m_folder->getFile(i), this));
  }

  // Some archives seem to have two folders with the same name that contain either the
  // subfolders or files repspectively
  // The following should combine the two to an appropriate item
  std::vector<BSA::Folder::Ptr> folders;
  const auto dirCount = m_folder->getNumSubFolders();
  for (unsigned int i = 0; i < dirCount; ++i) {
    folders.push_back(m_folder->getSubFolder(i));
  }

  // sort folders that contain only files to back
  std::ranges::sort(folders, [](const BSA::Folder::Ptr& a, const BSA::Folder::Ptr b) {
    bool aIsFileFolder = (a->getNumSubFolders() == 0 && a->getNumFiles() > 0);
    bool bIsFileFolder = (b->getNumSubFolders() == 0 && b->getNumFiles() > 0);
    if (aIsFileFolder != bIsFileFolder) {
      return !aIsFileFolder;
    }
    return false;
  });

  // If a folder has the same name then we will apply all its files to the existing
  // child. This assumes that any duplicates will contain only files
  std::unordered_set<std::string> folderNames;
  for (const auto& folder : folders) {
    std::string name = folder->getName();
    if (folderNames.contains(name)) {
      auto it = std::ranges::find_if(
          m_childItems, [&name](const std::unique_ptr<SimpleFileTreeItem>& item) {
            return item->m_name == name;
          });
      if (it != m_childItems.end()) {
        const auto fileCount = folder->getNumFiles();
        for (unsigned int i = 0; i < fileCount; ++i) {
          it->get()->appendChild(
              std::make_unique<SimpleFileTreeItem>(folder->getFile(i), it->get()));
        }
      }
    } else {
      folderNames.insert(name);
      appendChild(std::make_unique<SimpleFileTreeItem>(folder, this));
    }
  }
}
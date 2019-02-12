#ifndef DIFFVIEW_HPP
#define DIFFVIEW_HPP

#include <vector>

#include <QtCore/QString>

#include <git2/oid.h>

struct git_diff;
struct git_repository;

class DiffView {
public:
  struct Line {
    int OldLinenumber;
    int NewLinenumber;
    int NumberOfLine;
    int Offset;
    int Length;
  };

  struct File {
    git_oid OldId;
    QString OldName;
    git_oid NewId;
    QString NewName;
    std::vector<Line> Lines;
  };

  explicit DiffView(git_repository* repository, git_diff* diff);
  DiffView(DiffView&& other) noexcept;
  DiffView(const DiffView& other) = delete;
  DiffView& operator=(DiffView&& other) noexcept;
  DiffView& operator=(const DiffView& other) = delete;
  ~DiffView();

  const std::vector<File>& files() const noexcept;
  std::pair<QString, QString> revisions(const File& file) const;

private:
  git_repository* Repository;
  git_diff* Diff;
  std::vector<File> Files;
};

#endif // DIFFVIEW_HPP

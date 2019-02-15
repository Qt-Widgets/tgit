#include "Diff.hpp"

#include <algorithm>
#include <cassert>

#include <git2/diff.h>

#include "Tree.hpp"

static_assert(std::is_move_assignable_v<gitpp::Diff>, "");
static_assert(std::is_move_constructible_v<gitpp::Diff>, "");

namespace {

int fileCallback(const git_diff_delta* delta, float progress, void* capture) {
  auto leftPath = delta->old_file.path;
  auto rightPath = delta->new_file.path;
  assert(leftPath != nullptr || rightPath != nullptr);

  auto files = reinterpret_cast<gitpp::DeltaList*>(capture);
  auto& file = files->emplace_back();
  switch (delta->status) {
  case GIT_DELTA_ADDED:
    file.Status = gitpp::DeltaStatus::Added;
    file.RightId = gitpp::ObjectId{&delta->new_file.id};
    file.RightPath = rightPath;
    break;
  case GIT_DELTA_DELETED:
    file.Status = gitpp::DeltaStatus::Deleted;
    file.LeftId = gitpp::ObjectId{&delta->old_file.id};
    file.LeftPath = leftPath;
    break;
  case GIT_DELTA_RENAMED:
    file.Status = gitpp::DeltaStatus::Renamed;
    file.LeftId = gitpp::ObjectId{&delta->old_file.id};
    file.LeftPath = leftPath;
    file.RightId = gitpp::ObjectId{&delta->new_file.id};
    file.RightPath = rightPath;
    break;
  case GIT_DELTA_COPIED:
    file.Status = gitpp::DeltaStatus::Copied;
    file.LeftId = gitpp::ObjectId{&delta->old_file.id};
    file.LeftPath = leftPath;
    file.RightId = gitpp::ObjectId{&delta->new_file.id};
    file.RightPath = rightPath;
    break;
  case GIT_DELTA_MODIFIED:
  default:
    file.Status = gitpp::DeltaStatus::Modified;
    file.LeftPath = leftPath;
    file.RightPath = rightPath;
    break;
  }

  return 0;
}

static_assert(std::is_convertible_v<decltype(fileCallback), git_diff_file_cb>, "");

} // namespace

namespace gitpp {

Diff::~Diff() = default;

Diff Diff::create(const Tree* lhs, const Tree* rhs, std::vector<std::string> paths) noexcept {
  assert(lhs != nullptr || rhs != nullptr);
  git_diff* handle = nullptr;
  git_diff_options options = GIT_DIFF_OPTIONS_INIT;
  std::vector<char*> paths_(paths.size(), nullptr);
  std::transform(paths.begin(), paths.end(), paths_.begin(), [](std::string& str) -> char* { return str.data(); });
  if (!paths.empty()) {
    options.pathspec.strings = paths_.data();
    options.pathspec.count = paths.size();
  }
  if (lhs == nullptr) {
    git_diff_tree_to_tree(&handle, git_tree_owner(rhs->handle()), nullptr, rhs->handle(), &options);
  } else if (rhs == nullptr) {
    git_diff_tree_to_tree(&handle, git_tree_owner(lhs->handle()), lhs->handle(), nullptr, &options);
  } else {
    assert(git_tree_owner(lhs->handle()) == git_tree_owner(rhs->handle()));
    git_diff_tree_to_tree(&handle, git_tree_owner(lhs->handle()), lhs->handle(), rhs->handle(), &options);
  }
  assert(handle != nullptr);

  git_diff_find_options findOptions = GIT_DIFF_FIND_OPTIONS_INIT;
  findOptions.flags = GIT_DIFF_FIND_RENAMES | GIT_DIFF_FIND_COPIES | GIT_DIFF_FIND_FOR_UNTRACKED;
  git_diff_find_similar(handle, &findOptions);
  return Diff{handle};
}

Diff::Diff(git_diff* handle) noexcept : Handle(handle) {
  int deltaCount = git_diff_num_deltas(handle);
  for (int i = 0; i < deltaCount; ++i) {
    fileCallback(git_diff_get_delta(handle, i), static_cast<float>(i) / deltaCount, &Deltas);
  }
}

void Diff::deleter::operator()(git_diff* handle) noexcept {
  git_diff_free(handle);
}

} // namespace gitpp

#include "TGitMainWindow.hpp"

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QTableView>

TGitMainWindow::TGitMainWindow(QWidget* parent) : QMainWindow(parent) {
  auto fileMenu = menuBar()->addMenu("&File");
  auto openAction = fileMenu->addAction("&Open");
  openAction->setShortcut(QKeySequence::Open);
  connect(openAction, &QAction::triggered, this, &TGitMainWindow::openAction_triggered);

  View = new QTableView(this);
  setCentralWidget(View);
}

void TGitMainWindow::loadRepository(const QString& path) {
  qDebug() << path;
  QDir directory(path);
  if (!directory.exists()) {
    emit repositoryLoadFailed();
  }
}

void TGitMainWindow::openAction_triggered() {
  auto repositoryPath = QFileDialog::getExistingDirectory(this, "Open Repository");
  loadRepository(repositoryPath);
}
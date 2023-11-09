#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_CodegenMainWindow.h"

class CodegenMainWindow : public QMainWindow {
   Q_OBJECT
   public:
      CodegenMainWindow(QWidget *parent = nullptr);
      ~CodegenMainWindow();

   protected:
      QString _persistLocation() const;
      void _setUpPathFields();
      void _persistPaths();

      void _syncPathsToRegistry() const;

   private:
      Ui::CodegenMainWindowClass ui;
};

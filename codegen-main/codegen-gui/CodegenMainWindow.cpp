#include "./CodegenMainWindow.h"
#include <array>
#include <fstream>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include "./registry.h"

#include "./codegen/generate_all.h"
#include "./dumper/dump_sav.h"

namespace {
   constexpr const bool dumps_are_of_packed_data = true;
}

CodegenMainWindow::CodegenMainWindow(QWidget *parent) : QMainWindow(parent) {
   ui.setupUi(this);

   this->ui.dumpSavBrowse->setProperty("target", QVariant::fromValue<QWidget*>(this->ui.dumpSavPath));
   this->ui.dumpSavBrowse->setProperty("filter", "GBA Flash Memory Saves (*.sav)");
   this->ui.dumpOutBrowse->setProperty("target", QVariant::fromValue<QWidget*>(this->ui.dumpOutPath));
   {
      auto browse = std::array{
         this->ui.dumpSavBrowse,
         this->ui.dumpOutBrowse,
      };

      auto handler = [this]() {
         auto* target = (QLineEdit*) sender()->property("target").value<QWidget*>();
         if (!target)
            return;

         QString filter = sender()->property("filter").toString();

         auto change_to = QFileDialog::getOpenFileName(this, "Set path", target->text(), filter);
         if (change_to.isEmpty())
            return;
         target->setText(change_to);
      };

      for (auto* button : browse) {
         QObject::connect(button, &QPushButton::clicked, this, handler);
      }
   }

   this->_setUpPathFields();

   this->ui.buttonDoCodegen->setEnabled(false);
   this->ui.buttonDoDump->setEnabled(false);
   //
   QObject::connect(this->ui.buttonLoadAll, &QPushButton::clicked, this, [this]() {
      auto& reg = registry::get_or_create();
      this->_syncPathsToRegistry();
      reg.clear();
      try {
         reg.parse_all_xml_files();
      } catch (std::runtime_error& e) {
         QMessageBox::critical(this, "Error", QString("Exception thrown: std::runtime_error: ") + e.what());
         return;
      }
      QMessageBox::critical(this, "Success", QString("All definitions loaded!"));

      this->ui.buttonDoCodegen->setEnabled(true);
      this->ui.buttonDoDump->setEnabled(true);
   });

   QObject::connect(this->ui.buttonDoCodegen, &QPushButton::clicked, this, [this]() {
      auto& reg = registry::get_or_create();
      this->_syncPathsToRegistry();
      try {
         codegen::generate_all(
            this->ui.pathOutputH->text().toStdString(),
            this->ui.pathOutputC->text().toStdString(),
            //
            this->ui.pathOutputRelStructMembers->text().toStdString(),
            this->ui.pathOutputRelStructSerialize->text().toStdString(),
            this->ui.pathOutputRelSectorSerialize->text().toStdString(),
            //
            this->ui.pathOutputRelSaveFunctors->text().toStdString()
         );
         QMessageBox::critical(this, "Success", QString("Packed all sectors! See report for information."));
         return;
      } catch (std::runtime_error& e) {
         QMessageBox::critical(this, "Error", QString("Exception thrown: std::runtime_error: ") + e.what());
         return;
      }
   });

   QObject::connect(this->ui.buttonDoDump, &QPushButton::clicked, this, [this]() {
      auto path_to_report = std::filesystem::path(this->ui.dumpOutPath->text().toStdString());
      auto path_to_sav    = std::filesystem::path(this->ui.dumpSavPath->text().toStdString());
      
      if (!path_to_sav.is_absolute()) {
         QMessageBox::critical(this, "Error", QString("The savegame file path must be absolute."));
         return;
      }
      if (!path_to_report.is_absolute()) {
         QMessageBox::critical(this, "Error", QString("The output file path must be absolute."));
         return;
      }

      std::ofstream out_stream(path_to_report);
      if (!out_stream) {
         QMessageBox::critical(this, "Error", QString("Failed to open the output file, for writing a results report."));
         return;
      }

      std::vector<uint8_t> buffer;
      {
         std::ifstream stream(path_to_sav, std::ios::binary);
         if (!stream) {
            QMessageBox::critical(this, "Error", QString("Failed to open the *.sav file."));
            return;
         }

         auto sav_size = std::filesystem::file_size(path_to_sav);
         buffer.resize(sav_size);
         if (!stream.read((char*)buffer.data(), sav_size)) {
            QMessageBox::critical(this, "Error", QString("Failed to open the *.sav file."));
            return;
         }
      }
      auto& reg = registry::get_or_create();

      std::vector<std::string> sectioned_dump;
      sectioned_dump = dump_sav(buffer.data(), buffer.size(), dumps_are_of_packed_data);

      for (const auto& item : sectioned_dump) {
         out_stream << item << "\n\n";
      }

      QMessageBox::critical(this, "Success", QString("Dump complete."));
      return;
   });
}

CodegenMainWindow::~CodegenMainWindow() {}

QString CodegenMainWindow::_persistLocation() const {
   return QCoreApplication::applicationDirPath() + "/persisted-settings.txt";
}
void CodegenMainWindow::_persistPaths() {
   QString out;
   out += this->ui.pathInputXML->text() + "\n";
   out += this->ui.pathOutputH->text() + "\n";
   out += this->ui.pathOutputC->text() + "\n";
   out += this->ui.pathOutputRelStructMembers->text() + "\n";
   out += this->ui.pathOutputRelStructSerialize->text() + "\n";
   out += this->ui.pathOutputRelSectorSerialize->text() + "\n";
   out += this->ui.pathOutputRelSaveFunctors->text() + "\n";
   out += this->ui.codegenReportPath->text() + "\n";
   out += this->ui.dumpSavPath->text() + "\n";
   out += this->ui.dumpOutPath->text() + "\n";

   QFile file(_persistLocation());
   if (file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate)) {
      QTextStream stream(&file);
      stream << out;
   }
}
void CodegenMainWindow::_setUpPathFields() {
   QFile file(_persistLocation());
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      return;
   }

   auto handler = [this](const QString&) { this->_persistPaths(); };
   
   auto path_fields = std::array{
      this->ui.pathInputXML,
      this->ui.pathOutputH,
      this->ui.pathOutputC,
      this->ui.pathOutputRelStructMembers,
      this->ui.pathOutputRelStructSerialize,
      this->ui.pathOutputRelSectorSerialize,
      this->ui.pathOutputRelSaveFunctors,
      this->ui.codegenReportPath,
      this->ui.dumpSavPath,
      this->ui.dumpOutPath,
   };
   for (auto* field : path_fields) {
      auto value = file.readLine().trimmed();
      field->setText(value);
      QObject::connect(field, &QLineEdit::textChanged, this, handler);
   }
}

void CodegenMainWindow::_syncPathsToRegistry() const {
   auto& reg = registry::get_or_create();
   reg.set_paths({
      .input_paths = {
         .xml = this->ui.pathInputXML->text().toStdString(),
      },
      .output_paths = {
         .h = this->ui.pathOutputH->text().toStdString(),
         .c = this->ui.pathOutputC->text().toStdString(),
         //
         .struct_members   = this->ui.pathOutputRelStructMembers->text().toStdString(),
         .struct_serialize = this->ui.pathOutputRelStructSerialize->text().toStdString(),
         .sector_serialize = this->ui.pathOutputRelSectorSerialize->text().toStdString(),
         //
         .save_functors = this->ui.pathOutputRelSaveFunctors->text().toStdString(),
         //
         .codegen_report = this->ui.codegenReportPath->text().toStdString(),
      }
   });
}
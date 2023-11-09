/********************************************************************************
** Form generated from reading UI file 'CodegenMainWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CODEGENMAINWINDOW_H
#define UI_CODEGENMAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_CodegenMainWindowClass
{
public:
    QWidget *centralWidget;
    QGridLayout *gridLayout_3;
    QGroupBox *groupBox;
    QGridLayout *gridLayout;
    QLabel *label_3;
    QLabel *label_5;
    QLabel *label_9;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QPushButton *buttonLoadAll;
    QPushButton *buttonDoCodegen;
    QSpacerItem *horizontalSpacer_2;
    QSpacerItem *verticalSpacer;
    QLineEdit *pathOutputH;
    QLineEdit *pathOutputRelStructMembers;
    QLabel *label_6;
    QLineEdit *pathOutputRelStructSerialize;
    QLineEdit *codegenReportPath;
    QLineEdit *pathOutputC;
    QLineEdit *pathInputXML;
    QFrame *line;
    QLabel *label_2;
    QLineEdit *pathOutputRelSectorSerialize;
    QFrame *line_2;
    QLabel *label;
    QLabel *label_4;
    QLabel *label_10;
    QLineEdit *pathOutputRelSaveFunctors;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout_2;
    QPushButton *dumpSavBrowse;
    QLabel *label_7;
    QLineEdit *dumpOutPath;
    QLineEdit *dumpSavPath;
    QPushButton *dumpOutBrowse;
    QLabel *label_8;
    QFrame *line_3;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *buttonDoDump;
    QSpacerItem *horizontalSpacer_4;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *CodegenMainWindowClass)
    {
        if (CodegenMainWindowClass->objectName().isEmpty())
            CodegenMainWindowClass->setObjectName(QString::fromUtf8("CodegenMainWindowClass"));
        CodegenMainWindowClass->resize(600, 492);
        centralWidget = new QWidget(CodegenMainWindowClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        gridLayout_3 = new QGridLayout(centralWidget);
        gridLayout_3->setSpacing(6);
        gridLayout_3->setContentsMargins(11, 11, 11, 11);
        gridLayout_3->setObjectName(QString::fromUtf8("gridLayout_3"));
        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        gridLayout = new QGridLayout(groupBox);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        gridLayout->addWidget(label_3, 3, 0, 1, 1);

        label_5 = new QLabel(groupBox);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        gridLayout->addWidget(label_5, 5, 0, 1, 1);

        label_9 = new QLabel(groupBox);
        label_9->setObjectName(QString::fromUtf8("label_9"));

        gridLayout->addWidget(label_9, 8, 0, 1, 1);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        buttonLoadAll = new QPushButton(groupBox);
        buttonLoadAll->setObjectName(QString::fromUtf8("buttonLoadAll"));

        horizontalLayout->addWidget(buttonLoadAll);

        buttonDoCodegen = new QPushButton(groupBox);
        buttonDoCodegen->setObjectName(QString::fromUtf8("buttonDoCodegen"));

        horizontalLayout->addWidget(buttonDoCodegen);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);


        gridLayout->addLayout(horizontalLayout, 10, 0, 1, 2);

        verticalSpacer = new QSpacerItem(8, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        gridLayout->addItem(verticalSpacer, 11, 0, 1, 2);

        pathOutputH = new QLineEdit(groupBox);
        pathOutputH->setObjectName(QString::fromUtf8("pathOutputH"));

        gridLayout->addWidget(pathOutputH, 2, 1, 1, 1);

        pathOutputRelStructMembers = new QLineEdit(groupBox);
        pathOutputRelStructMembers->setObjectName(QString::fromUtf8("pathOutputRelStructMembers"));

        gridLayout->addWidget(pathOutputRelStructMembers, 4, 1, 1, 1);

        label_6 = new QLabel(groupBox);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        gridLayout->addWidget(label_6, 6, 0, 1, 1);

        pathOutputRelStructSerialize = new QLineEdit(groupBox);
        pathOutputRelStructSerialize->setObjectName(QString::fromUtf8("pathOutputRelStructSerialize"));

        gridLayout->addWidget(pathOutputRelStructSerialize, 5, 1, 1, 1);

        codegenReportPath = new QLineEdit(groupBox);
        codegenReportPath->setObjectName(QString::fromUtf8("codegenReportPath"));

        gridLayout->addWidget(codegenReportPath, 8, 1, 1, 1);

        pathOutputC = new QLineEdit(groupBox);
        pathOutputC->setObjectName(QString::fromUtf8("pathOutputC"));

        gridLayout->addWidget(pathOutputC, 3, 1, 1, 1);

        pathInputXML = new QLineEdit(groupBox);
        pathInputXML->setObjectName(QString::fromUtf8("pathInputXML"));

        gridLayout->addWidget(pathInputXML, 0, 1, 1, 1);

        line = new QFrame(groupBox);
        line->setObjectName(QString::fromUtf8("line"));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line, 1, 0, 1, 2);

        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        gridLayout->addWidget(label_2, 2, 0, 1, 1);

        pathOutputRelSectorSerialize = new QLineEdit(groupBox);
        pathOutputRelSectorSerialize->setObjectName(QString::fromUtf8("pathOutputRelSectorSerialize"));

        gridLayout->addWidget(pathOutputRelSectorSerialize, 6, 1, 1, 1);

        line_2 = new QFrame(groupBox);
        line_2->setObjectName(QString::fromUtf8("line_2"));
        line_2->setFrameShape(QFrame::HLine);
        line_2->setFrameShadow(QFrame::Sunken);

        gridLayout->addWidget(line_2, 9, 0, 1, 2);

        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        label_4 = new QLabel(groupBox);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        gridLayout->addWidget(label_4, 4, 0, 1, 1);

        label_10 = new QLabel(groupBox);
        label_10->setObjectName(QString::fromUtf8("label_10"));

        gridLayout->addWidget(label_10, 7, 0, 1, 1);

        pathOutputRelSaveFunctors = new QLineEdit(groupBox);
        pathOutputRelSaveFunctors->setObjectName(QString::fromUtf8("pathOutputRelSaveFunctors"));

        gridLayout->addWidget(pathOutputRelSaveFunctors, 7, 1, 1, 1);


        gridLayout_3->addWidget(groupBox, 0, 0, 1, 1);

        groupBox_2 = new QGroupBox(centralWidget);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        gridLayout_2 = new QGridLayout(groupBox_2);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QString::fromUtf8("gridLayout_2"));
        dumpSavBrowse = new QPushButton(groupBox_2);
        dumpSavBrowse->setObjectName(QString::fromUtf8("dumpSavBrowse"));

        gridLayout_2->addWidget(dumpSavBrowse, 0, 2, 1, 1);

        label_7 = new QLabel(groupBox_2);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        gridLayout_2->addWidget(label_7, 0, 0, 1, 1);

        dumpOutPath = new QLineEdit(groupBox_2);
        dumpOutPath->setObjectName(QString::fromUtf8("dumpOutPath"));

        gridLayout_2->addWidget(dumpOutPath, 1, 1, 1, 1);

        dumpSavPath = new QLineEdit(groupBox_2);
        dumpSavPath->setObjectName(QString::fromUtf8("dumpSavPath"));

        gridLayout_2->addWidget(dumpSavPath, 0, 1, 1, 1);

        dumpOutBrowse = new QPushButton(groupBox_2);
        dumpOutBrowse->setObjectName(QString::fromUtf8("dumpOutBrowse"));

        gridLayout_2->addWidget(dumpOutBrowse, 1, 2, 1, 1);

        label_8 = new QLabel(groupBox_2);
        label_8->setObjectName(QString::fromUtf8("label_8"));

        gridLayout_2->addWidget(label_8, 1, 0, 1, 1);

        line_3 = new QFrame(groupBox_2);
        line_3->setObjectName(QString::fromUtf8("line_3"));
        line_3->setFrameShape(QFrame::HLine);
        line_3->setFrameShadow(QFrame::Sunken);

        gridLayout_2->addWidget(line_3, 2, 0, 1, 3);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_3);

        buttonDoDump = new QPushButton(groupBox_2);
        buttonDoDump->setObjectName(QString::fromUtf8("buttonDoDump"));

        horizontalLayout_2->addWidget(buttonDoDump);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_4);


        gridLayout_2->addLayout(horizontalLayout_2, 3, 0, 1, 3);


        gridLayout_3->addWidget(groupBox_2, 1, 0, 1, 1);

        CodegenMainWindowClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(CodegenMainWindowClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 600, 21));
        CodegenMainWindowClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(CodegenMainWindowClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        CodegenMainWindowClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(CodegenMainWindowClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        CodegenMainWindowClass->setStatusBar(statusBar);

        retranslateUi(CodegenMainWindowClass);

        QMetaObject::connectSlotsByName(CodegenMainWindowClass);
    } // setupUi

    void retranslateUi(QMainWindow *CodegenMainWindowClass)
    {
        CodegenMainWindowClass->setWindowTitle(QCoreApplication::translate("CodegenMainWindowClass", "CodegenMainWindow", nullptr));
        groupBox->setTitle(QCoreApplication::translate("CodegenMainWindowClass", "Data definition and output paths", nullptr));
        label_3->setText(QCoreApplication::translate("CodegenMainWindowClass", "Base output path (*.c):", nullptr));
        label_5->setText(QCoreApplication::translate("CodegenMainWindowClass", "Whole-struct serialize path (relative):", nullptr));
        label_9->setText(QCoreApplication::translate("CodegenMainWindowClass", "Save a report to:", nullptr));
        buttonLoadAll->setText(QCoreApplication::translate("CodegenMainWindowClass", "Load definitions", nullptr));
        buttonDoCodegen->setText(QCoreApplication::translate("CodegenMainWindowClass", "Run code generation", nullptr));
        label_6->setText(QCoreApplication::translate("CodegenMainWindowClass", "Sector group serialize path (relative):", nullptr));
        label_2->setText(QCoreApplication::translate("CodegenMainWindowClass", "Base output path (*.h):", nullptr));
        label->setText(QCoreApplication::translate("CodegenMainWindowClass", "XML base path:", nullptr));
        label_4->setText(QCoreApplication::translate("CodegenMainWindowClass", "Struct member list path (relative):", nullptr));
        label_10->setText(QCoreApplication::translate("CodegenMainWindowClass", "Save-functor serialize path (relative):", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("CodegenMainWindowClass", "Dump savedata info", nullptr));
        dumpSavBrowse->setText(QCoreApplication::translate("CodegenMainWindowClass", "Browse", nullptr));
        label_7->setText(QCoreApplication::translate("CodegenMainWindowClass", "Savegame file (*.sav):", nullptr));
        dumpOutBrowse->setText(QCoreApplication::translate("CodegenMainWindowClass", "Browse", nullptr));
        label_8->setText(QCoreApplication::translate("CodegenMainWindowClass", "Write report to:", nullptr));
        buttonDoDump->setText(QCoreApplication::translate("CodegenMainWindowClass", "Dump", nullptr));
    } // retranslateUi

};

namespace Ui {
    class CodegenMainWindowClass: public Ui_CodegenMainWindowClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CODEGENMAINWINDOW_H

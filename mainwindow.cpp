#include "mainwindow.h"
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QTextStream>
#include "./ui_mainwindow.h"
#include "QApplication"
#include "QMessageBox"
#include "paramswindows.h"
#include "signalwindows.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_p_button_exit_clicked()
{
    QMessageBox msgbox;
    msgbox.setWindowTitle("Диагностика");
    msgbox.setStyleSheet("background-color:rgb(224, 224, 224);");
    msgbox.setText("Вы уверены, что хотите закрыть программу?");
    msgbox.setInformativeText("Да - Закрыть программу\nНет - Остаться");
    msgbox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgbox.setButtonText(QMessageBox::Yes, "Да");
    msgbox.setButtonText(QMessageBox::No, "Нет");

    msgbox.setIcon(QMessageBox::Warning);
    msgbox.setDefaultButton(QMessageBox::Yes);
    int res = msgbox.exec();
    if (res == QMessageBox::Yes) {
        QApplication::quit();
    }
}

void MainWindow::on_p_button_params_clicked()
{
    QString filepath = QDir::currentPath() + "/params.ini";

    QFile paramsFile(filepath);
    if (!paramsFile.exists()) {
        if (paramsFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&paramsFile);
            out << "frequency=0\n";
            out << "hz=0\n";
            paramsFile.close();
        } else {
            return;
        }
    }
    Paramswindows *params = new Paramswindows(this);
    params->move(this->pos());
    params->show();
    MainWindow::hide();
}

void MainWindow::on_p_button_start_clicked()
{
    SignalWindows *signal = new SignalWindows(this); // Исправлено: передаем this как родитель
    signal->show();
    MainWindow::hide();
}

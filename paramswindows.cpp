#include "paramswindows.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QGroupBox>
#include <QMessageBox>
#include <QTextStream>
#include "mainwindow.h"
#include "ui_paramswindows.h"

Paramswindows::Paramswindows(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Paramswindows)
{
    ui->setupUi(this);

    debugUIElements();

    loadParams();
}

Paramswindows::~Paramswindows()
{
    delete ui;
}

void Paramswindows::debugUIElements()
{
    qDebug() << "=== Доступные элементы UI ===";

    qDebug() << "GroupBox:";
    foreach (QGroupBox *gb, findChildren<QGroupBox *>()) {
        qDebug() << "  " << gb->objectName();
    }

    qDebug() << "LineEdits:";
    foreach (QLineEdit *le, findChildren<QLineEdit *>()) {
        qDebug() << "  " << le->objectName() << "в" << le->parent()->objectName();
    }
}

void Paramswindows::loadParams()
{
    QString filePath = "/sandbox/users/malyshev/diag_test_last/diagnostic_mal_Med/params.ini";
    QFile file(filePath);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split("=");
            if (parts.size() == 2) {
                QString key = parts[0].trimmed();
                QString value = parts[1].trimmed();

                if (key == "param" && ui->lineEdit)
                    ui->lineEdit->setText(value);
                else if (key == "param2" && ui->lineEdit_2)
                    ui->lineEdit_2->setText(value);
                else if (key == "param5" && ui->lineEdit_5)
                    ui->lineEdit_5->setText(value);
                else if (key == "param12" && ui->lineEdit_12)
                    ui->lineEdit_12->setText(value);
                else if (key == "param8" && ui->lineEdit_8)
                    ui->lineEdit_8->setText(value);
            }
        }
        file.close();
    }
}

void Paramswindows::on_p_button_main_clicked()
{
    MainWindow *mainwindow = new MainWindow(this);
    mainwindow->move(this->pos());
    mainwindow->show();
    this->hide();
}

void Paramswindows::on_b_button_save_clicked()
{
    QString filePath = "/sandbox/users/malyshev/diag_test_last/diagnostic_mal_Med/params.ini";

    if (!ui->lineEdit || !ui->lineEdit_2 || !ui->lineEdit_5 || !ui->lineEdit_12 || !ui->lineEdit_8) {
        QMessageBox::critical(this, "Ошибка", "Не все элементы интерфейса инициализированы!");
        debugUIElements();
        return;
    }

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);

        out << "param=" << ui->lineEdit->text() << "\n";
        out << "param2=" << ui->lineEdit_2->text() << "\n";
        out << "param5=" << ui->lineEdit_5->text() << "\n";
        out << "param12=" << ui->lineEdit_12->text() << "\n";
        out << "param8=" << ui->lineEdit_8->text() << "\n";

        file.close();

        qDebug() << "Параметры сохранены в" << filePath;
        QMessageBox::information(this, "Успех", "Параметры успешно сохранены в params.ini!");
    } else {
        qDebug() << "Ошибка при открытии файла:" << file.errorString();
        QMessageBox::critical(this,
                              "Ошибка",
                              QString("Не удалось сохранить параметры!\nОшибка: %1")
                                  .arg(file.errorString()));
    }
}

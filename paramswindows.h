#ifndef PARAMSWINDOWS_H
#define PARAMSWINDOWS_H

#include <QMainWindow>

namespace Ui {
class Paramswindows;
}

class Paramswindows : public QMainWindow
{
    Q_OBJECT

public:
    explicit Paramswindows(QWidget *parent = nullptr);
    ~Paramswindows();

private slots:
    void on_p_button_main_clicked();
    void on_b_button_save_clicked();

private:
    Ui::Paramswindows *ui;

    void loadParams();
    void debugUIElements();
};

#endif // PARAMSWINDOWS_H

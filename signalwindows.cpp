#include "signalwindows.h"
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QTextStream>
#include <QVector>
#include <QWheelEvent>
#include "mainwindow.h"
#include "paramswindows.h"
#include "ui_signalwindows.h"
#include <cmath>
#include <fftw3.h>
#include <iostream>
#include <vector>

SignalWindows::SignalWindows(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SignalWindows)
    , m_phaseAnimation(new QPropertyAnimation(this, "phaseShift", this))
    , m_phaseShift(0)
{
    ui->setupUi(this);


    m_channelData.resize(16);
    for (int i = 0; i < 16; ++i) {
        m_channelData[i].resize(100, 0.0);
    }


    connect(&m_updateTimer, &QTimer::timeout, this, &SignalWindows::updateAllChannels);
    m_updateTimer.start(30); // 30 мс ~ 33 FPS


    QMap<QString, QString> params;
    QString appDir = QCoreApplication::applicationDirPath();
    QString filePath = appDir + "/params.ini";
    QFile paramFile(filePath);

    if (!paramFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this,
                              "Ошибка",
                              "Не удалось открыть файл params.ini!\nГрафики не будут построены.");
        return;
    }

    QTextStream paramStream(&paramFile);
    while (!paramStream.atEnd()) {
        QString line = paramStream.readLine();
        QStringList parts = line.split("=");
        if (parts.size() == 2) {
            params[parts[0].trimmed()] = parts[1].trimmed();
        }
    }
    paramFile.close();

    QStringList requiredParams = {"param", "param2", "param5", "param12", "param8"};
    for (const QString &param : requiredParams) {
        if (!params.contains(param)) {
            QMessageBox::critical(this,
                                  "Ошибка",
                                  QString("В файле params.ini отсутствует обязательный параметр: %1")
                                      .arg(param));
            return;
        }
    }

    initializeSimulators(params);


    QCustomPlot *plots[] = {ui->widget, ui->widget_2, ui->widget_3, ui->widget_4,
                            ui->widget_5, ui->widget_6, ui->widget_7, ui->widget_8,
                            ui->widget_9, ui->widget_10, ui->widget_11, ui->widget_12,
                            ui->widget_13, ui->widget_14, ui->widget_15, ui->widget_16};

    for (int i = 0; i < 16; i++) {
        setupPlot(i);
    }


    m_phaseAnimation->setStartValue(0);
    m_phaseAnimation->setEndValue(0);
    m_phaseAnimation->setDuration(1000);
    m_phaseAnimation->setEasingCurve(QEasingCurve::Linear);
    m_phaseAnimation->setLoopCount(-1);

    for (QCustomPlot *plot : plots) {
        plot->setInteraction(QCP::iRangeZoom, true);
        plot->setInteraction(QCP::iRangeDrag, true);
        plot->setSelectionRectMode(QCP::srmNone);
        connect(plot, &QCustomPlot::mousePress, this, &SignalWindows::handlePlotClick);
        connect(plot, &QCustomPlot::mouseRelease, this, &SignalWindows::handlePlotRelease);
        connect(plot, &QCustomPlot::mouseDoubleClick, this, &SignalWindows::handleDoubleClick);
        connect(plot, &QCustomPlot::mouseWheel, this, &SignalWindows::handleZoom);
        plot->setContextMenuPolicy(Qt::CustomContextMenu);
    }
}

SignalWindows::~SignalWindows()
{
    qDeleteAll(m_simulators);
    delete ui;
}

void SignalWindows::initializeSimulators(const QMap<QString, QString>& params)
{
    double A = params["param"].toDouble();
    double f = params["param2"].toDouble();
    double fd = params["param5"].toDouble();

    for (int i = 0; i < 16; ++i) {

        double channelF = f * (i + 1);
        m_simulators.append(new DeviceSimulator(A, channelF, fd));
    }
}

void SignalWindows::setupPlot(int index)
{
    QCustomPlot *plot = nullptr;
    switch (index) {
    case 0: plot = ui->widget; break;
    case 1: plot = ui->widget_2; break;
    case 2: plot = ui->widget_3; break;
    case 3: plot = ui->widget_4; break;
    case 4: plot = ui->widget_5; break;
    case 5: plot = ui->widget_6; break;
    case 6: plot = ui->widget_7; break;
    case 7: plot = ui->widget_8; break;
    case 8: plot = ui->widget_9; break;
    case 9: plot = ui->widget_10; break;
    case 10: plot = ui->widget_11; break;
    case 11: plot = ui->widget_12; break;
    case 12: plot = ui->widget_13; break;
    case 13: plot = ui->widget_14; break;
    case 14: plot = ui->widget_15; break;
    case 15: plot = ui->widget_16; break;
    }

    if (!plot) {
        qDebug() << "Ошибка: виджет QCustomPlot для канала" << index << "не найден.";
        return;
    }

    qDebug() << "Инициализация графика канала" << index;

    plot->clearGraphs();
    plot->addGraph();
    plot->graph(0)->setPen(QPen(QColor(100, 255, 100), 1.5));

    // Настройка осей
    plot->xAxis->setLabel("Time");
    plot->yAxis->setLabel("Amplitude");
    plot->xAxis->setRange(0, 100);
    plot->yAxis->setRange(-1.5, 1.5);

    // Сетка
    plot->xAxis->grid()->setPen(QPen(QColor(200, 200, 200), 0.5));
    plot->yAxis->grid()->setPen(QPen(QColor(200, 200, 200), 0.5));

    plot->replot();
}

void SignalWindows::updateAllChannels()
{

    for (int i = 0; i < 16; ++i) {
        double newValue = m_simulators[i]->generateSample();
        m_channelData[i].append(newValue);


        if (m_channelData[i].size() > 100) {
            m_channelData[i].removeFirst();
        }
    }


    QCustomPlot *plots[] = {ui->widget, ui->widget_2, ui->widget_3, ui->widget_4,
                            ui->widget_5, ui->widget_6, ui->widget_7, ui->widget_8,
                            ui->widget_9, ui->widget_10, ui->widget_11, ui->widget_12,
                            ui->widget_13, ui->widget_14, ui->widget_15, ui->widget_16};

    for (int i = 0; i < 16; i++) {
        if (!plots[i]) continue;

        QVector<double> x(m_channelData[i].size());
        for (int j = 0; j < x.size(); ++j) {
            x[j] = j;
        }

        plots[i]->graph(0)->setData(x, m_channelData[i]);
        plots[i]->replot();
    }
}

void SignalWindows::handlePlotClick(QMouseEvent *event)
{
    QCustomPlot *clickedPlot = qobject_cast<QCustomPlot *>(sender());
    if (!clickedPlot)
        return;

    if (event->button() == Qt::RightButton) {
        clickedPlot->setSelectionRectMode(QCP::srmSelect);
        clickedPlot->setInteraction(QCP::iRangeDrag, false);
        m_rightButtonPressed = true;
    } else if (event->button() == Qt::LeftButton) {
        clickedPlot->setSelectionRectMode(QCP::srmNone);
        clickedPlot->setInteraction(QCP::iRangeDrag, true);
        m_rightButtonPressed = false;
    }
}

void SignalWindows::handlePlotRelease(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_rightButtonPressed = false;
}

void SignalWindows::handleDoubleClick(QMouseEvent *event)
{
    QCustomPlot *clickedPlot = qobject_cast<QCustomPlot *>(sender());

    if (!clickedPlot || event->button() != Qt::LeftButton)
        return;

    QMenu menu(this);
    QAction *toggleAction = menu.addAction("Показать/Скрыть данные");
    QAction *saveAction = menu.addAction("Сохранить график");
    QAction *animationAction = menu.addAction(
        m_phaseAnimation->state() == QAbstractAnimation::Running ? "Остановить анимацию"
                                                                 : "Запустить анимацию");

    QAction *selected = menu.exec(clickedPlot->mapToGlobal(event->pos()));

    if (selected == toggleAction) {
        bool isVisible = clickedPlot->legend->visible();
        clickedPlot->legend->setVisible(!isVisible);
        clickedPlot->replot();
    } else if (selected == saveAction) {
        QString fileName = QFileDialog::getSaveFileName(this,
                                                        "Сохранить график",
                                                        "",
                                                        "PNG (*.png);;JPEG (*.jpg)");
        if (!fileName.isEmpty()) {
            clickedPlot->savePng(fileName);
        }
    } else if (selected == animationAction) {
        if (m_phaseAnimation->state() == QAbstractAnimation::Running) {
            m_phaseAnimation->pause();
        } else {
            m_phaseAnimation->start();
        }
    }
}

void SignalWindows::handleZoom(QWheelEvent *event)
{
    QCustomPlot *plot = qobject_cast<QCustomPlot *>(sender());
    if (!plot)
        return;

    QCPRange xRange = plot->xAxis->range();
    QCPRange yRange = plot->yAxis->range();

    const double minZoomRangeX = 30;
    const double maxZoomRangeX = 100.0;
    const double minZoomRangeY = 30;
    const double maxZoomRangeY = 100.0;

    bool zoomAllowed = true;
    if (event->angleDelta().y() > 0) {
        if ((xRange.size() < minZoomRangeX) || (yRange.size() < minZoomRangeY)) {
            zoomAllowed = false;
        }
    } else {
        if ((xRange.size() > maxZoomRangeX) || (yRange.size() > maxZoomRangeY)) {
            zoomAllowed = false;
        }
    }

    plot->setInteraction(QCP::iRangeZoom, zoomAllowed);
}

void SignalWindows::on_p_signal_stop_all_clicked()
{
    m_phaseAnimation->stop();
}

void SignalWindows::on_p_button_main_clicked()
{
    MainWindow *mainwindow = new MainWindow(this);
    mainwindow->show();
    this->hide();
}

void SignalWindows::setPhaseShift(double shift)
{
    m_phaseShift = shift;
    updateAnimatedGraphs();
}

double SignalWindows::phaseShift() const
{
    return m_phaseShift;
}

void SignalWindows::updateAnimatedGraphs()
{
    QCustomPlot *spectrumPlots[] = {ui->widget, ui->widget_2, ui->widget_3, ui->widget_4,
                                    ui->widget_5, ui->widget_6, ui->widget_7, ui->widget_8,
                                    ui->widget_9, ui->widget_10, ui->widget_11, ui->widget_12,
                                    ui->widget_13, ui->widget_14, ui->widget_15, ui->widget_16};

    double animPhase = fmod(rand(), rand());
    const double decayFactor = 0.1;
    const double responseSpeed = 0.8;

    if (m_animatedSpectrum.isEmpty()) {
        m_animatedSpectrum = m_originalSpectrum;
    }

    QVector<double> animatedSpectrum(m_originalSpectrum.size());
    for (int i = 0; i < m_originalSpectrum.size(); i++) {
        double noise = 0.9 + 0.1 * (rand() / (double) RAND_MAX);
        double targetValue = m_originalSpectrum[i] * noise;
        double currentValue = m_animatedSpectrum[i];

        if (targetValue > currentValue) {
            animatedSpectrum[i] = currentValue + (targetValue - currentValue) * responseSpeed;
        } else {
            animatedSpectrum[i] = currentValue * (1.0 - decayFactor);
        }

        double phaseFactor = 0.9 + 0.1 * sin(animPhase + i * 1);
        animatedSpectrum[i] *= phaseFactor;
    }

    m_animatedSpectrum = animatedSpectrum;

    QVector<double> stepFreq, stepSpectrum;
    for (int i = 0; i < m_freq.size(); i++) {
        stepFreq.append(m_freq[i]);
        stepSpectrum.append(0);

        stepFreq.append(m_freq[i]);
        stepSpectrum.append(animatedSpectrum[i]);

        if (i < m_freq.size() - 1) {
            stepFreq.append(m_freq[i + 1]);
            stepSpectrum.append(animatedSpectrum[i]);
        }
    }

    for (int i = 0; i < 16; i++) {
        spectrumPlots[i]->graph(0)->setData(stepFreq, stepSpectrum);

        QPen graphPen;
        graphPen.setColor(QColor(100, 255, 100));
        graphPen.setWidthF(1.5);
        spectrumPlots[i]->graph(0)->setPen(graphPen);

        spectrumPlots[i]->graph(0)->setBrush(QBrush(QColor(100, 255, 100, 50)));
        spectrumPlots[i]->graph(0)->setLineStyle(QCPGraph::lsStepCenter);

        spectrumPlots[i]->replot();
    }
}

void SignalWindows::on_startAnimation_clicked()
{
    m_phaseAnimation->start();
}

void SignalWindows::on_stopAnimation_clicked()
{
    m_phaseAnimation->stop();
}

void SignalWindows::on_animationSpeedChanged(int speed)
{
    m_phaseAnimation->setDuration(2000 * (1000 - speed) / 100);
    int interval = 100 - speed; // 0-100 мс
    if (interval < 10) interval = 10;
    m_updateTimer.setInterval(interval);
}

void SignalWindows::on_pb_params_signal_clicked()
{
    Paramswindows *param = new Paramswindows(this);
    param->move(this->pos());
    param->show();
    this->hide();
}

void SignalWindows::on_p_signal_start_all_clicked()
{
    m_phaseAnimation->start();
}


void SignalWindows::on_pb_params_update_clicked()
{
    qDebug() << "Обновление параметров...";

    // Чтение параметров из файла params.ini
    QMap<QString, QString> params;
    QString appDir = QCoreApplication::applicationDirPath();
    QString filePath = appDir + "/params.ini";
    QFile paramFile(filePath);

    if (!paramFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this,
                              "Ошибка",
                              "Не удалось открыть файл params.ini!\nПараметры не обновлены.");
        qDebug() << "Ошибка: файл params.ini не найден или не может быть открыт.";
        return;
    }

    QTextStream paramStream(&paramFile);
    while (!paramStream.atEnd()) {
        QString line = paramStream.readLine();
        QStringList parts = line.split("=");
        if (parts.size() == 2) {
            params[parts[0].trimmed()] = parts[1].trimmed();
        }
    }
    paramFile.close();

    QStringList requiredParams = {"param", "param2", "param5", "param12", "param8"};
    for (const QString &param : requiredParams) {
        if (!params.contains(param)) {
            QMessageBox::critical(this,
                                  "Ошибка",
                                  QString("В файле params.ini отсутствует обязательный параметр: %1")
                                      .arg(param));
            qDebug() << "Ошибка: отсутствует параметр" << param;
            return;
        }
    }


    double A = params["param"].toDouble();
    double f = params["param2"].toDouble();
    double fd = params["param5"].toDouble();

    qDebug() << "Обновленные параметры: A =" << A << ", f =" << f << ", fd =" << fd;


    qDeleteAll(m_simulators);
    m_simulators.clear();


    for (int i = 0; i < 16; ++i) {
        double channelF = f * (i + 1);
        DeviceSimulator *simulator = new DeviceSimulator(A, channelF, fd);
        m_simulators.append(simulator);
        qDebug() << "Обновлен симулятор для канала" << i << "с частотой" << channelF;
    }


    for (int i = 0; i < 16; ++i) {
        m_channelData[i].clear();
    }

    qDebug() << "Параметры успешно обновлены.";
}


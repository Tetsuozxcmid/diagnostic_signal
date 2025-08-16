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
#include "device_simulator.h"
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
    , m_phaseShift(0)
{
    ui->setupUi(this);

    QCustomPlot *plots[] = {ui->widget,
                            ui->widget_2,
                            ui->widget_3,
                            ui->widget_4,
                            ui->widget_5,
                            ui->widget_6,
                            ui->widget_7,
                            ui->widget_8,
                            ui->widget_9,
                            ui->widget_10,
                            ui->widget_11,
                            ui->widget_12,
                            ui->widget_13,
                            ui->widget_14,
                            ui->widget_15,
                            ui->widget_16};

    for (QCustomPlot *plot : plots) {
        plot->setInteraction(QCP::iRangeZoom, true);
        plot->setInteraction(QCP::iRangeDrag, true);
        plot->setSelectionRectMode(QCP::srmNone);
        m_phaseAnimation = new QPropertyAnimation(this, "phaseShift", plot);
        m_phaseAnimation->setStartValue(0);
        m_phaseAnimation->setEndValue(0);
        m_phaseAnimation->setDuration(1000);
        m_phaseAnimation->setEasingCurve(QEasingCurve::Linear);
        m_phaseAnimation->setLoopCount(-1);
        connect(plot, &QCustomPlot::mousePress, this, &SignalWindows::handlePlotClick);
        connect(plot, &QCustomPlot::mouseRelease, this, &SignalWindows::handlePlotRelease);
        connect(plot, &QCustomPlot::mouseDoubleClick, this, &SignalWindows::handleDoubleClick);
        connect(plot, &QCustomPlot::mouseWheel, this, &SignalWindows::handleZoom);

        plot->setContextMenuPolicy(Qt::CustomContextMenu);
    }

    QMap<QString, QString> params;
    QString filePath = "/sandbox/users/malyshev/diag_test_last/diagnostic_mal_Med/params.ini";
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
                                  QString(
                                      "В файле params.ini отсутствует обязательный параметр: %1")
                                      .arg(param));
            return;
        }
    }

    double A = params["param"].toDouble();
    double f = params["param2"].toDouble();
    double fd = params["param5"].toDouble();
    int n1 = params["param12"].toInt();
    int n2 = params["param8"].toInt();
    int N = n2 - n1;
    const double pi = 3.14159265358979323846;
    device_simulator *dev_sim = new device_simulator;
    //  QVec<double> J = dev_sim->generateSineWave(3,3,3);
    m_originalX.resize(N);
    m_originalRe.resize(N);
    m_originalIm.resize(N);
    m_originalSpectrum.resize(N);
    m_animatedSpectrum.resize(N);
    m_freq.resize(N);

    for (int i = 0; i < N; i++) {
        m_originalX[i] = n1 + i;
        m_originalRe[i] = cos(2 * M_PI * f * m_originalX[i] / fd); //косинусоидная тема
        m_originalIm[i] = dev_sim->generateSineWave(N, f, fd)[i]; //синусоидная тема
    }

    fftw_complex *fftIn = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex *fftOut = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * N);

    for (int i = 0; i < N; i++) {
        fftIn[i][0] = m_originalRe[i];
        fftIn[i][1] = m_originalIm[i];
    }

    fftw_plan fftPlan = fftw_plan_dft_1d(N, fftIn, fftOut, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(fftPlan);

    for (int i = 0; i < N; i++) {
        m_originalSpectrum[i] = sqrt(fftOut[i][0] * fftOut[i][0] + fftOut[i][1] * fftOut[i][1]);
        m_freq[i] = (i <= N / 2) ? i * fd / N : (i - N) * fd / N; //формула найквиста
    }
    m_animatedSpectrum = m_originalSpectrum;

    fftw_destroy_plan(fftPlan);
    fftw_free(fftIn);
    fftw_free(fftOut);

    double minY = *std::min_element(m_originalRe.constBegin(), m_originalRe.constEnd());
    double maxY = *std::max_element(m_originalRe.constBegin(), m_originalRe.constEnd());
    minY = std::min(minY, *std::min_element(m_originalIm.constBegin(), m_originalIm.constEnd()));
    maxY = std::max(maxY, *std::max_element(m_originalIm.constBegin(), m_originalIm.constEnd()));

    double minSpec = *std::min_element(m_originalSpectrum.constBegin(),
                                       m_originalSpectrum.constEnd());
    double maxSpec = *std::max_element(m_originalSpectrum.constBegin(),
                                       m_originalSpectrum.constEnd());

    const int numPlots = sizeof(plots) / sizeof(plots[0]);
    // device_simulator *dev_sim1 = new device_simulator;
    for (int i = 0; i < numPlots; i++) {
        //  double h = dev_sim->readData(N)[3];

        QCustomPlot *plot = plots[i];
        plot->clearGraphs();

        plot->setBackground(QBrush(QColor(30, 30, 40)));
        plot->axisRect()->setBackground(QBrush(QColor(50, 50, 60)));

        QPen gridPen(QColor(100, 100, 120), 0.5, Qt::DotLine);
        plot->xAxis->grid()->setPen(gridPen);
        plot->yAxis->grid()->setPen(gridPen);
        plot->xAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 90), 0.25, Qt::DotLine));
        plot->yAxis->grid()->setSubGridPen(QPen(QColor(80, 80, 90), 0.25, Qt::DotLine));
        plot->xAxis->grid()->setSubGridVisible(true);
        plot->yAxis->grid()->setSubGridVisible(true);

        if (i % 2 == 0) {
            plot->addGraph();
            plot->graph(0)->setData(m_freq, m_originalSpectrum);
            plot->graph(0)->setPen(QPen(QColor(100, 255, 100), 1.5));
            plot->graph(0)->setName(QString("A=%4 , f=%5 Hz, fd=%1 Hz, N=%2 ,Канал=%3")
                                        .arg(params["param5"])
                                        .arg(N)
                                        .arg(i + 1)
                                        .arg(params["param"])
                                        .arg(params["param2"]));

            plot->yAxis->setRange(minSpec, maxSpec);
            plot->xAxis->setRange(-fd / 2, fd / 2);
            plot->xAxis->setTickLabels(false);
            plot->yAxis->setTickLabels(false);
            plot->legend->setVisible(false);
            plot->legend->setBrush(QBrush(QColor(70, 70, 80, 200)));
        } else {
            plot->addGraph();
            plot->graph(0)->setData(m_freq, m_originalSpectrum);
            plot->graph(0)->setPen(QPen(QColor(100, 255, 100), 1.5));
            plot->graph(0)->setName(QString("A=%4 , f=%5 Hz, fd=%1 Hz, N=%2 ,Канал=%3")
                                        .arg(params["param5"])
                                        .arg(N)
                                        .arg(i + 1)
                                        .arg(params["param"])
                                        .arg(params["param2"]));
            // plot->xAxis->setLabel("Frequency (Hz)");
            //  plot->yAxis->setLabel("Magnitude");
            plot->yAxis->setRange(minSpec, maxSpec);
            plot->xAxis->setRange(-fd / 2, fd / 2);
            plot->xAxis->setTickLabels(false);
            plot->yAxis->setTickLabels(false);
            plot->legend->setVisible(false);
            plot->legend->setBrush(QBrush(QColor(70, 70, 80, 200)));
        }

        plot->legend->setTextColor(Qt::white);
        plot->legend->setBorderPen(QPen(QColor(100, 100, 120), 0.5));
        plot->legend->setFont(QFont("Arial", 9));
        plot->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop | Qt::AlignLeft);

        plot->xAxis->setLabelColor(Qt::white);
        plot->yAxis->setLabelColor(Qt::white);
        plot->xAxis->setBasePen(QPen(Qt::white, 1));
        plot->yAxis->setBasePen(QPen(Qt::white, 1));
        plot->xAxis->setTickPen(QPen(Qt::white, 1));
        plot->yAxis->setTickPen(QPen(Qt::white, 1));
        plot->xAxis->setSubTickPen(QPen(Qt::white, 1));
        plot->yAxis->setSubTickPen(QPen(Qt::white, 1));

        plot->replot();
        plot->show();
    }
}

SignalWindows::~SignalWindows()
{
    delete ui;
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
    // QAction *statsAction = menu.addAction("Статистика");
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
    MainWindow *mainwindow = new MainWindow;
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
    QCustomPlot *spectrumPlots[] = {ui->widget,
                                    ui->widget_2,
                                    ui->widget_3,
                                    ui->widget_4,
                                    ui->widget_5,
                                    ui->widget_6,
                                    ui->widget_7,
                                    ui->widget_8,
                                    ui->widget_9,
                                    ui->widget_10,
                                    ui->widget_11,
                                    ui->widget_12,
                                    ui->widget_13,
                                    ui->widget_14,
                                    ui->widget_15,
                                    ui->widget_16};

    // Параметры анимации
    double animPhase = fmod(rand(), rand());
    const double decayFactor = 0.1;   // Скорость затухания пиков
    const double responseSpeed = 0.8; // Скорость реакции столбцов

    // Если это первый вызов, инициализируем предыдущие значения
    if (m_animatedSpectrum.isEmpty()) {
        m_animatedSpectrum = m_originalSpectrum;
    }

    // Применяем анимацию с плавным изменением и затуханием
    QVector<double> animatedSpectrum(m_originalSpectrum.size());
    for (int i = 0; i < m_originalSpectrum.size(); i++) {
        // Добавляем небольшую случайную вибрацию для реалистичности
        double noise = 0.9 + 0.1 * (rand() / (double) RAND_MAX);

        // Плавное движение к целевому значению с эффектом инерции
        double targetValue = m_originalSpectrum[i] * noise;
        double currentValue = m_animatedSpectrum[i];

        // Если новое значение больше текущего - быстро растём, иначе медленно падаем (эффект затухания)
        if (targetValue > currentValue) {
            animatedSpectrum[i] = currentValue + (targetValue - currentValue) * responseSpeed;
        } else {
            animatedSpectrum[i] = currentValue * (1.0 - decayFactor);
        }

        // Небольшая анимация на основе фазы для оживления
        double phaseFactor = 0.9 + 0.1 * sin(animPhase + i * 1);
        animatedSpectrum[i] *= phaseFactor;
    }

    // Сохраняем текущие значения для следующего кадра
    m_animatedSpectrum = animatedSpectrum;

    // Создаем ступенчатый график (для эффекта столбцов)
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

    // Обновляем все графики
    for (int i = 0; i < 16; i++) {
        spectrumPlots[i]->graph(0)->setData(stepFreq, stepSpectrum);

        // Градиентная заливка для более профессионального вида
        QPen graphPen;
        graphPen.setColor(QColor(100, 255, 100));
        graphPen.setWidthF(1.5);
        spectrumPlots[i]->graph(0)->setPen(graphPen);

        // Настройка заливки
        spectrumPlots[i]->graph(0)->setBrush(QBrush(QColor(100, 255, 100, 50)));
        spectrumPlots[i]->graph(0)->setLineStyle(QCPGraph::lsStepCenter);

        // Автомасштабирование только по вертикали
        // spectrumPlots[i]->yAxis->rescale();
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

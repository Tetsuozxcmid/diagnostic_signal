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

{
    ui->setupUi(this);


    m_channelData.resize(16);
    for (int i = 0; i < 16; ++i) {
        m_channelData[i].resize(1024);
    }
    m_spectrumData.resize(16);
    m_frequencyData.resize(16);

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




    for (QCustomPlot *plot : plots) {

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
    double n1 = params["param8"].toDouble();
    double n2 = params["param12"].toDouble();
    double N = n1-n2;
    for (int i = 0; i < 16; ++i) {

        double channelF = f * (i + 1);

        m_simulators.append(new DeviceSimulator(A, channelF, fd,N));
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

    if (!plot) return;

    plot->clearGraphs();
    plot->addGraph();

    // Настройка для спектра
    plot->graph(0)->setPen(QPen(QColor(100, 255, 100), 1.5));
    plot->graph(0)->setBrush(QBrush(QColor(100, 255, 100, 50))); // Заполнение под графиком

    // Настройка осей для спектра
    plot->xAxis->setLabel("Частота, Гц");
    plot->yAxis->setLabel("Амплитуда");

    // Начальный диапазон (будет автоматически масштабироваться)


    // Включаем легенду


    plot->replot();
}

void SignalWindows::updateAllChannels()
{
    static int updateCounter = 0;

    for (int i = 0; i < 16; ++i) {
        double newValue = m_simulators[i]->generateSample();

        // Добавляем новое значение
        m_channelData[i].append(newValue);

        // Поддерживаем буфер достаточного размера для БПФ
        int requiredSize = m_simulators[i]->Nindex();
        while (m_channelData[i].size() > requiredSize * 2) {
            m_channelData[i].removeFirst();
        }

        // Вычисляем спектр каждые 10 обновлений (чтобы успевали накопиться данные)
        if (updateCounter % 10 == 0 && m_channelData[i].size() >= requiredSize) {
            performFFT(i);
        }
    }

    // Обновляем графики каждый раз
    updateSpectrumPlots();

    updateCounter++;
}
void SignalWindows::performFFT(int channelIndex)
{
    int N = m_simulators[channelIndex]->Nindex();
    if (N <= 0 || m_channelData[channelIndex].size() < N) return;

    // Берем последние N samples
    QVector<double> inputData = m_channelData[channelIndex].mid(m_channelData[channelIndex].size() - N);

    fftw_complex *fftIn = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
    fftw_complex *fftOut = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);

    if (!fftIn || !fftOut) return;

    // Простое заполнение без оконной функции для скорости
    for (int j = 0; j < N; j++) {
        fftIn[j][0] = inputData[j];
        fftIn[j][1] = 0.0;
    }

    fftw_plan fftPlan = fftw_plan_dft_1d(N, fftIn, fftOut, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(fftPlan);

    // ПОЛНЫЙ СПЕКТР (все частоты от -Fs/2 до +Fs/2)
    QVector<double> fullSpectrum(N);
    QVector<double> fullFrequencies(N);

    double sampleRate = m_simulators[channelIndex]->FrequencyD();
    double freqResolution = sampleRate / N;

    for (int j = 0; j < N; j++) {
        double real = fftOut[j][0];
        double imag = fftOut[j][1];
        fullSpectrum[j] = sqrt(real*real + imag*imag) / N;

        // Частоты от -Fs/2 до +Fs/2 (правильный порядок для отображения)
        if (j < N/2) {
            // Положительные частоты (0 до Fs/2)
            fullFrequencies[j] = j * freqResolution;
        } else {
            // Отрицательные частоты (-Fs/2 до 0)
            fullFrequencies[j] = (j - N) * freqResolution;
        }
    }

    // Переупорядочиваем для правильного отображения (от -Fs/2 до +Fs/2)
    QVector<double> reorderedSpectrum(N);
    QVector<double> reorderedFrequencies(N);

    // Первая половина: отрицательные частоты
    for (int j = N/2; j < N; j++) {
        reorderedSpectrum[j - N/2] = fullSpectrum[j];
        reorderedFrequencies[j - N/2] = fullFrequencies[j];
    }

    // Вторая половина: положительные частоты
    for (int j = 0; j < N/2; j++) {
        reorderedSpectrum[j + N/2] = fullSpectrum[j];
        reorderedFrequencies[j + N/2] = fullFrequencies[j];
    }

    m_spectrumData[channelIndex] = reorderedSpectrum;
    m_frequencyData[channelIndex] = reorderedFrequencies;

    fftw_destroy_plan(fftPlan);
    fftw_free(fftIn);
    fftw_free(fftOut);
}
void SignalWindows::updateSpectrumPlots()
{
    QCustomPlot *spectrumPlots[] = {
        ui->widget, ui->widget_2, ui->widget_3, ui->widget_4,
        ui->widget_5, ui->widget_6, ui->widget_7, ui->widget_8,
        ui->widget_9, ui->widget_10, ui->widget_11, ui->widget_12,
        ui->widget_13, ui->widget_14, ui->widget_15, ui->widget_16
    };

    for (int i = 0; i < 16 && i < m_spectrumData.size(); i++) {
        if (spectrumPlots[i] && !m_spectrumData[i].isEmpty()) {
            // Отображаем полный спектр (все частоты)
            spectrumPlots[i]->graph(0)->setData(m_frequencyData[i], m_spectrumData[i]);

            double maxAmplit = 0;
            double peakF = 0;
            for (int j =0;j <m_spectrumData[i].size();j++) {
                if (m_spectrumData[i][j] > maxAmplit) {
                    maxAmplit = m_spectrumData[i][j];
                    peakF = m_frequencyData[i][j];


                }


            }



            // Настройка осей для спектра
            spectrumPlots[i]->xAxis->setLabel("Частота, Гц");
            spectrumPlots[i]->yAxis->setLabel("Амплитуда");
                spectrumPlots[i]->yAxis->setTickLabels(false);
            // ДИАПАЗОН ДЛЯ ПОЛНОГО СПЕКТРА (-Fs/2 до +Fs/2)
            double sampleRate = m_simulators[i]->FrequencyD();
            double nyquistFrequency = sampleRate / 2.0;

            double xRange = nyquistFrequency * 2;
            double xCenter = peakF;
            //double maxAmplitude = *std::max_element(m_spectrumData[i].begin(), m_spectrumData[i].end());

            spectrumPlots[i]->xAxis->setScaleType(QCPAxis::stLogarithmic);  //setRange(xCenter - xRange/2, xCenter + xRange/2);
                spectrumPlots[i]->yAxis->setScaleType(QCPAxis::stLinear);
           spectrumPlots[i]->yAxis->setRange(0, maxAmplit *1.1 );

            // Обновление заголовка
            spectrumPlots[i]->graph(0)->setName(QString("Спектр - Канал %1 \n Частота - %2 \n Частота Д - %3 \n Амплитуда - %4 ")
                                                .arg(i+1).arg(m_simulators[i]->Frequency())
                                                .arg(m_simulators[i]->FrequencyD()).arg(m_simulators[i]->Amlitude()));
            spectrumPlots[i]->replot();
        }
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
    } else{

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

 // plot->setInteraction(QCP::iRangeZoom, zoomAllowed);
}


void SignalWindows::on_p_button_main_clicked()
{
    MainWindow *mainwindow = new MainWindow(this);
    mainwindow->show();
    this->hide();
}



void SignalWindows::on_pb_params_signal_clicked()
{
    Paramswindows *param = new Paramswindows(this);
    param->move(this->pos());
    param->show();
    this->hide();
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
    double n1 = params["param8"].toDouble();
    double n2 = params["param12"].toDouble();
    double N = n1-n2;
    qDebug() << "Обновленные параметры: A =" << A << ", f =" << f << ", fd =" << fd;


    qDeleteAll(m_simulators);
    m_simulators.clear();


    for (int i = 0; i < 16; ++i) {
        double channelF = f * (1 + i);
    //double AF = A *1.3;
        DeviceSimulator *simulator = new DeviceSimulator(A, f, fd,N);
        m_simulators.append(simulator);
        qDebug() << "Обновлен симулятор для канала" << i << "с частотой" << channelF;
    }


    for (int i = 0; i < 16; ++i) {
        m_channelData[i].clear();
    }

    qDebug() << "Параметры успешно обновлены.";
}


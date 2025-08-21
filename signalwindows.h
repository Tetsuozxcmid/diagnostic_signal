#ifndef SIGNALWINDOWS_H
#define SIGNALWINDOWS_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QTimer>
#include <cmath>
#include "qcustomplot.h" // Подключаем QCustomPlot
#include "devicesimulator.h" // Подключаем DeviceSimulator

namespace Ui {
class SignalWindows;
}

class SignalWindows : public QMainWindow
{
    Q_OBJECT


public:
    explicit SignalWindows(QWidget *parent = nullptr);
    ~SignalWindows();



private slots:
    void on_p_button_main_clicked();
    void handlePlotClick(QMouseEvent *event);
    void handlePlotRelease(QMouseEvent *event);
    void handleDoubleClick(QMouseEvent *event);
    void handleZoom(QWheelEvent *event);
    void on_pb_params_signal_clicked();
    void updateAllChannels();
    void on_pb_params_update_clicked();

private:
    Ui::SignalWindows *ui;
    bool m_rightButtonPressed = false;
    QList<QCPAbstractPlottable *> m_lastSelection;



    QVector<double> m_originalX, m_originalRe, m_originalIm;
    QVector<double> m_freq;
    QVector<double> m_originalSpectrum;
    QVector<double> m_animatedSpectrum;

    QVector<QVector<double>> m_channelData;   // Данные для каждого канала
    QList<DeviceSimulator*> m_simulators;     // Симуляторы для каждого канала
    QTimer m_updateTimer;                     // Таймер для обновления

    void initializeSimulators(const QMap<QString, QString>& params);
    void setupPlot(int index);
    QVector<QVector<double>> m_spectrumData;    // Спектры для каждого канала
    QVector<QVector<double>> m_frequencyData;   // Частоты для спектра

    void performFFT(int channelIndex);
    void updateSpectrumPlots();



};

#endif // SIGNALWINDOWS_H

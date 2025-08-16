#ifndef SIGNALWINDOWS_H
#define SIGNALWINDOWS_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include "qcustomplot.h"
#include <vector>

namespace Ui {
class SignalWindows;
}

class SignalWindows : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(double phaseShift READ phaseShift WRITE setPhaseShift)

public:
    explicit SignalWindows(QWidget *parent = nullptr);
    ~SignalWindows();

    double phaseShift() const;
    void setPhaseShift(double shift);

private slots:
    void on_p_signal_stop_all_clicked();
    void on_p_button_main_clicked();
    void handlePlotClick(QMouseEvent *event);
    void handlePlotRelease(QMouseEvent *event);
    void handleDoubleClick(QMouseEvent *event);
    void handleZoom(QWheelEvent *event);
    void updateAnimatedGraphs();
    void on_startAnimation_clicked();
    void on_stopAnimation_clicked();
    void on_animationSpeedChanged(int speed);

    void on_pb_params_signal_clicked();

    void on_p_signal_start_all_clicked();

private:
    Ui::SignalWindows *ui;
    bool m_rightButtonPressed = false;
    QList<QCPAbstractPlottable *> m_lastSelection;

    QPropertyAnimation *m_phaseAnimation;
    double m_phaseShift;

    QVector<double> m_originalX, m_originalRe, m_originalIm;
    QVector<double> m_freq;
    QVector<double> m_originalSpectrum;
    QVector<double> m_animatedSpectrum;
};

#endif // SIGNALWINDOWS_H

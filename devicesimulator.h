#ifndef DEVICESIMULATOR_H
#define DEVICESIMULATOR_H

class DeviceSimulator
{
public:
    DeviceSimulator(double amplitude, double frequency, double sampleRate);
    double generateSample();

private:
    double A;      // Амплитуда
    double f;      // Частота сигнала (Гц)
    double fd;     // Частота дискретизации (Гц)
    double t;      // Текущее время (с)
    static const double PI; // Объявление статической константы (переименовано)
};

#endif // DEVICESIMULATOR_H

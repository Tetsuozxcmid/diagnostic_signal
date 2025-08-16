#ifndef DEVICESIMULATOR_H
#define DEVICESIMULATOR_H

class DeviceSimulator
{
public:
    DeviceSimulator(double amplitude, double frequency, double sampleRate);
    double generateSample();

private:
    double A;
    double f;
    double fd;
    double t;
    static const double PI;
};

#endif // DEVICESIMULATOR_H

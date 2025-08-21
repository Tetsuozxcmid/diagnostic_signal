#ifndef DEVICESIMULATOR_H
#define DEVICESIMULATOR_H
#include "vector"
#include "qvector.h"



class DeviceSimulator
{
public:
    DeviceSimulator(double amplitude, double frequency, double sampleRate,int N);
    double generateSample();
    double generateSin();
    double  generateCos();
    int Nindex();
    int Frequency();
    double Amlitude();
    double FrequencyD();
private:
    double A;
    double f;
    double fd;
    double t;
    int N;
    static const double PI;
};

#endif // DEVICESIMULATOR_H

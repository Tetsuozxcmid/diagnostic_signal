#include "devicesimulator.h"
#include <cmath>

// Определение статической константы
const double DeviceSimulator::PI = 3.14159265358979323846;

DeviceSimulator::DeviceSimulator(double amplitude, double frequency, double sampleRate)
    : A(amplitude), f(frequency), fd(sampleRate), t(0.0)
{
}

double DeviceSimulator::generateSample()
{
    t += 1.0 / fd;
    return A * std::sin(2 * PI * f * t); // Используем PI вместо M_PI
}

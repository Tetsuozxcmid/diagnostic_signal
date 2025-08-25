#include "devicesimulator.h"
#include <cmath>
#include "fftw3.h"

// Определение статической константы
const double DeviceSimulator::PI = 3.14159265358979323846;

DeviceSimulator::DeviceSimulator(double amplitude, double frequency, double sampleRate,int N)
    : A(amplitude), f(frequency), fd(sampleRate), t(0.0),N(N)
{
}


double DeviceSimulator::generateSample()
{

    t += 1.0 / fd;
    return A * std::sin(2  * PI * f * t * A); // Используем PI вместо M_PI



}
double DeviceSimulator::generateSin()
{

    t += 1.0 / fd;
    return A * std::sin(2 * PI * f * t); // Используем PI вместо M_PI



}
double DeviceSimulator::generateCos()
{

    t += 1.0 / fd;
    return A * std::cos(2 * PI * f * t); // Используем PI вместо M_PI



}

int DeviceSimulator::Nindex()
{


    return N;



}
int DeviceSimulator::Frequency()
{


    return f;



}

double DeviceSimulator::Amlitude()
{


    return A;



}
double DeviceSimulator::FrequencyD()
{


    return fd;



}

/*double DeviceSimulator::generateNike()
{

    t += 1.0 / fd;
    return m_freq[i] = (i <= N/2) ? i * fd/N : (i - N) * fd/N;; // Используем PI вместо M_PI



}*/



//fftw_complex *fftIn = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
//fftw_complex *fftOut = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);





/*for (int i = 0; i < N; i++) {
    m_originalX[i] = n1 + i;
    m_originalRe[i] =  cos(2 * M_PI * f * m_originalX[i] / fd); //косинусоидная тема
    m_originalIm[i] =  sin(2 * M_PI * f * m_originalX[i] / fd); //синусоидная тема
}

 * N);

for (int i = 0; i < N; i++) {
    fftIn[i][0] = m_originalRe[i];
    fftIn[i][1] = m_originalIm[i];

}

fftw_plan fftPlan = fftw_plan_dft_1d(N, fftIn, fftOut, FFTW_FORWARD, FFTW_ESTIMATE);
fftw_execute(fftPlan);

for (int i = 0; i < N; i++) {
    m_originalSpectrum[i] = sqrt(fftOut[i][0]*fftOut[i][0] + fftOut[i][1]*fftOut[i][1]);
    m_freq[i] = (i <= N/2) ? i * fd/N : (i - N) * fd/N; //формула найквиста
}
m_animatedSpectrum = m_originalSpectrum;

fftw_destroy_plan(fftPlan);
fftw_free(fftIn);
fftw_free(fftOut);*/

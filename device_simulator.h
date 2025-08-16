#ifndef DEVICE_SIMULATOR_H
#define DEVICE_SIMULATOR_H
#include <vector>
#include <QVector>

class device_simulator

{
public:
     device_simulator(int numChannels = 3);

    QVector<double>readData( int samples);

    void setNoiseLevel(double level);
    void setFrequency(int channel, double freq);
   QVector<double> generateSineWave(int samples, double frequency, double sampleRate);

private:
    int numChannels;
    double noiseLevel = 0.1;

    std::vector<double>channelFrequencies;



  //  std::vector<double>addNoise(const std::vector<double>&signal) const;
};
#endif // DEVICE_SIMULATOR_H

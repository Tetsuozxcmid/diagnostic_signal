#include "device_simulator.h"
#include <cmath>
#include <random>
device_simulator::device_simulator(int numChannels)
    : numChannels(numChannels)
    , channelFrequencies(numChannels)
{
    for (int i = 0; i < numChannels; ++i) {
        channelFrequencies[i] = 50.0 + i * 20.0;
    }
}
QVector<double> device_simulator::readData(int samples)
{
    QVector<double> channels;
    const double sampleRate = 1000.0;
    for (int ch = 0; ch < numChannels; ++ch) {
        auto signal = generateSineWave(samples, channelFrequencies[ch], sampleRate);
        //  auto noisySignal = addNoise(signal);

        //   channels.push_back(noisySignal);
    }
    return channels;
}

QVector<double> device_simulator::generateSineWave(int samples, double frequency, double sampleRate)
{
    QVector<double> signal(samples);
    for (int i = 0; i < samples; ++i) {
        double t = i / sampleRate;
        signal[i] = sin(2 * M_PI * frequency * t);
    }

    return signal;
}

/*std::vector<double>device_simulator::addNoise(const std::vector<double> &signal) const{
    std::vector<double>noisySignal = signal;
    std::random_device rd;
    std::mt19937 gen(rd);
    std::normal_distribution<> dist(0.0,noiseLevel);
    for (auto& value : noisySignal) {

        value += dist(gen);

    }
  return noisySignal; dimaZ
} */

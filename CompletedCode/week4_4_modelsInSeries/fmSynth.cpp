#include "fmSynth.h"

void fmSynth::setup(double index, double ratio, double carrierFreq) {
	m_dIndex = index;
	m_dRatio = ratio;
	m_dCarrierFreq = carrierFreq;
}

double fmSynth::play(double index, double ratio, double carrierFreq) {
	m_dIndex = index;
	m_dRatio = ratio;
	m_dCarrierFreq = carrierFreq;

	double modFreq = m_dCarrierFreq / m_dRatio;
	double deviation = m_dIndex * modFreq;

	double modSignal = m_maxiModulator.saw(modFreq) * deviation;
	double carrierSignal = m_maxiCarrier.sinewave(carrierFreq + modSignal) * 0.3f;

	return carrierSignal;
}
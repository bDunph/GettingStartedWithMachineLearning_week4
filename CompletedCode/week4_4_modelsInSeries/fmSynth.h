#pragma once

#include "ofxMaxim.h"

class fmSynth {

public:
	void setup(double index, double ratio, double carrierFreq);
	double play(double index, double ratio, double carrierFreq);

private:
	double m_dIndex;
	double m_dRatio;
	double m_dCarrierFreq;
	maxiOsc m_maxiModulator;
	maxiOsc m_maxiCarrier;
};
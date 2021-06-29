#pragma once

#include "ofxMaxim.h"

class modalSynth {
public:
	void setup(int numResModes, int numExcModes);
	bool setResModeFrequencies(double baseFreq, std::vector<double>& ratios);
	bool setResModeResonances(double baseRes, std::vector<double>& ratios);
	bool setExcModeFrequencies(double baseFreq, std::vector<double>& ratios);
	bool setExcModeResonances(double baseRes, std::vector<double>& ratios);
	void setEnvelopeValues(double attack, double decay, double sustain, double release);
	double play(double contactFreq, double triggerFreq, double deviationAmount);


private:
	int m_iNumResModes;
	int m_iNumExcModes;
	std::vector<maxiSVF> m_vResModes;
	std::vector<double> m_vResSigs;
	std::vector<maxiSVF> m_vExcModes;
	std::vector<double> m_vExcSigs;
	maxiEnv m_maxiEnv;
	double m_dExcSignalTotal;
	double m_dResSignalTotal;
	maxiOsc m_maxiContactOsc;
	double m_dContactSignal;
	maxiOsc m_maxiTrigOsc;
	double m_dTrigSignal;
	double m_dEnvSignal;

};
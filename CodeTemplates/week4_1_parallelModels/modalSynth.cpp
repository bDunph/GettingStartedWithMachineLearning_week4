#include "modalSynth.h"

//---------------------------------------------------------------------------------
void modalSynth::setup(int numResModes, int numExcModes) {
	m_iNumResModes = numResModes;
	m_iNumExcModes = numExcModes;

	for (int i = 0; i < numResModes; i++) {
		maxiSVF tempMode;
		tempMode.setCutoff(0.0f);
		tempMode.setResonance(0.0f);
		m_vResModes.push_back(tempMode);
		m_vResSigs.push_back(0.0f);
	}

	for (int i = 0; i < numExcModes; i++) {
		maxiSVF tempMode;
		tempMode.setCutoff(0.0f);
		tempMode.setResonance(0.0f);
		m_vExcModes.push_back(tempMode);
		m_vExcSigs.push_back(0.0f);
	}
}

//---------------------------------------------------------------------------------
bool modalSynth::setResModeFrequencies(double baseFreq, std::vector<double>& ratios) {

	if (ratios.size() != m_iNumResModes) {
		cout << "ERROR: Ratios size must equal number of resonant modes!" << endl;
		return false;
	}

	for (int i = 0; i < ratios.size(); i++) {
		m_vResModes[i].setCutoff(baseFreq * ratios[i]);
	}

	return true;
}

//---------------------------------------------------------------------------------
bool modalSynth::setResModeResonances(double baseRes, std::vector<double>& ratios) {

	if (ratios.size() != m_iNumResModes) {
		cout << "ERROR: Ratios size must equal number of resonant modes!" << endl;
		return false;
	}

	for (int i = 0; i < ratios.size(); i++) {
		m_vResModes[i].setResonance(baseRes * ratios[i]);
	}

	return true;
}

//---------------------------------------------------------------------------------
bool modalSynth::setExcModeFrequencies(double baseFreq, std::vector<double>& ratios) {

	if (ratios.size() != m_iNumExcModes) {
		cout << "ERROR: Ratios size must equal number of excitation modes!" << endl;
		return false;
	}

	for (int i = 0; i < ratios.size(); i++) {
		m_vExcModes[i].setCutoff(baseFreq * ratios[i]);
	}

	return true;
}

//---------------------------------------------------------------------------------
bool modalSynth::setExcModeResonances(double baseRes, std::vector<double>& ratios) {

	if (ratios.size() != m_iNumExcModes) {
		cout << "ERROR: Ratios size must equal number of excitation modes!" << endl;
		return false;
	}

	for (int i = 0; i < ratios.size(); i++) {
		m_vExcModes[i].setResonance(baseRes * ratios[i]);
	}

	return true;
}

//---------------------------------------------------------------------------------
void modalSynth::setEnvelopeValues(double attack, double decay, double sustain, double release) {
	m_maxiEnv.setAttack(attack);
	m_maxiEnv.setDecay(decay);
	m_maxiEnv.setSustain(sustain);
	m_maxiEnv.setRelease(release);
}

//---------------------------------------------------------------------------------
double modalSynth::play(double contactFreq, double triggerFreq, double deviationAmount) {

	m_dContactSignal = m_maxiContactOsc.sinewave(contactFreq);

	m_dTrigSignal = m_maxiTrigOsc.phasor(triggerFreq * deviationAmount);

	m_dEnvSignal = m_maxiEnv.adsr(m_dContactSignal, (int)m_dTrigSignal);

	for (int i = 0; i < m_iNumExcModes; i++) {
		m_vExcSigs[i] = m_vExcModes[i].play(m_dEnvSignal, 0, 1, 0, 0);
		m_dExcSignalTotal += m_vExcSigs[i];
	}

	m_dExcSignalTotal *= 0.5f;

	if (m_dExcSignalTotal < -0.95f) {
		m_dExcSignalTotal = 0.0f;
	}

	for (int i = 0; i < m_iNumResModes; i++) {
		m_vResSigs[i] = m_vResModes[i].play(m_dExcSignalTotal, 0, 1, 0, 0);
		m_dResSignalTotal += m_vResSigs[i];
	}

	m_dResSignalTotal *= 0.2f;
	
	return (m_dResSignalTotal + m_dExcSignalTotal) * 0.5f;
}
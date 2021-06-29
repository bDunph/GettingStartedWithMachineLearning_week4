#include "parameterHelper.h"

//-------------------------------------------------------------------------------------------------------
void parameterHelper::setup(std::vector<double>& parameters) {
	for (int i = 0; i < parameters.size(); i++) {
		m_vOrigParameters.push_back(parameters[i]);
		m_vOutParameters.push_back(parameters[i]);
		m_vNormParameters.push_back(0);
	}
}

//-------------------------------------------------------------------------------------------------------
void parameterHelper::randomise(double range) {

	for (int i = 0; i < m_vOrigParameters.size(); i++) {
		double rangeVal = m_vOrigParameters[i] * range;
		double maxVal = m_vOrigParameters[i] + rangeVal;
		double minVal = m_vOrigParameters[i] - rangeVal;
		double randValue = ofRandom(minVal, maxVal);
		m_vOutParameters[i] = randValue;
	}
}

//-------------------------------------------------------------------------------------------------------
void parameterHelper::normalise(double range) {

	for (int i = 0; i < m_vOutParameters.size(); i++) {
		double rangeVal = m_vOrigParameters[i] * range;
		double maxVal = m_vOrigParameters[i] + rangeVal;
		double minVal = m_vOrigParameters[i] - rangeVal;
		m_vNormParameters[i] = ofMap(m_vOutParameters[i], minVal, maxVal, 0.0f, 1.0f);
	}

}

//-------------------------------------------------------------------------------------------------------
void parameterHelper::reMap(std::vector<double> inVector, double range) {

	for (int i = 0; i < m_vOrigParameters.size(); i++) {
		double rangeVal = m_vOrigParameters[i] * range;
		double maxVal = m_vOrigParameters[i] + rangeVal;
		double minVal = m_vOrigParameters[i] - rangeVal;
		m_vOutParameters[i] = ofMap(inVector[i], 0.0f, 1.0f, minVal, maxVal);
	}
}

//-------------------------------------------------------------------------------------------------------
double parameterHelper::getParameter(int index) {
	return m_vOutParameters[index];
}

//-------------------------------------------------------------------------------------------------------
std::vector<double> parameterHelper::getNormVector() {
	return m_vNormParameters;
}
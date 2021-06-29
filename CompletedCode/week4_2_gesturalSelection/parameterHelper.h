#pragma once
#include "ofMain.h"

class parameterHelper {
public:
	void setup(std::vector<double>& parameters);
	void randomise(double range);
	void normalise(double range);
	void reMap(std::vector<double> inVector, double range);
	double getParameter(int index);
	std::vector<double> getNormVector();

private:
	std::vector<double> m_vOrigParameters;
	std::vector<double> m_vOutParameters;
	std::vector<double> m_vNormParameters;
};
#pragma once

#include "ofMain.h"
#include "ofxRapidLib.h"
#include "fmSynth.h"
#include "modalSynth.h"
#include "parameterHelper.h"

class ofApp : public ofBaseApp {

public:
	void setup();
	void update();
	void draw();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y);
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);

	/* RapidLib */
	std::vector<rapidlib::trainingExample> data;
	std::vector<double> modelOutput;
	rapidlib::regression neuralNet;

	std::vector<rapidlib::trainingExample> data2;
	std::vector<double> modelOutput2;
	rapidlib::regression neuralNet2;

	/* TODO:	Declare a series classification object,
				a training set vector, a temporary series
				training example, an int to label the data
				and a string to hold the output label. */

	bool dtwActive, dtwRecording, dtwTrained, dtwEmpty, dtwRunning;
	bool neuralNet1Active, neuralNet2Active, regressionActive;
	bool isRecording, isRecording2;
	bool isTrained, isTrained2;
	bool isEmpty, isEmpty2;
	bool isRunning, isRunning2;

	/* GUI */
	std::vector<ofPolyline> lines;
	ofPolyline tempLine;

	ofFbo fbo1, fbo2, fbo3;
	float fboWidth, fboHeight;
	ofPolyline dtwLine, inputDTWLine;


	/* Audio Stuff */
	ofSoundStream soundStream;
	void audioOut(ofSoundBuffer& output) override;
	double finalOut;
	maxiMix mixer;
	double stereoSig[2];
	double* channel1;
	double channel1Pan;
	maxiMix mixer2;
	double stereoSig2[2];
	double* channel2;
	double channel2Pan;
	double leftOut, rightOut;

	fmSynth fm;
	double index, ratio, carrierFreq;
	double fmOut;

	modalSynth modal;
	double baseResFreq, baseResRes, baseExcFreq, baseExcRes, attack, decay, sustain, release, contactFreq, triggerFreq;
	std::vector<double> resFreqRatios;
	std::vector<double> resResRatios;
	std::vector<double> excFreqRatios;
	std::vector<double> excResRatios;
	double modalOut;

	parameterHelper fmParams;
	std::vector<double> fmParamVector;
	double fmRange;

	parameterHelper modalParams;
	std::vector<double> modalParamVector;
	double modalRange;
};

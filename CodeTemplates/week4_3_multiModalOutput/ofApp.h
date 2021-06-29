#pragma once

#include "ofMain.h"
#include "ofxRapidLib.h"
#include "fmSynth.h"
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
	bool isRecording;
	bool isTrained;
	bool isEmpty;
	bool isRunning;

	/* GUI */
	std::vector<ofPolyline> lines;
	ofPolyline tempLine;
	ofFbo fbo1;
	float fboWidth;
	float fboHeight;

	/* Audio Stuff */
	ofSoundStream soundStream;
	void audioOut(ofSoundBuffer& output) override;
	double masterOut;

	fmSynth fm;
	double index, ratio, carrierFreq;
	double fmOut;

	parameterHelper fmParams;
	std::vector<double> fmParamVector;
	double fmRange;

	maxiEnv envelope;
	double envSignal;
	maxiOsc envTrigger;
	double triggerSignal, triggerFreq;
	double attack, sustain, decay, release;

	/* Graphics Stuff */
	ofIcoSpherePrimitive icoSphere;

	ofShader shader;
	ofEasyCam cam;
	ofVec3f lightPos;
	ofVec3f camPos;
	ofMatrix4x4 modelMat;
	ofMatrix4x4 inverseMat;
	ofMatrix4x4 normalMat;

	ofImage noiseTex;
	float imageW;
	float imageH;

	std::vector<ofBoxPrimitive> boxes;
	std::vector<ofVec3f> boxPositions;

	/* TODO: Declare graphics parameters. */

	/* TODO:	Declare a parameter object to hold graphics parameters,
				a vector to hold the parameter values and a variable
				to hold the graphics parameter range. */
};

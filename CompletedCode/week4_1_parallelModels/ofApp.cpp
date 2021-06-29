#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {

    /* RapidLib Bools */
    isTrained = false;
    isRunning = false;
    isEmpty = true;
    isRecording = false;
    isTrained2 = false;
    isRunning2 = false;
    isEmpty2 = true;
    isRecording = false;

    /* Audio Stuff */
    int sampleRate = 44100;
    int bufferSize = 512;
    ofxMaxiSettings::setup(sampleRate, 2, bufferSize);

    ofSoundStreamSettings settings;
    settings.setOutListener(this);
    settings.sampleRate = sampleRate;
    settings.numOutputChannels = 2;
    settings.numInputChannels = 0;
    settings.bufferSize = bufferSize;
    soundStream.setup(settings);

    /* FM synth setup */
    index = 15.0f;
    ratio = 2.1f;
    carrierFreq = 200.0f;

    fm.setup(index, ratio, carrierFreq);

    fmRange = 0.5f;
    fmParamVector = { index, ratio, carrierFreq };
    fmParams.setup(fmParamVector);

    /* Modal synth setup */
    baseResFreq = 120.0f;
    baseResRes = 9.0f;
    baseExcFreq = 1000.0f;
    baseExcRes = 9.0f;
    attack = 12.0f;
    decay = 10.0f;
    sustain = 0.9f;
    release = 1000.0f;
    contactFreq = 120.0f;
    triggerFreq = 2.0f;

    int numResModes = 5;
    int numExcModes = 2;
    modal.setup(numResModes, numExcModes);

    resFreqRatios = { 1.0f, 2.32f, 4.25f, 6.63f, 9.38f };
    resResRatios = { 1.0f, 1.1f, 1.3f, 1.3f, 1.8f };
    excFreqRatios = { 1.0f, 1.8f };
    excResRatios = { 1.0f, 1.3f };

    modalRange = 0.5f;
    modalParamVector = { baseResFreq, baseResRes, baseExcFreq, baseExcRes, attack, sustain, decay, release, contactFreq, triggerFreq };
    modalParams.setup(modalParamVector);
}

//--------------------------------------------------------------
void ofApp::update() {
    modal.setResModeFrequencies(modalParams.getParameter(0), resFreqRatios);
    modal.setResModeResonances(modalParams.getParameter(1), resResRatios);
    modal.setExcModeFrequencies(modalParams.getParameter(2), excFreqRatios);
    modal.setExcModeResonances(modalParams.getParameter(3), excResRatios);

    modal.setEnvelopeValues(modalParams.getParameter(4), modalParams.getParameter(5), modalParams.getParameter(6), modalParams.getParameter(7));
}

//--------------------------------------------------------------
void ofApp::draw() {
    ofBackground(100, 100, 100);

    ofPushStyle();
    ofSetColor(ofColor::beige);
    ofDrawBitmapString("Number of training examples: " + ofToString(data.size() + data2.size()), 20, 68);
    ofDrawBitmapString("Model running: " + ofToString(isRunning), 20, 82);
    ofDrawBitmapString("Model Trained: " + ofToString(isTrained), 20, 96);
    ofDrawBitmapString("Recording: " + ofToString(isRecording), 20, 110);
    ofDrawBitmapString("Model running: " + ofToString(isRunning2), 200, 82);
    ofDrawBitmapString("Model Trained: " + ofToString(isTrained2), 200, 96);
    ofDrawBitmapString("Recording: " + ofToString(isRecording2), 200, 110);
    ofPopStyle();

    ofPushStyle();
    ofSetColor(ofColor::cyan);
    ofRectangle trainingExamplesBar;
    trainingExamplesBar.setPosition(20, 120);
    trainingExamplesBar.setHeight(50);
    trainingExamplesBar.setWidth(data.size() + data2.size());
    ofDrawRectangle(trainingExamplesBar);
    for (int i = 0; i < lines.size(); i++) {
        lines[i].draw();
    }
    ofPopStyle();

    ofDrawBitmapString("Instructions:\n\nPress 'm' to randomise audio parameters\nPick area on screen to enter training data\nPress space to record training examples", 20, 200);
    ofDrawBitmapString("Keyboard Controls:\n\n\tspace -> toggle record data\n\tt -> train model\n\tr -> toggle run model\n\tc -> clear training examples\n\tq -> reset model", 20, 280);
}

//--------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer& output) {

    std::size_t outChannels = output.getNumChannels();

    for (int i = 0; i < output.getNumFrames(); ++i) {

        contactFreq = modalParams.getParameter(8);
        triggerFreq = modalParams.getParameter(9);
        modalOut = modal.play(contactFreq, triggerFreq, 1.0f);

        index = fmParams.getParameter(0);
        ratio = fmParams.getParameter(1);
        carrierFreq = fmParams.getParameter(2);
        fmOut = fm.play(index, ratio, carrierFreq);

        channel1 = mixer.stereo(modalOut, stereoSig, 1.0f);
        channel2 = mixer2.stereo(fmOut, stereoSig2, 0.0f);
        
        leftOut = *channel1 + *channel2;
        rightOut = *(channel1 + 1) + *(channel2+ 1);

        output[i * outChannels] = leftOut;
        output[i * outChannels + 1] = rightOut;
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    if (key == 'c' || key == 'C') {
        data.clear();
        data2.clear();
        lines.clear();
    }

    if (key == 'r' || key == 'R') {
        isRunning = !isRunning;
        isRunning2 = !isRunning2;
    }

    if (key == 'q' || key == 'Q') {
        neuralNet.reset();
        neuralNet2.reset();
        isEmpty = true;
        isTrained = false;
        isEmpty2 = true;
        isTrained2 = false;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
    switch (key) {
    case 32:
        isRecording = !isRecording;
        isRecording2 = !isRecording2;
    }

    if (key == 'm' || key == 'M') {
        /* Randomise parameters here */
        fmParams.randomise(fmRange);
        modalParams.randomise(modalRange);
    }

    if (key == 't' || key == 'T') {
        isTrained = neuralNet.train(data);
        isEmpty = false;
        isTrained2 = neuralNet2.train(data2);
        isEmpty2 = false;
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
    std::vector<double> trainingInput;
    std::vector<double> trainingOutput;
    std::vector<double> trainingOutput2;

    float mappedX = ofMap(x, 0, ofGetWidth(), 0.0f, 1.0f);
    float mappedY = ofMap(y, 0, ofGetHeight(), 0.0f, 1.0f);

    /* Normalise parameters here */
    fmParams.normalise(fmRange);
    modalParams.normalise(modalRange);
    

    if (isRecording && isRecording2 && !isTrained && !isTrained2) {
        trainingInput.push_back(mappedX);
        trainingInput.push_back(mappedY);

        trainingOutput = fmParams.getNormVector();
        trainingOutput2 = modalParams.getNormVector();

        rapidlib::trainingExample tempExample;
        tempExample.input = trainingInput;
        tempExample.output = trainingOutput;
        rapidlib::trainingExample tempExample2;
        tempExample2.input = trainingInput;
        tempExample2.output = trainingOutput2;
        data.push_back(tempExample);
        data2.push_back(tempExample2);
        tempLine.addVertex(x, y);
        lines.push_back(tempLine);
    }

    if (isRunning && isTrained) {
        std::vector<double> modelInput;
        modelInput.push_back(mappedX);
        modelInput.push_back(mappedY);
        modelOutput = neuralNet.run(modelInput);
        modelOutput2 = neuralNet2.run(modelInput);

        for (int i = 0; i < modelOutput.size(); i++) {
            if (modelOutput[i] < 0.0f) {
                modelOutput[i] = 0.0f;
            }
            else if (modelOutput[i] > 1.0f) {
                modelOutput[i] = 1.0f;
            }
        }

        for (int i = 0; i < modelOutput2.size(); i++) {
            if (modelOutput2[i] < 0.0f) {
                modelOutput2[i] = 0.0f;
            }
            else if (modelOutput2[i] > 1.0f) {
                modelOutput2[i] = 1.0f;
            }
        }

        /* Re-map parameter values here */
        fmParams.reMap(modelOutput, fmRange);
        modalParams.reMap(modelOutput2, modalRange);
    }
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
    tempLine.clear();
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h) {

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg) {

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {

}

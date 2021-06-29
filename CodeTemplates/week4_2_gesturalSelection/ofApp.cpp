#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {

    /* FBO Setup */
    fboWidth = 100.0f;
    fboHeight = 100.0f;
    fbo1.allocate(fboWidth, fboHeight, GL_RGB);
    fbo1.begin();
    ofClear(255);
    fbo1.end();
    fbo2.allocate(fboWidth, fboHeight, GL_RGB);
    fbo2.begin();
    ofClear(255);
    fbo2.end();
    fbo3.allocate(fboWidth, fboHeight, GL_RGB);
    fbo3.begin();
    ofClear(255);
    fbo3.end();

    /* RapidLib Bools */
    isTrained = false;
    isRunning = false;
    isEmpty = true;
    isRecording = false;
    isTrained2 = false;
    isRunning2 = false;
    isEmpty2 = true;
    isRecording = false;

    dtwRecording = false;
    dtwTrained = false;
    dtwRunning = false;
    dtwEmpty = true;
    dtwActive = false;
    regressionActive = true;
    neuralNet1Active = true;
    neuralNet2Active = true;

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
    ofDrawBitmapString("DTW Active: " + ofToString(dtwActive), 20, 30);
    if (!dtwRunning) {
        ofDrawBitmapString("DTW Training Label: " + ofToString(label), 150, 30);
    }
    else if (dtwRunning) {
        ofDrawBitmapString("DTW Training Label: " + outputLabel, 150, 30);
    }
    ofDrawBitmapString("DTW Trained: " + ofToString(dtwTrained), 20, 45);
    ofDrawBitmapString("DTW Recording: " + ofToString(dtwRecording), 150, 45);
    ofDrawBitmapString("DTW Training Examples: " + ofToString(classData.size()), 300, 45);
    ofDrawBitmapString("DTW Empty: " + ofToString(dtwEmpty), 20, 60);
    ofDrawBitmapString("DTW Running: " + ofToString(dtwRunning), 150, 60);
    ofPopStyle();

    ofPushStyle();
    ofSetColor(ofColor::beige);
    
    ofDrawBitmapString("Net 1 Examples: " + ofToString(data.size()), 20, 175);
    ofDrawBitmapString("Net 2 Examples: " + ofToString(data2.size()), 200, 175);
    ofDrawBitmapString("Model 1 running: " + ofToString(isRunning), 20, 190);
    ofDrawBitmapString("Model 1 Trained: " + ofToString(isTrained), 20, 205);
    ofDrawBitmapString("Recording: " + ofToString(isRecording), 20, 220);
    ofDrawBitmapString("Model 2 running: " + ofToString(isRunning2), 200, 190);
    ofDrawBitmapString("Model 2 Trained: " + ofToString(isTrained2), 200, 205);
    ofDrawBitmapString("Recording: " + ofToString(isRecording2), 200, 220);
    ofDrawBitmapString("Net 1 Active: " + ofToString(neuralNet1Active), 20, 230);
    ofDrawBitmapString("Net 2 Active: " + ofToString(neuralNet2Active), 200, 230);
    ofPopStyle();

    ofPushStyle();
    ofSetColor(ofColor::cyan);

    for (int i = 0; i < lines.size(); i++) {
        lines[i].draw();
    }

    ofRectangle trainingExamplesBar;
    trainingExamplesBar.setPosition(20, 230);
    trainingExamplesBar.setHeight(25);
    trainingExamplesBar.setWidth(data.size());
    ofDrawRectangle(trainingExamplesBar);

    ofSetColor(ofColor::orangeRed);
    ofRectangle trainingExamplesBar2;
    trainingExamplesBar2.setPosition(20, 255);
    trainingExamplesBar2.setHeight(25);
    trainingExamplesBar2.setWidth(data2.size());
    ofDrawRectangle(trainingExamplesBar2);

    ofSetColor(ofColor::red);
    inputDTWLine.draw();
    ofPopStyle();

    ofDrawBitmapString("Regression Instructions:\n\nPress 'm' to randomise audio parameters\nPress space to toggle recording\nDrag mouse to record x and y data", 20, 300);
    ofDrawBitmapString("DTW Instructions: \n\nPress 'd' to activate DTW\nPress 1, 2, or 3 to pick label\nPress space to toggle record\nDraw a figure with mouse", 20, 400);
    ofDrawBitmapString("Keyboard Controls:\n\n\tspace -> toggle record data\n\tt -> train model\n\tr -> toggle run model\n\tc -> clear training examples\n\tq -> reset model\n\td -> toggle DTW", 20, 500);

    fbo1.draw(20, 65);
    fbo2.draw(130, 65);
    fbo3.draw(240, 65);
}

//--------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer& output) {

    std::size_t outChannels = output.getNumChannels();

    for (int i = 0; i < output.getNumFrames(); ++i) {

        contactFreq = modalParams.getParameter(8);
        triggerFreq = modalParams.getParameter(9);
        if(neuralNet2Active) modalOut = modal.play(contactFreq, triggerFreq, 1.0f);

        index = fmParams.getParameter(0);
        ratio = fmParams.getParameter(1);
        carrierFreq = fmParams.getParameter(2);
        if(neuralNet1Active) fmOut = fm.play(index, ratio, carrierFreq);

        channel1 = mixer.stereo(modalOut, stereoSig, 1.0f);
        channel2 = mixer2.stereo(fmOut, stereoSig2, 0.0f);

        leftOut = *channel1 + *channel2;
        rightOut = *(channel1 + 1) + *(channel2 + 1);

        output[i * outChannels] = leftOut;
        output[i * outChannels + 1] = rightOut;
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    if (key == 'c' || key == 'C') {
        if (dtwActive && !regressionActive) {
            classData.clear();
        }
        else if (!dtwActive && regressionActive) {
            if (neuralNet1Active) data.clear();
            if (neuralNet2Active) data2.clear();
            lines.clear();
        }
    }

    if (key == 'q' || key == 'Q') {
        if (dtwActive && !regressionActive) {
            dtw.reset();
            dtwEmpty = true;
            dtwTrained = false;
        }
        else if (!dtwActive && regressionActive) {
            if (neuralNet1Active) {
                neuralNet.reset();
                isEmpty = true;
                isTrained = false;
            }

            if (neuralNet2Active) {
                neuralNet2.reset();
                isEmpty2 = true;
                isTrained2 = false;
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
    switch (key) {
    case 49: // 1
        label = 1;
        break;
    case 50: // 2
        label = 2;
        break;
    case 51: // 3
        label = 3;
        break;
    case 32: // space
        if (dtwActive && !regressionActive) {
            dtwLine.clear();
            dtwRecording = !dtwRecording;
        }
        else if (!dtwActive && regressionActive) {
            if(neuralNet1Active) isRecording = !isRecording;
            if(neuralNet2Active) isRecording2 = !isRecording2;
        }
        break;
    }

    if (key == 'r' || key == 'R') {
        if (dtwActive && !regressionActive) {
            dtwRunning = !dtwRunning;
        }
        else if (!dtwActive && regressionActive) {
            if (neuralNet1Active) isRunning = !isRunning;
            if (neuralNet2Active) isRunning2 = !isRunning2;
        }
    }

    if (key == 'm' || key == 'M') {
        /* Randomise parameters here */
        fmParams.randomise(fmRange);
        modalParams.randomise(modalRange);
    }

    if (key == 't' || key == 'T') {
        /* TODO:    If the dtw model is currently active,
                    train it on the global series classification
                    training set. Then indicate the the model
                    has been trained and is not empty. */
        
        if (!dtwActive && regressionActive) {
            if (neuralNet2Active) {
                isTrained = neuralNet.train(data);
                isEmpty = false;
            }
            
            if (neuralNet2Active) {
                isTrained2 = neuralNet2.train(data2);
                isEmpty2 = false;
            }
        }
    }

    if (key == 'd' || key == 'D') {
        dtwActive = !dtwActive;
        regressionActive = !regressionActive;
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

    /* TODO: Declare an input vector for the dtw model. */

    /* Neural Net Input */
    std::vector<double> trainingInput;
    std::vector<double> trainingOutput;
    std::vector<double> trainingOutput2;

    float mappedX = ofMap(x, 0, ofGetWidth(), 0.0f, 1.0f);
    float mappedY = ofMap(y, 0, ofGetHeight(), 0.0f, 1.0f);

    /* Normalise parameters here */
    fmParams.normalise(fmRange);
    modalParams.normalise(modalRange);

    if (dtwActive && !regressionActive) {

        /* DTW Input */
        ofPoint inputPoint;
        inputPoint.set(x, y);
        inputDTWLine.addVertex(inputPoint);

        if (dtwRecording && !dtwTrained) {
            /* TODO:    Add the current input data to the dtw input vector,
                        add the vector to the temporary training example, give
                        the data an output label. */

            ofPoint point1, point2, point3;
            float xPos = ofMap(ofGetMouseX(), 0, ofGetWidth(), 0, fboWidth);
            float yPos = ofMap(ofGetMouseY(), 0, ofGetHeight(), 0, fboHeight);
            switch (label) {
            case 1:
                fbo1.begin();
                ofBackground(0);
                point1.set(xPos, yPos);
                dtwLine.addVertex(point1);
                ofSetColor(ofColor::beige);
                dtwLine.draw();
                fbo1.end();
                break;
            case 2:
                fbo2.begin();
                ofBackground(0);
                point2.set(xPos, yPos);
                dtwLine.addVertex(point2);
                ofSetColor(ofColor::beige);
                dtwLine.draw();
                fbo2.end();
                break;
            case 3:
                fbo3.begin();
                ofBackground(0);
                point3.set(xPos, yPos);
                dtwLine.addVertex(point3);
                ofSetColor(ofColor::beige);
                dtwLine.draw();
                fbo3.end();
                break;
            }
        }

        if (dtwRunning && dtwTrained) {
            /* TODO:    Create an input vector for the dtw model,
                        add the current input data, run the dtw model 
                        continuously on the input data. */
            
            /* TODO:    Use the output of the dtw model to
                        activate and deactive the neural networks. */
        }
            
    }
    else if (!dtwActive && regressionActive) {

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
            if (neuralNet1Active) {
                modelOutput = neuralNet.run(modelInput);

                for (int i = 0; i < modelOutput.size(); i++) {
                    if (modelOutput[i] < 0.0f) {
                        modelOutput[i] = 0.0f;
                    }
                    else if (modelOutput[i] > 1.0f) {
                        modelOutput[i] = 1.0f;
                    }
                }

                /* Re-map parameter values here */
                fmParams.reMap(modelOutput, fmRange);
            }
                
            if (neuralNet2Active) {
                modelOutput2 = neuralNet2.run(modelInput);

                for (int i = 0; i < modelOutput2.size(); i++) {
                    if (modelOutput2[i] < 0.0f) {
                        modelOutput2[i] = 0.0f;
                    }
                    else if (modelOutput2[i] > 1.0f) {
                        modelOutput2[i] = 1.0f;
                    }
                }

                /* Re-map parameter values here */
                modalParams.reMap(modelOutput2, modalRange);
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button) {
    inputDTWLine.clear();
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button) {
    tempLine.clear();

    if (dtwActive && !regressionActive && dtwRecording && !dtwTrained) {
        /* TODO:    Add the temporary series training example to 
                    the global training set. Then make sure to 
                    clear the temporary example. */
    }
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

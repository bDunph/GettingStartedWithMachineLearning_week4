#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup() {

    /* RapidLib Bools */
    isTrained = false;
    isRunning = false;
    isEmpty = true;

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

    fft.setup(1024, 512, 256);

    attack = 15.0f;
    decay = 10.0f;
    sustain = 0.9f;
    release = 1000.0f;
    triggerFreq = 0.5f;

    /* FM synth setup */
    index = 30.0f;
    ratio = 4.1f;
    carrierFreq = 400.0f;

    fm.setup(index, ratio, carrierFreq);

    fmRange = 0.5f;
    fmParamVector = { index, ratio, carrierFreq, triggerFreq };
    fmParams.setup(fmParamVector);

    /* Graphics Stuff */
    ofEnableDepthTest();
    ofSetSmoothLighting(true);

    radius = 250.0f;
    resolution = 2.0f;

    lightPos = ofVec3f(502.0f, 502.0f, -501.0f);

    colR = 0.75f;
    colG = 0.75f;
    colB = 0.75f;

    noiseSpeedFactor = 10.0f;
    noiseScaleFactor = 15.0f;
    noiseDisplacementFactor = 50.0f;

    imageW = 80.0f;
    imageH = 60.0f;
    noiseTex.allocate(imageW, imageH, OF_IMAGE_GRAYSCALE);

    icoSphere.mapTexCoordsFromTexture(noiseTex.getTexture());
    shader.load("shader");

    fboWidth = ofGetWidth() * 0.5f;
    fboHeight = ofGetHeight();
    fbo1.allocate(fboWidth, fboHeight, GL_RGBA);
    fbo1.begin();
    ofClear(255);
    fbo1.end();

    graphicsParamRange = 0.5f;
    graphicsParamVector = { noiseSpeedFactor, colR, colG, colB, resolution, noiseScaleFactor, noiseDisplacementFactor };
    graphicsParams.setup(graphicsParamVector);
}

//--------------------------------------------------------------
void ofApp::update() {

    envelope.setAttack(attack);
    envelope.setDecay(decay);
    envelope.setSustain(sustain);
    envelope.setRelease(release);

    /* Graphics code for rendering wireframe boxes in the FBO */
    camPos = cam.getGlobalPosition();

    radius = rms * 10000.0f;
    icoSphere.setRadius(radius);
    resolution = graphicsParams.getParameter(4);
    icoSphere.setResolution(resolution);

    modelMat = icoSphere.getLocalTransformMatrix();
    inverseMat = modelMat.getInverse();
    normalMat = inverseMat.getTransposedOf(inverseMat);

    /* Fill the texture with noise values to be read in the shader. */
    noiseSpeedFactor = graphicsParams.getParameter(0);
    float noiseVel = ofGetElapsedTimef() * noiseSpeedFactor;
    noiseScaleFactor = graphicsParams.getParameter(5);
    float noiseScale = noiseScaleFactor;

    ofPixels& pixels = noiseTex.getPixels();
    int w = noiseTex.getWidth();
    int h = noiseTex.getHeight();
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int i = y * w + x;
            float noiseValue = ofNoise(x * noiseScale, y * noiseScale, noiseVel);
            pixels[i] = 255 * noiseValue;
        }
    }
    noiseTex.update();

    /* Position the boxes on the surface of a sphere. */
    ofMesh icoMesh = icoSphere.getMesh();
    std::vector<glm::vec3> verts = icoMesh.getVertices();
    for (int i = 0; i < verts.size(); i++) {
        ofBoxPrimitive tempBox;
        boxes.push_back(tempBox);
        boxes[i].setPosition(verts[i]);
        boxes[i].set(radius * 0.05f);
    }

    noiseDisplacementFactor = graphicsParams.getParameter(6);
    colR = graphicsParams.getParameter(1);
    colG = graphicsParams.getParameter(2);
    colB = graphicsParams.getParameter(3);

    /* Render the wireframe boxes to an FBO */
    fbo1.begin();
    ofEnableLighting();
    ofBackground(0);

    noiseTex.getTexture().bind();
    shader.begin();

    ofPushMatrix();
    ofTranslate(fboWidth * 0.5f, fboHeight * 0.5f);
    cam.begin();

    shader.setUniform1f("colR", colR);
    shader.setUniform1f("colG", colG);
    shader.setUniform1f("colB", colB);
    shader.setUniform1f("noiseDisplacementFactor", noiseDisplacementFactor);
    shader.setUniform3f("lightPos", lightPos);
    shader.setUniform3f("camPos", camPos);
    shader.setUniformMatrix4f("normalMat", normalMat);

    for (int i = 0; i < boxes.size(); i++)
    {
        boxes[i].drawWireframe();
    }

    cam.end();
    ofPopMatrix();

    shader.end();
    noiseTex.getTexture().unbind();
    fbo1.end();

    boxes.clear();
}

//--------------------------------------------------------------
void ofApp::draw() {
    ofBackground(100, 100, 100);

    ofPushStyle();
    ofSetColor(ofColor::beige);
    ofDrawBitmapString("Number of training examples: " + ofToString(data.size() + graphicsTrainingSet.size()), 20, 68);
    ofDrawBitmapString("Model running: " + ofToString(isRunning), 20, 82);
    ofDrawBitmapString("Model Trained: " + ofToString(isTrained), 20, 96);
    ofDrawBitmapString("Recording: " + ofToString(isRecording), 20, 110);
    ofPopStyle();

    ofPushStyle();
    ofSetColor(ofColor::cyan);
    ofRectangle trainingExamplesBar;
    trainingExamplesBar.setPosition(20, 120);
    trainingExamplesBar.setHeight(50);
    trainingExamplesBar.setWidth(data.size() + graphicsTrainingSet.size());
    ofDrawRectangle(trainingExamplesBar);
    for (int i = 0; i < lines.size(); i++) {
        lines[i].draw();
    }
    ofPopStyle();

    ofPushStyle();
    ofSetColor(ofColor::beige);
    ofDrawBitmapString("Instructions:\n\nPress 'm' to randomise audio and visual parameters\nPick area on screen to enter training data\nPress space to record training examples", 20, 200);
    ofDrawBitmapString("Keyboard Controls:\n\n\tspace -> toggle record data\n\tt -> train model\n\tr -> toggle run model\n\tc -> clear training examples\n\tq -> reset model", 20, 280);
    ofPopStyle();

    fbo1.draw(ofGetWidth() * 0.5f, 0);
}

//--------------------------------------------------------------
void ofApp::audioOut(ofSoundBuffer& output) {

    std::size_t outChannels = output.getNumChannels();

    for (int i = 0; i < output.getNumFrames(); ++i) {

        index = fmParams.getParameter(0);
        ratio = fmParams.getParameter(1);
        carrierFreq = fmParams.getParameter(2);
        fmOut = fm.play(index, ratio, carrierFreq);

        triggerFreq = fmParams.getParameter(3);
        triggerSignal = envTrigger.phasor(triggerFreq);
        
        envSignal = envelope.adsr(fmOut, (int)triggerSignal);

        masterOut = envSignal;

        /* FFT Analysis */
        fft.process(masterOut);
        magsDBVals = fft.magsToDB();

        /* RMS Analysis */
        rapidStream.pushToWindow(masterOut);
        rms = rapidStream.rms();

        if (masterOut > 0.95f) {
            masterOut = 0.95f;
        }

        output[i * outChannels] = masterOut * 0.8f;
        output[i * outChannels + 1] = output[i * outChannels];
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    if (key == 'c' || key == 'C') {
        data.clear();
        graphicsTrainingSet.clear();
        lines.clear();
    }

    if (key == 'r' || key == 'R') {
        isRunning = !isRunning;
    }

    if (key == 'q' || key == 'Q') {
        neuralNet.reset();
        graphicsNeuralNet.reset();
        isEmpty = true;
        isTrained = false;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
    switch (key) {
    case 32:
        isRecording = !isRecording;
    }

    if (key == 'm' || key == 'M') {
        /* Randomise parameters here */
        fmParams.randomise(fmRange);
        graphicsParams.randomise(graphicsParamRange);
    }

    if (key == 't' || key == 'T') {
        isTrained = neuralNet.train(data);
        isTrained = graphicsNeuralNet.train(graphicsTrainingSet);
        isEmpty = false;
    }
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {
    std::vector<double> trainingInput;
    std::vector<double> trainingOutput;

    std::vector<double> graphicsTrainingInput;
    std::vector<double> graphicsTrainingOutput;

    float mappedX = ofMap(x, 0, ofGetWidth(), 0.0f, 1.0f);
    float mappedY = ofMap(y, 0, ofGetHeight(), 0.0f, 1.0f);

    /* Normalise parameters here */
    fmParams.normalise(fmRange);
    graphicsParams.normalise(graphicsParamRange);

    if (isRecording && !isTrained) {
        trainingInput.push_back(mappedX);
        trainingInput.push_back(mappedY);

        trainingOutput = fmParams.getNormVector();

        rapidlib::trainingExample tempExample;
        tempExample.input = trainingInput;
        tempExample.output = trainingOutput;
        data.push_back(tempExample);

        // retrieve the db values of each bin, set max val 120, normalise and store in vector
        std::vector<double> dbMagnitudes;
        for (int i = 0; i < fft.bins; i+=10) {
            if (*(magsDBVals + i) > 120.0f) *(magsDBVals + i) = 120.0f;
            double normVal = ofMap((double)*(magsDBVals + i), 0.0f, 120.0f, 0.0f, 1.0f);
            dbMagnitudes.push_back(normVal);
        }
        graphicsTrainingInput = dbMagnitudes;

        graphicsTrainingOutput = graphicsParams.getNormVector();

        rapidlib::trainingExample tempExample2;
        tempExample2.input = graphicsTrainingInput;
        tempExample2.output = graphicsTrainingOutput;
        graphicsTrainingSet.push_back(tempExample2);

        tempLine.addVertex(x, y);
        lines.push_back(tempLine);
    }

    if (isRunning && isTrained) {
        std::vector<double> modelInput;
        modelInput.push_back(mappedX);
        modelInput.push_back(mappedY);
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

        // retrieve the db values of each bin, set max val 120, normalise and store in vector
        std::vector<double> dbMagnitudes;
        for (int i = 0; i < fft.bins; i+=10) {
            if (*(magsDBVals + i) > 120.0f) *(magsDBVals + i) = 120.0f;
            double normVal = ofMap((double)*(magsDBVals + i), 0.0f, 120.0f, 0.0f, 1.0f);
            dbMagnitudes.push_back(normVal);
        }

        std::vector<double> graphicsModelInput;
        graphicsModelInput = dbMagnitudes;
        graphicsModelOut = graphicsNeuralNet.run(graphicsModelInput);

        for (int i = 0; i < graphicsModelOut.size(); i++) {
            if (graphicsModelOut[i] < 0.0f) {
                graphicsModelOut[i] = 0.0f;
            }
            else if (graphicsModelOut[i] > 1.0f) {
                graphicsModelOut[i] = 1.0f;
            }
        }

        graphicsParams.reMap(graphicsModelOut, graphicsParamRange);
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

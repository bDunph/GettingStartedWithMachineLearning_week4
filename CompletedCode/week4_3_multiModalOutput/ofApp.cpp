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

    attack = 15.0f;
    decay = 10.0f;
    sustain = 0.9f;
    release = 1000.0f;
    triggerFreq = 5.0f;

    /* FM synth setup */
    index = 15.0f;
    ratio = 2.1f;
    carrierFreq = 200.0f;

    fm.setup(index, ratio, carrierFreq);

    fmRange = 0.5f;
    fmParamVector = { index, ratio, carrierFreq, triggerFreq };
    fmParams.setup(fmParamVector);

    /* Graphics Stuff */
    ofEnableDepthTest();
    ofSetSmoothLighting(true);

    radius = 100.0f;
    resolution = 2.0f;

    lightPos = ofVec3f(200.0f, 200.0f, -200.0f);

    colR = 0.75f;
    colG = 0.75f;
    colB = 0.75f;

    noiseSpeedFactor = 12.0f;
    noiseScaleFactor = 0.9f;
    noiseDisplacementFactor = 12.0f;

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
    graphicsParamVector = { radius, resolution, noiseSpeedFactor, noiseScaleFactor, noiseDisplacementFactor, colR, colG, colB };
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

    radius = graphicsParams.getParameter(0);
    icoSphere.setRadius(radius);
    radius = graphicsParams.getParameter(1);
    icoSphere.setResolution(resolution);

    modelMat = icoSphere.getLocalTransformMatrix();
    inverseMat = modelMat.getInverse();
    normalMat = inverseMat.getTransposedOf(inverseMat);

    /* Fill the texture with noise values to be read in the shader. */
    noiseSpeedFactor = graphicsParams.getParameter(2);
    float noiseVel = ofGetElapsedTimef() * noiseSpeedFactor;
    noiseScaleFactor = graphicsParams.getParameter(3);
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
        boxes[i].set(radius * 0.25f);
    }

    noiseDisplacementFactor = graphicsParams.getParameter(4);
    colR = graphicsParams.getParameter(5);
    colG = graphicsParams.getParameter(6);
    colB = graphicsParams.getParameter(7);

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
    ofDrawBitmapString("Number of training examples: " + ofToString(data.size()), 20, 68);
    ofDrawBitmapString("Model running: " + ofToString(isRunning), 20, 82);
    ofDrawBitmapString("Model Trained: " + ofToString(isTrained), 20, 96);
    ofDrawBitmapString("Recording: " + ofToString(isRecording), 20, 110);
    ofPopStyle();

    ofPushStyle();
    ofSetColor(ofColor::cyan);
    ofRectangle trainingExamplesBar;
    trainingExamplesBar.setPosition(20, 120);
    trainingExamplesBar.setHeight(50);
    trainingExamplesBar.setWidth(data.size());
    ofDrawRectangle(trainingExamplesBar);
    for (int i = 0; i < lines.size(); i++) {
        lines[i].draw();
    }
    ofPopStyle();

    ofPushStyle();
    ofSetColor(ofColor::beige);
    ofDrawBitmapString("Instructions:\n\nPress 'm' to randomise audio parameters\nPick area on screen to enter training data\nPress space to record training examples", 20, 200);
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
        lines.clear();
    }

    if (key == 'r' || key == 'R') {
        isRunning = !isRunning;
    }

    if (key == 'q' || key == 'Q') {
        neuralNet.reset();
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

    float mappedX = ofMap(x, 0, ofGetWidth(), 0.0f, 1.0f);
    float mappedY = ofMap(y, 0, ofGetHeight(), 0.0f, 1.0f);

    /* Normalise parameters here */
    fmParams.normalise(fmRange);
    graphicsParams.normalise(graphicsParamRange);

    if (isRecording && !isTrained) {
        trainingInput.push_back(mappedX);
        trainingInput.push_back(mappedY);

        std::vector<double> fmNormVector = fmParams.getNormVector();
        std::vector<double> graphicsNormVector = graphicsParams.getNormVector();
        trainingOutput = fmNormVector;
        trainingOutput.insert(trainingOutput.end(), graphicsNormVector.begin(), graphicsNormVector.end());

        rapidlib::trainingExample tempExample;
        tempExample.input = trainingInput;
        tempExample.output = trainingOutput;
        data.push_back(tempExample);

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
        std::vector<double> fmOutVector = { modelOutput[0], modelOutput[1], modelOutput[2], modelOutput[3] };
        fmParams.reMap(fmOutVector, fmRange);

        std::vector<double> graphicsOutVector = { modelOutput[4], modelOutput[5], modelOutput[6], modelOutput[7], modelOutput[8], modelOutput[9], modelOutput[10], modelOutput[11] };
        graphicsParams.reMap(graphicsOutVector, graphicsParamRange);
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

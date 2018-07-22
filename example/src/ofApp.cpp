#include "ofApp.h"

using namespace ofxCv;
using namespace cv;


//--------------------------------------------------------------
void ofApp::setup(){
    isCam = false;
    
    ofSetWindowShape(1280, 800);
    ofSetVerticalSync(true);
    
    if (isCam) {
        cam.setup(width, height);
    } else {
        src.load("test1.jpg");
        src.resize(width, height);
    }
    
    ofFbo::Settings settings;
    settings.useStencil = true;
    settings.height = height;
    settings.width = width;
    settings.internalformat = GL_RGB; //GL_RGBA32F_ARB;
    settings.numSamples = 1;
    
    filterFbo.allocate(settings);
    
    diff.resize(numTrackingColors);
    diffInverted.resize(numTrackingColors);
    diffThresholded.resize(numTrackingColors);
    
    colors.resize(numTrackingColors);
    targetColors.resize(numTrackingColors);
    filters.resize(numTrackingColors);
    
    for (int i=0; i<numTrackingColors; i++) {
        colors[i] = ofColor(0, 0, 0);
        filters[i].allocate(width, height, OF_IMAGE_COLOR);
    }
    
    targetColors[0].set(255, 0, 0);
    targetColors[1].set(0, 255, 0);
    targetColors[2].set(0, 0, 255);
    
}

//--------------------------------------------------------------
void ofApp::update(){
    
    if (isCam) {
        cam.update();
        if(cam.isFrameNew()) {
            src.setFromPixels(cam.getPixels());
            analyzeImage();
        }
    }
    
    analyzeImage();
}

//--------------------------------------------------------------
void ofApp::analyzeImage() {
    
    // take the absolute difference of prev and src and save it inside diff
    
    float x = ofMap(ofGetMouseX(), 0, ofGetWidth(), 0, 255);
    
    for (int i=0; i<numTrackingColors; i++) {
        absdiff(src, filters[i], diff[i]);
        invert(diff[i]);
        copyGray(diff[i], diffInverted[i]);
        threshold(diffInverted[i], diffThresholded[i], x);
        diffThresholded[i].update();
    }
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);
    
    ofSetColor(255);
    src.draw(0, 0);
    for (int i=0; i<numTrackingColors; i++) {
        diffThresholded[i].draw(320*(i+1), 0);
    }
    
    //diffI.draw(640, 0);
    
    filters[0].draw(0, 240, 20, 20);
    filters[1].draw(320, 240, 20, 20);
    filters[2].draw(640, 240, 20, 20);
    
    
    ofPushStyle();
    ofSetColor(255);
    for (int i=0; i<numTrackingColors; i++) {
        ofEnableBlendMode(OF_BLENDMODE_ADD);
        ofSetColor(targetColors[i]);
        diffThresholded[i].draw(0, 280);
        ofDisableBlendMode();
    }
    ofSetColor(255);
    ofPopStyle();
    
}
//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    int x = ofGetMouseX();
    int y = ofGetMouseY();
    
    unsigned char * pixels = src.getPixels().getData();
    int index = 3 * (y * src.getWidth() + x);
    int r = pixels[index];
    int g = pixels[index+1];
    int b = pixels[index+2];
    
    if (key=='1') {
        colors[0].set(r, g, b);
        updateFilter(0);
    }
    else if (key=='2') {
        colors[1].set(r, g, b);
        updateFilter(1);
    }
    else if (key=='3') {
        colors[2].set(r, g, b);
        updateFilter(2);
    }
}

//--------------------------------------------------------------
void ofApp::updateFilter(int idx) {
    filterFbo.begin();
    ofSetColor(colors[idx]);
    ofFill();
    ofDrawRectangle(0, 0, filterFbo.getWidth(), filterFbo.getHeight());
    filterFbo.end();
    filterFbo.readToPixels(filters[idx]);
    filters[idx].update();
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){
    
}


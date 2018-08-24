#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
    ofSetWindowShape(1280, 800);
    ofSetVerticalSync(true);
    
    sandbox.setup(width, height, numTrackingColors);

    if (isCam) {
        cam.setup(width, height);
    } else {
        src.load("test1.jpg");
        src.resize(width, height);
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    
    if (isCam) {
        cam.update();
        if(cam.isFrameNew()) {
            src.setFromPixels(cam.getPixels());
        }
    }
    
    float thresh = ofMap(ofGetMouseX(), 0, ofGetWidth(), 0, 255);
    cout << "update " << thresh << endl;
    sandbox.setThreshold(thresh);
    sandbox.update(src);
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);
    
    ofSetColor(255);
    src.draw(0, 0);
    sandbox.draw(width, 0);
    sandbox.drawDebug(0, height);
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
        sandbox.setFilterColor(0, ofColor(r, g, b));
    }
    else if (key=='2') {
        sandbox.setFilterColor(1, ofColor(r, g, b));
    }
    else if (key=='3') {
        sandbox.setFilterColor(2, ofColor(r, g, b));
    }
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


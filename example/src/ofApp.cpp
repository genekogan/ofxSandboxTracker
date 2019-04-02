#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
    ofSetWindowShape(1440, 1080);
    ofSetVerticalSync(true);
    
    width = 640;
    height = 480;
    srcMode = 0;

    sandbox.setup(width, height);
    sandbox.setDebugPosition(0, 0);

    sandbox.setTrackColor(0, ofColor(255, 0, 0));
    sandbox.setTrackColor(1, ofColor(10, 255, 10));
    sandbox.setTrackColor(2, ofColor(0, 0, 255));
    sandbox.setTrackColor(3, ofColor(255, 255, 255));
    sandbox.setTrackColor(4, ofColor(0, 0, 0));
    sandbox.setOutColor(0, ofColor(255, 0, 0));
    sandbox.setOutColor(1, ofColor(0, 255, 0));
    sandbox.setOutColor(2, ofColor(0, 0, 255));
    sandbox.setOutColor(3, ofColor(255, 255, 255));
    sandbox.setOutColor(4, ofColor(0, 0, 0));
    
    sandbox.loadSettings();
    
    if (srcMode==0) {
        cam.setDeviceID(1);
        cam.setup(640, 480);
    } else if (srcMode==1) {
        video.load("/Users/gene/Documents/futurium_test2.mp4");
        video.setLoopState(OF_LOOP_NORMAL);
        video.play();
    } else {
        src.load("test1.jpg");
        src.resize(width, height);
    }
}

//--------------------------------------------------------------
void ofApp::exit() {
    sandbox.saveSettings();
}

//--------------------------------------------------------------
void ofApp::update(){
    
    if (srcMode==0) {
        cam.update();
        if(cam.isFrameNew()) {
            src.setFromPixels(cam.getPixels());
            src.resize(width, height);
        }
    } else if (srcMode==1) {
        video.update();
        if(video.isFrameNew()) {
            src.setFromPixels(video.getPixels());
            src.resize(width, height);
        }
    }
    
    
    
    
    //float thresh = ofMap(ofGetMouseX(), 0, ofGetWidth(), 0, 255);
    //cout << "update " << thresh << endl;
    //sandbox.setThreshold(thresh);
    sandbox.update(src);
}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(0);
    
    ofSetColor(255);

//    src.draw(0, 0);
//    sandbox.draw(width, 0);
    sandbox.drawDebug();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    sandbox.keyEvent(key);
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


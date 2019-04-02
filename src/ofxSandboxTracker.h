#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxGui.h"
#include "ofxCv.h"
#include "ofxHomography.h"


class ofxSandboxTracker {
public:
    ofxSandboxTracker();
    ~ofxSandboxTracker();
    
    void setup(int width, int height);
    void update(ofPixels & src);
    void setDebugPosition(int x, int y);
    
    bool isMotionTripped();
    
    void draw(int x, int y);
    void drawDebug();

    ofFbo & getFbo() {return shaderFbo;}
    
    void setThreshold(float thresh) {this->thresh = thresh;}
    void setTrackColor(int idx, ofColor clr);
    void setOutColor(int idx, ofColor clr);
    void setCorner(int idx, int x, int y);
    
    void saveSettings(string filename);
    void loadSettings(string filename);
    void saveSettings() {saveSettings("settings.xml");}
    void loadSettings() {loadSettings("settings.xml");}

    void keyEvent(int key);
    
protected:
    
    vector<ofColor> trackColors;
    vector<ofColor> outColors;

    ofFbo shaderFbo;
    ofShader shader;
    ofImage sandboxCurrent;
    ofImage sandboxPrev;
    ofImage srcImage;
    ofImage diff;
    cv::Scalar motion;
    
    ofParameter<float> amtMotion;
    ofParameter<float> motionLerp;
    ofParameter<float> motionThreshLow;
    ofParameter<float> motionThreshHigh;
    ofParameter<float> gBlurRadius;
    ofParameter<float> gNewFrameIndicator;
    
    ofxPanel gui;
    
    int dx, dy;
    int width;
    int height;
    int numTrackingColors;
    float thresh;
    
    bool newFrame;
    bool overwritePrev;
    bool motionTrip;
    bool motionReady;
    
    ofPoint originalCorners[4];
    ofPoint distortedCorners[4];
    ofMatrix4x4 homography;
    ofFbo distortedFbo;
};



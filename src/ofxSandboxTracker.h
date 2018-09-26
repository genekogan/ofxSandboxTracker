#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxCv.h"


class ofxSandboxTracker {
public:
    ofxSandboxTracker();
    ~ofxSandboxTracker();
    
    void setup(int width, int height);
    void update(ofPixels & src);
    void setDebugPosition(int x, int y);
    
    void draw(int x, int y);
    void drawDebug();

    ofFbo & getFbo() {return shaderFbo;}
    
    void setThreshold(float thresh) {this->thresh = thresh;}
    void setTrackColor(int idx, ofColor clr);
    void setOutColor(int idx, ofColor clr);

    void keyEvent(int key);
    
protected:
    
    vector<ofColor> trackColors;
    vector<ofColor> outColors;

    ofFbo shaderFbo;
    ofShader shader;
    ofImage srcImage;
    ofPixels previous;
    ofImage diff;
    cv::Scalar motion;
    
    ofParameter<float> amtMotion;
    ofParameter<float> motionLerp;
    ofParameter<float> motionThreshLow;
    ofParameter<float> motionThreshHigh;
    
    int width;
    int height;
    int numTrackingColors;
    float thresh;
    
    ofxPanel gui;
    int dx, dy;
};



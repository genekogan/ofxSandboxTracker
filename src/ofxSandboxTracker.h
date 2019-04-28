#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
#include "ofxGui.h"
#include "ofxCv.h"
#include "ofxHomography.h"
#include "ofxDraggable.h"
#include "ofxClickable.h"



//--------------------------------------------------------------
class ClickableColor : public ofxClickable {
public:
    ClickableColor(int idx) {this->idx = idx;}
    void buttonClicked() {ofNotifyEvent(event, idx, this);}
    ofEvent<int> event;
    int idx;
};


//--------------------------------------------------------------
class ofxSandboxTracker {
public:
    
    bool colorSelected;
    ofTrueTypeFont font;
    
    void setAllColorSelectorsInactive(){
        colorSelected = false;
        for (auto c : colorSelectorsIn) {
            c->setActive(false);
        }
        for (auto c : colorSelectorsOut) {
            c->setActive(false);
        }
    }
    
    void colorInEvent(int & idx) {
        if (colorSelectorsIn[idx]->getActive()) {
            colorSelectorsIn[idx]->setActive(false);
            return;
        }
        setAllColorSelectorsInactive();
        colorSelectorsIn[idx]->setActive(true);
        colorSelected = true;
    }
    
    void colorOutEvent(int & idx) {
        if (colorSelectorsOut[idx]->getActive()) {
            colorSelectorsOut[idx]->setActive(false);
            return;
        }
        setAllColorSelectorsInactive();
        colorSelectorsOut[idx]->setActive(true);
        colorSelected = true;
    }
    
    
    
    
    
    
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
    
    void mouseMoved(int x, int y);
    void mousePressed(int x, int y);
    void mouseDragged(int x, int y);
    void mouseReleased(int x, int y);
    
protected:
    
    void updateHomography();
    
    ofxDraggable draggable;

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
    
    //ofPoint originalCorners[4];
    ofPoint distortedCorners[4];
    ofMatrix4x4 homography;
    ofFbo distortedFbo;
    

    
    
    
    vector<ClickableColor*> colorSelectorsIn;
    vector<ClickableColor*> colorSelectorsOut;
    
};



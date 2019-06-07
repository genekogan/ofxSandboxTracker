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
    
    ofxSandboxTracker();
    ~ofxSandboxTracker();
    
    void setup(int width, int height);
    void update(ofPixels & src);
    void setDebugPosition(int x, int y);
    
    bool isMotionTripped();
    bool isSettingsChanged();
    
    void draw(int x, int y);
    void drawDebug();

    ofFbo & getFbo() {return shaderFbo;}
    
    void setThreshold(float thresh) {this->thresh = thresh;}
    void setTrackColor(int idx, ofColor clr);
    void setOutColor(int idx, ofColor clr);
    void setCorner(int idx, int x, int y);
    void setEllipseSize(int ellipseSize) {draggable.setEllipseSize(ellipseSize);}
    
    void saveSettings(string guiFilename, string sandboxFilename);
    void loadSettings(string guiFilename, string sandboxFilename);
    void saveSettings() {saveSettings("sandboxSettings.xml", "settings.xml");}
    void loadSettings() {loadSettings("sandboxSettings.xml", "settings.xml");}

    void keyEvent(int key);
    
    void mouseMoved(int x, int y);
    void mousePressed(int x, int y);
    void mouseDragged(int x, int y);
    void mouseReleased(int x, int y);
    
protected:
    
    void updateHomography();
    
    void setAllColorSelectorsInactive();
    void colorInEvent(int & idx);
    void colorOutEvent(int & idx);
    
    // cv
    ofFbo shaderFbo;
    ofShader shader;
    ofImage sandboxCurrent;
    ofImage sandboxPrev;
    ofImage srcImage;
    ofImage diff;
    cv::Scalar motion;
    
    // gui
    ofxPanel gui;
    ofVec2f sandbox_margin;
    ofParameter<float> amtMotion;
    ofParameter<float> motionLerp;
    ofParameter<float> motionThreshLow;
    ofParameter<float> motionThreshHigh;
    ofParameter<float> gBlurRadius;
    ofParameter<float> gNewFrameIndicator;
    
    // internal variables
    int dx, dy;
    int width;
    int height;
    int numTrackingColors;
    float thresh;
    bool newFrame;
    bool overwritePrev;
    bool motionTrip;
    bool motionReady;
    bool settingsChanged;
    
    //homography
    ofxDraggable draggable;
    ofPoint distortedCorners[4];
    ofMatrix4x4 homography;
    ofFbo distortedFbo;
    
    // color picker
    vector<ofColor> trackColors;
    vector<ofColor> outColors;
    vector<ClickableColor*> colorSelectorsIn;
    vector<ClickableColor*> colorSelectorsOut;
    bool colorSelected;
    ofTrueTypeFont font;
    ofColor selectedColor;
    int selectedColorIdx;
    
};



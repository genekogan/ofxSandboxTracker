#pragma once

#include "ofMain.h"
#include "ofxCv.h"


class ofxSandboxTracker {
public:
    ofxSandboxTracker();
    ~ofxSandboxTracker();
    
    void setup(int width, int height, int numTrackingColors);
    void update(ofPixels & src);
    
    void draw(int x, int y);
    void drawDebug(int x, int y);

    ofFbo & getFbo() {return shaderFbo;}
    
    void setThreshold(float thresh) {this->thresh = thresh;}
    void setFilterColor(int idxFilter, ofColor color);
    
protected:
    
    void updateFilter(int idx);
    
    vector<ofImage> diff;
    vector<ofImage> diffInverted;
    vector<ofImage> diffThresholded;
    vector<ofImage> filters;
    
    vector<ofColor> colors, targetColors;
    ofFbo filterFbo;
    ofFbo shaderFbo;
    ofShader shader;
    
    int width;
    int height;
    int numTrackingColors;
    
    float thresh;
};



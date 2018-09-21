#include "ofxSandboxTracker.h"

#define STRINGIFY(A) #A
using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
ofxSandboxTracker::ofxSandboxTracker() {
    thresh = 128.0;
}

//--------------------------------------------------------------
ofxSandboxTracker::~ofxSandboxTracker() {
    
}

//--------------------------------------------------------------
void ofxSandboxTracker::setup(int width, int height, int numTrackingColors) {
    this->width = width;
    this->height = height;
    this->numTrackingColors = numTrackingColors;
    
    string shaderProgram = STRINGIFY(
        uniform sampler2DRect tex0;
        uniform sampler2DRect tex1;
        uniform sampler2DRect tex2;

        void main (void){
         vec2 pos = gl_TexCoord[0].st;
         
         vec4 rTxt = texture2DRect(tex0, pos);
         vec4 gTxt = texture2DRect(tex1, pos);
         vec4 bTxt = texture2DRect(tex2, pos);
         
         vec4 color = vec4(1,1,1,1);
         color = mix(color, vec4(1,0,0,1), rTxt.r );
         color = mix(color, vec4(0.039,1,0.039,1), gTxt.r );
         color = mix(color, vec4(0,0,1,1), bTxt.r );
         
         gl_FragColor = color;
        }
    );
    
    shader.setupShaderFromSource(GL_FRAGMENT_SHADER, shaderProgram);
    shader.linkProgram();
    shaderFbo.allocate(width, height);

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
void ofxSandboxTracker::update(ofPixels & src) {
    
    for (int i=0; i<numTrackingColors; i++) {
        absdiff(src, filters[i], diff[i]);
        invert(diff[i]);
        copyGray(diff[i], diffInverted[i]);
        threshold(diffInverted[i], diffThresholded[i], thresh);
        diffThresholded[i].update();
    }
    
    shaderFbo.begin();
    ofClear(0, 255);
    shader.begin();
    shader.setUniformTexture("tex0", diffThresholded[0], 1);
    shader.setUniformTexture("tex1", diffThresholded[1], 2);
    shader.setUniformTexture("tex2", diffThresholded[2], 3);
    diffThresholded[0].draw(0, 0);
    shader.end();
    shaderFbo.end();
}

//--------------------------------------------------------------
void ofxSandboxTracker::draw(int x, int y) {
    shaderFbo.draw(x, y);
}

//--------------------------------------------------------------
void ofxSandboxTracker::drawDebug(int x, int y) {
    ofPushMatrix();
    ofTranslate(x, y);
    for (int i=0; i<numTrackingColors; i++) {
        diffThresholded[i].draw(width * i, 0);
    }
    for (int i=0; i<numTrackingColors; i++) {
        filters[i].draw(20 * i, 0, 20, 10);
    }
    ofPopMatrix();
}

//--------------------------------------------------------------
void ofxSandboxTracker::setFilterColor(int idxFilter, ofColor color) {
    colors[idxFilter].set(color);
    updateFilter(idxFilter);
}

//--------------------------------------------------------------
void ofxSandboxTracker::updateFilter(int idx) {
    filterFbo.begin();
    ofSetColor(colors[idx]);
    ofFill();
    ofDrawRectangle(0, 0, filterFbo.getWidth(), filterFbo.getHeight());
    filterFbo.end();
    filterFbo.readToPixels(filters[idx]);
    filters[idx].update();
}

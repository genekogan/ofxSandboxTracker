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
void ofxSandboxTracker::setup(int width, int height) {
    this->width = width;
    this->height = height;
    this->numTrackingColors = 4;
    
    string shaderProgram = STRINGIFY(
         uniform sampler2DRect tex0;
 
         uniform vec3 track_color0;
         uniform vec3 track_color1;
         uniform vec3 track_color2;
         uniform vec3 track_color3;
         uniform vec3 track_color4;
         uniform vec3 out_color0;
         uniform vec3 out_color1;
         uniform vec3 out_color2;
         uniform vec3 out_color3;
         uniform vec3 out_color4;
                                     
         void main (void){
             vec2 pos = gl_TexCoord[0].st;
             vec3 clr = texture2DRect(tex0, pos).rgb;
             
             vec3 diff0 = clr - track_color0;
             vec3 diff1 = clr - track_color1;
             vec3 diff2 = clr - track_color2;
             vec3 diff3 = clr - track_color3;
             vec3 diff4 = clr - track_color4;
             
             float dist0 = dot(diff0, diff0);
             float dist1 = dot(diff1, diff1);
             float dist2 = dot(diff2, diff2);
             float dist3 = dot(diff3, diff3);
             float dist4 = dot(diff4, diff4);
             
             float minDist = min(min(min(min(dist0, dist1), dist2), dist3), dist4);
             
             vec3 color = vec3(1,1,1);
             color = mix(color, out_color0, float(dist0<=minDist));
             color = mix(color, out_color1, float(dist1<=minDist));
             color = mix(color, out_color2, float(dist2<=minDist));
             color = mix(color, out_color3, float(dist3<=minDist));
             color = mix(color, out_color4, float(dist4<=minDist));

             gl_FragColor = vec4(color, 1.0);
         }
    );

    shader.setupShaderFromSource(GL_FRAGMENT_SHADER, shaderProgram);
    shader.linkProgram();
    shaderFbo.allocate(width, height);

    trackColors.resize(numTrackingColors);
    outColors.resize(numTrackingColors);
    
    for (int i=0; i<numTrackingColors; i++) {
        trackColors[i].set(0, 0, 0);
        outColors[i].set(0, 0, 0);
    }
    
    // this can be setup like in doodle tunes
    gui.setup();
    gui.setName("Sandbox");
    gui.add(amtMotion.set("motion", 0, 0, 30));
    gui.add(motionLerp.set("motion lerp", 0.1, 0, 1));
    gui.add(motionThreshLow.set("motion trip low", 10, 0, 20));
    gui.add(motionThreshHigh.set("motion trip high", 0, 0, 20));
    
    // setup homography
    distortedFbo.allocate(width, height);
    originalCorners[0].set(0, 0);
    originalCorners[1].set(width, 0);
    originalCorners[2].set(width, height);
    originalCorners[3].set(0, height);
    distortedCorners[0].set(0, 0);
    distortedCorners[1].set(width, 0);
    distortedCorners[2].set(width, height);
    distortedCorners[3].set(0, height);
    homography = ofxHomography::findHomography(originalCorners, distortedCorners);
}

//--------------------------------------------------------------
void ofxSandboxTracker::setCorner(int idx, int x, int y) {
    originalCorners[idx].set(x, y);
    homography = ofxHomography::findHomography(originalCorners, distortedCorners);
}

//--------------------------------------------------------------
void ofxSandboxTracker::setDebugPosition(int x, int y) {
    this->dx = x;
    this->dy = y;
    gui.setPosition(dx, dy);
}

//--------------------------------------------------------------
void ofxSandboxTracker::update(ofPixels & src) {
    this->srcImage.setFromPixels(src);
    
    absdiff(srcImage, previous, diff);
    diff.update();
    copy(srcImage, previous);
    motion = mean(toCv(diff));
    float sumMotion = motion[0]+motion[1]+motion[2];
    amtMotion.set(ofLerp(amtMotion, sumMotion, motionLerp));
    
    // distort original image
    distortedFbo.begin();
    ofPushMatrix();
    ofMultMatrix(homography);
    ofClear(0, 255);
    srcImage.draw(0, 0, width, height);
    ofPopMatrix();
    distortedFbo.end();

    // go through color shader
    shaderFbo.begin();
    ofClear(0, 255);
    shader.begin();
    shader.setUniformTexture("tex0", distortedFbo, 1);
    for (int idx=0; idx<numTrackingColors; idx++) {
        shader.setUniform3f("track_color"+ofToString(idx), (float) trackColors[idx].r / 255.0f, (float) trackColors[idx].g / 255.0f, (float) trackColors[idx].b / 255.0f);
        shader.setUniform3f("out_color"+ofToString(idx), (float) outColors[idx].r / 255.0f, (float) outColors[idx].g / 255.0f, (float) outColors[idx].b / 255.0f);
    }
    distortedFbo.draw(0, 0, width, height);
    shader.end();
    shaderFbo.end();
    
}

//--------------------------------------------------------------
void ofxSandboxTracker::draw(int x, int y) {
    shaderFbo.draw(x, y);
}

//--------------------------------------------------------------
void ofxSandboxTracker::drawDebug() {
    gui.draw();
    
    ofPushMatrix();
    ofTranslate(dx, dy);
    
    // original image
    ofTranslate(gui.getWidth()+5, 0);
    ofSetColor(255);
    srcImage.draw(0, 0);
    
    // draw homography corners
    ofSetColor(ofColor::lightBlue);
    ofDrawEllipse(originalCorners[0].x, originalCorners[0].y, 20, 20);
    ofDrawEllipse(originalCorners[1].x, originalCorners[1].y, 20, 20);
    ofDrawEllipse(originalCorners[2].x, originalCorners[2].y, 20, 20);
    ofDrawEllipse(originalCorners[3].x, originalCorners[3].y, 20, 20);

    // draw distorted image
    ofSetColor(255);
    ofTranslate(srcImage.getWidth()+5, 0);
    distortedFbo.draw(0, 0);
    
    // draw shader image
    ofTranslate(-srcImage.getWidth()-5, srcImage.getHeight()+5);
    shaderFbo.draw(0, 0);
    
    ofPopMatrix();
}

//--------------------------------------------------------------
void ofxSandboxTracker::setTrackColor(int idx, ofColor clr) {
    trackColors[idx].set(clr);
}

//--------------------------------------------------------------
void ofxSandboxTracker::setOutColor(int idx, ofColor clr) {
    cout << "COLOR TO " << idx << " " << ofToString(clr) << endl;
    outColors[idx].set(clr);
}

//--------------------------------------------------------------
void ofxSandboxTracker::keyEvent(int key) {

    int x = ofGetMouseX() - dx - (gui.getWidth()+5);
    int y = ofGetMouseY() - dy;
    
    unsigned char * pixels = srcImage.getPixels().getData();
    int index = 3 * (y * srcImage.getWidth() + x);
    int r = pixels[index];
    int g = pixels[index+1];
    int b = pixels[index+2];
    
    if (key=='1') {
        setTrackColor(0, ofColor(r, g, b));
    } else if (key=='2') {
        setTrackColor(1, ofColor(r, g, b));
    } else if (key=='3') {
        setTrackColor(2, ofColor(r, g, b));
    } else if (key=='4') {
        setTrackColor(3, ofColor(r, g, b));
    } else if (key=='5') {
        setTrackColor(4, ofColor(r, g, b));
    
    } else if (key=='q') {
        setOutColor(0, ofColor(r, g, b));
    } else if (key=='w') {
        setOutColor(1, ofColor(r, g, b));
    } else if (key=='e') {
        setOutColor(2, ofColor(r, g, b));
    } else if (key=='r') {
        setOutColor(3, ofColor(r, g, b));
    } else if (key=='t') {
        setOutColor(4, ofColor(r, g, b));
    
    } else if (key=='!') {
        setCorner(0, x, y);
    } else if (key=='@') {
        setCorner(1, x, y);
    } else if (key=='#') {
        setCorner(2, x, y);
    } else if (key=='$') {
        setCorner(3, x, y);
    }
}

//--------------------------------------------------------------

void ofxSandboxTracker::saveSettings(string filename) {
    ofxXmlSettings xml;
    xml.setValue("corners:p0:x", originalCorners[0].x);
    xml.setValue("corners:p0:y", originalCorners[0].y);
    xml.setValue("corners:p1:x", originalCorners[1].x);
    xml.setValue("corners:p1:y", originalCorners[1].y);
    xml.setValue("corners:p2:x", originalCorners[2].x);
    xml.setValue("corners:p2:y", originalCorners[2].y);
    xml.setValue("corners:p3:x", originalCorners[3].x);
    xml.setValue("corners:p3:y", originalCorners[3].y);
    
    // track colors
    for (int i=0; i<numTrackingColors; i++) {
        xml.setValue("trackColors:c"+ofToString(i), trackColors[i].getHex());
        xml.setValue("outColors:c"+ofToString(i), outColors[i].getHex());
    }

    // save
    xml.saveFile(filename);
}

void ofxSandboxTracker::loadSettings(string filename) {
    ofxXmlSettings xml;
    
    bool success = xml.loadFile(filename);
    if (!success) {
        ofLog(OF_LOG_WARNING) << "No settings file found";
        return;
    }
    
    // corners
    setCorner(0, xml.getValue("corners:p0:x", originalCorners[0].x), xml.getValue("corners:p0:y", originalCorners[0].y));
    setCorner(1, xml.getValue("corners:p1:x", originalCorners[1].x), xml.getValue("corners:p1:y", originalCorners[1].y));
    setCorner(2, xml.getValue("corners:p2:x", originalCorners[2].x), xml.getValue("corners:p2:y", originalCorners[2].y));
    setCorner(3, xml.getValue("corners:p3:x", originalCorners[3].x), xml.getValue("corners:p3:y", originalCorners[3].y));

    // track colors
    ofColor clr;
    for (int i=0; i<numTrackingColors; i++) {
        clr.setHex(xml.getValue("trackColors:c"+ofToString(i), trackColors[i].getHex()));
        setTrackColor(i, clr);
        clr.setHex(xml.getValue("outColors:c"+ofToString(i), outColors[i].getHex()));
        setOutColor(i, clr);
    }
}

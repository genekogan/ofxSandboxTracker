#include "ofxSandboxTracker.h"

#define STRINGIFY(A) #A
using namespace ofxCv;
using namespace cv;

//--------------------------------------------------------------
ofxSandboxTracker::ofxSandboxTracker() {
    thresh = 128.0;
    settingsChanged = false;
    sandbox_margin.set(50, 30);
}

//--------------------------------------------------------------
ofxSandboxTracker::~ofxSandboxTracker() {
    for (auto c : colorSelectorsIn){
        delete c;
    }
    for (auto c : colorSelectorsOut){
        delete c;
    }
    colorSelectorsIn.clear();
    colorSelectorsOut.clear();
}

//--------------------------------------------------------------
void ofxSandboxTracker::setup(int width, int height) {
    this->width = width;
    this->height = height;
    this->numTrackingColors = 5;
    newFrame = false;
    overwritePrev = false;
    
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
    sandboxPrev.allocate(width, height, OF_IMAGE_COLOR_ALPHA);
    sandboxCurrent.allocate(width, height, OF_IMAGE_COLOR_ALPHA);
    
    // colors
    trackColors.resize(numTrackingColors);
    outColors.resize(numTrackingColors);
    for (int i=0; i<numTrackingColors; i++) {
        trackColors[i].set(0, 0, 0);
        outColors[i].set(0, 0, 0);
    }
    
    // color selector elements
    font.load("verdana.ttf", 20);
    for (int i=0; i<numTrackingColors; i++) {
        ClickableColor *ci = new ClickableColor(i);
        ClickableColor *co = new ClickableColor(i);
        ci->setup("i"+ofToString(i), 32, 200 + 40*i, 44, 32);
        co->setup("o"+ofToString(i), 142, 200 + 40*i, 44, 32);
        colorSelectorsIn.push_back(ci);
        colorSelectorsOut.push_back(co);
        ofAddListener(ci->event, this, &ofxSandboxTracker::colorInEvent);
        //ofAddListener(co->event, this, &ofxSandboxTracker::colorOutEvent);
    }
    
    
    // options
    gui.setup();
    gui.setName("Sandbox");
    gui.add(amtMotion.set("motion", 0, 0, 100));
    gui.add(motionLerp.set("motion lerp", 0.1, 0, 1));
    gui.add(motionThreshHigh.set("motion trip", 20, 0, 100));
    //gui.add(motionThreshLow.set("motion trip low", 10, 0, 100));
    gui.add(gNewFrameIndicator.set("motion indicator", 0, 0, 1));
    gui.add(gBlurRadius.set("blur radius", 0, 0, 20));
    
    // setup homography
    distortedFbo.allocate(width, height);
    draggable.addPoint(0, 0);
    draggable.addPoint(width, 0);
    draggable.addPoint(width, height);
    draggable.addPoint(0, height);
    distortedCorners[0].set(0, 0);
    distortedCorners[1].set(width, 0);
    distortedCorners[2].set(width, height);
    distortedCorners[3].set(0, height);
    
    // setup draggable
    draggable.setBoundingBox(dx+gui.getWidth()+sandbox_margin.x, dy+sandbox_margin.y, width, height);
    draggable.setAuto(false);
    draggable.getPoint(0)->setMessage("TL");
    draggable.getPoint(1)->setMessage("TR");
    draggable.getPoint(2)->setMessage("BR");
    draggable.getPoint(3)->setMessage("BL");
    setEllipseSize(40);    
    updateHomography();
}

//--------------------------------------------------------------
void ofxSandboxTracker::updateHomography() {
    ofPoint originalCorners[4];
    for (int i=0; i<4; i++) {
        originalCorners[i] = draggable.get(i);
    }
    homography = ofxHomography::findHomography(originalCorners, distortedCorners);
    settingsChanged = true;
}

//--------------------------------------------------------------
void ofxSandboxTracker::setCorner(int idx, int x, int y) {
    draggable.set(idx, x, y);
    updateHomography();
}

//--------------------------------------------------------------
void ofxSandboxTracker::setDebugPosition(int x, int y) {
    this->dx = x;
    this->dy = y;
    gui.setPosition(dx, dy);
}

//--------------------------------------------------------------
bool ofxSandboxTracker::isMotionTripped() {
    if (newFrame) {
        newFrame = false;
        gNewFrameIndicator = 1;
        overwritePrev = true;
        return true;
    } else {
        return false;
    }
}

//--------------------------------------------------------------
bool ofxSandboxTracker::isSettingsChanged() {
    if (settingsChanged) {
        settingsChanged = false;
        return true;
    } else {
        return false;
    }
}


//--------------------------------------------------------------
void ofxSandboxTracker::update(ofPixels & src) {
    if (draggable.getIsChanged()) {
        updateHomography();
    }
    
    this->srcImage.setFromPixels(src);
    
    // add some gaussian blur
    if (gBlurRadius > 0) {
        GaussianBlur(srcImage, srcImage, gBlurRadius);
        srcImage.update();
    }
    
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
    
    // motion sensor
    shaderFbo.readToPixels(sandboxCurrent);
    sandboxCurrent.update();
    absdiff(sandboxCurrent, sandboxPrev, diff);
    diff.update();
    if (overwritePrev){
        copy(sandboxCurrent, sandboxPrev);
        sandboxPrev.update();
        overwritePrev = false;
    }
    motion = mean(toCv(diff));
    amtMotion.set(ofLerp(amtMotion, motion[0]+motion[1]+motion[2], motionLerp));
    motionTrip = (amtMotion > motionThreshHigh);
    if (motionTrip) {
        newFrame = true;
    }
    //motionReady = (amtMotion < motionThreshLow);
    
    gNewFrameIndicator = ofLerp(gNewFrameIndicator, 0, 0.1);
}

//--------------------------------------------------------------
void ofxSandboxTracker::draw(int x, int y) {
    sandboxCurrent.draw(x, y);
}

//--------------------------------------------------------------
void ofxSandboxTracker::drawDebug() {
    int margin = 35;

    gui.draw();
    
    ofPushMatrix();
    ofTranslate(dx, dy);
    
    // original image
    ofTranslate(gui.getWidth()+sandbox_margin.x, sandbox_margin.y);
    ofSetColor(255);
    
    if (colorSelected) {
        srcImage.draw(0, 0, 2*srcImage.getWidth(), 2*srcImage.getHeight());
    } else {
        srcImage.draw(0, 0);
    }
    font.drawString("webcam", 2, -4);
    
    if (!colorSelected) {
        // draw distorted image
        ofSetColor(255);
        ofTranslate(srcImage.getWidth()+margin, 0);
        font.drawString("distorted", 2, -4);
        distortedFbo.draw(0, 0);
        
        // draw shader image
        ofTranslate(-srcImage.getWidth()-margin, srcImage.getHeight()+margin);
        sandboxPrev.draw(0, 0);
        font.drawString("last input", 2, -4);
        sandboxCurrent.draw(shaderFbo.getWidth()+margin, 0);
        font.drawString("current", shaderFbo.getWidth()+margin+2, -4);
}
    
    ofPopMatrix();
    
    // draw homography corners
    if (!colorSelected) {
        draggable.draw();
    }
    
    if (colorSelected) {
        float tx = dx + gui.getWidth() + sandbox_margin.x;
        float ty = sandbox_margin.y;
        ofPoint mouse(ofGetMouseX(), ofGetMouseY());
        ofRectangle rect(tx, ty, 2 * srcImage.getWidth(), 2 * srcImage.getHeight());
        if (rect.inside(mouse)) {
            float selectedColorRadius = ofMap(sin(0.1*ofGetFrameNum()), -1, 1, 24, 64);
            selectedColor = srcImage.getColor(int(0.5*(mouse.x - tx)), int(0.5*(mouse.y - ty)));
            ofPushStyle();
            ofSetColor(ofColor::black);
            ofDrawEllipse(mouse, selectedColorRadius+1, selectedColorRadius+1);
            ofSetColor(selectedColor);
            ofDrawEllipse(mouse, selectedColorRadius, selectedColorRadius);
            ofPopStyle();
        }
    }

    // draw color selectors
    font.drawString("Color selector", 30, 192);
    for (auto c : colorSelectorsIn) {
        c->draw();
        font.drawString("->", c->getRectangle().x + c->getRectangle().width + 18, c->getRectangle().y + 25);
    }
    for (auto c : colorSelectorsOut) {
        c->draw();
    }
}

//--------------------------------------------------------------
void ofxSandboxTracker::setTrackColor(int idx, ofColor clr) {
    trackColors[idx].set(clr);
    colorSelectorsIn[idx]->setBackgroundColor(clr);
    settingsChanged = true;
}

//--------------------------------------------------------------
void ofxSandboxTracker::setOutColor(int idx, ofColor clr) {
    outColors[idx].set(clr);
    colorSelectorsOut[idx]->setBackgroundColor(clr);
    settingsChanged = true;
}

//--------------------------------------------------------------
void ofxSandboxTracker::setAllColorSelectorsInactive(){
    colorSelected = false;
    for (auto c : colorSelectorsIn) {
        c->setActive(false);
    }
    for (auto c : colorSelectorsOut) {
        c->setActive(false);
    }
}

//--------------------------------------------------------------
void ofxSandboxTracker::colorInEvent(int & idx) {
    if (colorSelectorsIn[idx]->getActive()) {
        colorSelectorsIn[idx]->setActive(false);
        return;
    }
    setAllColorSelectorsInactive();
    selectedColorIdx = idx;
    colorSelectorsIn[idx]->setActive(true);
    colorSelected = true;
}

//--------------------------------------------------------------
void ofxSandboxTracker::colorOutEvent(int & idx) {
    if (colorSelectorsOut[idx]->getActive()) {
        colorSelectorsOut[idx]->setActive(false);
        return;
    }
    setAllColorSelectorsInactive();
    selectedColorIdx = idx;
    colorSelectorsOut[idx]->setActive(true);
    colorSelected = true;
}

//--------------------------------------------------------------
void ofxSandboxTracker::keyEvent(int key) {

    int x = ofGetMouseX() - dx - (gui.getWidth() + sandbox_margin.x);
    int y = ofGetMouseY() - dy - sandbox_margin.y;
    
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
    } else if (key=='6') {
        //setTrackColor(5, ofColor(r, g, b));
      
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
    } else if (key=='y') {
       // setOutColor(5, ofColor(r, g, b));
    
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
void ofxSandboxTracker::saveSettings(string guiFilename, string sandboxFilename) {
    ofxXmlSettings xml;
    for (int i=0; i<4; i++) {
        xml.setValue("corners:p"+ofToString(i)+":x", draggable.get(i).x);
        xml.setValue("corners:p"+ofToString(i)+":y", draggable.get(i).y);
    }

    // track colors
    for (int i=0; i<numTrackingColors; i++) {
        xml.setValue("trackColors:c"+ofToString(i), trackColors[i].getHex());
        xml.setValue("outColors:c"+ofToString(i), outColors[i].getHex());
    }

    // save
    xml.saveFile(sandboxFilename);
    gui.saveToFile(guiFilename);
}

//--------------------------------------------------------------
void ofxSandboxTracker::loadSettings(string guiFilename, string sandboxFilename) {
    gui.loadFromFile(guiFilename);
    ofxXmlSettings xml;
    
    bool success = xml.loadFile(sandboxFilename);
    if (!success) {
        ofLog(OF_LOG_WARNING) << "No settings file found";
        return;
    }
    
    // corners
    for (int i=0; i<4; i++) {
        setCorner(i, xml.getValue("corners:p"+ofToString(i)+":x", draggable.get(i).x), xml.getValue("corners:p"+ofToString(i)+":y", draggable.get(i).y));
    }

    // track colors
    ofColor clr;
    for (int i=0; i<numTrackingColors; i++) {
        clr.setHex(xml.getValue("trackColors:c"+ofToString(i), trackColors[i].getHex()));
        setTrackColor(i, clr);
        clr.setHex(xml.getValue("outColors:c"+ofToString(i), outColors[i].getHex()));
        setOutColor(i, clr);
    }
}

//--------------------------------------------------------------
void ofxSandboxTracker::mouseMoved(int x, int y) {
    for (auto c : colorSelectorsIn) {
        c->mouseMoved(x, y);
    }
    for (auto c : colorSelectorsOut) {
        c->mouseMoved(x, y);
    }
    if (!colorSelected) {
        draggable.mouseMoved(x, y);
    }
}

//--------------------------------------------------------------
void ofxSandboxTracker::mousePressed(int x, int y) {
    for (auto c : colorSelectorsIn) {
        c->mousePressed(x, y);
    }
    for (auto c : colorSelectorsOut) {
        c->mousePressed(x, y);
    }
    if (!colorSelected) {
        draggable.mousePressed(x, y);
    }
}

//--------------------------------------------------------------
void ofxSandboxTracker::mouseDragged(int x, int y) {
    for (auto c : colorSelectorsIn) {
        c->mouseDragged(x, y);
    }
    for (auto c : colorSelectorsOut) {
        c->mouseDragged(x, y);
    }
    if (!colorSelected) {
        draggable.mouseDragged(x, y);
    }
}

//--------------------------------------------------------------
void ofxSandboxTracker::mouseReleased(int x, int y) {
    if (colorSelected) {
        setTrackColor(selectedColorIdx, selectedColor);
        setAllColorSelectorsInactive();
        colorSelected = false;
    }
    for (auto c : colorSelectorsIn) {
        c->mouseReleased(x, y);
    }
    for (auto c : colorSelectorsOut) {
        c->mouseReleased(x, y);
    }
    if (!colorSelected) {
        draggable.mouseReleased(x, y);
    }
}

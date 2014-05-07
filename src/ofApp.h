#pragma once

#include "ofMain.h"
#include "ofxPuppetInteractive.h"
#include "ofxDelaunay.h"
#include "ofxXmlSettings.h"
#include "ofxCv.h"
#include "ofxFaceTracker.h"
#include "ofxOpenCv.h"
#include <sstream>

#define XML_TAG_MESH_POINT  "meshPoint"
#define XML_TAG_FACE_POINT  "facePoint"
#define XML_TAG_FACE_INDEX  "faceIndex"
#define SELECTION_DISTANCE  5

struct Triangle{
	ofVec2f p0, p1, p2;
	void set(const ofVec2f & p0_, const ofVec2f & p1_, const ofVec2f& p2_){
		p0 = p0_;
		p1 = p1_;
		p2 = p2_;
	}
};

struct TriangleIndexPtrs{
	int * p1; int * p2; int * p3;
};

class ofApp : public ofBaseApp{

public:
    string meshFolderName = "couple4";
    
	void setup();
	void update();
	void draw();

	void mousePressed( int x, int y, int button );
	void mouseMoved( int x, int y );
	void mouseDragged( int x, int y, int button);
	void mouseReleased( int x, int y, int button );
	void keyPressed( int key );

	ofMesh makeGrid(ofRectangle square, int nHoriz, int nVert);
    void makePuppetFromSelectedTriangleMesh(ofxDelaunay & triangles, ofxPuppetInteractive & pup);
    void saveMesh(ofxDelaunay & points);
	void loadMesh(ofxDelaunay & points);
    string wrapText(string loadedText, int maxChars);

	ofxPuppetInteractive puppet;
	ofxDelaunay tri;

    ofImage bgImg;
	ofImage puppetImg;
	ofImage contourImg;
	Triangle tt; //temp triangle to draw mouseOver
	ofVec2f tempVertex;
	int mouseOverVertexIndex; //index of the vertex that we are mouseOvering on, otherwise -1

    float screenWidth;
    float screenHeight;
    
	bool draggingVertex;
	bool puppetMode;
    bool createMode = false;
	ofVec3f cameraOffset;

	bool drawIDs;
	bool drawMesh;
	bool drawCtrlpoints;
    bool isInit = true;
    
    ofVideoGrabber cam;
	ofxFaceTracker tracker;

    int numClicks;
    int clicksRecorded = 0;
    int indRecorded = 0;
    
    vector<int>     indexFace;
    vector<ofPoint> coordFace;    
    vector<ofPoint> diffFace;
    
    vector<ofPoint> puppetTorso;
    vector<ofPoint> puppetLeftArm;
    vector<ofPoint> puppetRightArm;
    vector<ofPoint> puppetPotato;
    
    int count = 0;
    float avgHue = 0;
    float avgBri = 0;
    float avgLight = 0;
    float avgTopColorR = 0;
    float avgTopColorB = 0;
    float avgTopColorG = 0;
    ofColor avgTopColor;
    
    float noseHue = 0;
    float noseBri = 0;
    float noseLight = 0;
    
    ofxCvColorImage	colorImg;
    ofxCvGrayscaleImage grayImage;
    ofxCvGrayscaleImage grayBg;
    ofxCvGrayscaleImage grayDiff;
    ofxCvContourFinder contourFinder;
    
    int clicksThreshold = 24;
    int threshold;
    
    ofTrueTypeFont titleFont;
    ofTrueTypeFont descriptionFont;
    
    ofxXmlSettings xml;
    string title;
    string date;
    string artist;
    string culture;
    string description;
    string fit;
    
    bool fitByHeight;
    
    int titleXPos;
    int textYPos;
    int artistXPos;
    int descriptionXPos;

    ofRectangle titleBox;
    ofRectangle artistBox;
};

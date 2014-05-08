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
    int currentCouple = 0;
    int totalCouples = 15;
    string meshFolderPrefix = "couple";
    string meshFolderName;
    
	void setup();
	void update();
	void draw();

    void animateCouple(int coupleID);
    void showLandingPage();
    void loadAndPuppeteer();
    
	void mousePressed( int x, int y, int button );
	void mouseMoved( int x, int y );
	void mouseDragged( int x, int y, int button);
	void mouseReleased( int x, int y, int button );
	void keyPressed( int key );

    void makePuppetFromSelectedTriangleMesh(ofxDelaunay & triangles, ofxPuppetInteractive & pup);
    void saveMesh(ofxDelaunay & points);
	void loadMesh(ofxDelaunay & points);
    string wrapText(string loadedText, int maxChars);

	ofxPuppetInteractive puppet;
	ofxDelaunay tri;
    
    vector<ofImage> allBgs;
    vector<ofImage> allContours;
    vector<ofImage> allPuppets;

    ofImage bgImg;
	ofImage puppetImg;
	ofImage contourImg;
    ofImage fgImg;

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
        
    ofxCvColorImage	colorImg;
    ofxCvGrayscaleImage grayImage;
    ofxCvGrayscaleImage grayBg;
    ofxCvGrayscaleImage grayDiff;
    ofxCvContourFinder contourFinder;
    
    int clicksThreshold = 24;
    
    ofTrueTypeFont titleFont;
    ofTrueTypeFont descriptionFont;
    
    ofxXmlSettings xml;
    string title = "";
    string date = "";
    string artist = "";
    string culture = "";
    string description = "";
    string fit = "";
    string foreground = "";
    string description2 = "";
    
    bool fitByHeight;
    
    int titleXPos;
    int titleYPos;
    int textYPos;
    int artistXPos;
    int descriptionXPos;
    int description2XPos;

    ofRectangle titleBox;
    ofRectangle artistBox;
    
    int currentVolumeUps = 0;
    int currentVolumeDowns = 0;
    int volumeThresh = 10;
};

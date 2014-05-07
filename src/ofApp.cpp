#include "ofApp.h"

void ofApp::makePuppetFromSelectedTriangleMesh(ofxDelaunay & triangles, ofxPuppetInteractive & pup){
    triangles.triangleMesh.setMode(OF_PRIMITIVE_LINES);
    triangles.triangleMesh.enableIndices();
    triangles.triangleMesh.enableTextures();
	vector<ofVec3f> verts = triangles.triangleMesh.getVertices();
	vector<ofVec2f> texCoords;
	for( int i = 0 ; i < verts.size() ; i++ ){
		texCoords.push_back(verts[i]);
	}
	triangles.triangleMesh.addTexCoords(texCoords);
	pup.setup(triangles.triangleMesh);
}

void ofApp::setup(){
    screenWidth = ofGetWindowSize().x;
    screenHeight = ofGetWindowSize().y;
    
    ofBackground(255, 227, 191);
	ofSetVerticalSync(true);
	puppet.setEvents(false);
    
    bgImg.loadImage(meshFolderName + "/bg.jpg");
    contourImg.loadImage(meshFolderName + "/couple.jpg");
    puppetImg.loadImage(meshFolderName + "/couple.png");    
    
    xml.loadFile(meshFolderName + "/data.xml");
    title = ofToUpper(xml.getAttribute("data", "title", ""));
    date = xml.getAttribute("data", "date", "");
    artist = xml.getAttribute("data", "artist", "");
    culture = xml.getAttribute("data", "culture", "");
    description = xml.getAttribute("data", "description", "");
    fit = xml.getAttribute("data", "fit", "");
    fit = "height";
    
    if (fit == "width") {
        float occupyingWidth = 0.75 * screenWidth;
        
        if (bgImg.getWidth() > occupyingWidth) {
            float resizeRatio = occupyingWidth / bgImg.getWidth();
            bgImg.resize(bgImg.getWidth() * resizeRatio, bgImg.getHeight() * resizeRatio);
            contourImg.resize(contourImg.getWidth() * resizeRatio, contourImg.getHeight() * resizeRatio);
            puppetImg.resize(puppetImg.getWidth() * resizeRatio, puppetImg.getHeight() * resizeRatio);
        }
    } else {
        if (bgImg.getHeight() > screenHeight) {
            float resizeRatio = screenHeight / bgImg.getHeight();
            bgImg.resize(bgImg.getWidth() * resizeRatio, bgImg.getHeight() * resizeRatio);
            contourImg.resize(contourImg.getWidth() * resizeRatio, contourImg.getHeight() * resizeRatio);
            puppetImg.resize(puppetImg.getWidth() * resizeRatio, puppetImg.getHeight() * resizeRatio);
        }
    }
    
    draggingVertex = false;
	puppetMode = false;
    
    cam.initGrabber(1280, 780);
	tracker.setup();
    
    colorImg.allocate(contourImg.getWidth(),contourImg.getHeight());
    grayImage.allocate(contourImg.getWidth(),contourImg.getHeight());
    grayBg.allocate(contourImg.getWidth(),contourImg.getHeight());
    grayDiff.allocate(contourImg.getWidth(),contourImg.getHeight());
    
    colorImg.setFromPixels(contourImg.getPixelsRef());
    grayImage = colorImg; // convert our color image to a grayscale image
    grayDiff.absDiff(grayBg, grayImage);
    grayDiff.threshold(100);
    contourFinder.findContours(grayDiff, 5, (contourImg.getWidth()*contourImg.getHeight())/4, 4, false, false);
    
    description = wrapText(description, 50);
    title = wrapText(title, 15);
    
    titleFont.loadFont("font/Znikomit.otf", 46, true, true, true);
    descriptionFont.loadFont("font/Didot-Light16.otf", 18, true, true, true);
    descriptionFont.setLineHeight(35);
    descriptionFont.setSpaceSize(0.7);
    
    titleXPos = 325;
    textYPos = 1950;
    titleBox = titleFont.getStringBoundingBox(title, textYPos, titleXPos);
    artistXPos = titleXPos + titleBox.height + 35;
    artistBox = titleFont.getStringBoundingBox(artist + " (" + date + "), " + culture, textYPos, artistXPos);
    descriptionXPos = artistXPos + artistBox.height + 30;
}

void ofApp::update(){
	puppet.setScreenOffset(cameraOffset);
	puppet.update();
    cam.update();
    tracker.update(ofxCv::toCv(cam));
    
	if(cam.isFrameNew() && puppet.getNumControlPoints() > 0) {
		tracker.update(ofxCv::toCv(cam));
        if (tracker.getFound()){
            if (isInit) {
                for (int i = 0; i < coordFace.size(); i++) {
                    ofPoint p;
                    if (i < 5){
                        p = tracker.getImageFeature(tracker.LEFT_EYEBROW).getVertices()[i]      - coordFace[i];
                    } else if (i < 10) {
                        p = tracker.getImageFeature(tracker.RIGHT_EYEBROW).getVertices()[i - 5] - coordFace[i];
                    } else if (i < 12) {
                        p = tracker.getImageFeature(tracker.NOSE_BRIDGE).getVertices()[i - 8]   - coordFace[i];
                    } else if (i < 24) {
                        p = tracker.getImageFeature(tracker.OUTER_MOUTH).getVertices()[i - 12]  - coordFace[i];
                    }
                    diffFace.push_back(p);
                }                
                isInit = false;
            }            
            for (int i = 0; i < indexFace.size(); i++) {
                if (i < 5){
                    puppet.setControlPoint(indexFace[i], tracker.getImageFeature(tracker.LEFT_EYEBROW).getVertices()[i] - diffFace[i]);
                } else if (i < 10) {
                    puppet.setControlPoint(indexFace[i], tracker.getImageFeature(tracker.RIGHT_EYEBROW).getVertices()[i - 5] - diffFace[i]);
                } else if (i < 12) {
                    puppet.setControlPoint(indexFace[i], tracker.getImageFeature(tracker.NOSE_BRIDGE).getVertices()[i - 8] - diffFace[i]);
                } else if (i < 24) {
                    puppet.setControlPoint(indexFace[i], tracker.getImageFeature(tracker.OUTER_MOUTH).getVertices()[i - 12] - diffFace[i]);
                }
            }
        }
	}
}


void ofApp::draw(){
	ofSetColor(255);
    bgImg.draw(0, 0);
    //contourImg.draw(0, 0);
    
    cam.draw(screenWidth - 300, screenHeight - 190, cam.getWidth() * 0.2, cam.getHeight() * 0.2);
    if(!puppetMode){
        puppetImg.draw(0,0, puppetImg.getWidth(), puppetImg.getHeight());
    }

	//anim guidelines
	if(puppetMode){
		ofNoFill();
		ofSetColor(0);
		ofPushMatrix();
		ofFill();
	}    

	ofSetColor(255);
	puppetImg.getTextureReference().bind();
	puppet.drawFaces();
	puppetImg.getTextureReference().unbind();
    
	ofSetColor(128, 24);
	if(drawMesh)puppet.drawWireframe();
	if(drawIDs)puppet.drawVertexIDs();
	if(drawCtrlpoints)puppet.drawControlPoints();

    if (!puppetMode) {
        ofSetColor(0,255,128);
        tri.triangleMesh.drawWireframe();
        ofSetColor(255,0,0, 128);
        glPointSize(3);
        tri.triangleMesh.draw(OF_MESH_POINTS);
        glPointSize(1);
    }

	//draw mouseover triangle
	ofSetColor(0,255,255, 64 * fabs(cos(10 * ofGetElapsedTimef())));
	ofBeginShape();
		ofVertex(tt.p0.x, tt.p0.y);
		ofVertex(tt.p1.x, tt.p1.y);
		ofVertex(tt.p2.x, tt.p2.y);
	ofEndShape();

	//draw mouseover vertex
	if(tempVertex.length() > 0.0f){
		ofSetColor(255,255,0, 128 * fabs(cos(10 * ofGetElapsedTimef())));
		ofCircle(tempVertex, 5);
	}

	ofSetupScreen();
    if (!puppetMode) {
        tri.draw();
    }
                
    ofSetHexColor(0);
    titleFont.drawString(title, textYPos, titleXPos);
    descriptionFont.drawString(artist + " (" + date + "), " + culture, textYPos, artistXPos);
    descriptionFont.drawString(description, textYPos, descriptionXPos);
}


void ofApp::mousePressed( int x, int y, int button ){
    if(puppetMode) return;
    
    if(createMode) {
        if (clicksRecorded < clicksThreshold) {
            coordFace.push_back(ofPoint(x, y, 0));
            tri.addPoint(x, y, 0);
        }
        if (clicksRecorded == clicksThreshold - 1) {
            for (int k = 0; k < contourFinder.blobs.size(); k++) {
                int j = 0;
                for(int i = 0; i < contourFinder.blobs[k].pts.size() - 3; i = i + 10) {
                    float ax = contourFinder.blobs[k].pts[i].x;
                    float ay = contourFinder.blobs[k].pts[i].y;
                    
                    float bx = contourFinder.blobs[k].pts[i + 1].x;
                    float by = contourFinder.blobs[k].pts[i + 1].y;
                    
                    float cx = contourFinder.blobs[k].pts[i + 2].x;
                    float cy = contourFinder.blobs[k].pts[i + 2].y;
                    
                    float top = ((ax - bx) * (cx - bx) + (ay - by) * (cy - by));
                    float bot = sqrt( pow((ax - bx), 2) + pow((ay - by),2)) * sqrt( pow((bx - cx),2) + pow((by - cy),2));
                    
                    float angle = top / bot;
                    if (angle > - 0.8) {
                        tri.addPoint(bx, by, 0);
                        j--;
                    }
                    if (j == 2) {
                        tri.addPoint(bx, by, 0);
                        j = 0;
                    }
                    j++;
                }            
            }
            tri.addMidTris(puppetImg);
            for(int i = 0; i < coordFace.size(); i++) {
                tri.getPointNear(coordFace[i], SELECTION_DISTANCE, mouseOverVertexIndex);
                indexFace.push_back(mouseOverVertexIndex);
            }
            createMode = false;
        }
        clicksRecorded++;
    }
    
	x -= cameraOffset.x;
	y -= cameraOffset.y;

	//add points to mesh
	if(button == 0) {
		if (tempVertex.length() > 0.0f){ //mouse over a vertex
			draggingVertex = true;
		}
	}
}


void ofApp::mouseReleased(int x, int y, int button ){
	draggingVertex = false;
}


void ofApp::mouseDragged(int x, int y, int button){
    if(puppetMode) return;
    
	if(button == 1){
		cameraOffset.x += x - ofGetPreviousMouseX();
		cameraOffset.y += y - ofGetPreviousMouseY();
	}

	x -= cameraOffset.x;
	y -= cameraOffset.y;

	if(draggingVertex){
		tempVertex = ofVec2f(x,y);
		tri.setPointAtIndex(tempVertex, mouseOverVertexIndex, puppetImg);
	}
}


void ofApp::mouseMoved( int x, int y ){
	if(puppetMode) return;

	x -= cameraOffset.x;
	y -= cameraOffset.y;

	if (tri.getNumPoints() > 2) {
		tempVertex = tri.getPointNear(ofVec2f(x,y), SELECTION_DISTANCE, mouseOverVertexIndex);
		if (tempVertex.length() == 0.0f){ // only select triangles if not selectng vertexs
			ITRIANGLE t = tri.getTriangleForPos(ofPoint(x,y));
			vector<ofPoint> pts = tri.getPointsForITriangle(t);
			tt.set(pts[0], pts[1], pts[2]);
		} else {
			tt.set(ofVec2f(), ofVec2f(), ofVec2f());
		}
	}
}


void ofApp::keyPressed( int key ){
	switch(key){
		case 's': saveMesh(tri); break;
		case 'l': {
            loadMesh(tri);
            tri.triangulate(puppetImg);
        } break;
        case 'p': {
            makePuppetFromSelectedTriangleMesh(tri, puppet);
			puppetMode = true;
			puppet.setEvents(true);
            for (int i = 0; i < indexFace.size(); i++){
                puppet.setControlPoint(indexFace[i]);
            }
        } break;
        case 'c' : {
            createMode = true;
        } break;        
    }
}


void ofApp::saveMesh(ofxDelaunay & t){
    string xmlName = meshFolderName + "/coupleMesh.xml";
	ofxXmlSettings xml;
	xml.loadFile(xmlName);
	xml.clear();
	for (int i = 0; i < t.triangleMesh.getNumVertices(); i++){
		ofVec3f * pts = t.triangleMesh.getVerticesPointer();
		xml.addTag(XML_TAG_MESH_POINT);
		xml.setAttribute(XML_TAG_MESH_POINT, "x", pts[i].x, i);
		xml.setAttribute(XML_TAG_MESH_POINT, "y", pts[i].y, i);
	}
    for (int i = 0; i < coordFace.size(); i++){
        xml.addTag(XML_TAG_FACE_POINT);
        xml.setAttribute(XML_TAG_FACE_POINT, "x", coordFace[i].x, i);
        xml.setAttribute(XML_TAG_FACE_POINT, "y", coordFace[i].y, i);
    }
    for (int i = 0; i < indexFace.size(); i++){
        xml.addTag(XML_TAG_FACE_INDEX);
        xml.setAttribute(XML_TAG_FACE_INDEX, "i", indexFace[i], i);
    }
	xml.saveFile(xmlName);
}


void ofApp::loadMesh(ofxDelaunay & t){
    string xmlName = meshFolderName + "/coupleMesh.xml";
	tri.reset();
	ofxXmlSettings xml;
	xml.loadFile(xmlName);
	for (int i = 0; i < xml.getNumTags(XML_TAG_MESH_POINT); i++){
		float x = xml.getAttribute(XML_TAG_MESH_POINT, "x", 0.0, i);
		float y = xml.getAttribute(XML_TAG_MESH_POINT, "y", 0.0, i);
		tri.addPoint(x, y, 0);
	}
    for (int i = 0; i < xml.getNumTags(XML_TAG_FACE_POINT); i++){
		float x = xml.getAttribute(XML_TAG_FACE_POINT, "x", 0.0, i);
		float y = xml.getAttribute(XML_TAG_FACE_POINT, "y", 0.0, i);
        coordFace.push_back(ofPoint(x, y, 0));
	}
    for (int i = 0; i < xml.getNumTags(XML_TAG_FACE_INDEX); i++){
        int index = xml.getAttribute(XML_TAG_FACE_INDEX, "i", 0.0, i);
        indexFace.push_back(index);
    }
}

string ofApp::wrapText(string text, int maxChars) {
    int currentChars = 0;
    int potentialChars = 0;
    stringstream textStream(text);
    string word;
    string wrappedText = "";
    
    for (int i = 0; textStream >> word; i++)
    {
        potentialChars = currentChars + word.size() + 1;
                
        if (potentialChars >= maxChars) {
            wrappedText = wrappedText + "\n" + word + " ";
            currentChars = word.size() + 1;
        } else {
            wrappedText = wrappedText + word + " ";
            currentChars = potentialChars;
        }
    }
    
    return wrappedText;
}
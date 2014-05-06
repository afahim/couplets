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
    ofBackground(255, 227, 191);
	ofSetVerticalSync(true);
	puppet.setEvents(false);
    
    bgImg.loadImage("couple1/bg.jpg");
    contourImg.loadImage("couple1/couple.jpg");
    contourImg.resize(contourImg.getWidth(), contourImg.getHeight());
    puppetImg.loadImage("couple1/couple.png");
    puppetImg.resize(puppetImg.getWidth(), puppetImg.getHeight());
    
    //imgTracker.update(ofxCv::toCv(legImg));
    
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
    
    xml.loadFile("couple1/data.xml");
    title = xml.getAttribute("data", "title", "");
    date = xml.getAttribute("data", "date", "");
    artist = xml.getAttribute("data", "artist", "");
    culture = xml.getAttribute("data", "culture", "");
    description = xml.getAttribute("data", "description", "");
    
    description = wrapText(description, 50);
    title = wrapText(title, 15);
    
    titleFont.loadFont("font/Znikomit.otf", 46, true, true, true);
    descriptionFont.loadFont("font/Didot-Light16.otf", 18, true, true, true);
    descriptionFont.setLineHeight(35);
    descriptionFont.setSpaceSize(0.7);
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
    cam.draw(15, 15, cam.getWidth() * 0.2, cam.getHeight() * 0.2);
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
    
    if (tracker.getFound()) {
        ofSetColor(255);
        //tracker.draw();
        
        float x1 = (tracker.getImageFeature(tracker.NOSE_BASE).getVertices()[0].x + tracker.getImageFeature(tracker.OUTER_MOUTH).getVertices()[2].x) / 2;
        float y1 = (tracker.getImageFeature(tracker.NOSE_BASE).getVertices()[0].y + tracker.getImageFeature(tracker.OUTER_MOUTH).getVertices()[2].y) / 2;
        float x2 = (tracker.getImageFeature(tracker.NOSE_BASE).getVertices()[4].x + tracker.getImageFeature(tracker.OUTER_MOUTH).getVertices()[4].x) / 2;
        float y2 = (tracker.getImageFeature(tracker.NOSE_BASE).getVertices()[4].y + tracker.getImageFeature(tracker.OUTER_MOUTH).getVertices()[4].y) / 2;
        float x3 = ((tracker.getImageFeature(tracker.OUTER_MOUTH).getVertices()[9].x + tracker.getImageFeature(tracker.JAW).getVertices()[8].x) / 2 + tracker.getImageFeature(tracker.OUTER_MOUTH).getVertices()[9].x) / 2;
        float y3 = ((tracker.getImageFeature(tracker.OUTER_MOUTH).getVertices()[9].y + tracker.getImageFeature(tracker.JAW).getVertices()[8].y) / 2 + tracker.getImageFeature(tracker.OUTER_MOUTH).getVertices()[9].y) / 2;
        
        ofPixels px = cam.getPixelsRef();
        ofColor c1 = px.getColor(x1, y1);
        ofColor c2 = px.getColor(x2, y2);
        ofColor c3 = px.getColor(x3, y3);
        
        int n1 = tracker.getImageFeature(tracker.NOSE_BRIDGE).getVertices()[2].x;
        int n2 = tracker.getImageFeature(tracker.NOSE_BRIDGE).getVertices()[2].y;
        int n3 = tracker.getImageFeature(tracker.NOSE_BRIDGE).getVertices()[3].x;
        int n4 = tracker.getImageFeature(tracker.NOSE_BRIDGE).getVertices()[3].y;
        ofColor c4 = px.getColor(n1, n2);
        ofColor c5 = px.getColor(n3, n4);
        
        count ++;
        
        avgBri = (avgBri * (count - 1) + (c1.getBrightness() + c2.getBrightness() + c3.getBrightness()) / 3) / count;
        avgLight = (avgLight * (count - 1) + (c1.getLightness() + c2.getLightness() + c3.getLightness()) / 3) / count;
        
        noseBri = (noseBri * (count - 1) + (c4.getBrightness() + c5.getBrightness()) / 2) / count;
        noseLight = (noseLight * (count - 1) + (c4.getLightness() + c5.getLightness()) / 2) / count;
        
        float x4 = tracker.getImageFeature(tracker.OUTER_MOUTH).getVertices()[3].x;
        float y4 = tracker.getImageFeature(tracker.OUTER_MOUTH).getVertices()[3].y;
        float x5 = tracker.getImageFeature(tracker.JAW).getVertices()[8].x;
        float y5 = tracker.getImageFeature(tracker.JAW).getVertices()[8].y;
        float topX = (x4 + x5) / 2;
        float topY = y4 + y5;
        if (cam.getHeight() < topY) {
            topY = cam.getHeight() - 2;
        }
        ofColor topColor = px.getColor(topX, topY);
        avgTopColorR += topColor.r;
        avgTopColorB += topColor.b;
        avgTopColorG += topColor.g;
        
        if (count == 50) {
            int bright = int(noseBri - avgBri);
            int light = int(noseLight - avgLight);
            if (bright < 50 && light < 50) {
                cout << "YOU DON'T HAVE A BEARD!";
                cout << "\n";
            } else {
                cout << "YOU HAVE A BEARD!";
                cout << "\n";
            }
            avgTopColor = ofColor(avgTopColorR/50, avgTopColorB/50, avgTopColorG/50);
        }
        
        if (count > 50) {
            ofSetColor(avgTopColor.r, avgTopColor.b, avgTopColor.g, 160);
            if (puppetTorso.size() == 4 && puppetMode) {
                ofSetPolyMode(OF_POLY_WINDING_NONZERO);
                ofBeginShape();
                for (int i =  0; i <  puppetTorso.size(); i++) {
                    ofPoint p = puppet.getDeformedMesh().getVertices()[i+24];
                    ofVertex(p.x, p.y);
                }
                ofEndShape();
            }
        }
    }
    
    //ofSetHexColor(0xffffff);
    //colorImg.draw(0, 0, 320, 240);
    //grayDiff.draw(0, 240, 320, 240);
    //ofRect(320, 0, 320, 240);
    //contourFinder.draw(0, 0);
    
    
    /*ofSetHexColor(0x111111);
    ofRectangle rectT = myFont.getStringBoundingBox(title, 100, 1100);
    ofRect(rectT.x - 2, rectT.y - 2, rectT.width + 4, rectT.height + 3);
    ofRectangle rectA = myFont.getStringBoundingBox(artist + " (" + date + "), " + culture, 100, 1130);
    ofRect(rectA.x - 2, rectA.y - 2, rectA.width + 4, rectA.height + 3);
    ofRectangle rectD = myFont.getStringBoundingBox(description, 100, 1160);
    ofRect(rectD.x - 2, rectD.y - 2, rectD.width + 4, rectD.height + 3);*/
    
    ofSetHexColor(0);
    titleFont.drawString(ofToUpper(title), 1900, 325);
    descriptionFont.drawString(artist + " (" + date + "), " + culture, 1900, 475);
    descriptionFont.drawString(description, 1900, 550);

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
        cout << clicksRecorded;
        cout << "\n";
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
        // restart beard detection
        case 'r' : {
            count = 0;
            avgHue = 0;
            avgBri = 0;
            avgLight = 0;
            noseHue = 0;
            noseBri = 0;
            noseLight = 0;
            avgTopColorR = 0;
            avgTopColorB = 0;
            avgTopColorG = 0;
        }
	}
}


void ofApp::saveMesh(ofxDelaunay & t){
	ofxXmlSettings xml;
	xml.loadFile(MESH_XML_FILENAME);
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
	xml.saveFile(MESH_XML_FILENAME);
}


void ofApp::loadMesh(ofxDelaunay & t){
	tri.reset();
	ofxXmlSettings xml;
	xml.loadFile(MESH_XML_FILENAME);
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
    stringstream textStream(text);
    string word;
    string wrappedText = "";
    
    for (int i = 0; textStream >> word; i++)
    {
        wrappedText = wrappedText + word + " ";
        currentChars = currentChars + word.size() + 1;
        if (currentChars >= maxChars) {
            wrappedText = wrappedText + "\n";
            currentChars = 0;
        }
    }
    
    return wrappedText;
}



#include "ofApp.h"

ofMesh ofApp::makeGrid(ofRectangle square, int nHoriz, int nVert) {

	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_LINES);
	int c = 0;
	for (int j = 0; j < nVert; j++){
    for (int i = 0; i < nHoriz; i++){
		float x = ofMap(i, 0, nHoriz-1, square.x, square.x + square.width);
		float y = ofMap(j, 0, nVert-1, square.y, square.y + square.height);
		mesh.addVertex(ofPoint(x,y));
		printf("(%d)  %.0f %.0f\n", c, x, y);
		c++;
	}
	}

	printf("################\n" );

	for ( unsigned int y = 0; y < (nVert-1); y++ ) {
		for ( unsigned int x = 0; x < nHoriz-1; x++ ) {
			unsigned int nRow1 = y * nHoriz;
			unsigned int nRow2 = (y+1) * nHoriz;
			mesh.addIndex(nRow1 + x); printf("%d\n", nRow1 + x);
			mesh.addIndex(nRow2 + x + 1); printf("%d\n",nRow2 + x + 1);
			mesh.addIndex(nRow1 + x + 1); printf("%d\n",nRow1 + x + 1);
			mesh.addIndex(nRow1 + x); printf("%d\n",nRow1 + x);
			mesh.addIndex(nRow2 + x); printf("%d\n",nRow2 + x);
			mesh.addIndex(nRow2 + x + 1); printf("%d\n",nRow2 + x + 1);
			printf("------\n");
		}
	}

	return mesh;
}


void ofApp::makePuppetFromSelectedTriangleMesh(ofxDelaunay & triangles, vector<ITRIANGLE>selected, ofxPuppetInteractive & pup){

	triangles.triangulate(); //just in case
	ofMesh m; m.setMode(OF_PRIMITIVE_LINES);

	//init array of used vertices (all false)
	vector<bool>usedIndexes;
	for(int i = 0; i < triangles.triangleMesh.getVertices().size(); i++){
		usedIndexes.push_back(false);
	}

	int numIndices = usedIndexes.size(); //num of vertex == num of indices

	//make one referance table to all possible indices
	int * indexTable = (int*) malloc(sizeof(int) * usedIndexes.size());

	//fill in the table [0, 1, 2 .. numIndices-1]
	for(int i = 0; i < triangles.triangleMesh.getVertices().size(); i++){
		indexTable[i] = i;
	}

	//fill in the usedIndexes vector accordingly;
	//any vertex whose index matches a false in the "usedIndexes" vec can be dropped bc its unused
	for( int i = 0 ; i < selectedTriangles.size() ; i++ ){
		ITRIANGLE t = selectedTriangles[i];
		usedIndexes[t.p1] = true;
		usedIndexes[t.p2] = true;
		usedIndexes[t.p3] = true;
	}

	//clone our selectedTriangles vector into referencedIndexesTriangles, but instead of each
	//triangle having 3 indexes, each triangle will have 3 references to an index in the indexTable
	//this way we can easily recalc all triangle indexes as some vertices are dropped bc they are unused
	//bc no selected triangle uses them
	vector<TriangleIndexPtrs> referencedIndexesTriangles;
	for( int i = 0 ; i < selectedTriangles.size() ; i++ ){
		ITRIANGLE t = selectedTriangles[i];
		TriangleIndexPtrs tp;
		tp.p1 = &indexTable[t.p1];
		tp.p2 = &indexTable[t.p2];
		tp.p3 = &indexTable[t.p3];
		referencedIndexesTriangles.push_back(tp);
	}

	//all vertices
	vector<ofVec3f> verts = triangles.triangleMesh.getVertices();

	vector<int> vertexIndexesToDrop;
	//walk the usedIndexes vector, when see a false, drop that vertex from the vector
	// AND decrease by one all indices higher than the current in the indexTable
	//this way we automagically have a selected triangles vector with correct indices
	for( int i = 0 ; i < usedIndexes.size() ; i++ ){
		if(usedIndexes[i] == false){ //this vertex not used
			vertexIndexesToDrop.push_back(i);
			for(int j = i; j < numIndices; j++){ //decrease all indexes ahead of me by one
				indexTable[j]--;
			}
		}
	}

	//erase items from the final vertices vector the back
	for( int i = vertexIndexesToDrop.size() - 1 ; i >= 0; i-- ){
		verts.erase( verts.begin() + vertexIndexesToDrop[i]);
	}

	vector<ofVec2f> texCoords;
	for( int i = 0 ; i < verts.size() ; i++ ){
		texCoords.push_back(verts[i]);
	}
	m.addVertices(verts);
	m.addTexCoords(texCoords);

	for( int i = 0 ; i < selectedTriangles.size() ; i++ ){
		TriangleIndexPtrs t = referencedIndexesTriangles[i];
		m.addIndex(*t.p1); m.addIndex(*t.p2); m.addIndex(*t.p3);
	}

	pup.setup(m);
	tri.reset();
	free(indexTable);
}


void ofApp::setup(){
	ofSetVerticalSync(true);
	puppet.setEvents(false);

    //imgTracker.setup();
    legImg.loadImage("picture.jpg");
    legImg.resize(320, 240);
    
    //imgTracker.update(ofxCv::toCv(legImg));
    
	draggingVertex = false;
	puppetMode = false;

    // OFX_REMOTEUI_SERVER_SETUP(10000);
    // OFX_REMOTEUI_SERVER_SET_NEW_COLOR();
    // OFX_REMOTEUI_SERVER_SHARE_PARAM(drawIDs);
    // OFX_REMOTEUI_SERVER_SHARE_PARAM(drawMesh);
    // OFX_REMOTEUI_SERVER_SHARE_PARAM(drawCtrlpoints);
    // OFX_REMOTEUI_SERVER_LOAD_FROM_XML();
    
    cam.initGrabber(320, 240);
	tracker.setup();
    
    colorImg.allocate(320,240);
    grayImage.allocate(320,240);
    grayBg.allocate(320,240);
    grayDiff.allocate(320,240);
}

void ofApp::update(){

    OFX_REMOTEUI_SERVER_UPDATE(1./60);

	puppet.setScreenOffset(cameraOffset);
	puppet.update();

	vector<int> ids = puppet.getControlPointIDs();    
    cam.update();
    tracker.update(ofxCv::toCv(cam));
    
    colorImg.setFromPixels(legImg.getPixelsRef());
    grayImage = colorImg; // convert our color image to a grayscale image
    grayDiff.absDiff(grayBg, grayImage);
    grayDiff.threshold(100);
    contourFinder.findContours(grayDiff, 5, (340*240)/4, 4, false, true);
    
	if(cam.isFrameNew() && ids.size() > 0) {
		tracker.update(ofxCv::toCv(cam));
        if (tracker.getFound()){
            
            if (isInit) {
                for (int i = 0; i < 5; i++) {
                    ofPoint p = tracker.getImageFeature(tracker.LEFT_EYEBROW).getVertices()[i] - initLeftEye[i];
                    diffLeftEye.push_back(p);
                }
                
                for (int i = 0; i < 5; i++) {
                    ofPoint p = tracker.getImageFeature(tracker.RIGHT_EYEBROW).getVertices()[i] - initRightEye[i];
                    diffRightEye.push_back(p);
                }
                
                for (int i = 2; i < 4; i++) {
                    ofPoint p = tracker.getImageFeature(tracker.NOSE_BRIDGE).getVertices()[i] - initNose[i - 2];
                    diffNose.push_back(p);
                }
                isInit = false;
            }
            for (int i = 0; i < 12; i++) {
                ofPoint p = tracker.getImageFeature(tracker.OUTER_MOUTH).getVertices()[i] - initMouth[i];
                diffMouth.push_back(p);
            }
            puppet.setControlPoint(0,   tracker.getImageFeature(tracker.LEFT_EYEBROW).getVertices()[0] - diffLeftEye[0]);
            puppet.setControlPoint(1,   tracker.getImageFeature(tracker.LEFT_EYEBROW).getVertices()[1] - diffLeftEye[1]);
            puppet.setControlPoint(2,   tracker.getImageFeature(tracker.LEFT_EYEBROW).getVertices()[2] - diffLeftEye[2]);
            puppet.setControlPoint(3,   tracker.getImageFeature(tracker.LEFT_EYEBROW).getVertices()[3] - diffLeftEye[3]);
            puppet.setControlPoint(4,   tracker.getImageFeature(tracker.LEFT_EYEBROW).getVertices()[4] - diffLeftEye[4]);
            
            puppet.setControlPoint(5,  tracker.getImageFeature(tracker.RIGHT_EYEBROW).getVertices()[0] - diffRightEye[0]);
            puppet.setControlPoint(6,  tracker.getImageFeature(tracker.RIGHT_EYEBROW).getVertices()[1] - diffRightEye[1]);
            puppet.setControlPoint(7,  tracker.getImageFeature(tracker.RIGHT_EYEBROW).getVertices()[2] - diffRightEye[2]);
            puppet.setControlPoint(8,  tracker.getImageFeature(tracker.RIGHT_EYEBROW).getVertices()[3] - diffRightEye[3]);
            puppet.setControlPoint(9,  tracker.getImageFeature(tracker.RIGHT_EYEBROW).getVertices()[4] - diffRightEye[4]);
            
            puppet.setControlPoint(10,  tracker.getImageFeature(tracker.NOSE_BRIDGE).getVertices()[2] - diffNose[0]);
            puppet.setControlPoint(11,  tracker.getImageFeature(tracker.NOSE_BRIDGE).getVertices()[3] - diffNose[1]);
            
            puppet.setControlPoint(12,   tracker.getImageFeature(tracker.OUTER_MOUTH).getVertices()[0] - diffMouth[0]);
            puppet.setControlPoint(13,   tracker.getImageFeature(tracker.OUTER_MOUTH).getVertices()[1] - diffMouth[1]);
            puppet.setControlPoint(14,  tracker.getImageFeature(tracker.OUTER_MOUTH).getVertices()[2] - diffMouth[2]);
            puppet.setControlPoint(15,  tracker.getImageFeature(tracker.OUTER_MOUTH).getVertices()[3] - diffMouth[3]);
            puppet.setControlPoint(16,  tracker.getImageFeature(tracker.OUTER_MOUTH).getVertices()[4] - diffMouth[4]);
            puppet.setControlPoint(17,  tracker.getImageFeature(tracker.OUTER_MOUTH).getVertices()[5] - diffMouth[5]);
            puppet.setControlPoint(18,  tracker.getImageFeature(tracker.OUTER_MOUTH).getVertices()[6] - diffMouth[6]);
            puppet.setControlPoint(19,  tracker.getImageFeature(tracker.OUTER_MOUTH).getVertices()[7] - diffMouth[7]);
            puppet.setControlPoint(20,  tracker.getImageFeature(tracker.OUTER_MOUTH).getVertices()[8] - diffMouth[8]);
            puppet.setControlPoint(21,  tracker.getImageFeature(tracker.OUTER_MOUTH).getVertices()[9] - diffMouth[9]);
            puppet.setControlPoint(22,  tracker.getImageFeature(tracker.OUTER_MOUTH).getVertices()[10] - diffMouth[10]);
            puppet.setControlPoint(23,   tracker.getImageFeature(tracker.OUTER_MOUTH).getVertices()[11] - diffMouth[11]);
        
        }

	}

}

void ofApp::exit(){
	OFX_REMOTEUI_SERVER_CLOSE();
	OFX_REMOTEUI_SERVER_SAVE_TO_XML();
}


void ofApp::draw(){
    
    ofSetHexColor(0xffffff);
    colorImg.draw(0, 0, 320, 240);
    grayDiff.draw(0, 240, 320, 240);
    ofRect(320, 0, 320, 240);
    contourFinder.draw(320, 0, 320, 240);
    ofColor c(255, 255, 255);
    /*for(int i = 0; i < contourFinder.nBlobs; i++) {
        ofRectangle r = contourFinder.blobs.at(i).boundingRect;
        r.x += 320; r.y += 240;
        c.setHsb(i * 64, 255, 255);
        ofSetColor(c);
        ofRect(r);
    }*/
    
	ofSetColor(255);
    //cam.draw(0, 0);
    cam.draw(15, 15, cam.getWidth() * 0.2, cam.getHeight() * 0.2);
    //tracker.draw();
    
	//ofTranslate(cameraOffset.x, cameraOffset.y);
    ofSetColor(255);
    if(!puppetMode){
        legImg.draw(0,0, legImg.getWidth(), legImg.getHeight());
    }

	//anim guidelines
	if(puppetMode){
		ofNoFill();
		ofSetColor(0);
		ofPushMatrix();
		ofFill();
	}
    

	ofSetColor(255);
	legImg.getTextureReference().bind();
	puppet.drawFaces();
	legImg.getTextureReference().unbind();

	ofSetColor(128, 24);
	if(drawMesh)puppet.drawWireframe();
	if(drawIDs)puppet.drawVertexIDs();
	if(drawCtrlpoints)puppet.drawControlPoints();

	ofSetColor(0,255,128);
	tri.triangleMesh.drawWireframe();
	ofSetColor(255,0,0, 128);
	glPointSize(3);
	tri.triangleMesh.draw(OF_MESH_POINTS);
	glPointSize(1);

	//draw current tri selection
	ofSetColor(255,0,0,64);
	for( int i = 0 ; i < selectedTriangles.size() ; i++ ){
		vector<ofPoint> pts = tri.getPointsForITriangle(selectedTriangles[i]);
		ofBeginShape();
			ofVertex(pts[0].x, pts[0].y);ofVertex(pts[1].x, pts[1].y);ofVertex(pts[2].x, pts[2].y);
		ofEndShape();
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
	tri.draw();
    
    if (tracker.getFound()) {
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
            //ofCircle(topX, topY, 50);
            if (puppetTorso.size() == 7 && puppetMode) {
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
    
	//ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 10, 20);
    //imgTracker.draw();*/

}


void ofApp::mousePressed( int x, int y, int button ){

    cout << "\n";
    cout << "puppetTorso.push_back(ofPoint(";
    cout << x;
    cout << ", ";
    cout << y;
    cout << ", 0));";
    cout << "\n";
    
	if(puppetMode) return;

	x -= cameraOffset.x;
	y -= cameraOffset.y;

	//add points to mesh
	if(button == 0){
		if (tempVertex.length() > 0.0f){ //mouse over a vertex
			draggingVertex = true;
			selectedTriangles.clear();
		}
	}else{
		if (tempVertex.length() > 0.0f){ //mouse over a vertex
			selectedTriangles.clear();
			tri.removePointAtIndex(mouseOverVertexIndex);
			tri.triangulate();
		}
	}
}

void ofApp::mouseReleased(int x, int y, int button ){
	draggingVertex = false;
}

void ofApp::mouseDragged(int x, int y, int button){

	if(button == 1){
		cameraOffset.x += x - ofGetPreviousMouseX();
		cameraOffset.y += y - ofGetPreviousMouseY();
	}

	x -= cameraOffset.x;
	y -= cameraOffset.y;

	if(puppetMode) return;
	
	if(draggingVertex){
		tempVertex = ofVec2f(x,y);
		tri.setPointAtIndex(tempVertex, mouseOverVertexIndex);
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

	ITRIANGLE zero;

	switch(key){
            
        case 'p': {
            selectedTriangles.clear();
            int nTri = tri.getNumTriangles();
            for (int i = 0; i < nTri ; i++){
                ITRIANGLE ti = tri.getTriangleAtIndex(i);
                if (ti != zero){
                    selectedTriangles.push_back(ti);
                }
            }
            makePuppetFromSelectedTriangleMesh(tri, selectedTriangles, puppet);
			clear();
			puppetMode = true;
			puppet.setEvents(true);
            for (int i = 0; i < 24; i++){
                puppet.setControlPoint(i);
            }

            
        } break;
        
        case 'o': {
            bool foundOpaque = false;
            for (int x = 0; x < legImg.getWidth(); x = x + 60 ) {
                for (int y = 0; y < legImg.getHeight(); y = y + 60) {
                    if (x == 0 || y == 0) {
                        foundOpaque = false;
                    }
                    if (!foundOpaque && legImg.getColor(x, y).a != 0) {
                        tri.addPoint(x, y, 0);
                        tri.triangulate();
                        //selectedTriangles.clear();
                        foundOpaque = true;
                    }
                    else if (foundOpaque && legImg.getColor(x, y).a == 0) {
                        tri.addPoint(x, y, 0);
                        tri.triangulate();
                        //selectedTriangles.clear();
                        foundOpaque = false;
                    }
                }
            }
                        
            
        } break;
            
        case 'i' : {
            initLeftEye.push_back(ofPoint(740, 421, 0));
            initLeftEye.push_back(ofPoint(750, 414, 0));
            initLeftEye.push_back(ofPoint(764, 408, 0));
            initLeftEye.push_back(ofPoint(780, 406, 0));
            initLeftEye.push_back(ofPoint(799, 407, 0));
            
            for (int i = 0; i < initLeftEye.size(); i++) {
                tri.addPoint(initLeftEye[i].x, initLeftEye[i].y, 0);
                tri.triangulate();
            }
            
            initRightEye.push_back(ofPoint(844, 411, 0));
            initRightEye.push_back(ofPoint(857, 400, 0));
            initRightEye.push_back(ofPoint(873, 394, 0));
            initRightEye.push_back(ofPoint(888, 393, 0));
            initRightEye.push_back(ofPoint(900, 403, 0));
            
            for (int i = 0; i < initRightEye.size(); i++) {
                tri.addPoint(initRightEye[i].x, initRightEye[i].y, 0);
                tri.triangulate();
            }

            initNose.push_back(ofPoint(845, 479, 0));
            initNose.push_back(ofPoint(847, 494, 0));

            for (int i = 0; i < initNose.size(); i++) {
                tri.addPoint(initNose[i].x, initNose[i].y, 0);
                tri.triangulate();
            }
            
            initMouth.push_back(ofPoint(805, 541, 0));
            initMouth.push_back(ofPoint(819, 537, 0));
            initMouth.push_back(ofPoint(833, 532, 0));
            initMouth.push_back(ofPoint(846, 528, 0));
            initMouth.push_back(ofPoint(860, 530, 0));
            initMouth.push_back(ofPoint(874, 531, 0));
            initMouth.push_back(ofPoint(866, 542, 0));
            initMouth.push_back(ofPoint(855, 550, 0));
            initMouth.push_back(ofPoint(840, 553, 0));
            initMouth.push_back(ofPoint(825, 552, 0));
            initMouth.push_back(ofPoint(811, 553, 0));
            initMouth.push_back(ofPoint(794, 549, 0));

            for (int i = 0; i < initMouth.size(); i++) {
                tri.addPoint(initMouth[i].x, initMouth[i].y, 0);
                tri.triangulate();
            }
            
            puppetTorso.push_back(ofPoint(928, 811, 0));
            puppetTorso.push_back(ofPoint(977, 1031, 0));
            puppetTorso.push_back(ofPoint(960, 1206, 0));
            puppetTorso.push_back(ofPoint(741, 1282, 0));
            puppetTorso.push_back(ofPoint(522, 1063, 0));
            puppetTorso.push_back(ofPoint(617, 862, 0));
            puppetTorso.push_back(ofPoint(514, 709, 0));
            
            for (int i = 0; i < puppetTorso.size(); i++) {
                tri.addPoint(puppetTorso[i].x, puppetTorso[i].y, 0);
                tri.triangulate();
            }
            
            
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


void ofApp::clear(){
	tri.reset();
	selectedTriangles.clear();
	tt = Triangle();
	tempVertex = ofVec2f();
}


void ofApp::saveMesh(ofxDelaunay & t, vector<ITRIANGLE> & selected){
    
	ofxXmlSettings xml;

	xml.loadFile(MESH_XML_FILENAME);
	xml.clear();
	int nP = t.triangleMesh.getNumVertices();
	for(int i = 0; i < nP; i++){
		ofVec3f * pts = t.triangleMesh.getVerticesPointer();
		xml.addTag(XML_TAG_POINT_NAME);
		xml.setAttribute(XML_TAG_POINT_NAME, "x", pts[i].x, i);
		xml.setAttribute(XML_TAG_POINT_NAME, "y", pts[i].y, i);
	}

	ITRIANGLE zero;
	for(int i = 0; i < selectedTriangles.size() ; i++){
		ITRIANGLE ti = selectedTriangles[i];
		if (ti != zero){
			xml.addTag(XML_TAG_SELECTION_TRIANGLE_NAME);
			xml.setAttribute(XML_TAG_SELECTION_TRIANGLE_NAME, "p1", ti.p1, i);
			xml.setAttribute(XML_TAG_SELECTION_TRIANGLE_NAME, "p2", ti.p2, i);
			xml.setAttribute(XML_TAG_SELECTION_TRIANGLE_NAME, "p3", ti.p3, i);
		}
	}

	xml.saveFile(MESH_XML_FILENAME);
}


void ofApp::loadMesh(ofxDelaunay & t, vector<ITRIANGLE> & selected){

	tri.reset();
	selectedTriangles.clear();
	ofxXmlSettings xml;

	xml.loadFile(MESH_XML_FILENAME);
	int numPts = xml.getNumTags(XML_TAG_POINT_NAME);
	for (int i=0; i< numPts; i++){
		float x = xml.getAttribute(XML_TAG_POINT_NAME, "x", 0.0, i);
		float y = xml.getAttribute(XML_TAG_POINT_NAME, "y", 0.0, i);
		tri.addPoint(x,y, 0);
	}

	ITRIANGLE zero;
	int numT = xml.getNumTags(XML_TAG_SELECTION_TRIANGLE_NAME);
	for (int i=0; i< numT; i++){
		ITRIANGLE t;
		t.p1 = xml.getAttribute(XML_TAG_SELECTION_TRIANGLE_NAME, "p1", 0, i);
		t.p2 = xml.getAttribute(XML_TAG_SELECTION_TRIANGLE_NAME, "p2", 0, i);
		t.p3 = xml.getAttribute(XML_TAG_SELECTION_TRIANGLE_NAME, "p3", 0, i);
		if (t != zero){
			selectedTriangles.push_back(t);
		}
	}
}


bool ofApp::triangleInVector(const vector<ITRIANGLE> &selectedTriangles, ITRIANGLE t ){
	return std::find(selectedTriangles.begin(), selectedTriangles.end(), t) != selectedTriangles.end();
}


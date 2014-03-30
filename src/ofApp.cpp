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
	//ofMesh mesh = makeGrid(ofRectangle(100,100,600,600), 5, 5);
	//puppet.setup(mesh); //do this when presssing 'p' now

	//puppet.setControlPoint(0); // pin the top left
	//puppet.setControlPoint(9); // pin the top right
	puppet.setEvents(false);

	legImg.loadImage("leg.png");
    legImg.resize(ofGetWidth(), ofGetHeight());

	draggingVertex = false;
	puppetMode = false;

	OFX_REMOTEUI_SERVER_SETUP(10000);

	OFX_REMOTEUI_SERVER_SET_NEW_COLOR();
	OFX_REMOTEUI_SERVER_SHARE_PARAM(upperLegLen, 100, 500);
	OFX_REMOTEUI_SERVER_SHARE_PARAM(lowerLegLen, 100, 500);
	OFX_REMOTEUI_SERVER_SHARE_PARAM(legForwardness, -100, 500);
	OFX_REMOTEUI_SERVER_SHARE_PARAM(footXOffset, -200, 200);

	OFX_REMOTEUI_SERVER_SET_NEW_COLOR();
	OFX_REMOTEUI_SERVER_SHARE_PARAM(spinSpeed, -10, 10);

	OFX_REMOTEUI_SERVER_SET_NEW_COLOR();
	OFX_REMOTEUI_SERVER_SHARE_PARAM(kneeRotRadius, 0, 150);
	OFX_REMOTEUI_SERVER_SHARE_PARAM(kneeRotYness, 0.3, 1.5);

	OFX_REMOTEUI_SERVER_SHARE_PARAM(footRotRadius, 0, 150);
	OFX_REMOTEUI_SERVER_SET_NEW_COLOR();
	OFX_REMOTEUI_SERVER_SHARE_PARAM(footRotTimeOffset, -5, 5);

	OFX_REMOTEUI_SERVER_SET_NEW_COLOR();
	OFX_REMOTEUI_SERVER_SHARE_PARAM(drawIDs);
	OFX_REMOTEUI_SERVER_SHARE_PARAM(drawMesh);
	OFX_REMOTEUI_SERVER_SHARE_PARAM(drawCtrlpoints);

	OFX_REMOTEUI_SERVER_LOAD_FROM_XML();
    
    cam.initGrabber(1280, 720);
	tracker.setup();
}

void ofApp::update(){

    OFX_REMOTEUI_SERVER_UPDATE(1./60);

	puppet.setScreenOffset(cameraOffset);
	puppet.update();

	vector<int> ids = puppet.getControlPointIDs();
    
    cam.update();
	if(cam.isFrameNew() && ids.size() > 9) {
		tracker.update(ofxCv::toCv(cam));
        if (tracker.getImageFeature(tracker.LEFT_EYEBROW).getVertices().size() > 4){
            puppet.setControlPoint(ids[0], tracker.getImageFeature(tracker.LEFT_EYEBROW).getVertices()[0]);
            puppet.setControlPoint(ids[1], tracker.getImageFeature(tracker.LEFT_EYEBROW).getVertices()[1]);
            puppet.setControlPoint(ids[2], tracker.getImageFeature(tracker.LEFT_EYEBROW).getVertices()[2]);
            puppet.setControlPoint(ids[3], tracker.getImageFeature(tracker.LEFT_EYEBROW).getVertices()[3]);
            puppet.setControlPoint(ids[4], tracker.getImageFeature(tracker.LEFT_EYEBROW).getVertices()[4]);
            
            puppet.setControlPoint(ids[5], tracker.getImageFeature(tracker.RIGHT_EYEBROW).getVertices()[0]);
            puppet.setControlPoint(ids[6], tracker.getImageFeature(tracker.RIGHT_EYEBROW).getVertices()[1]);
            puppet.setControlPoint(ids[7], tracker.getImageFeature(tracker.RIGHT_EYEBROW).getVertices()[2]);
            puppet.setControlPoint(ids[8], tracker.getImageFeature(tracker.RIGHT_EYEBROW).getVertices()[3]);
            puppet.setControlPoint(ids[9], tracker.getImageFeature(tracker.RIGHT_EYEBROW).getVertices()[4]);
        }

	}

}

void ofApp::exit(){
	OFX_REMOTEUI_SERVER_CLOSE();
	OFX_REMOTEUI_SERVER_SAVE_TO_XML();
}


void ofApp::draw(){
    
	ofTranslate(cameraOffset.x, cameraOffset.y);

	float s = 1;
	ofSetColor(255);
	if(!puppetMode){
		legImg.draw(0,0, legImg.getWidth()*s, legImg.getHeight()*s);
	}

	//anim guidelines
	if(puppetMode){
		ofNoFill();
		ofSetColor(0);
		ofPushMatrix();
		//ofTranslate(kneeCenter);
			//ofScale(1, kneeRotYness);
			//ofCircle(0,0, kneeRotRadius);
			//ofCircle(0,0, 1);
		//ofPopMatrix();
		//ofCircle(footCenter.x, footCenter.y, footRotRadius);
		//ofCircle(footCenter.x, footCenter.y, 1);
		ofFill();
	}


	ofSetColor(255 );
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

	ofSetColor(255);

	ofSetupScreen();

	string instr =	string("left click to add mesh points (first 2 not visible!)\n") +
					"right click on mesh point to delete it\n" +
					"s : save Mesh and selected triangles\n" +
					"l : load Mesh and selected triangles\n" +
					"c : clear All Mesh Points\n" +
					"C : clear Triangle Selection\n" +
					"a : add/remove mouse-over Triangle to/from selection\n" +
					"A : add all triangles to selection\n" +
					"p : make Puppet from selected triangles\n";

	ofDrawBitmapString(instr, 20,ofGetHeight() - 130);

	tri.draw();
    
    //cam.draw(0, 0);
	ofSetLineWidth(2);
	tracker.getImageFeature(tracker.LEFT_EYEBROW).draw();
	ofDrawBitmapString(ofToString((int) ofGetFrameRate()), 10, 20);

}


void ofApp::mousePressed( int x, int y, int button ){

	if(puppetMode) return;

	x -= cameraOffset.x;
	y -= cameraOffset.y;

	//add points to mesh
	if(button == 0){
		if (tempVertex.length() > 0.0f){ //mouse over a vertex
			draggingVertex = true;
			selectedTriangles.clear();
		}else{ //mouse not over a vertex
			tri.addPoint(x, y, 0);
			tri.triangulate();
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


	TIME_SAMPLE_START("moved");
	if (tri.getNumPoints() > 2) {
		tempVertex = tri.getPointNear(ofVec2f(x,y), SELECTION_DISTANCE, mouseOverVertexIndex);
		if (tempVertex.length() == 0.0f){ // only select triangles if not selectng vertexs
			ITRIANGLE t = tri.getTriangleForPos(ofPoint(x,y));
			vector<ofPoint> pts = tri.getPointsForITriangle(t);
			tt.set(pts[0], pts[1], pts[2]);
		}else{
			tt.set(ofVec2f(), ofVec2f(), ofVec2f());
		}
	}
	TIME_SAMPLE_STOP("moved");
}


void ofApp::keyPressed( int key ){

	ITRIANGLE zero;

	switch(key){

		case 's': saveMesh(tri, selectedTriangles); break;
		case 'l': loadMesh(tri, selectedTriangles); tri.triangulate(); break;
		case 'c': clear(); selectedTriangles.clear();break;
		case 'C':{
			selectedTriangles.clear();
		} break;

		case 'a':{
			ITRIANGLE ti = tri.getTriangleForPos(ofVec2f(mouseX - cameraOffset.x,mouseY - cameraOffset.y));

			if (ti != zero){
				if(!triangleInVector(selectedTriangles, ti)){ //not in current selection, add
					selectedTriangles.push_back(ti);
				}else{ //already in selection, remove from selection
					for(int i = 0; i < selectedTriangles.size(); i++){
						if (selectedTriangles[i] == ti){
							selectedTriangles.erase(selectedTriangles.begin()+i);
							break;
						}
					}
				}
			}
		}break;

		case 'A':{//select ALL triangles
				selectedTriangles.clear();
				int nTri = tri.getNumTriangles();
				for(int i = 0; i < nTri ; i++){
					ITRIANGLE ti = tri.getTriangleAtIndex(i);
					if (ti != zero){
						selectedTriangles.push_back(ti);
					}
				}
			}break;

		case 'p':
			makePuppetFromSelectedTriangleMesh(tri, selectedTriangles, puppet);
			clear();
			puppetMode = true;
			puppet.setEvents(true);
			break;
	}
}


void ofApp::clear(){
	tri.reset();
	selectedTriangles.clear();
	tt = Triangle();
	tempVertex = ofVec2f();
}


void ofApp::saveMesh(ofxDelaunay & t, vector<ITRIANGLE> & selected){

	cout << "saveMesh\n";
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

	cout << "loadMesh\n";
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


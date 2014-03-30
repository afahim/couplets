#include "ofAppGlutWindow.h"
#include "ofApp.h"

int main() {
	ofAppGlutWindow window;
	//window.setGlutDisplayString("rgba double samples>=8 depth");
	ofSetupOpenGL(&window, 1024, 780, OF_WINDOW);
	ofRunApp(new ofApp());
}

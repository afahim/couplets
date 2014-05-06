#include "ofAppGlutWindow.h"
#include "ofApp.h"

int main() {
	ofAppGlutWindow window;
	ofSetupOpenGL(&window, 1400, 1800, OF_FULLSCREEN);
	ofRunApp(new ofApp());
}

#include "ofAppGlutWindow.h"
#include "ofApp.h"

int main() {
	ofAppGlutWindow window;
	ofSetupOpenGL(&window, 1400, 1400, OF_WINDOW);
	ofRunApp(new ofApp());
}

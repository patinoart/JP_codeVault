#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    
    // get the width and height, and allocate color and grayscale images: 
	width = 320;
	height = 240;

	// setup video grabber:
	video.initGrabber(width, height);
	
	// allocate the pixels for the grayscale + color video
    videoColorCvImage.allocate(width, height);
	videoGrayscaleCvImage.allocate(width, height);
	
    // allocate pixels for the adjusted video
    videoBgImage.allocate(width, height);
	videoDiffImage.allocate(width, height);
	
	// set background color to be white: 
	ofBackground(255, 255, 255);

	// setup the main panel for the GUI
    panel.setup("cv settings", 1024, 0, 276, 800);
	panel.addPanel("control", 1, false);
	
	panel.setWhichPanel("control");
	panel.setWhichColumn(0);
	
    // toggle to turn on bg
    panel.addToggle("learn background ", "B_LEARN_BG", true);
	panel.addSlider("threshold ", "THRESHOLD", 127, 0, 255, true);
	
	// slider for dilations and erosions
    panel.addSlider("nDilations ", "N_DILATIONS", 2, 0, 20, true);  // dilation to make brighter
	panel.addSlider("nErosions ", "N_EROSION", 0, 0, 20, true);     // erosion to make darker
	
	// vals are stored in this file in the data folder
    panel.loadSettings("cvSettings.xml");
}

//--------------------------------------------------------------
void testApp::update(){
    
    // set background color
    ofBackground(100,100,100);
		
	panel.update();
	
	
	bool bLearnBg			= panel.getValueB("B_LEARN_BG");
	int threshold			= panel.getValueI("THRESHOLD");
	int nDilations			= panel.getValueI("N_DILATIONS");
	int nErosions			= panel.getValueI("N_EROSION");
	int maxOperations		= MAX(nDilations, nErosions);

	video.update();
	
	if (video.isFrameNew()){
		
		videoColorCvImage.setFromPixels(video.getPixels(), width, height);
		videoGrayscaleCvImage = videoColorCvImage;
		
		if ( bLearnBg ){ 
			videoBgImage = videoGrayscaleCvImage;
			panel.setValueB("B_LEARN_BG", false);
		}
		
		if (ofGetElapsedTimef() < 1.5){
			videoBgImage = videoGrayscaleCvImage;
		}
		
		videoDiffImage.absDiff(videoGrayscaleCvImage, videoBgImage);
		videoDiffImage.threshold(threshold);
		
		for (int i = 0; i < maxOperations; i++){
			if (i < nErosions)	videoDiffImage.erode();
			if (i < nDilations)	videoDiffImage.dilate();
		}
	}
}

//--------------------------------------------------------------
void testApp::draw(){
	
	ofSetColor(255, 255, 255);
    
    // draw first column of images, color
    video.draw(20, 20);
	
    
    // second column, grayscale
    videoGrayscaleCvImage.draw(360, 20, 320, 240);
	videoBgImage.draw(360, 280, 320, 240);
	
    // third column, difference
    videoDiffImage.draw(700, 20, 320, 240);	
	
	panel.draw();

}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
	
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	panel.mouseDragged(x,y,button);
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
	panel.mousePressed(x,y,button);
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	panel.mouseReleased();
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}


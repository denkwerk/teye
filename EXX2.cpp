//============================================================================
// Name        : EXX2.cpp
// Author      : benjamin wolf
// Version     : 0.9.0b
// Copyright   : 2015 denkwerk GmbH
// Description : EXX2 in C++, Ansi-style
//============================================================================


#include <stdio.h>
#include <string>

#include "Additionals/ArgvParser/argvparser.h"

#include "Classes/MainController/EXMainController.h"
//#include "Classes/UI/UIWindow.h"

#ifndef _WITHOUT_SPEECH_RECOGNITION
#include "Classes/SpeechDetection/EXSpeechController.h"
#endif

#ifdef _OPEN_CV
#include "Classes/ImageRecognition/EXImageRecognition.h"
#endif

#include "Classes/GPIO/GPIOController.h"
#include "Classes/SIP/SIPController.h"

#include "Classes/GPIOFunctions/GPIOServoFunction.h"
#include "Classes/GPIOFunctions/GPIOMCP3008Function.h"



/*
 * ARGC
 *
 * io_bypass
 * speak_bypass
 * sip_bypass
 *
 * gui
 */

/*static int32 ReadFile (ad_rec_t * ad, int16 * buf, int32 max) {
    size_t nread;

    nread = fread(buf, sizeof(int16), max, rawfd);

    return (nread > 0 ? nread : -1);
}*/

int main(int argc, char **argv) {

	//UI::UIWindow *WindowController					= 0;
#ifndef _WITHOUT_SPEECH_RECOGNITION
	EXSpeech::EXSpeechController *SpeechController			= 0;
#endif

#ifdef _OPEN_CV
	ImageRecognition::EXImageRecognition *IRController  = 0;
#endif

	GPIO::GPIOController *IOController					= 0;
	SIP::SIPController *_SIPController					= 0;

	std::streambuf *coutbuf;

	std::string AbsolutePath;
	if (argc > 0) {
		AbsolutePath = std::string(*argv);
		AbsolutePath = AbsolutePath.substr(0, AbsolutePath.rfind("EXX2"));
	}

	// Initialize ArgumentParser
	CommandLineProcessing::ArgvParser ARGParser;


	// Initialize MainLoopController
	EX::EXMainController *MainController = new EX::EXMainController(argc, argv, AbsolutePath, &ARGParser);


	// Configurate ArgumentParser
	ARGParser.setIntroductoryDescription("This is foo written by bar.");

	ARGParser.addErrorCode(0, "Success");
	ARGParser.addErrorCode(1, "Error");

	ARGParser.defineOption("fileOut", "", CommandLineProcessing::ArgvParser::NoOptionAttribute);
	ARGParser.defineOptionAlternative("fileOut","f");

	ARGParser.defineOption("gui", "", CommandLineProcessing::ArgvParser::NoOptionAttribute);
	ARGParser.defineOptionAlternative("gui","g");


	// Parse Arguments or Options
	int result = ARGParser.parse(argc, argv);
	if (result != CommandLineProcessing::ArgvParser::NoParserError) {
	    exit(1);
	}

	if (ARGParser.foundOption("fileOut")) {
		std::ofstream out("out.txt");
		coutbuf = std::cout.rdbuf();
		std::cout.rdbuf(out.rdbuf());
	}


	// Define and Initialize the Speak Recognition
#ifndef _WITHOUT_SPEECH_RECOGNITION
	SpeechController = new EXSpeech::EXSpeechController();
	SpeechController->SetAbsolutePath(AbsolutePath);
	MainController->setSpeechController(SpeechController);
#endif

	// Define and Initialize the Image Recognition Controller
#ifdef _OPEN_CV
	IRController = new ImageRecognition::EXImageRecognition();
	MainController->setIRController(IRController);
#endif

	// Define and Initialize the IO Controller Unit
	IOController = new GPIO::GPIOController();
	MainController->setIOController(IOController);


	// Define and Initialize the IP Telefony Unit
	_SIPController = new SIP::SIPController();
	_SIPController->SetAbsolutePath(AbsolutePath);
	MainController->setSIPController(_SIPController);


	// Start the Run Loop -----------------------------------------------------------------
	int Return = MainController->RunEventHandling();
	// ------------------------------------------------------------------------------------

	printf("Uninitialize Application...\n");
	delete MainController;

#ifndef _WITHOUT_SPEECH_RECOGNITION
	if (SpeechController != 0) {
		printf("Delete SpeechController\n");
		delete SpeechController;
	}
#endif

	if (IOController != 0) {
		printf("Delete IOController\n");
		delete IOController;
	}
	if (_SIPController != 0) {
		printf("Delete SIPController\n");
		delete _SIPController;
	}
	/*if (WindowController != 0) {
		printf("Delete WindowController\n");
		delete WindowController;
	}*/

	if (ARGParser.foundOption("fileOut")) {
		std::cout.rdbuf(coutbuf);
	}

	return Return;
}

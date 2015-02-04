/*
 * 		EXImageRecognition.cpp
 *
 *  	Created on: 29.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#include "EXImageRecognition.h"

#if !(TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR)
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include <opencv2/highgui/highgui_c.h>
#else
#include <opencv2/imgproc/imgproc_c.h>
#include <opencv2/imgproc/imgproc.hpp>
//#include "EXIntHaarFrontFaceClassifierCascade.cpp"
#endif


#include <unistd.h>
#include <sys/time.h>
#include <ctime>

namespace ImageRecognition {
    
    
#define OFFSET_CHECK(x1, x2, Offset)(abs(x1 - x2) > Offset)
#define SORT_SIZE [](size_t const &a, size_t const &b) {return a < b;}
#define SORT_RECT [](cv::Rect const &a, cv::Rect const &b){return a.size().width * a.size().height < b.size().width * b.size().height;}


    // Class Initialization

    EXImageRecognition::EXImageRecognition() : FrontCamera(false), COMPUTING_SCALE_FACTOR(0.8), OUTPUT_GREY(false), SMILE_RECOGNITION(false), OUTPUT_RESIZE(false), OUTPUT_DETECTION_CYCLES(false), OUTPUT_DETECTED_FACES(false), OUTPUT_SMILE_INTENSITY(false), OUTPUT_AVERAGE_FACES(false), CALCULATE_SMILE_INTENSITY(true), SMILE_SMALL_IMAGE(true), SMILE_USING_MIDDLE(false), ENABLE_MANUAL_FACE_RECOGNITION_SETTINGS(false), ENABLE_AUTOMATIC_CALIBRATED_FACE_SETTINGS(true), ENABLE_MANUAL_SMILE_SETTINGS(false), ENABLE_AUTOMATIC_CALIBRATED_SMILE_SETTINGS(true), OUTPUT_DEBUG_IMAGES(true), FRAMERATE(10), FRAMERATE_CURRENT(0), ROI_AVERAGE_SIZE(5), ROI_RESET_OFFSET_VALUE(10), SMILE_AVERAGE_SIZE(5), SMILE_RESET_OFFSET_VALUE(10), SMILE_SMALL_ROI_PERCENTAGE_HEIGHT(30.0), IMAGE_OUT_ITERATOR_1(0),  _thread(0), _video_thread(0), _exit(0), _videoexit(0), _ImageVectorSize(0), HaarFrontFaceClassifierCascade(0), HaarSmileClassifierCascade(0), VideoCapture(0), VideoWriter(0), _HappyCounter(0), _SadCounter(0), _Initialized(false), _CurrentSmileIntensity(0.0), _CurrentEmotion(ImageRecognizationTypeNone), _Device(CaptureDeviceNone) {
        // TODO Auto-generated constructor stub
        
        INPUT_SIZE = cv::Size(640, 480);

#ifdef __APPLE__
    	Apple_Dispatch = false;
        Apple_Video_Dispatch = false;
#endif

#if __arm__ && _OPEN_CV_RASPI
    	VideoCaptureHandle = 0;
#endif

        pthread_mutex_init(&_mutex_task_lock, NULL);
        pthread_mutex_init(&_mutex_average_lock, NULL);
        pthread_mutex_init(&_mutex_video_lock, NULL);
        
    }

    EXImageRecognition::~EXImageRecognition() {
        // TODO Auto-generated destructor stub

        pthread_mutex_destroy(&_mutex_task_lock);
        pthread_mutex_destroy(&_mutex_average_lock);
        pthread_mutex_destroy(&_mutex_video_lock);
        
    }


    // Public Methods

    bool EXImageRecognition::Initialize(CaptureDevice Device) {
    	return this->InitializeLibrary(Device);
    }

    bool EXImageRecognition::StartRecognition() {
        return this->StartKeyThread();
    }

    bool EXImageRecognition::StopRecognition() {
        return this->StopKeyThread();
    }

    void EXImageRecognition::LockImageBuffer() {
        pthread_mutex_lock(&_mutex_task_lock);
    }

    void EXImageRecognition::UnlockImageBuffer() {
        pthread_mutex_unlock(&_mutex_task_lock);
    }
    
    bool EXImageRecognition::EnableVideoOutput(std::string Filename) {
        VideoWriter = new cv::VideoWriter(Filename, CV_FOURCC('Y','U','V','1'), FRAMERATE, INPUT_SIZE, true);
        return VideoWriter->isOpened();
    }

    cv::Mat& EXImageRecognition::GetImage(bool Mini) {
        
        return _CurrentFrame;
        
        // Old
        
        /*if (Mini == true) {
            return _SUBMiniImage;
        }
        return _SUBImage;
         */
    }

    std::vector<cv::Rect> EXImageRecognition::GetSmiles() {
        return _DetectedSmiles;
    }

    std::vector<cv::Rect> EXImageRecognition::GetFaces() {
        return _DetectedFaces;
    }

    cv::Rect EXImageRecognition::GetComputedFace() {
        return _ComputedFace;
    }

    void EXImageRecognition::SetShowDetectedSmiles(bool Show) {
        OUTPUT_DETECTION_CYCLES = Show;
    }

    bool EXImageRecognition::ShowDetectedSmiles() {
        return OUTPUT_DETECTION_CYCLES;
    }

    void EXImageRecognition::SetShowDetectedFaces(bool Show) {
        OUTPUT_DETECTED_FACES = Show;
    }

    bool EXImageRecognition::ShowDetectedFaces() {
        return OUTPUT_DETECTED_FACES;
    }

    void EXImageRecognition::SetShowGreyOutput(bool Show) {
        OUTPUT_GREY = Show;
    }

    bool EXImageRecognition::ShowGreyOutput() {
        return OUTPUT_GREY;
    }
    
    void EXImageRecognition::SetEnableSmileDetection(bool Enable) {
        SMILE_RECOGNITION = Enable;
    }
    
    bool EXImageRecognition::SmileDetectionEnabled() {
        return SMILE_RECOGNITION;
    }

    void EXImageRecognition::SetResizeOutput(bool Resize) {
        OUTPUT_RESIZE = Resize;
    }

    bool EXImageRecognition::ResizeOutput() {
        return OUTPUT_RESIZE;
    }

    void EXImageRecognition::SetScaleFactor(float Scale) {
        COMPUTING_SCALE_FACTOR = Scale;
    }
    
    float EXImageRecognition::ScaleFactor() {
        return COMPUTING_SCALE_FACTOR;
    }
    
    void EXImageRecognition::EnableUsingSmileMiddleSmoothing(bool Using) {
        SMILE_USING_MIDDLE = Using;
    }
    
    bool EXImageRecognition::UsingSmileMiddleSmoothing() {
        return SMILE_USING_MIDDLE;
    }
    
    void EXImageRecognition::EnableManualSmileDetectionSettings(bool Enable) {
        ENABLE_MANUAL_SMILE_SETTINGS = Enable;
    }
    
    bool EXImageRecognition::ManualSmileDetectionSettings() {
        return ENABLE_MANUAL_SMILE_SETTINGS;
    }
    
    void EXImageRecognition::SetManualSmileDetectionSettings(DetectionSettings Settings) {
        MANUAL_SMILE_DETECTION_SETTINGS = Settings;
    }
    
    DetectionSettings EXImageRecognition::GetManualSmileDetectionSettings() {
        return MANUAL_SMILE_DETECTION_SETTINGS;
    }
    
    void EXImageRecognition::EnableManualFaceDetectionSettings(bool Enable) {
        ENABLE_MANUAL_FACE_RECOGNITION_SETTINGS = Enable;
    }
    
    bool EXImageRecognition::ManualFaceDetectionSettings() {
        return ENABLE_MANUAL_FACE_RECOGNITION_SETTINGS;
    }
    
    void EXImageRecognition::SetManualFaceDetectionSettings(DetectionSettings Settings) {
        MANUAL_FACE_DETECTION_SETTINGS = Settings;
    }
    
    void EXImageRecognition::SetFaceROIResetValue (float Offset) {
        if (Offset < 0.0) {
            Offset = 0.0;
        }
        ROI_RESET_OFFSET_VALUE = Offset;
    }
    
    float EXImageRecognition::GetFaceROIResetValue () {
        return ROI_RESET_OFFSET_VALUE;
    }
    
    DetectionSettings EXImageRecognition::GetManualFaceDetectionSettings() {
        return MANUAL_FACE_DETECTION_SETTINGS;
    }
    
    void EXImageRecognition::SetInputSize(cv::Size Size) {
        INPUT_SIZE = Size;
    }
    
    cv::Size EXImageRecognition::GetInputSize() {
        return INPUT_SIZE;
    }

    void EXImageRecognition::SetShowSmileIntensity(bool Show) {
    	OUTPUT_SMILE_INTENSITY = Show;
    }

    bool EXImageRecognition::ShowSmileIntensity() {
    	return OUTPUT_SMILE_INTENSITY;
    }
    
    void EXImageRecognition::SetFramerate(unsigned int FPS) {
        FRAMERATE = FPS;
    }
    
    unsigned long long EXImageRecognition::GetFramerate() {
        return (unsigned long long)FRAMERATE;
    }
    
    unsigned int EXImageRecognition::GetCurrentFramerate() {
        return FRAMERATE_CURRENT;
    }
    
    void EXImageRecognition::EnableRecognizeMiniImage(bool Enable) {
        SMILE_SMALL_IMAGE = Enable;
    }
    
    bool EXImageRecognition::RecognizeMiniImage() {
        return SMILE_SMALL_IMAGE;
    }
    
    void EXImageRecognition::SetRecognizeImagePercentage(unsigned int Percent) {
        SMILE_SMALL_ROI_PERCENTAGE_HEIGHT = Percent;
    }
    
    unsigned int EXImageRecognition::RecognizeImagePercentage() {
        return SMILE_SMALL_ROI_PERCENTAGE_HEIGHT;
    }
    
    void EXImageRecognition::SetROIAverageSize(unsigned int ROISize) {
        if (ROISize < 1) {
            ROISize = 1;
        }
        ROI_AVERAGE_SIZE = ROISize;
    }
    
    unsigned int EXImageRecognition::ROIAverageSize() {
        return ROI_AVERAGE_SIZE;
    }
    
    void EXImageRecognition::SetSmoothFaces(bool Smooth) {
        OUTPUT_AVERAGE_FACES = Smooth;
    }
    
    bool EXImageRecognition::SmoothFaces() {
        return OUTPUT_AVERAGE_FACES;
    }
    
    cv::Rect EXImageRecognition::GetAveragePositionOfBiggestFace() {
        pthread_mutex_lock(&_mutex_average_lock);
        cv::Rect Average;
        size_t Size = 0;
        if ((Size = _LastROIs.size()) != 0) {
            std::deque<cv::Rect>::iterator it = _LastROIs.begin();
            Average.x = (*it).x;
            Average.y = (*it).y;
            Average.width = (*it).width;
            Average.height = (*it).height;
            it++;
            while (it != _LastROIs.end()) {
                Average.x += (*it).x;
                Average.y += (*it).y;
                Average.width += (*it).width;
                Average.height += (*it).height;
                it++;
            }
        
            Average.x = Average.x / Size;
            Average.y = Average.y / Size;
            Average.width = Average.width / Size;
            Average.height = Average.height / Size;
        }
        pthread_mutex_unlock(&_mutex_average_lock);
        return Average;
    }
    
    cv::Rect EXImageRecognition::GetMiddlePositionOfBiggestFace() {
        pthread_mutex_lock(&_mutex_average_lock);
        cv::Rect Buffered;
        size_t Size = 0;
        if ((Size = _LastROIs.size()) > 0) {
            std::deque<cv::Rect> SortedList = _LastROIs;
            std::sort(SortedList.begin(), SortedList.end(), SORT_RECT);
            int InTheMiddle = (float)Size * 0.5;
            Buffered = SortedList[InTheMiddle];
        }
        pthread_mutex_unlock(&_mutex_average_lock);
        return Buffered;
    }
    
    void EXImageRecognition::SetSMILEAverageSize(unsigned int SMILESize) {
        SMILE_AVERAGE_SIZE = SMILESize;
    }
    
    unsigned int EXImageRecognition::SMILEAverageSize() {
        return SMILE_AVERAGE_SIZE;
    }
    
    float EXImageRecognition::GetAverageSmileIntensity() {
        pthread_mutex_lock(&_mutex_average_lock);
        float Average;
        size_t Size = 0;
        if ((Size = _LastSMILEs.size()) != 0) {
            std::deque<size_t>::iterator it = _LastSMILEs.begin();
            while (it != _LastSMILEs.end()) {
                Average += (*it);
                it++;
            }
            Average = Average / (float)Size;
        }
        pthread_mutex_unlock(&_mutex_average_lock);
        return Average;
    }
    
    float EXImageRecognition::GetMiddleSmileIntensity() {
        pthread_mutex_lock(&_mutex_average_lock);
        float Buffered = 0.0;
        size_t Size = 0;
        if ((Size = _LastSMILEs.size()) > 0) {
            std::deque<size_t> SortedList = _LastSMILEs;
            std::sort(SortedList.begin(), SortedList.end(), SORT_SIZE);
            int InTheMiddle = (float)Size * 0.5;
            Buffered = SortedList[InTheMiddle];
        }
        pthread_mutex_unlock(&_mutex_average_lock);
        return Buffered;
    }
    
    ImageRecognizationType EXImageRecognition::GetCurrentEmotion() {
        return _CurrentEmotion;
    }
    
    cv::Rect EXImageRecognition::ConvertToSize(cv::Mat Matrice, cv::Size Size, cv::Rect Rect) {
        if (Matrice.empty() == false) {
            return EXImageRecognition::ConvertToSize(Matrice.size(), Size, Rect);
        }
        return cv::Rect(0,0,0,0);
    }

    cv::Rect EXImageRecognition::ConvertToSize(cv::Size SourceSize, cv::Size Size, cv::Rect Rect) {
        if (SourceSize.width != 0.0) {
            float Factor = (float)Size.width / (float)SourceSize.width;
            return cv::Rect(Rect.x * Factor, Rect.y * Factor, Rect.width * Factor, Rect.height * Factor);
        }
        return cv::Rect(0,0,0,0);
    }
    
    cv::Rect EXImageRecognition::ConvertToSize(cv::Size Size, cv::Rect Rect) {
        if (Rect.width != 0.0) {
            float Factor = (float)Size.width / (float)Rect.width;
            return cv::Rect(Rect.x * Factor, Rect.y * Factor, Rect.width * Factor, Rect.height * Factor);
        }
        return cv::Rect(0,0,0,0);
    }

#ifdef __APPLE__
    void EXImageRecognition::SetBundlePath(std::string Path) {
        BundlePath = Path;

        if (BundlePath.length() > 0 && BundlePath.substr(BundlePath.length()-1, BundlePath.length()) != std::string("/")) {
#ifdef __MAC_OS_X_VERSION_MAX_ALLOWED
            BundlePath.append("/Contents/Resources/");
#else
            BundlePath.append("/");
#endif
        }
    }

    void EXImageRecognition::SetFrontCamera(bool Front) {
        bool OldValue = FrontCamera;
        FrontCamera = Front;

        if (VideoCapture != 0 && VideoCapture->isOpened() && Front != OldValue) {
            this->ActivateVideoCapture();
        }
    }

    bool EXImageRecognition::IsSetFrontCamera() {
        return FrontCamera;
    }
    
    void EXImageRecognition::SetVideoBlock(dispatch_block_t Block) {
        Apple_Video_Dispatch = true;
        ImageBlock = Block;
    }

    bool EXImageRecognition::StartRecognition(dispatch_block_t Block) {
        Apple_Dispatch = true;
        DispatchBlock = Block;
        return this->StartKeyThread();
    }
    
    cv::Mat EXImageRecognition::ConvertToView (cv::Mat& Input) {
        if (Input.empty() == false) {
            cv::flip(Input,Input,1);
        }
        return Input;
    }
    
    cv::Mat EXImageRecognition::ConvertColorRange (cv::Mat& Input) {
        if (Input.empty() == false) {
            cv::cvtColor(Input, Input, CV_BGR2RGB);
        }
        return Input;
    }
#endif


    // Threading

    bool EXImageRecognition::StartKeyThread () {
        if (_thread == 0) {
            pthread_attr_t attr;

            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

            int error = pthread_create(&_thread, NULL, &RunThread, (void*)this);

            pthread_attr_destroy(&attr);

            if (error == 0) {
                return true;
            }
        }
        return false;
    }

    bool EXImageRecognition::StopKeyThread () {
        if (_thread != 0) {
            _exit = 1;

            //pthread_mutex_unlock(&_mutex_task_lock);

            //void *status;

            int error = pthread_kill(_thread, 0);
            //int error = pthread_join(_thread, &status);

            // Reconfig
            /*int er = tcsetattr (0, TCSAFLUSH, &oldtio);
            if (er) {
                return 0;
            }*/

            if (error == 0) {
            	this->Cleanup();
                _thread = 0;
                _exit = 0;
                return true;
            }
        }
        return false;
    }

    void *EXImageRecognition::RunThread (void *self) {
        EXImageRecognition *Class = (EXImageRecognition*)self;

        if (Class->InitializeLibrary()) {
#ifdef EX_EVENT_HANDLING
        	Class->SendEvent(new ImageEvent(ThreadWorkingTypeStartRecognize));
#endif
            
            Class->StartVideoInputThread();
            unsigned long long TimeStamp = Class->GetMillisecondTime();
            unsigned int FPS = 0;
            
            while (Class->_exit == 0) {
                if (Class->Recognition() == false) {
#ifdef EX_EVENT_HANDLING
                	Class->SendEvent(new ImageEvent(ThreadWorkingTypeRecognizeFailed));
#endif
                }
                FPS++;
                unsigned long long BufferTime = 0;
                if ((TimeStamp + 1000) <= (BufferTime = Class->GetMillisecondTime())) {
                    Class->SetCurrentFPS(FPS);
                    TimeStamp = BufferTime;
                    FPS = 0;
                }
                usleep((1000 / Class->GetFramerate()) * 1000);
            }
            
            Class->StopVideoInputThread();
            
#ifdef EX_EVENT_HANDLING
            Class->SendEvent(new ImageEvent(ThreadWorkingTypeUnInitialize));
#endif
            if (Class->Cleanup()) {
#ifdef EX_EVENT_HANDLING
            	Class->SendEvent(new ImageEvent(ThreadWorkingTypeUnInitialized));
#endif
            }
        }

        return 0;
    }
    
    bool EXImageRecognition::StartVideoInputThread () {
        if (_video_thread == 0) {
            pthread_attr_t attr;
            
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
            
            int error = pthread_create(&_video_thread, NULL, &RunVideoThread, (void*)this);
            
            pthread_attr_destroy(&attr);
            
            if (error == 0) {
                return true;
            }
        }
        return false;
    }
    
    bool EXImageRecognition::StopVideoInputThread () {
        if (_video_thread != 0) {
            
            _videoexit = 1;
            int error = pthread_kill(_video_thread, 0);
            //int error = pthread_join(_video_thread, &status);
            
            if (error == 0) {
                _video_thread = 0;
                _videoexit = 0;
                return true;
            }
        }
        return false;
    }
    
    void *EXImageRecognition::RunVideoThread (void *self) {
        EXImageRecognition *Class = (EXImageRecognition*)self;
        
        unsigned long long TimeStamp = Class->GetMillisecondTime();
        unsigned int FPS = 0;
        
        while (Class->_videoexit == 0) {
            
            
            // Check Capture Device
            
            bool OK = false;
            if (Class->_Device == CaptureDeviceRaspiCam) {
    #if __arm__ && _OPEN_CV_RASPI
                if (VideoCaptureHandle != 0) {
                    OK = true;
                }
    #endif
            } else {
                if (Class->VideoCapture != 0 && Class->VideoCapture->isOpened()) {
                    OK = true;
                }
            }
            
            
            // If Good Capture Device exists
            
            if (OK == true) {
                
                
                // Get New Frame
                
                cv::Mat SourceFrame;
                if (Class->_Device == CaptureDeviceRaspiCam) {
        #if __arm__ && _OPEN_CV_RASPI
                    SourceFrame = cv::cvarrToMat(raspiCamCvQueryFrame(Class->VideoCaptureHandle));
        #endif
                } else {
                    *Class->VideoCapture >> SourceFrame;
                }
                
                Class->SetNewFrame(SourceFrame);
                
                #ifdef __APPLE__
                
                // Call Callback Method for Obj-C
                if (Class->Apple_Video_Dispatch) {
                    Class->ImageBlock();
                }
                
                #endif
                
            }
            
            
            // Calculate REAL Framerate
            
            FPS++;
            unsigned long long BufferTime = 0;
            if ((TimeStamp + 1000) <= (BufferTime = Class->GetMillisecondTime())) {
                Class->SetRealFPS(FPS);
                TimeStamp = BufferTime;
                FPS = 0;
            }
            usleep((1000 / Class->GetFramerate()) * 1000);
        }
        
        return 0;
    }


    // Logic

    bool EXImageRecognition::InitializeLibrary(CaptureDevice Device) {

    	if (_Initialized == true) {
    		return true;
    	}

    	if (Device == CaptureDeviceNone) {
    		Device = CaptureDeviceUSBCamera;
    	} else {
    		_Device = Device;
    	}

        bool Success = true;

        //Success &= this->CalculateImageVectorSize();
        //Success &= (Success == true ? this->ReadTrainingImages() : false);
        //Success &= (Success == true ? this->CalculateEigenVectors() : false);
        //Success &= (Success == true ? this->ComputeProjectionMatrix() : false);
        Success &= (Success == true ? this->PrepareForFaceRecognition() :false);

        _Initialized = Success;

#ifdef EX_EVENT_HANDLING
        if (Success == true) {
        	SendEvent(new ImageEvent(ThreadWorkingTypeInit));
        }
#endif

        return Success;
    }

    bool EXImageRecognition::CalculateImageVectorSize() {
#ifdef __APPLE__
        std::string Path = BundlePath;
#else
        std::string Path;
#endif
        Path.append("data/happy00.png");
        cv::Mat ImageRef = cv::imread(Path, cv::IMREAD_COLOR);

        if (ImageRef.empty() == false) {
            cv::Size s = ImageRef.size();
            _ImageVectorSize = s.width * s.height;
            return true;
        }

        return false;
    }

    bool EXImageRecognition::ReadTrainingImages() {
        char file[64] = {0};
        I = cv::Mat(_ImageVectorSize, _NUMBER_OF_TRAINING_IMAGES, CV_32FC1);
        S = cv::Mat::zeros(_ImageVectorSize, 1, CV_32FC1);

        bool Check[2] = {false};

#ifdef __APPLE__
        std::string TmpBundlePath = BundlePath;
#else
        std::string TmpBundlePath;
#endif

        for (int i = 0; i < _NUMBER_OF_TRAINING_IMAGES / 2; i++) {
            std::string Path = TmpBundlePath;
            sprintf(file, "data/happy%02d.png", i);
            Path.append(file);
            cv::Mat m = cv::imread(Path, cv::IMREAD_GRAYSCALE);

            if (m.empty() == false) {
                m = m.t();
                m = m.reshape(1, _ImageVectorSize);
                m.convertTo(m, CV_32FC1);
                m.copyTo(I.col(i));
                S = S + m;
                Check[0] = true;
            }
        }

        for (int i = 0; i < _NUMBER_OF_TRAINING_IMAGES / 2; i++) {
            std::string Path = TmpBundlePath;
            sprintf(file, "data/sad%02d.png", i);
            Path.append(file);
            cv::Mat m = cv::imread(Path, cv::IMREAD_GRAYSCALE);
            if (m.empty() == false) {
                m = m.t();
                m = m.reshape(1, _ImageVectorSize);
                m.convertTo(m, CV_32FC1);
                m.copyTo(I.col(i + _NUMBER_OF_TRAINING_IMAGES / 2));
                S = S + m;
                Check[1] = true;
            }
        }

        return (Check[0] == true && Check[1] == true ? true : false);
    }

    bool EXImageRecognition::CalculateEigenVectors() {
        cv::Mat Mean = S / (float)_NUMBER_OF_TRAINING_IMAGES;
        A = cv::Mat(_ImageVectorSize, _NUMBER_OF_TRAINING_IMAGES, CV_32FC1);

        for (int i = 0; i < _NUMBER_OF_TRAINING_IMAGES; i++) {
            A.col(i) = I.col(i) - Mean;
        }

        cv::Mat C = A.t() * A;
        eigen(C, L, V);

        return true;
    }

    bool EXImageRecognition::ComputeProjectionMatrix() {

        // compute projection matrix Ut
        cv::Mat U = A * V;
        Ut = U.t();

        // project the training set to the faces space
        Trainset = Ut * A;

        return true;
    }

    bool EXImageRecognition::PrepareForFaceRecognition() {

#ifdef __APPLE__
        std::string Path = BundlePath;
#else
        std::string Path;
#endif

        Path.append("data/haarcascade_frontalface_default.xml");

        HaarFrontFaceClassifierCascade = new cv::CascadeClassifier(Path);

#ifdef __APPLE__
        Path = BundlePath;
#else
        Path.clear();
#endif

        if (SMILE_RECOGNITION == true) {
            //Path.append("data/haarcascade_smile.xml");
            Path.append("data/smiled_01.xml");
            
            HaarSmileClassifierCascade = new cv::CascadeClassifier(Path);
        }

        

        if (HaarFrontFaceClassifierCascade != 0 && HaarFrontFaceClassifierCascade->empty() == false && (SMILE_RECOGNITION == true ? HaarSmileClassifierCascade != 0 && HaarSmileClassifierCascade->empty() == false : true)) {
            return this->ActivateVideoCapture();
        }

        return false;
    }

    bool EXImageRecognition::ActivateVideoCapture() {

    	if (_Device != CaptureDeviceRaspiCam) {
			if (VideoCapture != 0 && VideoCapture->isOpened()) {
				VideoCapture->release();
				delete VideoCapture;
				VideoCapture = 0;
			}

			VideoCapture = this->GetCaptureDevice();

			if (VideoCapture->isOpened()) {
	//#if !(TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR)
				VideoCapture->set(EX_CAP_PROP_FRAME_WIDTH, INPUT_SIZE.width);
				VideoCapture->set(EX_CAP_PROP_FRAME_HEIGHT, INPUT_SIZE.height);
                INPUT_SIZE.width = VideoCapture->get(EX_CAP_PROP_FRAME_WIDTH);
                INPUT_SIZE.height = VideoCapture->get(EX_CAP_PROP_FRAME_HEIGHT);
	//#endif

				return true;
			}
    	} else {
	#if __arm__ && _OPEN_CV_RASPI
    		VideoCaptureHandle = raspiCamCvCreateCameraCapture(EX_CAPTURE_ANY);
    		if (VideoCaptureHandle != 0) {
    			raspiCamCvSetCaptureProperty(VideoCaptureHandle, EX_CAP_PROP_FRAME_WIDTH, INPUT_SIZE.width);
    			raspiCamCvSetCaptureProperty(VideoCaptureHandle, EX_CAP_PROP_FRAME_HEIGHT, INPUT_SIZE.height);
    			return true;
    		}
	#endif
    	}

        return false;
    }

    bool EXImageRecognition::Recognition() {

    	bool OK = false;
    	if (_Device == CaptureDeviceRaspiCam) {
#if __arm__ && _OPEN_CV_RASPI
    		if (VideoCaptureHandle != 0) {
    			OK = true;
    		}
#endif
    	} else {
    		if (VideoCapture != 0 && VideoCapture->isOpened()) {
    			OK = true;
    		}
    	}

        if (OK && HaarFrontFaceClassifierCascade != 0 && HaarFrontFaceClassifierCascade->empty() == false) {

            // Get New Frame
            #ifdef __APPLE__
                cv::Mat SourceFrame = this->GetNewFrame();
            #else
                cv::Mat SourceFrame = this->GetNewFrame();
            #endif
            
            /*if (_Device == CaptureDeviceRaspiCam) {
#if __arm__ && _OPEN_CV_RASPI
            	SourceFrame = cv::cvarrToMat(raspiCamCvQueryFrame(VideoCaptureHandle));
#endif
            } else {
            	*VideoCapture >> SourceFrame;
            }*/


            // Reset Smiles
            _DetectedSmiles.clear();

            if (SourceFrame.empty() == false) {

                // Find a Smile
                std::vector<cv::Rect> Smiles;

                // Find a Face
                std::vector<cv::Rect> Objects;

                // Face Detected
                cv::Rect RangeOfInterest;

                // Color Scalars
                CvScalar FaceDetectionColor     = CV_RGB(0, 255, 0);
                CvScalar FaceAverageColor       = CV_RGB(0, 0, 255);

                // Mini Image Matrice
                cv::Mat SUBImage;
                cv::Mat GREYMat;

                // Grey Matrice create
                cv::cvtColor(SourceFrame, GREYMat, CV_BGR2GRAY);

                // Inverted Scale Factor
                //float InvScaleFactor = 1.0 + (1.0 - COMPUTING_SCALE_FACTOR);

                if (OUTPUT_GREY) {
                    SourceFrame = GREYMat;
                }
                
                
                // BRIGHTNESS TEST
                //SourceFrame.convertTo(SourceFrame, -1, 2, 50);

                
                if (OUTPUT_RESIZE) {
                    cv::resize(SourceFrame, SourceFrame, cv::Size(SourceFrame.size().width * COMPUTING_SCALE_FACTOR, SourceFrame.size().height * COMPUTING_SCALE_FACTOR));
                    cv::resize(GREYMat, GREYMat, cv::Size(GREYMat.size().width * COMPUTING_SCALE_FACTOR, GREYMat.size().height * COMPUTING_SCALE_FACTOR));
                } else {
                    if (COMPUTING_SCALE_FACTOR != 1.0) {
                        cv::resize(GREYMat, GREYMat, cv::Size(GREYMat.size().width * COMPUTING_SCALE_FACTOR, GREYMat.size().height * COMPUTING_SCALE_FACTOR));
                    }
                }

                // Equalize the Histogram
                cv::equalizeHist(GREYMat, GREYMat);
                
                
                // Create Face Recognition Settings
                
                double      Face_ScaleFactor    = 1.1;
                int         Face_MinNeighbors   = 3;
                int         Face_Flags          = 0;
                cv::Size    Face_MinSize        = cv::Size(30, 30);
                cv::Size    Face_MaxSize        = cv::Size();
                
                if (ENABLE_MANUAL_FACE_RECOGNITION_SETTINGS) {
                    Face_ScaleFactor = MANUAL_FACE_DETECTION_SETTINGS.ScaleFactor;
                    Face_MinNeighbors = MANUAL_FACE_DETECTION_SETTINGS.MinNeighbors;
                    Face_Flags = MANUAL_FACE_DETECTION_SETTINGS.Flags;
                    Face_MinSize = MANUAL_FACE_DETECTION_SETTINGS.MinSize;
                    Face_MaxSize = MANUAL_FACE_DETECTION_SETTINGS.MaxSize;
                } else if (ENABLE_AUTOMATIC_CALIBRATED_FACE_SETTINGS) {
                    Face_ScaleFactor    = 1.5;
                    Face_MinNeighbors   = 1;
                    Face_Flags          = 0 | EX_HAAR_SCALE_IMAGE | EX_HAAR_FIND_BIGGEST_OBJECT;
                    Face_MinSize        = cv::Size(30, 30);
                }

                // Detect Front of a Face
                HaarFrontFaceClassifierCascade->detectMultiScale(GREYMat, Objects, Face_ScaleFactor, Face_MinNeighbors, Face_Flags, Face_MinSize, Face_MaxSize);

                if (Objects.size() != 0) {

                    std::vector<cv::Rect>::iterator it = Objects.begin();
                    while (it != Objects.end()){
                        cv::Rect CurrentRect = *it;


                        // Paint Rects around all detected Faces
                        if (OUTPUT_DETECTED_FACES) {
                            rectangle(SourceFrame, cvPoint(this->GetUpScaledFactor(CurrentRect.x), this->GetUpScaledFactor(CurrentRect.y)), cvPoint(this->GetUpScaledFactor(CurrentRect.x) + this->GetUpScaledFactor(CurrentRect.width), this->GetUpScaledFactor(CurrentRect.y) + this->GetUpScaledFactor(CurrentRect.height)), FaceDetectionColor, 1);
                        }


                        // Set the Biggest Rect as ROI
                        if (RangeOfInterest.width * RangeOfInterest.height < CurrentRect.width * CurrentRect.height) {
                            RangeOfInterest = CurrentRect;
                        }

                        it++;
                    }
                    
                    // Smooth the Face ROI
                    RangeOfInterest = this->SmoothFaces(RangeOfInterest);
                    
                    if (OUTPUT_AVERAGE_FACES) {
                        // Paint Rects around the Average Face
                        rectangle(SourceFrame, cvPoint(this->GetUpScaledFactor(RangeOfInterest.x), this->GetUpScaledFactor(RangeOfInterest.y)), cvPoint(this->GetUpScaledFactor(RangeOfInterest.x) + this->GetUpScaledFactor(RangeOfInterest.width), this->GetUpScaledFactor(RangeOfInterest.y) + this->GetUpScaledFactor(RangeOfInterest.height)), FaceAverageColor, 1);
                    }

                    
                    if (SMILE_RECOGNITION == true) {

                        // Half of the Image
                        if (SMILE_SMALL_IMAGE) {
                            int ShrinkedRIO = cvRound(((float)RangeOfInterest.height / 100.0) * SMILE_SMALL_ROI_PERCENTAGE_HEIGHT);
                            RangeOfInterest.y = RangeOfInterest.y + (RangeOfInterest.height - ShrinkedRIO);
                            RangeOfInterest.height = ShrinkedRIO;
                        }
                        SUBImage = GREYMat(RangeOfInterest);


                        // Smile Detection

                        if (SUBImage.empty() == false) {

                            /*if (OUTPUT_DEBUG_IMAGES) {
                                std::string out2 = "output/out";
                                out2.append(std::to_string(IMAGE_OUT_ITERATOR_1));
                                out2.append("s.jpg");
                                imwrite(out2, SUBImage);
                            }*/

                            if (HaarSmileClassifierCascade != 0 && HaarSmileClassifierCascade->empty() == false) {
                                
                                double Smile_ScaleFactor    = 1.1;
                                int Smile_MinNeighbors      = 3;
                                int Smile_Flags             = 0;
                                cv::Size Smile_MinSize      = cv::Size();
                                cv::Size Smile_MaxSize      = cv::Size();
                                
                                float TwentySixPercent = (26.5 * (INPUT_SIZE.width / 100.0));
                                
                                if (ENABLE_MANUAL_SMILE_SETTINGS) {
                                    Smile_ScaleFactor = MANUAL_SMILE_DETECTION_SETTINGS.ScaleFactor;
                                    Smile_MinNeighbors = MANUAL_SMILE_DETECTION_SETTINGS.MinNeighbors;
                                    Smile_Flags = MANUAL_SMILE_DETECTION_SETTINGS.Flags;
                                    Smile_MinSize = MANUAL_SMILE_DETECTION_SETTINGS.MinSize;
                                    Smile_MaxSize = MANUAL_SMILE_DETECTION_SETTINGS.MaxSize;
                                } else if (ENABLE_AUTOMATIC_CALIBRATED_SMILE_SETTINGS) {
                                    Smile_ScaleFactor = 1.1;
                                    Smile_MinNeighbors = 0;
                                    Smile_Flags = 0 | EX_HAAR_SCALE_IMAGE;
                                    Smile_MinSize = cv::Size(SUBImage.size().width * 0.23333, SUBImage.size().width * 0.23333);
                                } else {
                                    if (RangeOfInterest.width >= TwentySixPercent) {
                                        Smile_ScaleFactor = 1.1;
                                        Smile_MinNeighbors = 28;
                                        Smile_Flags = 0 | EX_HAAR_SCALE_IMAGE;
                                        Smile_MinSize = cv::Size(0, 0);
                                    } else if (RangeOfInterest.width < TwentySixPercent) {
                                        Smile_ScaleFactor = 1.1;
                                        Smile_MinNeighbors = 0;
                                        Smile_Flags = 0 | EX_HAAR_SCALE_IMAGE;
                                        Smile_MinSize = cv::Size(10, 10);
                                    
                                    }
                                }

                                HaarSmileClassifierCascade->detectMultiScale(SUBImage, Smiles, Smile_ScaleFactor, Smile_MinNeighbors, Smile_Flags, Smile_MinSize, Smile_MaxSize);

                                if (OUTPUT_DETECTION_CYCLES) {
                                    if(Smiles.size() != 0) {
                                        std::vector<cv::Rect>::iterator it = Smiles.begin();
                                        while (it != Smiles.end()) {
                                            cv::Point center(this->GetUpScaledFactor(RangeOfInterest.x) + this->GetUpScaledFactor((*it).x) + this->GetUpScaledFactor((*it).width) * 0.5, this->GetUpScaledFactor(RangeOfInterest.y) + this->GetUpScaledFactor((*it).y) + this->GetUpScaledFactor((*it).height) * 0.5);
                                            int radius = cvRound( (this->GetUpScaledFactor((*it).width) + this->GetUpScaledFactor((*it).height)) * 0.25 );
                                            circle(SourceFrame, center, radius, cv::Scalar( 255, 0, 0 ), 1, 8, 0);

                                            it++;
                                        }
                                    }
                                }
                                
                                if (OUTPUT_DEBUG_IMAGES) {
                                    if (Smiles.size() != 0) {
                                        std::string out3 = "output/out";
                                        out3.append(std::to_string(IMAGE_OUT_ITERATOR_1));
                                        out3.append("sd.jpg");
                                        imwrite(out3, SUBImage);
                                    }
                                }

                                if (CALCULATE_SMILE_INTENSITY) {
                                    const int SmileNeighbors = (int)Smiles.size();
                                    static int SmileNeighborsMax = -1;
                                    static int SmileNeighborsMin = -1;
                                    if (SmileNeighborsMin == -1) {
                                        SmileNeighborsMin = SmileNeighbors;
                                    }
                                    SmileNeighborsMax = MAX(SmileNeighborsMax, SmileNeighbors);
                                    
                                    _CurrentSmileIntensity = ((float)SmileNeighbors - SmileNeighborsMin) / (SmileNeighborsMax - SmileNeighborsMin + 1);
                                }

                                if (OUTPUT_SMILE_INTENSITY) {
                                    int rect_height = cvRound((float)SourceFrame.rows * _CurrentSmileIntensity);
                                    CvScalar col = CV_RGB((float)255 * _CurrentSmileIntensity, 0, 0);
                                    rectangle(SourceFrame, cvPoint(0, SourceFrame.rows), cvPoint(2, SourceFrame.rows - rect_height), col, -1);
                                }

                            } // End Smile Detection On

                        } // End SubImage.empty()
                    }
                    
                    pthread_mutex_lock(&_mutex_average_lock);
                    if (_LastSMILEs.size() >= SMILE_AVERAGE_SIZE) {
                        _LastSMILEs.pop_front();
                    }
                    
                    _LastSMILEs.push_back(Smiles.size());
                    pthread_mutex_unlock(&_mutex_average_lock);

                } // End Objects.size()

                // Set Output Data
                pthread_mutex_lock(&_mutex_task_lock);

                _CurrentEmotion = (Objects.size() == 0 ? ImageRecognizationTypeNone : (this->GetSmileIntensity() == 0 ? ImageRecognizationTypeNeutral : ImageRecognizationTypeLaughing));
                _SUBImage.release();
                _SUBImage = SourceFrame;
                _DetectedSmiles = Smiles;
                _DetectedFaces = Objects;
                _ComputedFace = cv::Rect(this->GetUpScaledFactor(RangeOfInterest.x), this->GetUpScaledFactor(RangeOfInterest.y), this->GetUpScaledFactor(RangeOfInterest.width), this->GetUpScaledFactor(RangeOfInterest.height));

                pthread_mutex_unlock(&_mutex_task_lock);
                
                if (OUTPUT_DEBUG_IMAGES) {
                    // TEST
                    if (Smiles.size() != 0) {
                        std::string out = "output/out";
                        out.append(std::to_string(IMAGE_OUT_ITERATOR_1));
                        out.append(".jpg");
                        imwrite(out, SourceFrame);
                    }
                    IMAGE_OUT_ITERATOR_1++;
                }

                if (VideoWriter != 0 && VideoWriter->isOpened()) {
                    VideoWriter->write(SourceFrame);
                }
#ifdef __APPLE__
                // Call Callback Method for Obj-C
                if (Apple_Dispatch) {
                    DispatchBlock();
                }
#endif
#ifdef EX_EVENT_HANDLING
                SendEvent(new ImageRecognizeEvent(ImageRecognizationTypeNone, _DetectedSmiles, _DetectedFaces, _ComputedFace));
#endif
            }

        }

        return false;
    }

    bool EXImageRecognition::Cleanup() {

        bool Return = true;

        if (_Device == CaptureDeviceRaspiCam) {
#if __arm__ && _OPEN_CV_RASPI
        	if (VideoCaptureHandle != 0) {
        		raspiCamCvReleaseCapture(&VideoCaptureHandle);
        		Return |= true;
        	} else {
        		Return |= false;
        	}
#endif
        } else {
			if ((Return |= (VideoCapture->isOpened()))) {
				VideoCapture->release();
				delete VideoCapture;
				VideoCapture = 0;
			}
        }

        if ((Return |= (HaarFrontFaceClassifierCascade != 0))) {
            delete HaarFrontFaceClassifierCascade;
            HaarFrontFaceClassifierCascade = 0;
        }

        if ((Return |= (HaarSmileClassifierCascade != 0))) {
            delete HaarSmileClassifierCascade;
            HaarSmileClassifierCascade = 0;
        }

        _Initialized = false;

        return Return;
    }
    
    
    
    // Private Setter
    void EXImageRecognition::SetCurrentFPS(unsigned int FPS) {
        FRAMERATE_CURRENT = FPS;
    }
    
    void EXImageRecognition::SetRealFPS(unsigned int FPS) {
        FRAMERATE_REAL_CURRENT = FPS;
    }
    
    void EXImageRecognition::SetNewFrame(cv::Mat SourceFrame) {
        pthread_mutex_lock(&_mutex_video_lock);
        _CurrentFrame = SourceFrame;
        pthread_mutex_unlock(&_mutex_video_lock);
    }
    
    cv::Mat& EXImageRecognition::GetNewFrame() {
        pthread_mutex_lock(&_mutex_video_lock);
        cv::Mat &tmp =  _CurrentFrame;
        pthread_mutex_unlock(&_mutex_video_lock);
        return tmp;
    }


    // Helper

    cv::VideoCapture* EXImageRecognition::GetCaptureDevice() {
#if (TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR)
        if (FrontCamera == false) {
            return new cv::VideoCapture(0);
        } else {
            return new cv::VideoCapture(1);
        }
#else
        return new cv::VideoCapture(EX_CAPTURE_ANY);
#endif
    }
    
    long long EXImageRecognition::GetMillisecondTime() {
        struct timeval  te;
        gettimeofday(&te, 0);
        
        return te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds
    }
    
    cv::Rect EXImageRecognition::SmoothFaces(cv::Rect ROI) {
        cv::Rect Average_Face = this->GetAveragePositionOfBiggestFace();
        
        pthread_mutex_lock(&_mutex_average_lock);
        if (this->OutOfRectOffset(Average_Face, ROI, ROI_RESET_OFFSET_VALUE)) {
            _LastROIs.clear();
        }
        
        
        if (_LastROIs.size() >= ROI_AVERAGE_SIZE) {
            _LastROIs.pop_front();
        }
        
        _LastROIs.push_back(ROI);
        pthread_mutex_unlock(&_mutex_average_lock);
        
        return this->GetAveragePositionOfBiggestFace();
    }
    
    bool EXImageRecognition::OutOfRectOffset(cv::Rect Reference, cv::Rect ObjectOfInterest, int OffsetValue) {
        cv::Point A1 = cv::Point(Reference.x, Reference.y);
        cv::Point A2 = cv::Point(Reference.x + Reference.width, Reference.y + Reference.height);
        cv::Point B1 = cv::Point(ObjectOfInterest.x, ObjectOfInterest.y);
        cv::Point B2 = cv::Point(ObjectOfInterest.x + ObjectOfInterest.width, ObjectOfInterest.y + ObjectOfInterest.height);
        if (OFFSET_CHECK(A1.x, B1.x, OffsetValue)) {
            return true;
        }
        if (OFFSET_CHECK(A1.y, B1.y, OffsetValue)) {
            return true;
        }
        if (OFFSET_CHECK(A2.x, B2.x, OffsetValue)) {
            return true;
        }
        if (OFFSET_CHECK(A2.y, B2.y, OffsetValue)) {
            return true;
        }
        return false;
    }
    
    float EXImageRecognition::GetSmileIntensity() {
        return (SMILE_USING_MIDDLE ? this->GetMiddleSmileIntensity() : this->GetAverageSmileIntensity());
    }

} /* namespace ImageRecognition */

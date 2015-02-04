/*
 * 		EXImageRecognition.h
 *
 *  	Created on: 29.08.2014
 *      Author: 	benjamin wolf
 *      Company:	denkwerk GmbH - 2015
 */

#ifndef EXIMAGERECOGNITION_H_
#define EXIMAGERECOGNITION_H_

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/objdetect/objdetect.hpp"

#include <pthread.h>
#include <signal.h>
#include <termios.h>

#include <deque>

#ifdef EX_EVENT_HANDLING
#include "../Event/EventHandler.h"
#endif

#if __arm__ && _OPEN_CV_RASPI
#include <RaspiCamCV.h>
#endif

#ifdef __APPLE__
#include <dispatch/dispatch.h>
#endif

namespace ImageRecognition {
    
#ifdef __APPLE__
    #define EX_CAPTURE_ANY CV_CAP_ANY
    #define EX_HAAR_DO_CANNY_PRUNING CV_HAAR_DO_CANNY_PRUNING
    #define EX_HAAR_SCALE_IMAGE CV_HAAR_SCALE_IMAGE
    #define EX_HAAR_FIND_BIGGEST_OBJECT CV_HAAR_FIND_BIGGEST_OBJECT
    #define EX_HAAR_DO_ROUGH_SEARCH CV_HAAR_DO_ROUGH_SEARCH
    #define EX_CAP_PROP_FRAME_WIDTH CV_CAP_PROP_FRAME_WIDTH
    #define EX_CAP_PROP_FRAME_HEIGHT CV_CAP_PROP_FRAME_HEIGHT
#else
    #define EX_CAPTURE_ANY cv::CAP_ANY
    #define EX_HAAR_DO_CANNY_PRUNING cv::CASCADE_DO_CANNY_PRUNING
    #define EX_HAAR_SCALE_IMAGE cv::CASCADE_SCALE_IMAGE
    #define EX_HAAR_FIND_BIGGEST_OBJECT cv::CASCADE_FIND_BIGGEST_OBJECT
    #define EX_HAAR_DO_ROUGH_SEARCH cv::CASCADE_DO_ROUGH_SEARCH
    #define EX_CAP_PROP_FRAME_WIDTH cv::CAP_PROP_FRAME_WIDTH
    #define EX_CAP_PROP_FRAME_HEIGHT cv::CAP_PROP_FRAME_HEIGHT
#endif

    // Number of Training Images
    #define _NUMBER_OF_TRAINING_IMAGES 2

	enum CaptureDevice {
		CaptureDeviceNone,
		CaptureDeviceUSBCamera,
		CaptureDeviceIPhoneCamera,
		CaptureDeviceRaspiCam
	};

	enum ThreadWorkingType {
		ThreadWorkingTypeRecognizeFailed = -1,
		ThreadWorkingTypeNone = 0,
		ThreadWorkingTypeInit = 1,
		ThreadWorkingTypeStartRecognize,
		ThreadWorkingTypeRecognize,
		ThreadWorkingTypeUnInitialize,
		ThreadWorkingTypeUnInitialized,
		ThreadWorkingTypeCalibrate,
	};

	enum ImageRecognizationType {
		ImageRecognizationTypeNone = -1,
        ImageRecognizationTypeNeutral = 0,
		ImageRecognizationTypeLaughing,
		ImageRecognizationTypeSad
	};
    
    struct DetectionSettings {
        
        public:
            DetectionSettings(double SF, int MN, int FL, cv::Size MinS = cv::Size(), cv::Size MaxS = cv::Size()) : ScaleFactor(SF), MinNeighbors(MN), Flags(FL), MinSize(MinS), MaxSize(MaxS) {};
            DetectionSettings() : ScaleFactor(1.1), MinNeighbors(3), Flags(0), MinSize(cv::Size()), MaxSize(cv::Size()){};
            DetectionSettings(const DetectionSettings &m) : ScaleFactor(m.ScaleFactor), MinNeighbors(m.MinNeighbors), Flags(m.Flags), MinSize(m.MinSize), MaxSize(m.MaxSize) {};
            ~DetectionSettings() {};
        
            double ScaleFactor;
            int MinNeighbors;
            int Flags;
            cv::Size MinSize;
            cv::Size MaxSize;
        
    };

    #ifdef EX_EVENT_HANDLING
    class ImageEvent : public Event {
        public:
            ImageEvent(ThreadWorkingType Type) : Type(Type) {};

            ThreadWorkingType Type;
    };

    class ImageRecognizeEvent : public ImageEvent {
        public:
            ImageRecognizeEvent(ImageRecognizationType Type) : ImageEvent(ThreadWorkingTypeRecognize), Type(Type) {};
            ImageRecognizeEvent(ImageRecognizationType Type, std::vector<cv::Rect> DS, std::vector<cv::Rect> DF, cv::Rect CF) : ImageEvent(ThreadWorkingTypeRecognize), Type(Type) {
            	DetectedSmiles = DS;
            	DetectedFaces = DF;
            	ComputedFace = CF;
            };

            ImageRecognizationType	Type;
            std::vector<cv::Rect>	DetectedSmiles;
            std::vector<cv::Rect>	DetectedFaces;
            cv::Rect				ComputedFace;
    };

    class EXImageRecognition : public EventHandler {
    #else
    class EXImageRecognition {
    #endif

        public:

            // Class Initialization
            EXImageRecognition();
            virtual ~EXImageRecognition();

            // Public Methods
            bool Initialize(CaptureDevice Device = CaptureDeviceUSBCamera);
            bool StartRecognition();
            bool StopRecognition();
            void LockImageBuffer();
            void UnlockImageBuffer();
            bool EnableVideoOutput(std::string Filename);
            cv::Mat& GetImage(bool Mini = false);
            std::vector<cv::Rect> GetSmiles();
            std::vector<cv::Rect> GetFaces();
            cv::Rect GetComputedFace();
            void SetShowDetectedSmiles(bool Show);
            bool ShowDetectedSmiles();
            void SetShowDetectedFaces(bool Show);
            bool ShowDetectedFaces();
            void SetShowGreyOutput(bool Show);
            bool ShowGreyOutput();
            void SetEnableSmileDetection(bool Enable);
            bool SmileDetectionEnabled();
            void SetResizeOutput(bool Resize);
            bool ResizeOutput();
            void SetScaleFactor(float Scale);
            float ScaleFactor();
            void EnableUsingSmileMiddleSmoothing(bool Using);
            bool UsingSmileMiddleSmoothing();
            void EnableManualSmileDetectionSettings(bool Enable);
            bool ManualSmileDetectionSettings();
            void SetManualSmileDetectionSettings(DetectionSettings Settings);
            DetectionSettings GetManualSmileDetectionSettings();
            void EnableManualFaceDetectionSettings(bool Enable);
            bool ManualFaceDetectionSettings();
            void SetManualFaceDetectionSettings(DetectionSettings Settings);
            void SetFaceROIResetValue (float Offset);
            float GetFaceROIResetValue ();
            DetectionSettings GetManualFaceDetectionSettings();
            void SetInputSize(cv::Size Size);
            cv::Size GetInputSize();
            void SetShowSmileIntensity(bool Show);
            bool ShowSmileIntensity();
            void SetFramerate(unsigned int FPS);
            unsigned long long GetFramerate();
            unsigned int GetCurrentFramerate();
            void EnableRecognizeMiniImage(bool Enable);
            bool RecognizeMiniImage();
            void SetRecognizeImagePercentage(unsigned int Percent);
            unsigned int RecognizeImagePercentage();
            void SetROIAverageSize(unsigned int ROISize);
            unsigned int ROIAverageSize();
            void SetSmoothFaces(bool Smooth);
            bool SmoothFaces();
            cv::Rect GetAveragePositionOfBiggestFace();
            cv::Rect GetMiddlePositionOfBiggestFace();
            void SetSMILEAverageSize(unsigned int SMILESize);
            unsigned int SMILEAverageSize();
            float GetAverageSmileIntensity();
            float GetMiddleSmileIntensity();
            ImageRecognizationType GetCurrentEmotion();
            static cv::Rect ConvertToSize(cv::Mat Matrice, cv::Size Size, cv::Rect Rect);
            static cv::Rect ConvertToSize(cv::Size SourceSize, cv::Size Size, cv::Rect Rect);
            static cv::Rect ConvertToSize(cv::Size Size, cv::Rect Rect);

    #ifdef __APPLE__
            void SetBundlePath(std::string Path);
            void SetFrontCamera(bool Front);
            bool IsSetFrontCamera();
            void SetVideoBlock(dispatch_block_t Block);
            bool StartRecognition (dispatch_block_t Block);
            static cv::Mat ConvertToView (cv::Mat &Input);
            static cv::Mat ConvertColorRange (cv::Mat& Input);
    #endif

        private:

    #ifdef __APPLE__
            // Apple Methods
            std::string                 BundlePath;
            bool                        Apple_Dispatch;
            bool                        Apple_Video_Dispatch;
            dispatch_block_t            DispatchBlock;
            dispatch_block_t            ImageBlock;
    #endif
            bool                        FrontCamera;
            float                       COMPUTING_SCALE_FACTOR;
            bool                        OUTPUT_GREY;
            bool                        SMILE_RECOGNITION;
            bool                        OUTPUT_RESIZE;
            bool                        OUTPUT_DETECTION_CYCLES;
            bool                        OUTPUT_DETECTED_FACES;
            bool                        OUTPUT_SMILE_INTENSITY;
            bool                        OUTPUT_AVERAGE_FACES;
            bool                        CALCULATE_SMILE_INTENSITY;
            bool                        SMILE_SMALL_IMAGE;
            bool                        SMILE_USING_MIDDLE;
            bool                        ENABLE_MANUAL_FACE_RECOGNITION_SETTINGS;
            bool                        ENABLE_AUTOMATIC_CALIBRATED_FACE_SETTINGS;
            bool                        ENABLE_MANUAL_SMILE_SETTINGS;
            bool                        ENABLE_AUTOMATIC_CALIBRATED_SMILE_SETTINGS;
            bool                        OUTPUT_DEBUG_IMAGES;
            cv::Size                    INPUT_SIZE;
            unsigned int                FRAMERATE;
            volatile unsigned int       FRAMERATE_CURRENT;
            volatile unsigned int       FRAMERATE_REAL_CURRENT;
            unsigned int                ROI_AVERAGE_SIZE;
            unsigned int                ROI_RESET_OFFSET_VALUE;
            unsigned int                SMILE_AVERAGE_SIZE;
            unsigned int                SMILE_RESET_OFFSET_VALUE;
            float                       SMILE_SMALL_ROI_PERCENTAGE_HEIGHT;
            DetectionSettings           MANUAL_SMILE_DETECTION_SETTINGS;
            DetectionSettings           MANUAL_FACE_DETECTION_SETTINGS;
            int							IMAGE_OUT_ITERATOR_1;

            // Thread
            pthread_t					_thread;
            pthread_t                   _video_thread;
            pthread_mutex_t				_mutex_task_lock;
            pthread_mutex_t				_mutex_average_lock;
            pthread_mutex_t				_mutex_video_lock;
            volatile int				_exit;
            volatile int                _videoexit;
            struct termios				oldtio;

            // OPENCV
            cv::Mat						_HappyImage;
            cv::Mat						_SadImage;
            int							_ImageVectorSize;

            cv::Mat                     _SUBImage;
            cv::Mat                     _SUBMiniImage;

            cv::Mat						I;
            cv::Mat						S;

            cv::Mat						A;
            cv::Mat						V;
            cv::Mat						L;

            cv::Mat						Ut;
            cv::Mat						Trainset;
        
            cv::Mat                     _CurrentFrame;

            cv::CascadeClassifier		*HaarFrontFaceClassifierCascade;
            cv::CascadeClassifier		*HaarSmileClassifierCascade;
            cv::VideoCapture			*VideoCapture;
#if __arm__ && _OPEN_CV_RASPI
            RaspiCamCvCapture			*VideoCaptureHandle;
#endif
            cv::VideoWriter             *VideoWriter;

            // General
            int							_HappyCounter;
            int							_SadCounter;
            std::vector<cv::Rect>       _DetectedSmiles;
            std::vector<cv::Rect>       _DetectedFaces;
            cv::Rect                    _ComputedFace;
            bool						_Initialized;
            std::deque<cv::Rect>        _LastROIs;
            std::deque<size_t>          _LastSMILEs;
            volatile float              _CurrentSmileIntensity;
            ImageRecognizationType      _CurrentEmotion;
            CaptureDevice				_Device;

            // Threading
            bool StartKeyThread ();
            bool StopKeyThread ();
            static void *RunThread (void *self);
        
            bool StartVideoInputThread ();
            bool StopVideoInputThread ();
            static void *RunVideoThread (void *self);

            // Logic
            bool InitializeLibrary(CaptureDevice Device = CaptureDeviceUSBCamera);
            bool CalculateImageVectorSize();
            bool ReadTrainingImages();
            bool CalculateEigenVectors();
            bool ComputeProjectionMatrix();
            bool PrepareForFaceRecognition();
            bool ActivateVideoCapture();

            bool Recognition();

            bool Cleanup();
        
            // Private Setter
            void SetCurrentFPS(unsigned int FPS);
            void SetRealFPS(unsigned int FPS);
            void SetNewFrame(cv::Mat SourceFrame);
            cv::Mat& GetNewFrame();

            // Helper
            cv::VideoCapture* GetCaptureDevice();
            long long GetMillisecondTime();
            cv::Rect SmoothFaces(cv::Rect ROI);
            bool OutOfRectOffset(cv::Rect Reference, cv::Rect ObjectOfInterest, int OffsetValue);
            float GetSmileIntensity();
        
            template<typename T>
            T GetUpScaledFactor(T Value) {
                return (T)(OUTPUT_RESIZE ? Value : Value / COMPUTING_SCALE_FACTOR);
            };
	};

} /* nition */

#endif /* EXIMAGERECOGNITION_H_ */

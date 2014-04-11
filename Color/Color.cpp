#include "stdafx.h"
#include <Windows.h>
#include <NuiApi.h>
#include <opencv2\opencv.hpp>

using namespace cv;

int _tmain(int argc, _TCHAR* argv[])
{
	cv::setUseOptimized( true );

	INuiSensor* pSensor;
	HRESULT hResult = S_OK;
	hResult = NuiCreateSensorByIndex( 0, &pSensor );
	if( FAILED( hResult ) ){
		std::cerr << "Error : NuiCreateSensorByIndex" << std::endl;
		return -1;
	}

	hResult = pSensor->NuiInitialize( NUI_INITIALIZE_FLAG_USES_COLOR );
	if( FAILED( hResult ) ){
		std::cerr << "Error : NuiInitialize" << std::endl;
		return -1;
	}

	HANDLE hColorEvent = INVALID_HANDLE_VALUE;
	HANDLE hColorHandle = INVALID_HANDLE_VALUE;
	hColorEvent = CreateEvent( nullptr, true, false, nullptr );
	hResult = pSensor->NuiImageStreamOpen( NUI_IMAGE_TYPE_COLOR, NUI_IMAGE_RESOLUTION_640x480, 0, 2, hColorEvent, &hColorHandle );
	if( FAILED( hResult ) ){
		std::cerr << "Error : NuiImageStreamOpen( COLOR )" << std::endl;
		return -1;
	}

	INuiColorCameraSettings* pCameraSettings;
	hResult = pSensor->NuiGetColorCameraSettings( &pCameraSettings );
	if( FAILED( hResult ) ){
		std::cerr << "Error : NuiGetColorCameraSettings" << std::endl;
		return -1;
	}

	unsigned long refWidth = 0;
	unsigned long refHeight = 0;
	NuiImageResolutionToSize( NUI_IMAGE_RESOLUTION_640x480, refWidth, refHeight );
	int width = static_cast<int>( refWidth );
	int height = static_cast<int>( refHeight );

	HANDLE hEvents[1] = { hColorEvent };

	cv::namedWindow( "Color" );

	while( 1 ){

		ResetEvent( hColorEvent );
		WaitForMultipleObjects( ARRAYSIZE( hEvents ), hEvents, true, INFINITE );

		NUI_IMAGE_FRAME colorImageFrame = { 0 };
		hResult = pSensor->NuiImageStreamGetNextFrame( hColorHandle, 0, &colorImageFrame );
		if( FAILED( hResult ) ){
			std::cerr << "Error : NuiImageStreamGetNextFrame( COLOR )" << std::endl;
			return -1;
		}

		INuiFrameTexture* pColorFrameTexture = colorImageFrame.pFrameTexture;
		NUI_LOCKED_RECT colorLockedRect;
		pColorFrameTexture->LockRect( 0, &colorLockedRect, nullptr, 0 );

		cv::Mat colorMat( height, width, CV_8UC4, reinterpret_cast<unsigned char*>( colorLockedRect.pBits ) );
		cv::imshow( "Color", colorMat );
		
		pColorFrameTexture->UnlockRect( 0 );
		pSensor->NuiImageStreamReleaseFrame( hColorHandle, &colorImageFrame );

		int key = cv::waitKey( 30 );
		if( key == VK_ESCAPE ){
			break;
		}

	}
	
	pSensor->NuiShutdown();
	pCameraSettings->Release();
	CloseHandle( hColorEvent );

	cv::destroyAllWindows();

	return 0;
}

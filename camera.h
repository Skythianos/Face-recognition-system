#ifndef CAMERA_H
#define CAMERA_H
#include "uEye.h"
class camera{
private:
	HICON	m_hIcon;

	// uEye varibles
	HIDS	hCam;			// handle to camera
	HWND	m_hWndDisplay;	// handle to diplay window
	INT		nRet;			// return value of uEye SDK functions
	INT		m_nColorMode;	// Y8/RGB16/RGB24/REG32
	INT		m_nBitsPerPixel;// number of bits needed store one pixel
	INT		m_nSizeX;		// width of video 
	INT		m_nSizeY;		// height of video
	INT		m_lVideoId;	// grabber memory - buffer ID for videos
	INT		m_lPictureId; // buffer ID for pictures
	char*	m_pcImageMemory;// grabber memory - pointer to buffer
	char*	m_pcVideoMemory;
    INT     m_nRenderMode; // render  mode
	INT		m_nNumberOfFaces;
    SENSORINFO m_sInfo;	    // sensor information struct
public:
	camera();
	void InitCamera ();
	void Acquire_picture();
	void Save_Image(wchar_t* filePath);
	void Close_Camera();
	int CheckReturnValue();
	void DisplayVideo(HWND hWnd);
	void CaptureVideo();
	void EnableFaceDetection(int mode);
	void StopVideo();
	void SetSaturation(int value);
	void SetRGB(int Master,int RED,int GREEN, int BLUE);
	void DefaultValues();
	void Zoom(double* zoomfactor);
	void Mirror(int mode,int onoff);
	int GetFacePos(int *x,int *y, int *height,int* width);
};
#endif
#include "camera.h"


 
camera::camera(){
	hCam=NULL;
	m_hWndDisplay=NULL;	// handle to diplay window
	nRet=0;			// return value of uEye SDK functions
		// Y8/RGB16/RGB24/REG32
	m_nBitsPerPixel=24;// number of bits needed store one pixel
	m_lVideoId=0;	// grabber memory - buffer ID for videos
	m_lPictureId=0;
	m_pcImageMemory=NULL;// grabber memory - pointer to buffer
	m_pcVideoMemory=NULL;
    m_nRenderMode=0; // render  mode   
	m_nNumberOfFaces=0;
}


void camera::InitCamera()
{
    //Open camera with ID 1

 hCam = 1;

 nRet = is_InitCamera (&hCam, NULL);

 

if (nRet != IS_SUCCESS)

{

 //Check if GigE uEye SE needs a new starter firmware

 if (nRet == IS_STARTER_FW_UPLOAD_NEEDED)

 {

   //Calculate time needed for updating the starter firmware

   INT nTime;

  is_GetDuration (hCam, IS_SE_STARTER_FW_UPLOAD, &nTime);


 

   //Upload new starter firmware during initialization

   hCam =  hCam | IS_ALLOW_STARTER_FW_UPLOAD;

   nRet = is_InitCamera (&hCam, NULL);
	
 }

}
is_SetDisplayMode (hCam,IS_SET_DM_DIB);
is_ResetToDefault(hCam);
is_GetSensorInfo(hCam,&m_sInfo);
is_AllocImageMem (hCam,1280,720,m_nBitsPerPixel,&m_pcImageMemory,&m_lPictureId);
is_AllocImageMem (hCam,1280,720,m_nBitsPerPixel,&m_pcVideoMemory,&m_lVideoId);
is_SetImageMem (hCam,m_pcVideoMemory,m_lVideoId);

 

//Bitmap mode (images are digitized and stored in system memory):

//IS_SET_DM_DIB

 

//Direct3D mode

//Mode = IS_SET_DM_DIRECT3D
}

void camera::Acquire_picture(){
	nRet=is_FreezeVideo (hCam, IS_DONT_WAIT);
	is_CopyImageMem(hCam,m_pcVideoMemory,m_lVideoId,m_pcImageMemory);
}

void camera::Save_Image(wchar_t* filePath){
//Setting parameters for the file to save.
unsigned int m_lPictureId2=m_lPictureId;
IMAGE_FILE_PARAMS ImageFileParams;
ImageFileParams.pwchFileName = filePath;
ImageFileParams.pnImageID = & m_lPictureId2;
ImageFileParams.ppcImageMem = &m_pcImageMemory;
ImageFileParams.nQuality = 0;
ImageFileParams.nFileType = IS_IMG_BMP;
//Saving the file without browser dialog
nRet = is_ImageFile(hCam, IS_IMAGE_FILE_CMD_SAVE, (void*)&ImageFileParams,
                   sizeof(ImageFileParams));
}

void camera::Close_Camera(){
	is_ExitCamera (hCam);
}

int camera::CheckReturnValue(){
	return nRet;
}
void camera::DisplayVideo(HWND hWnd){
	is_RenderBitmap (hCam, m_lVideoId, hWnd, IS_RENDER_FIT_TO_WINDOW);
}

void camera::CaptureVideo(){
	is_CaptureVideo (hCam,IS_DONT_WAIT);
}

void camera::EnableFaceDetection(int mode){
	if(mode){
	is_FaceDetection(hCam,FDT_CMD_SET_INFO_MAX_NUM_OVL,&m_nNumberOfFaces,4);
	is_FaceDetection(hCam,FDT_CMD_SET_ENABLE,NULL,0);
	}
	else is_FaceDetection(hCam,FDT_CMD_SET_DISABLE,NULL,0);
}

void camera::StopVideo(){
	is_StopLiveVideo (hCam,IS_DONT_WAIT);
}

void camera::SetSaturation(int value){
	is_Saturation(hCam,SATURATION_CMD_SET_VALUE,&value,4);
}

void camera::DefaultValues(){
	is_ResetToDefault(hCam);
}
void camera::SetRGB(int Master,int RED,int GREEN, int BLUE){
	if(is_SetGainBoost (hCam,IS_GET_GAINBOOST)==IS_SET_GAINBOOST_OFF) is_SetGainBoost(hCam,IS_SET_GAINBOOST_ON);
	nRet=is_SetHardwareGain(hCam,Master,RED,GREEN,BLUE);
}

void camera::Zoom(double* zoomfactor){
	is_Zoom(hCam,ZOOM_CMD_DIGITAL_SET_VALUE,zoomfactor,8);
}

void camera::Mirror(int mode,int onoff){
	is_SetRopEffect(hCam,mode,1,0);
}

int camera::GetFacePos(int *x,int *y, int *height,int *width){
	unsigned int Status;
	FDT_INFO_LIST data;
	data.nSizeOfListEntry=sizeof(FDT_INFO_EL);
	data.nNumListElements=1;
	is_FaceDetection(hCam,FDT_CMD_GET_ENABLE,&Status,4);
	if(Status==0) return 0; // face detection is disabled
	else{
		is_FaceDetection(hCam,FDT_CMD_SET_SUSPEND,NULL,0);
		is_FaceDetection(hCam,FDT_CMD_GET_FACE_LIST,&data,sizeof(FDT_INFO_LIST));
		is_FaceDetection(hCam,FDT_CMD_SET_RESUME,NULL,0);
		if(data.nNumDetectedFaces==0) return -1; //No detected faces
		else{
			*height=data.FaceEntry->nFaceHeight*3;
			*width=data.FaceEntry->nFaceHeight*4;
			*x=data.FaceEntry->nFacePosX*4-(*width)/2;
			*y=data.FaceEntry->nFacePosY*3-(*height)/2;
		}
	}
	return 1; // ok
}
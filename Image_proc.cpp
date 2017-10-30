#include "Image_proc.h"

ImgProc::ImgProc(){
	cascade_e=0;
	cascade_mouth=0;
}
IplImage* ImgProc::CutPicture(char* filename,int x,int y, int height, int width){
	//variable declarations
	IplImage* pImg=0;
	CvSize Size;
	CvRect FaceRect;
	FaceRect.height=height*1.5; //Modified
	FaceRect.width=width;	//Modified
	FaceRect.x=x;
	FaceRect.y=y;
	//Resizing Image
	pImg=cvLoadImage(filename,CV_LOAD_IMAGE_GRAYSCALE);
	cvSetImageROI(pImg, FaceRect);
	Size.height=300;
	Size.width=300;
	IplImage * pFaceImg =cvCreateImage(Size , IPL_DEPTH_8U, 1 ); 
	cvResize(pImg, pFaceImg, CV_INTER_AREA );
	
	cvNamedWindow("Input",1);
	cvShowImage("Input",pFaceImg);

	return pFaceImg;
}

int ImgProc::EyeDetect(IplImage* pImage,IplImage** result,int x, int y, int height,int width){
CvRect* mouth_rect=0;
CvRect *eye=0;
IplImage* Image = cvCloneImage(pImage);
*result=0;
CvMemStorage* storage = cvCreateMemStorage(0);
 if(cascade_e==0){
	 cascade_e = (CvHaarClassifierCascade*)cvLoad("haarcascade_mcs_eyepair_big.xml", 0, 0, 0);
 }
 if(cascade_mouth==0){
	 cascade_mouth=(CvHaarClassifierCascade*)cvLoad("haarcascade_mcs_mouth.xml", 0, 0, 0);
 }
 
 // Set area of interest to the upper region of face
 // Eyes
 cvSetImageROI(Image,cvRect(x,y,width,height*0.8)); 

  CvSeq *eyes = cvHaarDetectObjects(
        Image,            /* the source image, with the
                           estimated location defined */
        cascade_e,      /* the eye classifier */
        storage,        /* memory buffer */
        1.15, 2, 0,     
        cvSize(25, 30)  /* minimum detection scale */
    );

  int i;
   
    /* draw a rectangle for each detected eye */
    for( i = 0; i < (eyes ? eyes->total : 0); i++ ) {
        /* get one eye */
        eye = (CvRect*)cvGetSeqElem(eyes, i);
       
        /* draw a red rectangle */
        cvRectangle(
            Image,
            cvPoint(eye->x, eye->y),
            cvPoint(eye->x + eye->width, eye->y + eye->height),
            CV_RGB(255, 0, 0),
            5, 8, 0
        );
    }

	cvNamedWindow("eyes",1);	//Experimental
	cvShowImage("eyes",Image);	//Experimental
	eye=(CvRect*)cvGetSeqElem(eyes, 0);

	if(eye==0){
		cascade_e = (CvHaarClassifierCascade*)cvLoad("haarcascade_mcs_eyepair_small.xml", 0, 0, 0);
	cvSetImageROI(Image,cvRect(x,y,width,height*0.8)); 
    eyes = cvHaarDetectObjects(
        Image,            /* the source image, with the
                           estimated location defined */
        cascade_e,      /* the eye classifier */
        storage,        /* memory buffer */
        1.15, 2, 0,     /* tune for your app */
        cvSize(25, 30)  /* minimum detection scale */
    );

 
   
    /* draw a rectangle for each detected eye */
    for( i = 0; i < (eyes ? eyes->total : 0); i++ ) {
        /* get one eye */
        eye = (CvRect*)cvGetSeqElem(eyes, i);
       
        /* draw a red rectangle */
        cvRectangle(
            Image,
            cvPoint(eye->x, eye->y),
            cvPoint(eye->x + eye->width, eye->y + eye->height),
            CV_RGB(255, 0, 0),
            5, 8, 0
        );
    }
	cvNamedWindow("eyes",1);	//Experimental
	cvShowImage("eyes",Image);	//Experimental

	eye=(CvRect*)cvGetSeqElem(eyes, 0);
	cvResetImageROI(Image);
	cvResetImageROI(pImage);
	cascade_e=0;
	}

	
	//mouth
	storage = cvCreateMemStorage(0);
	cvResetImageROI(Image);
	eye=(CvRect*)cvGetSeqElem(eyes, 0);
	CvSeq *mouth=0;
	if(eye){
	int mouthROI_y=y+eye->y+eye->height*2;
	cvSetImageROI(Image,cvRect(x,mouthROI_y,width,height*0.3));

	mouth = cvHaarDetectObjects(
        Image,            /* the source image, with the
                           estimated location defined */
        cascade_mouth,      /* the eye classifier */
        storage,        /* memory buffer */
        1.15, 2, 0,     /* tune for your app */
        cvSize(60, 40)  /* minimum detection scale */
    );
    /* draw a rectangle for each detected mouth */
    for( i = 0; i < (mouth ? mouth->total : 0); i++ ) {
        /* get the mouth */
        mouth_rect = (CvRect*)cvGetSeqElem(mouth, i);
       
        /* draw a red rectangle */
        cvRectangle(
            Image,
            cvPoint(mouth_rect->x, mouth_rect->y),
            cvPoint(mouth_rect->x + mouth_rect->width, mouth_rect->y + mouth_rect->height),
            CV_RGB(255, 0, 0),
            5, 8, 0
        );
    }
	cvNamedWindow("mouth",1);	//Experimental
	cvShowImage("mouth",Image);	//Experimental
	cvResetImageROI(Image);
	// Repositioning of face related to mouth and eye locations
	cvResetImageROI(pImage);
	mouth_rect=(CvRect*)cvGetSeqElem(mouth, 0);

	CvRect Face;
	if(eye&&mouth_rect){
	*result=cvCreateImage(cvSize(184,224) , IPL_DEPTH_8U, 1 );
	Face.width=eye->width*1.1;
	Face.height=((mouth_rect->y+mouthROI_y)-(eye->y))*2;
	Face.x=eye->x-width*0.015;
	Face.y=eye->y-height*0.15;
	cvSetImageROI(pImage,Face);
	cvResize(pImage,*result,CV_INTER_AREA);
	}
	}
	if(!(eye&&mouth_rect)){
		cvResetImageROI(Image);
		cvSaveImage("Sample.bmp",Image);
		return 0;
	}
	return 1;
}

IplImage* ImgProc::Brightness(IplImage* Image,int brightness,int Show,int half_face){
	IplImage* result=cvCloneImage(Image);
	cv::Mat Temp=(cv::Mat)Image;
	cv::Mat Temp2;
	if(half_face==0){
	Temp=(brightness/100.0)*Temp;
	}
	else{
		transpose(Temp,Temp);
		// Copying one half of the face
		for(int i=(Temp.rows/2)-1;i<Temp.rows;i++){
			Temp2.push_back(Temp.row(i));
		}
		for(i=0;i<Temp2.rows;i++) Temp.pop_back();
		// Setting brightness on the copied half
		Temp2=(brightness/100.0)*Temp2;
		// Restoring full face
		for(i=0;i<Temp2.rows;i++) Temp.push_back(Temp2.row(i));
		transpose(Temp,Temp);
	}
	result[0]=(IplImage)Temp;
	if(Show){
	cvNamedWindow("Brightness",1);
	cvShowImage("Brightness",result);
	}
	return result;
}

int ImgProc::CalculateBrightness(IplImage* Image){
	cv::Mat ImageMat=(cv::Mat)Image;
	int average=0;

	for( int y = 0; y < ImageMat.rows; y++ )
	{ 
		for( int x = 0; x < ImageMat.cols; x++ ){
			average+=ImageMat.at<unsigned char>(y,x);
			
		}
             
    }
	average=average/(ImageMat.rows*ImageMat.cols);
	return average;
    
}
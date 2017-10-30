#ifndef IMAGE_PROC_H
#define IMAGE_PROC_H
#include <opencv/cv.h>
#include <opencv/cvaux.h>
#include <opencv/highgui.h>

class ImgProc{
private:
	CvHaarClassifierCascade *cascade_e;
	CvHaarClassifierCascade *cascade_mouth;
public:
	ImgProc();
	IplImage* CutPicture(char* filename,int x,int y, int height, int width);
	int EyeDetect(IplImage *pImage,IplImage** result,int x, int y, int height, int width);
	IplImage* Brightness(IplImage* Image,int brightness,int Show,int half_face);
	int CalculateBrightness(IplImage* Image);
};
#endif
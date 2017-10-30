#ifndef EIGENCORE_H
#define EIGENCORE_H
#include <stdio.h>
#include <string.h>
#include <opencv/cv.h>
#include <opencv/cvaux.h>
#include <opencv/highgui.h>
#include "database.h"

class eigenfaces{
private:
	int nTrainFaces;
	int nEigens;
	IplImage** faceImgArr;
	CvMat* personNumTruthMat;
	IplImage* pAvgTrainImg;
	IplImage** eigenVectArr;
	CvMat * eigenValMat;
	CvMat* projectedTrainFaceMat;
	int Loaded;
	CvMat* trainPersonNumMat;
public:
	float* projectedTestFace;
	eigenfaces();
	void learn(char* filename);
	int recognise(char* filename,facedatabase &data);
	void storeTrainingData();
	int loadTrainingData(CvMat** pTrainPersonNumMat);
	int findNearestNeighbor(float* projectedTestFace,CvMat* ptrainPersonNumMat,facedatabase &data);
	int loadFaceImgArray(char* filename);
};
#endif
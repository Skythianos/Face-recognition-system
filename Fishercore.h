#ifndef FISHERCORE_H
#define FISHERCORE_H
#include <opencv2/core/core.hpp>
#include <opencv2/contrib/contrib.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "database.h"

using namespace cv;
using namespace std;

class fisherfaces{
private:
Mat X; //Vector of classes
Mat eigenVectors;
Mat projectedObjects;
int nClasses;
Mat eigenValues;
int Loaded;
Mat faceIDs;
public:
fisherfaces();
void loadFisherData();
void train(HWND hWnd,int* N,int nClass,int* faceIDArr);
void CopyRows(Mat src,Mat* output,int from,int to);
Mat mean(Mat samples);
void Project(Mat samples,Mat eigenVect,Mat* projectedObjs);
int recognise(float* projectedTestFace,facedatabase &database);
};
#endif
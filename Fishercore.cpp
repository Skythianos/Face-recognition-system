#include "Fishercore.h"
fisherfaces::fisherfaces(){
Loaded=0;
}
void fisherfaces::loadFisherData(){
	CvMat *S2;
	Mat Temp;
	CvFileStorage* fileStorage=cvOpenFileStorage("LDAdata.xml",0,CV_STORAGE_READ);
	S2=(CvMat*)cvReadByName(fileStorage,0,"eigenVectors",0);
	eigenVectors=cvarrToMat(S2,true,true,0);
	eigenVectors.convertTo(eigenVectors,CV_64F);
	S2=(CvMat*)cvReadByName(fileStorage,0,"eigenValues",0);
	eigenValues=cvarrToMat(S2,true,true,0);
	eigenValues.convertTo(eigenValues,CV_64F);
	S2=(CvMat*)cvReadByName(fileStorage,0,"projectedObjects",0);
	projectedObjects=cvarrToMat(S2,true,true,0);
	projectedObjects.convertTo(projectedObjects,CV_64F);
	S2=(CvMat*)cvReadByName(fileStorage,0,"PCAsamples",0);
	X=cvarrToMat(S2,true,true,0);
	X.convertTo(X,CV_64F);
	S2=(CvMat*)cvReadByName(fileStorage,0,"nClasses",0);
	Temp=cvarrToMat(S2,true,true,0);
	nClasses=Temp.at<int>(0,0);
	S2=(CvMat*)cvReadByName(fileStorage,0,"faceIDs",0);
	faceIDs=cvarrToMat(S2,true,true,0);
	Loaded=1;
	cvReleaseFileStorage(&fileStorage);

}

void fisherfaces::train(HWND hWnd,int* N,int nClass,int* faceIDArr){
	try{
	int i;
	Mat classes;
	CvMat S1;
	CvMat* S2;
	nClasses=nClass;
	//filling variables
	CvFileStorage* fileStorage=cvOpenFileStorage("LDAdata.xml",0,CV_STORAGE_WRITE);
	CvFileStorage* PCAdata=cvOpenFileStorage("facedata.xml",0,CV_STORAGE_READ);
	S2=(CvMat*)cvReadByName(PCAdata,0,"projectedTrainFaceMat",0);
	X=cvarrToMat(S2,true,true,0);
	// reducing image dimensions to N-C
	transpose(X,X);
	while(X.rows>X.cols-nClasses) X.pop_back();
	transpose(X,X);
	X.convertTo(X,CV_64F);
	classes=Mat(1,1,CV_32SC1,&nClasses);
	faceIDs=Mat(1,X.rows,CV_32SC1,faceIDArr);
	// these matrices contain the data of each class ordered under each other
	Mat cov; 
	Mat mu;
	int counter=0;
	// calculating covariance matrices and means for each class
	for(i=0;i<nClasses;i++){
		Mat presentX[2],presentMu, presentCov;
		for(int j=0;j<N[i];j++){
			presentX[0].push_back(X.row(counter+j));
		}
		counter+=N[i];
		cv::calcCovarMatrix(presentX[0],presentCov,presentMu, CV_COVAR_NORMAL | CV_COVAR_ROWS);
	mu.push_back(presentMu.row(0));
	for(int k=0;k<presentCov.rows;k++){
	cov.push_back(presentCov.row(k));
	}
	}
	//calculating within class scatter
	Mat Sw;
	CopyRows(cov,&Sw,0,cov.cols-1);
	for(i=1;i<nClasses;i++){
		Mat Temp;
		CopyRows(cov,&Temp,i*cov.cols,(i+1)*cov.cols-1);
		Sw+=Temp;
	}
	// calculating between class scatter
	Mat mu1,mu2,Sb,overAllMean;
	if(nClasses==2){
		mu1.push_back(mu.row(0));
		mu2.push_back(mu.row(1));
		Sb=(mu1.t()-mu2.t());
		Sb=Sb*Sb.t();
	}
	else{
		overAllMean=mean(X);
		Sb=Mat::zeros(X.cols,X.cols,CV_64F);
		for(i=0;i<nClasses;i++){
			Mat Temp;
			Temp.push_back(mu.row(i));
			Temp=(Temp.t()-overAllMean.t());
			Sb+=N[i]*(Temp*Temp.t());
		}
	}
	// calculating the LDA projection
	Mat W=Sw.inv()*Sb;
	eigen(W,eigenValues,eigenVectors);
	Project(X,eigenVectors,&projectedObjects);

	// storing data
	S1=(CvMat)overAllMean;
	cvWrite(fileStorage,"overAllMean",&S1,cvAttrList(0,0));
	S1=(CvMat)cov;
	cvWrite(fileStorage,"cov",&S1,cvAttrList(0,0));
	S1=(CvMat)Sb;
	cvWrite(fileStorage,"Sb",&S1,cvAttrList(0,0));
	S1=(CvMat)W;
	cvWrite(fileStorage,"W",&S1,cvAttrList(0,0));
	S1=(CvMat)eigenVectors;
	cvWrite(fileStorage,"eigenVectors",&S1,cvAttrList(0,0));
	S1=(CvMat)eigenValues;
	cvWrite(fileStorage,"eigenValues",&S1,cvAttrList(0,0));
	S1=(CvMat)projectedObjects;
	cvWrite(fileStorage,"projectedObjects",&S1,cvAttrList(0,0));
	S1=(CvMat)X;
	cvWrite(fileStorage,"PCAsamples",&S1,cvAttrList(0,0));
	S1=(CvMat)classes;
	cvWrite(fileStorage,"nClasses",&S1,cvAttrList(0,0));
	S1=(CvMat)faceIDs;
	cvWrite(fileStorage,"faceIDs",&S1,cvAttrList(0,0));
	cvReleaseFileStorage(&fileStorage);
	cvReleaseFileStorage(&PCAdata);
	Loaded=1;
	}
	catch(cv::Exception &e){
		MessageBox(hWnd,e.what(),"Button1",MB_OK|MB_ICONERROR);
	}
	
}

void fisherfaces::CopyRows(Mat src,Mat* output,int from,int to){
	for(int i=from;i<=to;i++){
	output->push_back(src.row(i));
	}
}

void fisherfaces::Project(Mat samples,Mat eigenVect,Mat* projectedObjs){

	while(eigenVect.rows>(nClasses-1)) eigenVect.pop_back();
			transpose(samples,samples);
			projectedObjs[0]=eigenVect*samples;
			transpose(projectedObjs[0],projectedObjs[0]);
}
Mat fisherfaces::mean(Mat samples){
	int i;
	double* mean=(double*)calloc(samples.cols,sizeof(double));
	Mat result;
	for(i=0;i<samples.cols;i++) mean[i]=0;
	for(i=0;i<samples.rows;i++){
		for(int j=0;j<samples.cols;j++){
			mean[j]+=samples.at<double>(i,j);
		}
	}
	for(i=0;i<samples.cols;i++) mean[i]=mean[i]/samples.rows;
	result=Mat(1,samples.cols,CV_64F,mean);
	return result;
}
int fisherfaces::recognise(float* projectedTestFace,facedatabase &database){
	if(Loaded==0) loadFisherData();
		Mat TestProjection;
		Mat testFace=Mat(1,X.rows-1,CV_32F,projectedTestFace);
		// reducing dimension to N-C
		transpose(testFace,testFace);
		while(testFace.rows>X.cols) testFace.pop_back();
		transpose(testFace,testFace);
		testFace.convertTo(testFace,CV_64F);
		// LDA projection of the input image
		Project(testFace,eigenVectors,&TestProjection);
		//nearest neighbour search
		int nearest;
		double MinDist=DBL_MAX;
		for(int i=0;i<projectedObjects.rows;i++){
			double distance;
			Mat Object=projectedObjects.row(i);
			double distSq=0;
			for(int j=0;j<projectedObjects.cols;j++){
					distance=TestProjection.at<double>(0,j)-Object.at<double>(0,j);
					distSq+=distance*distance;
					}
			if(distSq<MinDist){
					MinDist=distSq;
					nearest=faceIDs.at<int>(0,i);
			}
			database.SetDist(faceIDs.at<int>(0,i),distSq,2);
		}
		return nearest;
}
#include "Eigencore.h"

eigenfaces::eigenfaces(){
	nTrainFaces=0;
	nEigens=0;
	faceImgArr=0;
	personNumTruthMat=0;
	pAvgTrainImg=0;
	eigenVectArr=0;
	eigenValMat=0;
	projectedTrainFaceMat=0;
	Loaded=0;
	trainPersonNumMat=0;
	projectedTestFace=0;
}

void eigenfaces::learn(char* filename){
	int i;
	int j;
	CvTermCriteria calcLimit;
	CvSize faceImgSize;

	nTrainFaces=loadFaceImgArray(filename); //gets the number of training faces
	if(nTrainFaces<2){
		//Error message comes here
		return;
	}
	// PCA happens here
	nEigens=nTrainFaces-1; //number of eigenvalues
	//allocate space for eigenvector images
	faceImgSize.width=faceImgArr[0]->width;
	faceImgSize.height=faceImgArr[0]->height;
	eigenVectArr=(IplImage**)cvAlloc(sizeof(IplImage*)*nEigens);
	for(i=0;i<nEigens;i++){
		eigenVectArr[i]=cvCreateImage(faceImgSize,IPL_DEPTH_32F,1);
	}
	//eigenvalue array
	eigenValMat=cvCreateMat(1,nEigens,CV_32FC1);
	// space for the averaged image
	pAvgTrainImg=cvCreateImage(faceImgSize,IPL_DEPTH_32F,1);
	//PCA termination criteria
	calcLimit=cvTermCriteria(CV_TERMCRIT_ITER,nEigens,1);
	// calculating average image, igenvalues and eigenvectors
	cvCalcEigenObjects(nTrainFaces,(void*)faceImgArr,(void*)eigenVectArr,
		CV_EIGOBJ_NO_CALLBACK,0,0,&calcLimit,pAvgTrainImg,eigenValMat->data.fl);
	//PCA finishes here
	//cvNormalize(eigenValMat, eigenValMat, 1, 0, CV_L1, 0);
	// projecting training images into PCA subspace
	projectedTrainFaceMat=cvCreateMat(nTrainFaces,nEigens,CV_32FC1);
	for(i=0;i<nTrainFaces;i++){
		cvEigenDecomposite(faceImgArr[i],nEigens,eigenVectArr,0,0,pAvgTrainImg,projectedTrainFaceMat->data.fl+i*nEigens);
	}

	storeTrainingData();
	Loaded=0;
}

int eigenfaces::loadFaceImgArray(char * filename){

	FILE* imgListFile=NULL;
	char imgFileName[512];
	int iFace=0;
	int nFace=0;
	//open input image list
	imgListFile=fopen(filename,"r");
			
	if(imgListFile){
	while(fgets(imgFileName,512,imgListFile)) ++nFace;
	rewind(imgListFile);

	faceImgArr=(IplImage**)cvAlloc(nFace*sizeof(IplImage *));
	personNumTruthMat=cvCreateMat(1,nFace,CV_32SC1); // Creating matrix for training images
	
	//store face data in an array (reading data from a txt file)

	for(iFace=0;iFace<nFace;iFace++){
		fscanf(imgListFile,"%d %s",personNumTruthMat->data.i+iFace,imgFileName);
		faceImgArr[iFace]=cvLoadImage(imgFileName,CV_LOAD_IMAGE_GRAYSCALE);
	}
	fclose(imgListFile);
	return nFace;
	}
	return 0;
}
void eigenfaces::storeTrainingData(){
	CvFileStorage* fileStorage;
	int i;
	
	fileStorage=cvOpenFileStorage("facedata.xml",0,CV_STORAGE_WRITE);

	cvWriteInt(fileStorage,"nEigens",nEigens);
	cvWriteInt(fileStorage,"nTrainFaces",nTrainFaces);
	cvWrite(fileStorage,"trainPersonNumMat",personNumTruthMat,cvAttrList(0,0));
	cvWrite(fileStorage,"eigenValMat",eigenValMat,cvAttrList(0,0));
	cvWrite(fileStorage,"projectedTrainFaceMat",projectedTrainFaceMat,cvAttrList(0,0));
	cvWrite(fileStorage,"avgTrainImg",pAvgTrainImg,cvAttrList(0,0));
	for(i=0;i<nEigens;i++){
		char varname[200];
		sprintf(varname,"eigenVect_%d",i);
		cvWrite(fileStorage,varname,eigenVectArr[i],cvAttrList(0,0));
	}
	cvReleaseFileStorage(&fileStorage);
}

int eigenfaces::recognise(char* filename,facedatabase &data){
	int i, nTestFaces=0;
	int iNearest,nearest;
	nTestFaces=loadFaceImgArray(filename);
	// message comes here
	//Load saved training data
	if(Loaded==0){
	if(!loadTrainingData(&trainPersonNumMat)){
		// error message
		return 0;
	}
	Loaded=1;
	}
	//projecting input image to the PCA space
	projectedTestFace=(float*)cvAlloc(nEigens*sizeof(float));
	for(i=0;i<nTestFaces;i++){
		cvEigenDecomposite(faceImgArr[i],nEigens,eigenVectArr,
			0,
			0,
			pAvgTrainImg,
			projectedTestFace);
		iNearest=findNearestNeighbor(projectedTestFace,trainPersonNumMat,data);
		nearest=trainPersonNumMat->data.i[iNearest];// nearest is the ID of the recognised face
	}
	return nearest;
}
int eigenfaces::loadTrainingData(CvMat**pTrainPersonNumMat){
	CvFileStorage* fileStorage;
	int i;

	fileStorage=cvOpenFileStorage("facedata.xml",0,CV_STORAGE_READ);
	if(!fileStorage){
		//Error message comes here
		return 0;
	}
	nEigens=cvReadIntByName(fileStorage,0,"nEigens",0);
	nTrainFaces=cvReadIntByName(fileStorage,0,"nTrainFaces",0);
	*pTrainPersonNumMat=(CvMat*)cvReadByName(fileStorage,0,"trainPersonNumMat",0);
	eigenValMat=(CvMat*)cvReadByName(fileStorage,0,"eigenValMat",0);
	projectedTrainFaceMat=(CvMat*)cvReadByName(fileStorage,0,"projectedTrainFaceMat",0);
	pAvgTrainImg=(IplImage*)cvReadByName(fileStorage,0,"avgTrainImg",0);
	eigenVectArr=(IplImage **)cvAlloc(nTrainFaces*sizeof(IplImage*));
	for(i=0;i<nEigens;i++){
		char varname[200];
		sprintf(varname,"eigenVect_%d",i);
		eigenVectArr[i]=(IplImage*)cvReadByName(fileStorage,0,varname,0);
	}
	cvReleaseFileStorage(&fileStorage);
	return 1;
}

int eigenfaces::findNearestNeighbor(float* projectedTestFace,CvMat* ptrainPersonNumMat,facedatabase &data){
	double leastDistSq=DBL_MAX;
	int i, iTrain,iNearest=0;
	for(iTrain=0; iTrain<nTrainFaces;iTrain++){
		double distSq=0;
		for(i=0;i<nEigens;i++){
			float d_i=projectedTestFace[i]-projectedTrainFaceMat->data.fl[iTrain*nEigens+i];
			distSq += d_i*d_i/eigenValMat->data.fl[i]; // Mahalanobis distance
			//distSq+=d_i*d_i; //Euclidean distance
		}
		if(distSq<leastDistSq){
			leastDistSq=distSq;
			iNearest=iTrain;
		}
		//Distance logging
		data.SetDist(ptrainPersonNumMat->data.i[iTrain],distSq,1);
	}
	return iNearest;
}
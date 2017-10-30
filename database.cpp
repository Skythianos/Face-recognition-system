#include "database.h"
#include <string.h>

facedatabase::facedatabase(){
	dataListFilePath="train.txt";
	pStart=0;
	presentElement=0;
	nElements=0;
}

void facedatabase::InsertElement(int faceID,char* fileNamePath,char* Name,int classID){
	struct dataelement* pAct;
	struct dataelement* pTemp;
	if(pStart){
		pAct=pStart;
		while(pAct->pNext) pAct=pAct->pNext;
		pAct->pNext=new struct dataelement;
		pTemp=pAct;
		pAct=pAct->pNext;
		pAct->pPrev=pTemp;
		pAct->pNext=0;
		pAct->faceID=faceID;
		pAct->classID=classID;
		pAct->dist=0;
		pAct->LDAdist=0;
		for(int i=0;(i<99)&&fileNamePath[i];i++){
			pAct->fileNamePath[i]=fileNamePath[i];
			if(pAct->fileNamePath[i]==' ') pAct->fileNamePath[i]='_';
		}
		pAct->fileNamePath[i]=0;
		if(Name){
			for(i=0;(i<49)&&Name[i];i++){
				pAct->Name[i]=Name[i];
				if(pAct->Name[i]==' ') pAct->Name[i]='_';
			}
		pAct->Name[i]=0;
		}
	}
	else{
		pStart=new struct dataelement;
		pStart->pNext=0;
		pStart->pPrev=0;
		pStart->faceID=faceID;
		pStart->classID=classID;
		pStart->dist=0;
		pStart->LDAdist=0;
		for(int i=0;(i<99)&&fileNamePath[i];i++){
			pStart->fileNamePath[i]=fileNamePath[i];
			if(pStart->fileNamePath[i]==' ') pStart->fileNamePath[i]='_';
		}
		pStart->fileNamePath[i]=0;
		if(Name){
			for(i=0;(i<49)&&Name[i];i++){
				pStart->Name[i]=Name[i];
				if(pStart->Name[i]==' ') pStart->Name[i]='_';
			}
		pStart->Name[i]=0;
		}
		presentElement=pStart;
	}
	nElements++;
}
void facedatabase::NewFaceToClass(int faceID,char* fileNamePath,char* Name,int classID){
struct dataelement* pAct;
	struct dataelement* pTemp;
	if(SearchPerson(classID,&pAct)){
		pTemp=pAct->pNext;
		pAct->pNext=new struct dataelement;
		pTemp->pPrev=pAct->pNext;
		pAct->pNext->pPrev=pAct;
		pAct->pNext->pNext=pTemp;
		pAct->pNext->classID=classID;
		pAct->pNext->faceID=faceID;
		pAct->pNext->dist=0;
		pAct->pNext->LDAdist=0;
		pAct=pAct->pNext;
		for(int i=0;(i<99)&&fileNamePath[i];i++){
			pAct->fileNamePath[i]=fileNamePath[i];
			if(pAct->fileNamePath[i]==' ') pAct->fileNamePath[i]='_';
		}
		pAct->fileNamePath[i]=0;
		if(Name){
			for(i=0;(i<49)&&Name[i];i++){
				pAct->Name[i]=Name[i];
				if(pAct->Name[i]==' ') pAct->Name[i]='_';
			}
		pAct->Name[i]=0;
		}
		nElements++;
	}
	else InsertElement(faceID,fileNamePath,Name,classID);
}
void facedatabase::DeleteElement(int ID){
	struct dataelement* pAct;
	struct dataelement* next;
	struct dataelement* prev;
	if(pStart){
		pAct=pStart;
		while(pAct->faceID!=ID){
			if(pAct->pNext!=0)pAct=pAct->pNext;
			if((pAct->pNext==0)&&(pAct->faceID!=ID)) return;
		}
		 prev=pAct->pPrev;
		 next=pAct->pNext;
		 
		if(prev) prev->pNext=next;
		if(next) next->pPrev=prev;
		if((next==0)&&prev==0){
			presentElement=0;
			pStart=0;
		}
		if((next!=0)&&prev==0) pStart=next;
		delete pAct;
		nElements--;
	}
}
void facedatabase::ChangeDataBasePath(char* newPath){
	dataListFilePath=newPath;
}

int facedatabase::Loadlist(){
	int ID=0;
	int classID=0;
	char Path[100];
	struct dataelement* pTemp;
	FILE* datafile;
	datafile=fopen(dataListFilePath,"r");
	if(datafile==NULL)return 0;
	while(!feof(datafile)){
	fscanf(datafile,"%d %s",&ID,Path);
	InsertElement(ID,Path,NULL,0);
	}
	fclose(datafile);

	datafile=fopen("data.txt","r");
	if(datafile==NULL)return 0;
	while(!feof(datafile)){
	fscanf(datafile,"%d %s %d",&ID,Path,&classID);
	SearchID(ID,&pTemp);
	for(int i=0;(i<49)&&Path[i];i++) pTemp->Name[i]=Path[i];
	pTemp->Name[i]=0;
	pTemp->dist=0;
	pTemp->classID=classID;
	}
	fclose(datafile);
	return 1;
}

int facedatabase::SaveList(){
	struct dataelement* pAct;
	FILE* datafile;
	if(pStart){
		datafile=fopen(dataListFilePath,"w");
		if(datafile==NULL)return 0;
		pAct=pStart;
		while(pAct->pNext!=0){
			fprintf(datafile,"%d %s\n",pAct->faceID,pAct->fileNamePath);
			pAct=pAct->pNext;
		}
		fprintf(datafile,"%d %s",pAct->faceID,pAct->fileNamePath);
		fclose(datafile);

		datafile=fopen("data.txt","w");
		if(datafile==NULL)return 0;
		pAct=pStart;
		while(pAct->pNext!=0){
			fprintf(datafile,"%d %s %d\n",pAct->faceID,pAct->Name,pAct->classID);
			pAct=pAct->pNext;
		}
		fprintf(datafile,"%d %s %d",pAct->faceID,pAct->Name,pAct->classID);
		fclose(datafile);

		return 1;
	}
	return 0;
}

void facedatabase::SearchID(int ID,struct dataelement** pTemp){
	struct dataelement* pAct=*pTemp;
	if(pStart){
		pAct=pStart;
		while(pAct->faceID!=ID){
			if(pAct->pNext!=0)pAct=pAct->pNext;
			if((pAct->pNext==0)&&(pAct->faceID!=ID)) return ;
		}
		*pTemp=pAct;
		return;
	}
	*pTemp=NULL;
}
int facedatabase::SearchPerson(int personID,struct dataelement** pTemp){
	struct dataelement* pAct=*pTemp;
	if(pStart){
		pAct=pStart;
		while(pAct->classID!=personID){
			if(pAct->pNext!=0)pAct=pAct->pNext;
			if(pAct->pNext==0) return 0 ;
		}
		*pTemp=pAct;
		return 1;
	}
	return 0;
}
int facedatabase::GetMaxID(int person_or_face){
	struct dataelement* pTemp;
	int max=0;
	if(pStart){
		pTemp=pStart;
		while(pTemp!=0){
			if(person_or_face==0){
				if(pTemp->faceID>max) max=pTemp->faceID;
			}
			else{
				if(pTemp->classID>max) max=pTemp->classID;
			}
			pTemp=pTemp->pNext;
		}
	}
	return max;
}
void facedatabase::GetName(int ID,char* Name,int lenght,int IDtype){
	struct dataelement* pTemp;
	if(Name){
		if(ID==-1){
			char* Temp="Unknown";
			for(int i=0;(Temp[i])&&(i<lenght);i++) Name[i]=Temp[i];
			Name[i]=0;
		}
		else{
			if(IDtype==0) SearchID(ID,&pTemp);
			else SearchPerson(ID,&pTemp);
			for(int i=0;(i<lenght)&&pTemp->Name[i];i++) Name[i]=pTemp->Name[i];
			Name[i]=0;
		}
	}
}
void facedatabase::ClearList(){
	struct dataelement* pAct;
	struct dataelement* pTemp;
	if(pStart){
		pAct=pStart;
		while(pAct){
			pTemp=pAct->pNext;
			free(pAct);
			pAct=pTemp;
		}
		pStart=0;
		presentElement=0;
		nElements=0;
	}
}
void facedatabase::SetDist(int ID,double dist,int method){
	struct dataelement* pTemp;
	if(pStart){
		SearchID(ID,&pTemp);
		if(method==1) pTemp->dist=dist;
		if(method==2) pTemp->LDAdist=dist;
	}
}
double facedatabase::GetMinDist(){
	struct dataelement* pAct;
	double mindist;
	if(pStart){
		pAct=pStart;
		mindist=pAct->dist;
		pAct=pAct->pNext;
		while(pAct){
			if(pAct->dist<mindist) mindist=pAct->dist;
			pAct=pAct->pNext;
		}
		return mindist;
	}
	return 0;
}

void facedatabase::SaveDist(){
	FILE* datafile;
	struct dataelement* pAct;
	datafile=fopen("distances.txt","w");
	if(pStart){
		pAct=pStart;
		while(pAct->pNext!=0){
			if(pAct->classID!=0) fprintf(datafile,"%d %f %s %f\n",pAct->faceID,pAct->dist,pAct->Name,pAct->LDAdist);
			pAct=pAct->pNext;
		}
		fprintf(datafile,"%d %f %s %f\n",pAct->faceID,pAct->dist,pAct->Name,pAct->LDAdist);
	}
	fclose(datafile);
}
int facedatabase::BrowseDatabase(int direction,int* ID,char* Name, char* Path,int searchByName,int* classID){
	int i;
	if(presentElement){
		if(searchByName==0){
			if((direction==1)&&presentElement->pNext) presentElement=presentElement->pNext; //Step forward
			if(direction==0) presentElement=pStart; // Step to the first element
			if((direction==-1)&&presentElement->pPrev) presentElement=presentElement->pPrev; //Step backward
		}
		if(searchByName){
			if(direction==1){
				if(presentElement->pNext) presentElement=presentElement->pNext;
				while((presentElement->pNext)&&(strcmp(Name,presentElement->Name))){
					presentElement=presentElement->pNext; //Step forward
				}
			}
			if(direction==-1){
				if(presentElement->pPrev) presentElement=presentElement->pPrev;
				while((presentElement->pPrev)&&(strcmp(Name,presentElement->Name))){
					presentElement=presentElement->pPrev; //Step forward
				}
			}
			if(direction==0){
				presentElement=pStart;
				while((presentElement->pNext)&&(strcmp(Name,presentElement->Name))){
					presentElement=presentElement->pNext; //Step forward
				}
			}
		}
		if((presentElement->classID==0)&&(direction==1)){
			while((presentElement->classID==0)&&(presentElement->pNext!=0)) presentElement=presentElement->pNext;
		}
		if((presentElement->classID==0)&&(direction==-1)){
			while((presentElement->classID==0)&&(presentElement->pPrev!=0)) presentElement=presentElement->pPrev;
		}
		//Copying element's data
		for(i=0;(i<49)&&presentElement->Name[i];i++){
			Name[i]=presentElement->Name[i];
		}
		Name[i]=0;
		for(i=0;(i<99)&&presentElement->fileNamePath[i];i++){
			Path[i]=presentElement->fileNamePath[i];
		}
		Path[i]=0;
		*ID=presentElement->faceID;
		*classID=presentElement->classID;
		if(presentElement->pNext==0) return -1;
		if(presentElement->pPrev==0) return -1;
		return 1;
	}
	return 0;
}
int facedatabase::Is_ID(int ID){
	struct dataelement* pAct;
	int i=0;
	if(pStart){
		pAct=pStart;
		while(pAct){
			if(pAct->faceID==ID) i=1;
			pAct=pAct->pNext;	
		}
		if(i==1) return 1;
		return 0;
	}
	return -1;
}
void facedatabase::ClassifyData(int** classPopulations,int** personIDs,int* nClasses){
	*nClasses=0;
	int classPopulation=0;
	int currentClass=-1;
	int i=0;
	struct dataelement* pAct;
	personIDs[0]=new int[nElements];
	if(pStart){
		pAct=pStart;
		while(pAct){
			if(pAct->classID!=currentClass){
				*nClasses+=1;
				currentClass=pAct->classID;
			}
			personIDs[0][i]=pAct->faceID;
			i++;
			pAct=pAct->pNext;
		}
		classPopulations[0]=new int[12];
		pAct=pStart;
		currentClass=pStart->classID;
		i=0;
		while(pAct){
			if(pAct->classID!=currentClass){
				currentClass=pAct->classID;
				classPopulations[0][i]=classPopulation;
				i++;
				classPopulation=0;
			}
			classPopulation++;
			if(pAct->pNext==0) classPopulations[0][i]=classPopulation;
			pAct=pAct->pNext;
		}

	}
}
void facedatabase::AscendingOrder(int** LDAIDArr,int** personIDArr){
   personIDArr[0]=new int[nElements];
  double* PCAdistances=new double[nElements];
   LDAIDArr[0]=new int[nElements];
  double* LDAdistances=new double[nElements];
   struct dataelement* pAct;
   int i=0;
   if(pStart){
		pAct=pStart;
		while(pAct){
			personIDArr[0][i]=pAct->classID;
			LDAIDArr[0][i]=pAct->classID;
			PCAdistances[i]=pAct->dist;
			LDAdistances[i]=pAct->LDAdist;
			i++;
			pAct=pAct->pNext;
		}
		int j=1;
		int TempID;
		double TempDist;
		while(j){
			j=0;
			for(i=1;i<nElements;i++){
				if(PCAdistances[i-1]>PCAdistances[i]){
					TempDist=PCAdistances[i-1];
					PCAdistances[i-1]=PCAdistances[i];
					PCAdistances[i]=TempDist;
					TempID=personIDArr[0][i-1];
					personIDArr[0][i-1]=personIDArr[0][i];
					personIDArr[0][i]=TempID;
					j++;
				}
				if(LDAdistances[i-1]>LDAdistances[i]){
					TempDist=LDAdistances[i-1];
					LDAdistances[i-1]=LDAdistances[i];
					LDAdistances[i]=TempDist;
					TempID=LDAIDArr[0][i-1];
					LDAIDArr[0][i-1]=LDAIDArr[0][i];
					LDAIDArr[0][i]=TempID;
					j++;
				}
			}
		}
   FILE* file=fopen("statistics.txt","w");
   fprintf(file,"First six closest match\n%s\n","PCA:");
   for(i=0;i<6;i++) fprintf(file,"%d %f\n",personIDArr[0][i],PCAdistances[i]);
   fprintf(file,"\n%s\n","LDA:");
   for(i=0;i<6;i++) fprintf(file,"%d %f\n",LDAIDArr[0][i],LDAdistances[i]);
   fclose(file);
   }
}

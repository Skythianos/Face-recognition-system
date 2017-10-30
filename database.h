#ifndef DATABASE_H
#define DATABASE_H
#include <stdio.h>
#include <stdlib.h>

class facedatabase{
private:
	struct dataelement{
		int faceID;
		int classID;
		char fileNamePath[100];
		char Name[50];
		double dist;
		double LDAdist;
		struct dataelement* pNext;
		struct dataelement* pPrev;
	};
	struct dataelement* pStart;
	char* dataListFilePath;
	struct dataelement* presentElement;
public:
	int nElements;
	facedatabase();
	void InsertElement(int ID, char*fileNamePath,char* Name,int classID);
	void NewFaceToClass(int faceID,char* fileNamePath,char* Name,int classID);
	void ClearList();
	void DeleteElement(int ID);
	void SearchID(int ID,struct dataelement** pTemp);
	int SearchPerson(int personID,struct dataelement** pTemp);
	int GetMaxID(int person_or_face);
	double GetMinDist();
	void GetName(int ID,char *Name,int lenght,int IDtype);
	int SaveList();
	void ChangeDataBasePath(char* newPath);
	int Loadlist();
	void SetDist(int ID,double dist,int method);
	void SaveDist();
	int BrowseDatabase(int direction,int* ID,char* Name,char* Path,int searchByName,int* classID);
	int Is_ID(int ID);
	void ClassifyData(int** classPopulations,int** personIDs,int* nClasses);
	void AscendingOrder(int** LDAIDArr,int** personIDArr);
};
#endif
	
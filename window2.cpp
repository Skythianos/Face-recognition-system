#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include "camera.h"
#include "Eigencore.h"
#include "database.h"
#include "Fishercore.h"
#include "Image_proc.h"
#include <commctrl.h>
#pragma comment( lib, "comctl32" )

// Global variables

// Window handles of different dialogs of this program
HWND MainWindow=NULL;
HWND hWnd2=NULL;
HWND hWnd3=NULL;
HWND settings=NULL;
HWND database=NULL;
// handles for trackbars of the settings window
HWND trcbar1=NULL;
HWND trcbar2=NULL;
HWND trcbar3=NULL;
HWND trcbar4=NULL;
HWND trcbar5=NULL;
HWND trcbar6=NULL;
HWND accuracybar=NULL;
HWND mirrorbox=NULL;
HWND mirrorbox2=NULL;
HWND OK=NULL;
HWND schbyname=NULL;
//General constants
int i=0;
int x,y,height,width;
double accuracy=1.2;
int brightness=147;
//flags
int brgControl=0;
int SearchByName=0;
int PCAeval=1;
int LDAeval=0;
int DoubleMatch=1;
int half_face_brightness=0;
int data_modified=0;
// strings, global storages
IplImage* Img=NULL;
char Name[100]="Name";
char filePath[100];
int ID;
int classID;
char IDchar[20];
char name[50];
char* PicturePath="exclamation.bmp";
// special objects
eigenfaces eigface;
fisherfaces fisherface;
facedatabase facedata;
ImgProc ImagePreprocessor;
static TCHAR szWindowClass[] = _T("win32app");
TCHAR greeting[50] = _T("R.O.T.T");
camera cam1;

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("Face Recogniser");
HINSTANCE hInst;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WndProc2(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WndProc3(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK SettingsPrc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK DataPrc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
HWND WindowInit(char* szWindowClass2,char* Title,int xsize,int ysize,WNDPROC ProcFunction,HWND ParentWindow, WNDCLASSEX* wcex);
HWND TrackbarInit(char* name,int xpos,int ypos,int ID,HWND ParentWindow,int min,int max);
int CheckFace(HWND hWnd);
IplImage* CaptureFace(HWND hWnd);
void LearnFace(IplImage* Img,HWND hWnd);
char* LoadDistLog(char* filename);
int getIDfromFilename(char* Path);
int* ClassIDStat(int* IDs);
int OverAllResult(int PCAen,int LDAen,int double_match_only);

// Entry point of the program
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{	// Main window declarations
    WNDCLASSEX wcex;
    hInst = hInstance; // Store instance handle in our global variable

	MainWindow=WindowInit(szWindowClass,szTitle,1100,700,WndProc,NULL,&wcex);
	
	// Initialisation of child windows
	WNDCLASSEX wcex2;
	hWnd2=WindowInit("picture","Picture Taken",550,400,WndProc2,MainWindow,&wcex2); // Photos taken will appear in this window
	WNDCLASSEX wcex3;
	hWnd3=WindowInit("video","Live video",500,300,WndProc3,NULL,&wcex3); // This window the live video of the camera
	WNDCLASSEX wcex4;
	settings=WindowInit("settings","Settings",560,500,SettingsPrc,MainWindow,&wcex4); // Settings window
	WNDCLASSEX wcex5;
	database=WindowInit("Database","Edit Database",550,500,DataPrc,MainWindow,&wcex5); //Database window

	//trackbar initialisation
	trcbar1=TrackbarInit("Slide1",50,20,6,settings,0,100);
	trcbar2=TrackbarInit("Slide2",300,250,7,settings,0,255);
	trcbar3=TrackbarInit("Slide3",50,90,8,settings,0,100);
	trcbar4=TrackbarInit("Slide4",50,160,9,settings,0,100);
	trcbar5=TrackbarInit("Slide5",50,270,10,settings,0,100);
	trcbar6=TrackbarInit("Slide6",50,210,14,settings,1,16);
	accuracybar=TrackbarInit("Slide7",300,150,19,settings,0,200);
	EnableWindow(accuracybar,TRUE);
	SendMessage(accuracybar,TBM_SETPOS,TRUE,120);
	SendMessage(trcbar2,TBM_SETPOS,TRUE,147);

	mirrorbox=CreateWindow(TEXT("Button"),TEXT("Brightness Control"),WS_VISIBLE|WS_CHILD|BS_CHECKBOX,400,300,180,20,settings, (HMENU) 15,NULL,NULL);
	mirrorbox2=CreateWindow(TEXT("Button"),TEXT("Horizontal mirror"),WS_VISIBLE|WS_CHILD|BS_CHECKBOX,300,390,180,20,settings, (HMENU) 16,NULL,NULL);
	schbyname=CreateWindow(TEXT("Button"),TEXT("Search by name"),WS_VISIBLE|WS_CHILD|BS_CHECKBOX,350,360,180,20,database, (HMENU) 35,NULL,NULL);
	EnableWindow(mirrorbox,TRUE);
	EnableWindow(mirrorbox2,FALSE);
	EnableWindow(trcbar2,TRUE);

	facedata.Loadlist();
	if(facedata.BrowseDatabase(0,&ID,name,filePath,0,&classID)){
				sprintf(IDchar,"%d",ID);
				SetDlgItemText(database,26,IDchar);
				SetDlgItemText(database,27,name);
				SetDlgItemText(database,28,filePath);
				sprintf(IDchar,"%d",classID);
				SetDlgItemText(database,36,IDchar);
	}

    ShowWindow(MainWindow,
        nCmdShow);
    UpdateWindow(MainWindow);
    // Main message loop:
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int) msg.wParam;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
// Procedure function for the main window
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{	
	IplImage * pFaceImg=0;
	char text_buf[120];
	PAINTSTRUCT ps;
    HDC hdc;
	HBITMAP picture;
	BITMAP bitmap;
	HBITMAP hbmOld;
	HDC hdcMem;
    switch (message)
    {
	case WM_CREATE:
		CreateWindow(TEXT("Edit"),TEXT("No One"),WS_VISIBLE|WS_CHILD|WS_BORDER,300,400,200,20, hWnd, (HMENU) 18,NULL,NULL);
	    CreateWindow(TEXT("Button"),TEXT("Settings"),WS_VISIBLE|WS_CHILD|WS_BORDER,10,350,200,50, hWnd, (HMENU) 1,NULL,NULL);
		CreateWindow(TEXT("Button"),TEXT("Open Camera"),WS_VISIBLE|WS_CHILD|WS_BORDER,10,60,100,100, hWnd, (HMENU) 2,NULL,NULL);
		CreateWindow(TEXT("Button"),TEXT("Close Camera"),WS_VISIBLE|WS_CHILD|WS_BORDER,120,60,100,100, hWnd, (HMENU) 13,NULL,NULL);
		CreateWindow(TEXT("Button"),TEXT("Face Detection"),WS_VISIBLE|WS_CHILD|WS_BORDER,10,170,200,50, hWnd, (HMENU) 3,NULL,NULL);
		CreateWindow(TEXT("Button"),TEXT("Edit Database"),WS_VISIBLE|WS_CHILD|WS_BORDER,10,230,200,50, hWnd, (HMENU) 4,NULL,NULL);
		CreateWindow(TEXT("Button"),TEXT("Check Face"),WS_VISIBLE|WS_CHILD|WS_BORDER,10,290,200,50, hWnd, (HMENU) 5,NULL,NULL);		
		CreateWindow(TEXT("Button"),TEXT("Learn Face"),WS_VISIBLE|WS_CHILD|WS_BORDER,10,410,150,50, hWnd, (HMENU) 17,NULL,NULL);
		CreateWindow(TEXT("Edit"),TEXT("There is no Log"),WS_VISIBLE|WS_CHILD|WS_BORDER|ES_MULTILINE|ES_READONLY,550,60,320,550, hWnd, (HMENU) 24,NULL,NULL);
		CreateWindow(TEXT("Edit"),TEXT("There is no Log"),WS_VISIBLE|WS_CHILD|WS_BORDER|ES_MULTILINE|ES_READONLY,880,60,180,550, hWnd, (HMENU) 41,NULL,NULL);
		CreateWindow(TEXT("Edit"),TEXT("Ready"),WS_VISIBLE|WS_CHILD|ES_READONLY,5,490,300,20, hWnd, (HMENU) 25,NULL,NULL);
		CreateWindow(TEXT("Edit"),TEXT("No One"),WS_VISIBLE|WS_CHILD|WS_BORDER,300,430,200,20, hWnd, (HMENU) 40,NULL,NULL);
		CreateWindow(TEXT("Edit"),TEXT("No One"),WS_VISIBLE|WS_CHILD|WS_BORDER,300,460,200,20, hWnd, (HMENU) 42,NULL,NULL);
		break;

	case WM_COMMAND:
		if(LOWORD(wParam)==1){// Settings
			ShowWindow(settings,SW_SHOW);
			UpdateWindow(settings);
		}

		if(LOWORD(wParam)==2){// Open Camera
			if(i==0) cam1.InitCamera();
			if((i==0)&&(cam1.CheckReturnValue()==IS_SUCCESS)){
				cam1.CaptureVideo();
				i++;
			}
			if(hWnd3&&i){ 
					cam1.CaptureVideo();
					ShowWindow(hWnd3,SW_SHOW);
					UpdateWindow(hWnd3);
			}
			else MessageBox(hWnd,"Camera not Opened!","Button1",MB_OK|MB_ICONERROR);
		}
		if(LOWORD(wParam)==3){// Face detection
			cam1.EnableFaceDetection(1);
			SetDlgItemText(hWnd,25,"Face detection enabled");
		}
		if(LOWORD(wParam)==4){// Edit Database
			ShowWindow(database,SW_SHOW);
			UpdateWindow(database);
		}
		if(LOWORD(wParam)==5){//Check face
			if(CheckFace(hWnd)){
				facedata.GetName(OverAllResult(PCAeval,LDAeval,DoubleMatch),text_buf,100,1);
				SetDlgItemText(hWnd,42,text_buf);
				SetDlgItemText(hWnd,24,LoadDistLog("distances.txt"));
				SetDlgItemText(hWnd,41,LoadDistLog("statistics.txt"));
			}
		}
		
		if(LOWORD(wParam)==17){// Learn face
			Img=CaptureFace(hWnd);
		}	
		if(LOWORD(wParam)==13){// Close Camera
			cam1.Close_Camera();
			SetDlgItemText(hWnd,25,"Camera Closed");
			i=0;
			PicturePath="exclamation.bmp";
			ShowWindow(hWnd,SW_HIDE);
			ShowWindow(hWnd,SW_SHOW);
			UpdateWindow(hWnd);
		}
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
        TextOut(hdc,550,40,"Similarity (distance) Log", _tcslen("Similarity (distance) Log"));
		TextOut(hdc,260,400,"PCA:", _tcslen("PCA:"));
		TextOut(hdc,260,430,"LDA:", _tcslen("LDA:"));
		TextOut(hdc,240,460,"OverAll:", _tcslen("OverAll:"));
		picture=(HBITMAP)LoadImage(hInst,PicturePath,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
		//Display picture
		hdcMem = CreateCompatibleDC(hdc);
		hbmOld = (HBITMAP)SelectObject(hdcMem, picture);
        GetObject(picture, sizeof(bitmap), &bitmap);
        BitBlt(hdc,300,120,200,300, hdcMem, 0, 0, SRCCOPY);
        SelectObject(hdcMem, hbmOld);
        DeleteDC(hdcMem);
        EndPaint(hWnd, &ps);
	break;

    case WM_DESTROY:
		cam1.Close_Camera();
		facedata.ClearList();
		PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
}
// Procedure for the Training window
LRESULT CALLBACK WndProc2(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{	
	PAINTSTRUCT ps;
    HDC hdc;
	HBITMAP picture;
	BITMAP bitmap;
	HBITMAP hbmOld;
	HDC hdcMem;
    switch (message)
    {
	case WM_CREATE:
			OK=CreateWindow(TEXT("Button"),TEXT("Learn Face"),WS_VISIBLE|WS_CHILD|WS_BORDER,50,250,150,50, hWnd, (HMENU) 23,NULL,NULL);
			CreateWindow(TEXT("Edit"),TEXT("Name"),WS_VISIBLE|WS_CHILD|WS_BORDER,350,20,150,20, hWnd, (HMENU) 38,NULL,NULL);
			CreateWindow(TEXT("Edit"),TEXT("0"),WS_VISIBLE|WS_CHILD|WS_BORDER,350,60,150,20, hWnd, (HMENU) 39,NULL,NULL);
			EnableWindow(OK,FALSE);
		break;
	case WM_COMMAND:
		if(LOWORD(wParam)==23){// Learnface-OK
			LearnFace(Img,MainWindow);
			EnableWindow(OK,FALSE);
			ShowWindow(hWnd,SW_HIDE);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		TextOut(hdc,280,20,"Name:", _tcslen("Name:"));
		TextOut(hdc,280,60,"Person ID:",_tcslen("Person ID:"));
		picture=(HBITMAP)LoadImage(hInst,"Sample.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
		//Display picture
		hdcMem = CreateCompatibleDC(hdc);
		hbmOld = (HBITMAP)SelectObject(hdcMem, picture);
        GetObject(picture, sizeof(bitmap), &bitmap);
        BitBlt(hdc,50,20,184,224, hdcMem, 0, 0, SRCCOPY);
        SelectObject(hdcMem, hbmOld);
        DeleteDC(hdcMem);
        EndPaint(hWnd, &ps);
	break;

	case WM_CLOSE:
		ShowWindow(hWnd,SW_HIDE);
		break;
	default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
}
// Procedure for the video window
LRESULT CALLBACK WndProc3(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_PAINT:
		cam1.DisplayVideo(hWnd);
	break;

	case WM_CLOSE:
		ShowWindow(hWnd,SW_HIDE);
		cam1.StopVideo();
		break;
	default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
}
// Procedure for the database window
LRESULT CALLBACK DataPrc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
    HDC hdc;
	HBITMAP picture;
	BITMAP bitmap;
	HBITMAP hbmOld;
	HDC hdcMem;
	int RetVal;
	char error[300];
    switch (message)
    {
	case WM_CREATE:// Buton declarations
		CreateWindow(TEXT("Edit"),TEXT(""),WS_VISIBLE|WS_CHILD|WS_BORDER,80,20,40,20, hWnd, (HMENU) 26,NULL,NULL);
		CreateWindow(TEXT("Edit"),TEXT(""),WS_VISIBLE|WS_CHILD|WS_BORDER,80,50,200,20, hWnd, (HMENU) 27,NULL,NULL);
		CreateWindow(TEXT("Edit"),TEXT(""),WS_VISIBLE|WS_CHILD|WS_BORDER,80,80,200,20, hWnd, (HMENU) 28,NULL,NULL);
		CreateWindow(TEXT("Button"),TEXT("Update Database"),WS_VISIBLE|WS_CHILD|WS_BORDER,10,400,200,50, hWnd, (HMENU) 29,NULL,NULL);
		CreateWindow(TEXT("Button"),TEXT("Add New File"),WS_VISIBLE|WS_CHILD|WS_BORDER,10,340,200,50, hWnd, (HMENU) 30,NULL,NULL);
		CreateWindow(TEXT("Button"),TEXT("Delete File"),WS_VISIBLE|WS_CHILD|WS_BORDER,10,280,200,50, hWnd, (HMENU) 31,NULL,NULL);
		CreateWindow(TEXT("Button"),TEXT("<<Prev"),WS_VISIBLE|WS_CHILD|WS_BORDER,300,280,60,20, hWnd, (HMENU) 32,NULL,NULL);
		CreateWindow(TEXT("Button"),TEXT("Next>>"),WS_VISIBLE|WS_CHILD|WS_BORDER,400,280,60,20, hWnd, (HMENU) 33,NULL,NULL);
		CreateWindow(TEXT("Edit"),TEXT("To add a file type its path,\r\the person's name,\r\nand the classID.\r\nThen press Add New File"),
			WS_VISIBLE|WS_CHILD|ES_MULTILINE|ES_READONLY,10,140,250,140, hWnd, (HMENU) 34,NULL,NULL);
		CreateWindow(TEXT("Edit"),TEXT(""),WS_VISIBLE|WS_CHILD|WS_BORDER,80,110,200,20, hWnd, (HMENU) 36,NULL,NULL);
		CheckDlgButton(hWnd, 35, BST_UNCHECKED);
		break;
	case WM_COMMAND:
		if(LOWORD(wParam)==30){// Add New File
			GetDlgItemText(hWnd,27,name,49);  //Get name and file path
			GetDlgItemText(hWnd,28,filePath,99);
			GetDlgItemText(hWnd,36,IDchar,10);
			sscanf(IDchar,"%d",&classID);
			ID=getIDfromFilename(filePath);	//Get ID from file name
			if(ID!=-1){
				if(facedata.Is_ID(ID)!=1){ //Does this ID already exist?
			sprintf(IDchar,"%d",ID);
			SetDlgItemText(hWnd,26,IDchar);
			facedata.NewFaceToClass(ID,filePath,name,classID); //Add element to database
			facedata.SaveList();
			ShowWindow(hWnd,SW_HIDE);	//Refreshing window
			ShowWindow(hWnd,SW_SHOW);
			UpdateWindow(hWnd);
			data_modified=1;
				}
				else{
					sprintf(error,"This ID already exists! \r\nAdvised name is %d.bmp",facedata.GetMaxID(0)+1);
					SetDlgItemText(hWnd,34,error);
				}
			}
			else SetDlgItemText(hWnd,34,"Filename must be a number!");
		}
		if(LOWORD(wParam)==31){// Delete file
			GetDlgItemText(hWnd,26,IDchar,19); //Get the ID
			GetDlgItemText(hWnd,27,name,49);
			sscanf(IDchar,"%d",&ID);
			facedata.DeleteElement(ID); //Deleting element
			RetVal=facedata.BrowseDatabase(0,&ID,name,filePath,SearchByName,&classID);  // Move to the starting point
				//Displaying data
				sprintf(IDchar,"%d",ID);
				SetDlgItemText(hWnd,26,IDchar);
				SetDlgItemText(hWnd,27,name);
				SetDlgItemText(hWnd,28,filePath);
				ShowWindow(hWnd,SW_HIDE);
				ShowWindow(hWnd,SW_SHOW);
				UpdateWindow(hWnd);
			facedata.SaveList();
			ShowWindow(hWnd,SW_HIDE);
			ShowWindow(hWnd,SW_SHOW);
			UpdateWindow(hWnd);
			data_modified=1;
		}
		if(LOWORD(wParam)==33){// Next>>
			GetDlgItemText(hWnd,27,name,49);
			RetVal=facedata.BrowseDatabase(1,&ID,name,filePath,SearchByName,&classID);// Move forward
			 //Display Data
				sprintf(IDchar,"%d",ID);
				SetDlgItemText(hWnd,26,IDchar);
				SetDlgItemText(hWnd,27,name);
				SetDlgItemText(hWnd,28,filePath);
				sprintf(IDchar,"%d",classID);
				SetDlgItemText(hWnd,36,IDchar);
				ShowWindow(hWnd,SW_HIDE);
				ShowWindow(hWnd,SW_SHOW);
				UpdateWindow(hWnd);
			
		}
		if(LOWORD(wParam)==32){// <<Prev
			GetDlgItemText(hWnd,27,name,49);
			RetVal=facedata.BrowseDatabase(-1,&ID,name,filePath,SearchByName,&classID);// Step backward
			 //Display data
				sprintf(IDchar,"%d",ID);
				SetDlgItemText(hWnd,26,IDchar);
				SetDlgItemText(hWnd,27,name);
				SetDlgItemText(hWnd,28,filePath);
				sprintf(IDchar,"%d",classID);
				SetDlgItemText(hWnd,36,IDchar);
				ShowWindow(hWnd,SW_HIDE);
				ShowWindow(hWnd,SW_SHOW);
				UpdateWindow(hWnd);
		
		}
		if(LOWORD(wParam)==29){//Update Database
			facedata.ClearList();
			facedata.Loadlist();
			if(facedata.nElements>2){
				eigface.learn("train.txt");
				int* ClassPopulations;
				int nClasses;
				int* personIDs;
				facedata.ClassifyData(&ClassPopulations,&personIDs,&nClasses);
				fisherface.train(MainWindow,ClassPopulations,nClasses,personIDs);
				SetDlgItemText(MainWindow,25,"Database updated");
			}
			data_modified=0;
		}
		if(LOWORD(wParam)==35){// Search by name
	    if (IsDlgButtonChecked(hWnd, 35)) {
	        CheckDlgButton(hWnd, 35, BST_UNCHECKED);
			SearchByName=0;
			
	    } else {
		 CheckDlgButton(hWnd, 35, BST_CHECKED);
		 SearchByName=1;
		}
		}
		
		break;

	case WM_PAINT: // Display the actual picture
		hdc = BeginPaint(hWnd, &ps);
		picture=(HBITMAP)LoadImage(hInst,filePath,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
		//Display pictures
		hdcMem = CreateCompatibleDC(hdc);
		hbmOld = (HBITMAP)SelectObject(hdcMem, picture);
        GetObject(picture, sizeof(bitmap), &bitmap);
        BitBlt(hdc, 300, 20, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);
        SelectObject(hdcMem, hbmOld);
        DeleteDC(hdcMem);
		TextOut(hdc,5, 20,"ID:", _tcslen("ID:"));
		TextOut(hdc,5, 50,"Name:", _tcslen("Name:"));
		TextOut(hdc,5, 80,"File Path:", _tcslen("File Path:"));
		TextOut(hdc,5, 110,"Person ID:", _tcslen("Person ID:"));
        EndPaint(hWnd, &ps);
		
	break;

	case WM_CLOSE: //Close window and referesh database
		ShowWindow(hWnd,SW_HIDE);
		if(data_modified){
			facedata.ClearList();
			facedata.Loadlist();
			if(facedata.nElements>2){
				eigface.learn("train.txt");
				int* ClassPopulations;
				int nClasses;
				int* personIDs;
				facedata.ClassifyData(&ClassPopulations,&personIDs,&nClasses);
				fisherface.train(MainWindow,ClassPopulations,nClasses,personIDs);
				SetDlgItemText(MainWindow,25,"Database updated");
			}
		data_modified=0;
		}
		break;
	default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
}
// Procedure for the settings window
LRESULT CALLBACK SettingsPrc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
    HDC hdc;
	int Saturation;
	int BLUE;
	int MASTER;
	int GREEN;
	double ZOOM;
	char array[50];
	

    switch (message)
    {
	case WM_CREATE://Button declarations
		CreateWindow(TEXT("Button"),TEXT("OK"),WS_VISIBLE|WS_CHILD|WS_BORDER,100,400,100,50, hWnd, (HMENU) 11,NULL,NULL);
		CreateWindow(TEXT("Button"),TEXT("Reset to default"),WS_VISIBLE|WS_CHILD|BS_CHECKBOX,10,350,180,20, hWnd, (HMENU) 12,NULL,NULL);
		CreateWindow(TEXT("Button"),TEXT("TEST"),WS_VISIBLE|WS_CHILD|WS_BORDER,320,300,50,20, hWnd, (HMENU) 22,NULL,NULL);
		CreateWindow(TEXT("Edit"),TEXT("1.2"),WS_VISIBLE|WS_CHILD|WS_BORDER,450,120,40,20, hWnd, (HMENU) 20,NULL,NULL);
		CreateWindow(TEXT("Edit"),TEXT("147"),WS_VISIBLE|WS_CHILD|WS_BORDER,400,225,40,20, hWnd, (HMENU) 21,NULL,NULL);
		CreateWindow(TEXT("Button"),TEXT("PCA"),WS_VISIBLE|WS_CHILD|BS_CHECKBOX,300,50,50,20, hWnd, (HMENU) 43,NULL,NULL);
		CreateWindow(TEXT("Button"),TEXT("LDA"),WS_VISIBLE|WS_CHILD|BS_CHECKBOX,300,80,50,20, hWnd, (HMENU) 44,NULL,NULL);
		CreateWindow(TEXT("Button"),TEXT("double match only"),WS_VISIBLE|WS_CHILD|BS_CHECKBOX,380,50,180,20, hWnd, (HMENU) 45,NULL,NULL);
		CreateWindow(TEXT("Button"),TEXT("half face"),WS_VISIBLE|WS_CHILD|BS_CHECKBOX,400,320,180,20, hWnd, (HMENU) 46,NULL,NULL);
		CheckDlgButton(hWnd, 15, BST_UNCHECKED);
		CheckDlgButton(hWnd, 16, BST_UNCHECKED);
		CheckDlgButton(hWnd, 12, BST_CHECKED);
		CheckDlgButton(hWnd, 43, BST_CHECKED);
		CheckDlgButton(hWnd, 45, BST_CHECKED);
	break;
	
	case WM_HSCROLL:// Trackbar action
		// Querying trackar values
		ZOOM=SendMessage(trcbar6,TBM_GETPOS,NULL,NULL);
		brightness=SendMessage(trcbar2,TBM_GETPOS,NULL,NULL);
		GREEN=SendMessage(trcbar3,TBM_GETPOS,NULL,NULL);
		BLUE=SendMessage(trcbar4,TBM_GETPOS,NULL,NULL);
		MASTER=SendMessage(trcbar5,TBM_GETPOS,NULL,NULL);
		Saturation=SendMessage(trcbar1,TBM_GETPOS,NULL,NULL);
		accuracy=SendMessage(accuracybar,TBM_GETPOS,NULL,NULL);
		accuracy=accuracy/100;
		sprintf(greeting,"%f",accuracy);
		sprintf(array,"%d",brightness);
		// Setting values
		SetDlgItemText(hWnd,20,greeting);
		SetDlgItemText(hWnd,21,array);
		cam1.SetSaturation(Saturation);
		cam1.SetRGB(MASTER,IS_IGNORE_PARAMETER,GREEN,BLUE);
		cam1.Zoom(&ZOOM);
		break;

	case WM_COMMAND:

		if(LOWORD(wParam)==12){//Reset Default unchecked
	    if (IsDlgButtonChecked(hWnd, 12)) {
	        CheckDlgButton(hWnd, 12, BST_UNCHECKED);
			EnableWindow(trcbar1,TRUE);
			EnableWindow(trcbar2,TRUE);
			EnableWindow(trcbar3,TRUE);
			EnableWindow(trcbar4,TRUE);
			EnableWindow(trcbar5,TRUE);
			EnableWindow(trcbar6,TRUE);
			EnableWindow(mirrorbox,TRUE);
			EnableWindow(mirrorbox2,TRUE);
			
	    } else {// Reset Default Checked
		 CheckDlgButton(hWnd, 12, BST_CHECKED);
			EnableWindow(trcbar1,FALSE);
			//EnableWindow(trcbar2,FALSE);
			EnableWindow(trcbar3,FALSE);
			EnableWindow(trcbar4,FALSE);
			EnableWindow(trcbar5,FALSE);
			EnableWindow(trcbar6,FALSE);
			EnableWindow(mirrorbox,FALSE);
			EnableWindow(mirrorbox2,FALSE);
			cam1.DefaultValues();// Reset default values
			
	    }
		}
		if(LOWORD(wParam)==15){// Brightness control unchecked
	    if (IsDlgButtonChecked(hWnd, 15)) {
	        CheckDlgButton(hWnd, 15, BST_UNCHECKED);
			brgControl=0;
			
	    } else {// Brightness control checked
		 CheckDlgButton(hWnd, 15, BST_CHECKED);
		 brgControl=1;
		}
		}
		if(LOWORD(wParam)==43){// PCA unchecked
	    if (IsDlgButtonChecked(hWnd, 43)) {
	        CheckDlgButton(hWnd, 43, BST_UNCHECKED);
			PCAeval=0;
			
	    } else {// PCA checked
		 CheckDlgButton(hWnd, 43, BST_CHECKED);
		 PCAeval=1;
		}
		}
		if(LOWORD(wParam)==44){// LDA unchecked
	    if (IsDlgButtonChecked(hWnd, 44)) {
	        CheckDlgButton(hWnd, 44, BST_UNCHECKED);
			LDAeval=0;
			
	    } else {// LDA checked
		 CheckDlgButton(hWnd, 44, BST_CHECKED);
		 LDAeval=1;
		}
		}
		if(LOWORD(wParam)==45){// double match unchecked
	    if (IsDlgButtonChecked(hWnd, 45)) {
	        CheckDlgButton(hWnd, 45, BST_UNCHECKED);
			DoubleMatch=0;
			
	    } else {// double match checked
		 CheckDlgButton(hWnd, 45, BST_CHECKED);
		 DoubleMatch=1;
		}
		}

		if(LOWORD(wParam)==46){// half face unchecked
	    if (IsDlgButtonChecked(hWnd, 46)) {
	        CheckDlgButton(hWnd, 46, BST_UNCHECKED);
			half_face_brightness=0;
			
	    } else {// half face checked
		 CheckDlgButton(hWnd, 46, BST_CHECKED);
		 half_face_brightness=1;
		}
		}

		if(LOWORD(wParam)==16){// Horizontol mirror unchecked
	    if (IsDlgButtonChecked(hWnd, 16)) {
	        CheckDlgButton(hWnd, 16, BST_UNCHECKED);
			cam1.Mirror(IS_SET_ROP_MIRROR_UPDOWN,0);
			
	    } else {// Horizontal mirror checked
		 CheckDlgButton(hWnd, 16, BST_CHECKED);
	    cam1.Mirror(IS_SET_ROP_MIRROR_UPDOWN,1);
		}
		}
		if(LOWORD(wParam)==22){// Test button
			CaptureFace(NULL);
		}

		if(LOWORD(wParam)==11){// OK button
			ShowWindow(hWnd,SW_HIDE);
			GetDlgItemText(hWnd,20,greeting,49);
			GetDlgItemText(hWnd,21,array,49);
			sscanf(greeting,"%f",&accuracy);
			sscanf(array,"%d",&brightness);
			
		}
	break;

	 case WM_PAINT:// Window texts
        hdc = BeginPaint(hWnd, &ps);
        TextOut(hdc,5, 5,"Saturation", _tcslen("Saturation"));
		TextOut(hdc,255, 225,"Brightness:", _tcslen("Brightness:"));
		TextOut(hdc,5, 75,"GREEN", _tcslen("GREEN"));
		TextOut(hdc,5, 145,"BLUE", _tcslen("BLUE"));
		TextOut(hdc,5, 255,"Master gain", _tcslen("Master gain"));
		TextOut(hdc,5, 210,"Zoom", _tcslen("Zoom"));
		TextOut(hdc,280, 20,"Algorithms used for final evaluation:", _tcslen("Algorithms used for final evaluation:"));
		TextOut(hdc,290, 120,"Recognition accuracy:", _tcslen("Recognition accuracy:"));
		TextOut(hdc,265, 150,"High", _tcslen("High"));
		TextOut(hdc,500, 150,"Low", _tcslen("Low"));
        EndPaint(hWnd, &ps);
        break;
	case WM_CLOSE:
		ShowWindow(hWnd,SW_HIDE);
		break;
	default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
}
// Function that contains the general initialisation of windows
HWND WindowInit(char* szWindowClass2,char* Title,int xsize,int ysize,WNDPROC ProcFunction,HWND ParentWindow, WNDCLASSEX* wcex){
	HWND hWndx=NULL;

	wcex->cbSize = sizeof(WNDCLASSEX);
    wcex->style          = CS_HREDRAW | CS_VREDRAW;
    wcex->lpfnWndProc    = ProcFunction;
    wcex->cbClsExtra     = 0;
    wcex->cbWndExtra     = 0;
    wcex->hInstance      = hInst;
    wcex->hIcon          = LoadIcon(hInst, MAKEINTRESOURCE(IDI_APPLICATION));
    wcex->hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex->hbrBackground  = (HBRUSH)(COLOR_BTNFACE+1);
    wcex->lpszMenuName   = NULL;
    wcex->lpszClassName  = szWindowClass2;
    wcex->hIconSm        = LoadIcon(wcex->hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	 if (!RegisterClassEx(wcex))
    {
        MessageBox(NULL,
            _T("Call to RegisterClassEx failed!"),
            _T("Win32 Guided Tour"),
            NULL);

        return NULL;
    }

	hWndx = CreateWindow(
        szWindowClass2,
        Title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        xsize,ysize,
        ParentWindow,
        NULL,
        hInst,
        NULL
    );

    if (!hWndx)
    {
        MessageBox(NULL,
            _T("Call to CreateWindow failed!"),
            _T("Win32 Guided Tour"),
            NULL);

        return NULL;
    }
	return hWndx;

}
//Function for  general trackbar initialisation
HWND TrackbarInit(char* name,int xpos,int ypos,int ID,HWND ParentWindow,int min,int max){
	HWND trackbar=NULL;
	InitCommonControls(); // loads common control's DLL 

	trackbar=CreateWindowEx(0,TRACKBAR_CLASS,"name",WS_CHILD | WS_VISIBLE |
        TBS_AUTOTICKS,xpos,ypos,200,30,ParentWindow,(HMENU)ID,hInst,NULL);

	SendMessage(trackbar, TBM_SETRANGE,
        (WPARAM) TRUE,                   // redraw flag
        (LPARAM) MAKELONG(min, max));  // min. & max. positions
    SendMessage(trackbar, TBM_SETPAGESIZE,
        0, (LPARAM) 4);                  // new page size

    SendMessage(trackbar, TBM_SETSEL,
        (WPARAM) FALSE,                  // redraw flag
        (LPARAM) MAKELONG(min, max));
    SendMessage(trackbar, TBM_SETPOS,
        (WPARAM) TRUE,                   // redraw flag
        (LPARAM) 1);
	EnableWindow(trackbar,FALSE);		// Trackbar is disabled default

    SetFocus(trackbar); 
	return trackbar;
}
char* LoadDistLog(char* filename){// Loads the distance log from file
	char* textbuf;
	FILE* Logfile=fopen(filename,"r");
	char Temp[3];
	unsigned int counter=0;
	unsigned int RowCounter=0;
	if(Logfile==NULL){
		textbuf="There's no Log file!";
		return textbuf;
	}
	while(!feof(Logfile)){
		if(getc(Logfile)=='\n') RowCounter++;
		counter++;
	}
	rewind(Logfile);
	textbuf=new char[counter+RowCounter];
	textbuf[0]=0;
	while(!feof(Logfile)){
		// LDA dist
			Temp[0]=getc(Logfile);
			if(Temp[0]=='\n'){
				Temp[0]='\r';
				Temp[1]='\n';
				Temp[2]=0;
			}
			else Temp[1]=0;
		
		if(!feof(Logfile)) strcat(textbuf,Temp);
	}
	fclose(Logfile);
	return textbuf;
}
int* ClassIDStat(int* IDs){
	int i;
	// final PCA/LDA result statistics
	int** classPopulations=new int*[6];
	for(i=0;i<6;i++){
		classPopulations[i]=new int[2]; //initial values of the array
		classPopulations[i][1]=0;	//this array contains the number of the corresponding ID
		classPopulations[i][0]=0;	//this array contains the IDs that occur int the first six results
	}
	//counting the number of each ID
	classPopulations[0][0]=IDs[0];
	int NumberOfIDs=0;
	for(i=0;i<6;i++){
		int j=0;
		for(int k=0;k<=NumberOfIDs;k++){
			if(classPopulations[k][0]==IDs[i]) j++;
		}
		if(j==0){
			NumberOfIDs++;
			classPopulations[NumberOfIDs][0]=IDs[i];
		}
	}
	for(i=0;i<=NumberOfIDs;i++){
		for(int j=0;j<6;j++){
			if(classPopulations[i][0]==IDs[j]) classPopulations[i][1]++;
		}
	}
	// computing the maximum of ID populations
	int* result=new int[3];
	result[0]=0; result[1]=0; result[2]=0;
	for(i=0;i<=NumberOfIDs;i++){
		if(classPopulations[i][1]>result[1]){
			result[1]=classPopulations[i][1];
			result[0]=classPopulations[i][0];
		}
	}
	// Searching for IDs with the same population (result[2] indicates this situation)
	for(i=0;i<6;i++){
			if((classPopulations[i][0]!=result[0])&&(classPopulations[i][1]==result[1])) result[2]=1;
	}

	char NameBuffer[100];
	facedata.GetName(IDs[0],NameBuffer,100,1);

	FILE* file=fopen("statistics.txt","a+");
	fprintf(file,"%c",'\n');
	for(i=0;(i<6)&&classPopulations[i][0];i++) fprintf(file,"%d %d\n",classPopulations[i][0],classPopulations[i][1]);
	fprintf(file,"Closest: %s\n",NameBuffer);
	if(result[2]) fprintf(file,"Most frequent: %d x %d\n Coincidence: YES\n",result[0],result[1]);
	else fprintf(file,"Most frequent: %d x %d\n Coincidence: NO\n",result[0],result[1]);
	fclose(file);
	return result;
}
int OverAllResult(int PCAen,int LDAen,int double_match_only){
	int* PCAiDs;
	int* LDAiDs;
	int* PCAstat;
	int* LDAstat;
	char buffer[100];
	if(facedata.GetMinDist()>accuracy) return -1;
	// Making statistics
	facedata.AscendingOrder(&LDAiDs,&PCAiDs);
	PCAstat=ClassIDStat(PCAiDs);
	LDAstat=ClassIDStat(LDAiDs);
	facedata.GetName(PCAstat[0],buffer,100,1);
	SetDlgItemText(MainWindow,18,buffer);
	facedata.GetName(LDAstat[0],buffer,100,1);
	SetDlgItemText(MainWindow,40,buffer);
	//Evaluation
	if((PCAen||double_match_only)&&(facedata.GetMinDist()<0.5)) return PCAiDs[0];
	if(double_match_only){
		if(PCAstat[2]&&LDAstat[2]) return -1;
		if(PCAstat[0]==LDAstat[0]) return PCAstat[0];
		return -1;
	}
	if(PCAen&&LDAen){
	if(!(PCAstat[2]||LDAstat[2])){
		if(PCAstat[1]>LDAstat[1])return PCAstat[0];
		if(PCAstat[1]<LDAstat[1])return LDAstat[0];
		if(PCAstat[1]==LDAstat[1])return PCAstat[0];
	}
	if((PCAstat[2]==1)&&(LDAstat[2]==0)) return LDAstat[0];
	if((PCAstat[2]==0)&&(LDAstat[2]==1)) return PCAstat[0];
	if(PCAstat[2]&&LDAstat[2]) return -1;
	}
	else{
		if(PCAen){
			if(PCAstat[2]==0) return PCAstat[0];
			return -1;
		}
		if(LDAen){
			if(LDAstat[2]==0) return LDAstat[0];
			return -1;
		}
	}
	return -1;
}
int getIDfromFilename(char* Path){
int i;
int counter=0;
int ID;
char fileID[20];
fileID[0]=0;
for(i=0;Path[i];i++){
	if((Path[i]=='\\')||(Path[i]=='/')) counter=i+1;
}
for(i=0;(counter<20)&&(Path[counter]!='.')&&Path[counter];counter++,i++){
	fileID[i]=Path[counter];
}
i=sscanf(fileID,"%d",&ID);
if(i) return ID;
return -1;
}

int CheckFace(HWND hWnd){
	IplImage* Image;
	IplImage* pFaceImg=NULL;
	char ActFileName[100];
	double mindist;
	int eyes_mouth;
		if(facedata.nElements>2){
			// Acquiring face position
			if(cam1.GetFacePos(&x,&y,&height,&width)==1){
			//Taking picture
			cam1.Acquire_picture();
			//Window refreshing
			SendMessage(hWnd3,WM_CLOSE,NULL,NULL);
			if(cam1.CheckReturnValue()!=IS_SUCCESS){
				MessageBox(hWnd,"Picture not Taken!","Button1",MB_SETFOREGROUND|MB_OK|MB_ICONERROR);
				return 0;
			}
			else{
			cam1.CaptureVideo();
			ShowWindow(hWnd3,SW_SHOW);
			UpdateWindow(hWnd3);
			cam1.Save_Image(L"Sample.bmp");

			//Storing Face information
			Image=ImagePreprocessor.CutPicture("Sample.bmp",x,y,height,width);
			//Histogram equalization
			cv::equalizeHist((cv::Mat)Image,(cv::Mat)Image);
			//Repositioning face upon the eyes and mouth
			eyes_mouth=ImagePreprocessor.EyeDetect(Image,&pFaceImg,0,0,300,300);
			//Brightness control
			if(brgControl&&pFaceImg){ 
				pFaceImg=ImagePreprocessor.Brightness(pFaceImg,brightness,0,half_face_brightness);
			}
	
			if(pFaceImg){ cvSaveImage("Sample.bmp",pFaceImg);
			PicturePath="Sample.bmp";
			//recognition
			// with PCA
			eigface.recognise("test.txt",facedata);
			//with LDA
			fisherface.recognise(eigface.projectedTestFace,facedata);

			facedata.SaveDist();
			mindist=facedata.GetMinDist();
			sprintf(ActFileName,"Mindist: %f",mindist);
			SetDlgItemText(hWnd,25,ActFileName);

			if((mindist<accuracy)&&eyes_mouth){
			ShowWindow(hWnd,SW_HIDE);
			ShowWindow(hWnd,SW_SHOW);
			UpdateWindow(hWnd);
			return 1;
			}
			else{
				ShowWindow(hWnd3,SW_HIDE);
				MessageBox(hWnd,"Inaccurate recognition!\nPlease hold your face in another position.","Error"
				,MB_SETFOREGROUND|MB_OK|MB_ICONEXCLAMATION);
				ShowWindow(hWnd3,SW_SHOW);
				UpdateWindow(hWnd3);
				ShowWindow(hWnd,SW_HIDE);
				ShowWindow(hWnd,SW_SHOW);
				UpdateWindow(hWnd);
				SetDlgItemText(hWnd,18,"Unknown");
				SetDlgItemText(hWnd,40,"Unknown");
				SetDlgItemText(hWnd,42,"Unknown");
			}
			return 0;
			}
			else{
				ShowWindow(hWnd3,SW_HIDE);
				MessageBox(hWnd,"Incorrect recognition!\nPlease hold your face in another position.","Error"
				,MB_SETFOREGROUND|MB_OK|MB_ICONEXCLAMATION);
				ShowWindow(hWnd3,SW_SHOW);
				UpdateWindow(hWnd3);
				ShowWindow(hWnd,SW_HIDE);
				ShowWindow(hWnd,SW_SHOW);
				UpdateWindow(hWnd);
				SetDlgItemText(hWnd,18,"Unknown");
				SetDlgItemText(hWnd,40,"Unknown");
				SetDlgItemText(hWnd,42,"Unknown");
			}
			}
			}
			else{
				if(cam1.GetFacePos(&x,&y,&height,&width)==-1) sprintf(greeting,"%s","No Face");
				if(cam1.GetFacePos(&x,&y,&height,&width)==0) sprintf(greeting,"%s","FD DISABLED");
				SetDlgItemText(hWnd,25,greeting);
			}
		}
		else{
				ShowWindow(hWnd3,SW_HIDE);
				MessageBox(hWnd,"Too few images are trained!\nPlease teach some more!","Error"
				,MB_SETFOREGROUND|MB_OK|MB_ICONEXCLAMATION);
				ShowWindow(hWnd3,SW_SHOW);
				UpdateWindow(hWnd3);
				SetDlgItemText(hWnd,18,"Unknown");
				SetDlgItemText(hWnd,40,"Unknown");
				SetDlgItemText(hWnd,42,"Unknown");
		}
			return 0;
}
void LearnFace(IplImage* Image,HWND hWnd){
	if(Image){
	char ActFileName[100];
	int ClassID;
	int nClasses;
	int* ClassPopulations;
	int* personIDs;
	//Storing Face information
	//Image1
	GetDlgItemText(hWnd2,39,ActFileName,100);
	sscanf(ActFileName,"%d",&ClassID);
	GetDlgItemText(hWnd2,38,Name,100);
	sprintf(ActFileName,"s2/%d.bmp",facedata.GetMaxID(0)+1);
	cvSaveImage(ActFileName,Image);
	facedata.NewFaceToClass(facedata.GetMaxID(0)+1,ActFileName,Name,ClassID);
	facedata.SaveList();
	if(facedata.nElements>2){
		eigface.learn("train.txt");
		facedata.ClassifyData(&ClassPopulations,&personIDs,&nClasses);
		fisherface.train(MainWindow,ClassPopulations,nClasses,personIDs);
		SetDlgItemText(hWnd,25,"Face learned");
	}
	}
}
	
IplImage* CaptureFace(HWND hWnd){
	IplImage* Image;
	IplImage* pFaceImg=0;
	int eyes_mouth=0;
	// Acquiring face position
	if(cam1.GetFacePos(&x,&y,&height,&width)==1){
	//Taking picture
	cam1.Acquire_picture();
	//Window refreshing
	SendMessage(hWnd3,WM_CLOSE,NULL,NULL);
	if(cam1.CheckReturnValue()!=IS_SUCCESS){ if(hWnd) MessageBox(hWnd,"Picture not Taken!","Button1",MB_OK|MB_ICONERROR);}
	else{
	cam1.CaptureVideo();
	ShowWindow(hWnd3,SW_SHOW);
	UpdateWindow(hWnd3);
	cam1.Save_Image(L"Sample.bmp");

	Image=ImagePreprocessor.CutPicture("Sample.bmp",x,y,height,width);
	//Histogram equalization
	cv::equalizeHist((cv::Mat)Image,(cv::Mat)Image);
	//Detect eyes and mouth
	eyes_mouth=ImagePreprocessor.EyeDetect(Image,&pFaceImg,0,0,300,300);
	//Brightness control
	if(brgControl&&pFaceImg){ 
		pFaceImg=ImagePreprocessor.Brightness(pFaceImg,brightness,0,half_face_brightness);
	}
	if(eyes_mouth){
		cvSaveImage("Sample.bmp",pFaceImg);
		EnableWindow(OK,TRUE); // Enabling OK button that learns the face
	}
	if((eyes_mouth==0)&&hWnd){
				ShowWindow(hWnd3,SW_HIDE);
				MessageBox(hWnd,"Incorrect recognition!\nPlease hold your face in another position.","Error"
				,MB_SETFOREGROUND|MB_OK|MB_ICONEXCLAMATION);
				ShowWindow(hWnd3,SW_SHOW);
				UpdateWindow(hWnd3);
				EnableWindow(OK,FALSE);
	}
	
		ShowWindow(hWnd2,SW_HIDE);
		ShowWindow(hWnd2,SW_SHOW);
		UpdateWindow(hWnd);
	
	}
	}
	else{
		if(hWnd){
		if(cam1.GetFacePos(&x,&y,&height,&width)==-1) sprintf(greeting,"%s","No Face");
		if(cam1.GetFacePos(&x,&y,&height,&width)==0) sprintf(greeting,"%s","FD DISABLED");
		SetDlgItemText(hWnd,25,greeting);
		}
		}
	return pFaceImg;	
}
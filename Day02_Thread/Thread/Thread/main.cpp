#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <iostream>
#include <time.h>

using namespace std;

DWORD WINAPI ThreadRun1(LPVOID Arg);
DWORD WINAPI ThreadRun2(LPVOID Arg);


long long num = 0;

CRITICAL_SECTION CriticalSection;

#define ThreadNumber 20

// CriticalSection	UserMode, 제일 가벼움, 기능이 적음
// Mutex				KernelMode, 중간 성능, 기능 추가
// Semaphore			KernelMode, 제일 무거움, 기능이 많음

int main()
{
	time_t start;
	time_t end;
	HANDLE* ThreadHandle;
	// ThreadHandle = (HANDLE*)malloc(sizeof(ThreadHandle) * ThreadNumber);
	ThreadHandle = new HANDLE[ThreadNumber];


	InitializeCriticalSection(&CriticalSection);


	start = time(NULL);
	cout << "first : " << num << endl;

	for (int i = 0; i < ThreadNumber; ++i)
	{
		if (i % 2 == 0)
		{
			ThreadHandle[i] = CreateThread(NULL, 0, ThreadRun1, NULL, 0, NULL);
		}
		else
		{
			ThreadHandle[i] = CreateThread(NULL, 0, ThreadRun2, NULL, 0, NULL);
		}
	}
	WaitForMultipleObjects(ThreadNumber, ThreadHandle, TRUE, INFINITE);
	end = time(NULL);

	cout << "end : " << num << endl;
	cout << "time : " << end - start << endl;

	DeleteCriticalSection(&CriticalSection);
	delete[] ThreadHandle;
}

DWORD WINAPI ThreadRun1(LPVOID Arg)
{
	EnterCriticalSection(&CriticalSection);
	for (int i = 0; i < 900000; ++i)
	{
		num++;
	}
	LeaveCriticalSection(&CriticalSection);
	return 0;
}

DWORD WINAPI ThreadRun2(LPVOID Arg)
{
	EnterCriticalSection(&CriticalSection);
	for (int i = 0; i < 900000; ++i)
	{
		num--;
	}
	LeaveCriticalSection(&CriticalSection);
	return 0;
}

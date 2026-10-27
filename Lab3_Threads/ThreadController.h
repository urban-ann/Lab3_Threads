#pragma once
#include <windows.h>
#include <vector>

struct MarkerData {
    int id;
    int* array;
    int size;
    HANDLE hStartEvent;
    HANDLE hContinueEvent;
    HANDLE hTerminateEvent;
    HANDLE hBlockedEvent;
    CRITICAL_SECTION* pCs;
    int markedCount;
    int failedIndex;
};

DWORD WINAPI MarkerThread(LPVOID lpParam);

class ThreadController {
private:
    int arraySize;
    int markerCount;
    int* arr;
    CRITICAL_SECTION cs;
    HANDLE* hMarkerThreads;
    HANDLE* hStartEvents;
    HANDLE* hContinueEvents;
    HANDLE* hTerminateEvents;
    HANDLE hBlockedEvent;
    MarkerData* markerDataArray;
    std::vector<bool> activeMarkers;

public:
    ThreadController(int size, int count);
    ~ThreadController();
    void Run();

    int GetArraySize() const { return arraySize; }
    int GetMarkerCount() const { return markerCount; }
    int* GetArray() { return arr; }
    bool SimulateMarkerStep(int index, int markerId);
};
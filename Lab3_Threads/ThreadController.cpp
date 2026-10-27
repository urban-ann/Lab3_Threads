#include "ThreadController.h"
#include <iostream>
#include <cstdlib>

DWORD WINAPI MarkerThread(LPVOID lpParam) {
    MarkerData* data = (MarkerData*)lpParam;
    srand(data->id);

    WaitForSingleObject(data->hStartEvent, INFINITE);

    while (true) {
        int rndVal = rand();
        int index = rndVal % data->size;

        EnterCriticalSection(data->pCs);
        if (data->array[index] == 0) {
            LeaveCriticalSection(data->pCs);

            Sleep(5);

            EnterCriticalSection(data->pCs);
            if (data->array[index] == 0) {
                data->array[index] = data->id;
                data->markedCount++;
                LeaveCriticalSection(data->pCs);

                Sleep(5);
                continue;
            }
            else {
                data->failedIndex = index;
                LeaveCriticalSection(data->pCs);
            }
        }
        else {
            data->failedIndex = index;
            LeaveCriticalSection(data->pCs);
        }

        std::cout << "Marker " << data->id << ": marked=" << data->markedCount
            << ", fail_index=" << data->failedIndex << std::endl;

        SetEvent(data->hBlockedEvent);

        HANDLE handles[] = { data->hContinueEvent, data->hTerminateEvent };
        DWORD waitResult = WaitForMultipleObjects(2, handles, FALSE, INFINITE);

        if (waitResult == WAIT_OBJECT_0 + 1) {
            EnterCriticalSection(data->pCs);
            for (int i = 0; i < data->size; ++i) {
                if (data->array[i] == data->id) {
                    data->array[i] = 0;
                }
            }
            LeaveCriticalSection(data->pCs);
            break;
        }
        else if (waitResult == WAIT_OBJECT_0) {
            ResetEvent(data->hContinueEvent);
        }
    }
    return 0;
}

ThreadController::ThreadController(int size, int count)
    : arraySize(size), markerCount(count), hBlockedEvent(NULL) {
    arr = new int[arraySize]();
    InitializeCriticalSection(&cs);

    hMarkerThreads = new HANDLE[markerCount];
    hStartEvents = new HANDLE[markerCount];
    hContinueEvents = new HANDLE[markerCount];
    hTerminateEvents = new HANDLE[markerCount];
    markerDataArray = new MarkerData[markerCount];

    hBlockedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    activeMarkers = std::vector<bool>(markerCount, true);

    for (int i = 0; i < markerCount; ++i) {
        hStartEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        hContinueEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
        hTerminateEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);

        markerDataArray[i] = {
            i + 1,
            arr,
            arraySize,
            hStartEvents[i],
            hContinueEvents[i],
            hTerminateEvents[i],
            hBlockedEvent,
            &cs,
            0,
            -1
        };

        hMarkerThreads[i] = CreateThread(NULL, 0, MarkerThread, &markerDataArray[i], 0, NULL);
    }
}

ThreadController::~ThreadController() {
    for (int i = 0; i < markerCount; ++i) {
        CloseHandle(hMarkerThreads[i]);
        CloseHandle(hStartEvents[i]);
        CloseHandle(hContinueEvents[i]);
        CloseHandle(hTerminateEvents[i]);
    }
    CloseHandle(hBlockedEvent);

    delete[] hMarkerThreads;
    delete[] hStartEvents;
    delete[] hContinueEvents;
    delete[] hTerminateEvents;
    delete[] markerDataArray;
    delete[] arr;

    DeleteCriticalSection(&cs);
}

void ThreadController::Run() {
    for (int i = 0; i < markerCount; ++i) {
        SetEvent(hStartEvents[i]);
    }

    int activeCount = markerCount;
    while (activeCount > 0) {
        WaitForSingleObject(hBlockedEvent, INFINITE);
        ResetEvent(hBlockedEvent);

        EnterCriticalSection(&cs);
        for (int i = 0; i < arraySize; ++i) {
            std::cout << arr[i] << " ";
        }
        std::cout << std::endl;
        LeaveCriticalSection(&cs);

        int targetId;
        std::cout << "Enter marker id to terminate: ";
        std::cin >> targetId;

        int targetIndex = -1;
        for (int i = 0; i < markerCount; ++i) {
            if (markerDataArray[i].id == targetId && activeMarkers[i]) {
                targetIndex = i;
                break;
            }
        }

        if (targetIndex != -1) {
            SetEvent(hTerminateEvents[targetIndex]);
            WaitForSingleObject(hMarkerThreads[targetIndex], INFINITE);
            activeMarkers[targetIndex] = false;
            activeCount--;

            EnterCriticalSection(&cs);
            for (int i = 0; i < arraySize; ++i) {
                std::cout << arr[i] << " ";
            }
            std::cout << std::endl;
            LeaveCriticalSection(&cs);

            for (int i = 0; i < markerCount; ++i) {
                if (activeMarkers[i]) {
                    SetEvent(hContinueEvents[i]);
                }
            }
        }
    }
}

bool ThreadController::SimulateMarkerStep(int index, int markerId) {
    EnterCriticalSection(&cs);
    if (arr[index] == 0) {
        arr[index] = markerId;
        LeaveCriticalSection(&cs);
        return true;
    }
    LeaveCriticalSection(&cs);
    return false;
}
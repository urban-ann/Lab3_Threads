#include "pch.h"
#include "../Lab3_Threads/ThreadController.h"

TEST(ThreadControllerTest, InitializationTest) {
    ThreadController controller(10, 2);
    EXPECT_EQ(controller.GetArraySize(), 10);
    EXPECT_EQ(controller.GetMarkerCount(), 2);

    int* arr = controller.GetArray();
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(arr[i], 0);
    }
}

TEST(ThreadControllerTest, CriticalSectionMarkingTest) {
    ThreadController controller(5, 1);
    bool marked = controller.SimulateMarkerStep(2, 1);
    EXPECT_TRUE(marked);

    int* arr = controller.GetArray();
    EXPECT_EQ(arr[2], 1);
}

TEST(ThreadControllerTest, ConflictMarkingTest) {
    ThreadController controller(5, 1);
    controller.SimulateMarkerStep(2, 1);

    bool markedAgain = controller.SimulateMarkerStep(2, 2);
    EXPECT_FALSE(markedAgain);

    int* arr = controller.GetArray();
    EXPECT_EQ(arr[2], 1);
}
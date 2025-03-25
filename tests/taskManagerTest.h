//
// Created by anton on 3/22/25.
//

#ifndef TASKWEAVER_TASKMANAGERTEST_H
#define TASKWEAVER_TASKMANAGERTEST_H

#include "../src/taskManager.h"
#include <gtest/gtest.h>

class TaskManagerTest : public testing::Test
{
public:
    TaskManagerTest() = default;

protected:
    void SetUp() override
    {
        taskManager_ = std::make_unique<taskweaver::TaskManager>();
        taskManager_->Start();
    }

    void TearDown() override
    {
        taskManager_->Stop();
        taskManager_.reset();
    }

private:
    std::unique_ptr<taskweaver::TaskManager> taskManager_;
};

#endif // TASKWEAVER_TASKMANAGERTEST_H

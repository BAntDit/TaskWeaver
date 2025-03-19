//
// Created by anton on 3/19/25.
//

#ifndef TASKWEAVER_TASKSTEALINGDEQUETEST_H
#define TASKWEAVER_TASKSTEALINGDEQUETEST_H

#include "../src/taskStealingDeque.h"
#include <gtest/gtest.h>

class TaskStealingDequeTest : public ::testing::Test
{
protected:
    void SetUp() override { deque = std::make_unique<taskweaver::TaskStealingDeque<int32_t>>(1024); }

    void TearDown() override { deque.reset(); }

    std::unique_ptr<taskweaver::TaskStealingDeque<int>> deque;
};

#endif // TASKWEAVER_TASKSTEALINGDEQUETEST_H

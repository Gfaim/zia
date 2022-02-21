#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "TSQueue.hpp"
#include "TSRequestOutputQueue.hpp"
#include "TSResponseInputQueue.hpp"

// Basic queue actions - push,pop,size,empty - no threads
TEST(ts_queues, queue_basics)
{
    zia::TSQueue<int> queue;

    queue.Push(1337);
    queue.Push(42069);
    queue.Push(80082);

    EXPECT_EQ(queue.Size(), 3);
    EXPECT_EQ(queue.Front(), 1337);

    auto leet = queue.Pop();

    EXPECT_EQ(queue.Size(), 2);
    EXPECT_EQ(queue.Empty(), false);
    EXPECT_EQ(leet.has_value(), true);
    EXPECT_EQ(leet.value(), 1337);

    queue.Clear();

    EXPECT_EQ(queue.Size(), 0);
    EXPECT_EQ(queue.Empty(), true);
    EXPECT_EQ(queue.Pop(), std::nullopt);
}

TEST(ts_queues, queue_wait)
{
    zia::TSQueue<short> queue;
    std::thread waiting_thread_test([&queue]() {
        EXPECT_EQ(queue.Empty(), true);
        queue.Wait();
        EXPECT_EQ(queue.Empty(), false);
    });

    {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(0.1s);
    }

    queue.Push(-4321);

    waiting_thread_test.join();
}

TEST(ts_queues, queue_race)
{
    zia::TSQueue<short> queue;
    auto action_x_time = [](const unsigned int &times, std::function<void()> action) {
        for (unsigned int i = 0; i < times; i++) {
            action();
        }
    };
    const unsigned int pushes = 10000;
    const unsigned int pops = 9999;
    unsigned int final_size = pushes - pops;

    std::thread push_a_lot_thread(action_x_time, 10000, [&queue]() { queue.Push(std::rand() % 100); });
    std::thread pop_a_lot_thread(action_x_time, 9999, [&queue, &final_size]() {
        if (queue.Pop() == std::nullopt)
            final_size++;
    });

    push_a_lot_thread.join();
    pop_a_lot_thread.join();
    EXPECT_EQ(queue.Size(), final_size);
}

TEST(ts_queues, request_queue_implem)
{
    using namespace ziapi::http;
    zia::TSRequestOutputQueue queue;

    Request req{};
    Request req_v1{};
    Context ctx{};

    req.version = Version::kV2;
    req_v1.version = Version::kV1;
    queue.Push(std::make_pair(req, ctx));
    queue.Push(std::make_pair(req, ctx));
    queue.Push(std::make_pair(req_v1, ctx));
    queue.Push(std::make_pair(req, ctx));

    EXPECT_EQ(queue.Size(), 4);

    queue.Pop();
    queue.Pop();
    auto v1 = queue.Pop();
    auto v2 = queue.Pop();

    EXPECT_EQ(v1->first.version, Version::kV1);
    EXPECT_EQ(v2->first.version, Version::kV2);
    EXPECT_EQ(queue.Size(), 0);
}

TEST(ts_queues, response_queue_implem)
{
    using namespace ziapi::http;
    zia::TSResponseInputQueue queue;

    Response res{};
    Response res_v1{};
    Context ctx{};

    res.version = Version::kV2;
    res_v1.version = Version::kV1;
    queue.Push(std::make_pair(res, ctx));
    queue.Push(std::make_pair(res, ctx));
    queue.Push(std::make_pair(res_v1, ctx));
    queue.Push(std::make_pair(res, ctx));

    EXPECT_EQ(queue.Size(), 4);

    queue.Pop();
    queue.Pop();
    auto v1 = queue.Pop();
    auto v2 = queue.Pop();

    EXPECT_EQ(v1->first.version, Version::kV1);
    EXPECT_EQ(v2->first.version, Version::kV2);
    EXPECT_EQ(queue.Empty(), true);
}

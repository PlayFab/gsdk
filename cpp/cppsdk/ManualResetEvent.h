// Copyright (C) Microsoft Corporation. All rights reserved.

#pragma once

#include <chrono>
#include <mutex>
#include <condition_variable>

namespace Microsoft
{
    namespace Azure
    {
        namespace Gaming
        {
            class ManualResetEvent
            {
            private:
                bool m_isGateOpen;
                std::mutex m_mutex;
                std::condition_variable m_condition;

            public:
                ManualResetEvent();
                ~ManualResetEvent();

                // If the gate is closed, this blocks until signal is called, without a timeout.
                // If the gate is open, then no blocking occurs.
                void Wait();

                // If the gate is closed, this blocks until either a signal is called or we timeout.
                // Returns false if there was a timeout, true if the event was signaled.
                // If the gate is open, then no blocking occurs and we return true.
                bool Wait(unsigned long milliseconds);

                // Unblocks any waits and sets the gate to open
                void Signal();

                // Closes the gate so new waits get blocked again
                void Reset();
            };

        }
    }
}
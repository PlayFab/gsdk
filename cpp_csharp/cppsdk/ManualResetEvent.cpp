#include "gsdkCommonPch.h"
#include "ManualResetEvent.h"

namespace Microsoft
{
    namespace Azure
    {
        namespace Gaming
        {
            ManualResetEvent::ManualResetEvent() : m_isGateOpen(false)
            {
            }


            ManualResetEvent::~ManualResetEvent()
            {
            }

            void ManualResetEvent::Wait()
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_condition.wait(lock, [&]() -> bool { return m_isGateOpen; });
            }

            bool ManualResetEvent::Wait(unsigned long milliseconds)
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                return m_condition.wait_for(lock, std::chrono::milliseconds(milliseconds), [&]() -> bool { return m_isGateOpen; });
            }

            void ManualResetEvent::Signal()
            {
                m_mutex.lock();
                m_isGateOpen = true;
                m_mutex.unlock();
                m_condition.notify_all();
            }

            void ManualResetEvent::Reset()
            {
                m_mutex.lock();
                m_isGateOpen = false;
                m_mutex.unlock();
            }

        }
    }
}
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0
#include "iceoryx_hoofs/posix_wrapper/signal_watcher.hpp"
#include "iceoryx_hoofs/cxx/helplets.hpp"
#include "iceoryx_hoofs/internal/log/hoofs_logging.hpp"
#include "iceoryx_hoofs/platform/unistd.hpp"

namespace iox
{
namespace posix
{
void internalSignalHandler(int) noexcept
{
    auto& instance = SignalWatcher::getInstance();
    instance.m_hasSignalOccurred.store(true);

    for (uint64_t remainingNumberOfWaiters = instance.m_numberOfWaiters.load(); remainingNumberOfWaiters > 0;
         --remainingNumberOfWaiters)
    {
        if (instance.m_semaphore->post().has_error())
        {
            // we use write since internalSignalHandler can be called from within a
            // signal handler and write is signal safe
            constexpr const char MSG[] = "Unable to increment semaphore in signal handler";
            auto result = write(STDERR_FILENO, MSG, strlen(MSG));
            IOX_DISCARD_RESULT(result);
            std::abort();
        }
    }
}

SignalWatcher::SignalWatcher() noexcept
    : m_sigTermGuard(registerSignalHandler(Signal::TERM, internalSignalHandler))
    , m_sigIntGuard(registerSignalHandler(Signal::INT, internalSignalHandler))
{
    UnnamedSemaphoreBuilder()
        .isInterProcessCapable(false)
        .create(m_semaphore)

        // This can be safely used despite getInstance is used in the internalSignalHandler
        // since this object has to be created first before internalSignalHandler can be called.
        // The only way this object can be created is by calling getInstance.
        .expect("Unable to create semaphore for signal watcher");
}

SignalWatcher& SignalWatcher::getInstance() noexcept
{
    static SignalWatcher instance;
    return instance;
}

void SignalWatcher::waitForSignal() const noexcept
{
    ++m_numberOfWaiters;
    if (m_hasSignalOccurred.load())
    {
        return;
    }

    m_semaphore->wait().expect("Unable to wait on semaphore in signal watcher");
}

bool SignalWatcher::wasSignalTriggered() const noexcept
{
    return m_hasSignalOccurred.load();
}

void waitForTerminationRequest() noexcept
{
    SignalWatcher::getInstance().waitForSignal();
}

bool hasTerminationRequested() noexcept
{
    return SignalWatcher::getInstance().wasSignalTriggered();
}
} // namespace posix
} // namespace iox

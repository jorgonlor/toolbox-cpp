// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2021 Reactive Markets Limited
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

#include "Thread.hpp"

#include <toolbox/sys/Error.hpp>
#include <toolbox/util/String.hpp>
#include <toolbox/util/Tokeniser.hpp>

namespace toolbox {
inline namespace sys {
using namespace std;
namespace {
pair<int, int> split_range(string_view s) noexcept
{
    auto [first, last] = split_pair(s, '-');
    const auto i = ston<int>(first);
    return {i, last.empty() ? i : ston<int>(last)};
}
} // namespace

cpu_set_t parse_cpu_set(string_view s) noexcept
{
    cpu_set_t bs;
    CPU_ZERO(&bs);

    Tokeniser toks{s, ","sv};
    while (!toks.empty()) {
        auto [i, j] = split_range(toks.top());
        for (; i <= j; ++i) {
            CPU_SET(i, &bs);
        }
        toks.pop();
    }
    return bs;
}

int parse_sched_policy(string_view s)
{
    int policy;
    if (s == "other"sv) {
        policy = SCHED_OTHER;
    } else if (s == "fifo"sv) {
        policy = SCHED_FIFO;
    } else if (s == "rr"sv) {
        policy = SCHED_RR;
    } else if (s == "batch"sv) {
        policy = SCHED_BATCH;
    } else if (s == "idle"sv) {
        policy = SCHED_IDLE;
    } else {
        throw invalid_argument{"invalid sched policy"};
    }
    return policy;
}

void set_thread_attrs(const ThreadConfig& config)
{
    const auto tid = pthread_self();
    if (!config.name.empty()) {
        if (const auto err = pthread_setname_np(tid, config.name.c_str()); err != 0) {
            throw system_error{make_error(err), "pthread_setname_np"};
        }
    }
    if (!config.affinity.empty()) {
        const auto bs = parse_cpu_set(config.affinity);
        if (const auto err = pthread_setaffinity_np(tid, sizeof(bs), &bs); err != 0) {
            throw system_error{make_error(err), "pthread_setaffinity_np"};
        }
    }
    if (!config.sched_policy.empty()) {
        const auto policy = parse_sched_policy(config.sched_policy);
        const auto priority = sched_get_priority_max(policy);
        if (priority == -1) {
            throw system_error{make_error(priority), "sched_get_priority_max"};
        }
        sched_param param{.sched_priority = priority};
        if (const auto err = pthread_setschedparam(tid, policy, &param); err != 0) {
            throw system_error{make_error(err), "pthread_setschedparam"};
        }
    }
    config.init_fn();
}

} // namespace sys
} // namespace toolbox

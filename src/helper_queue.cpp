/*
   helper_queue.cpp: マルチスレッド探索用のスレッドのキューの実装。

   The MIT License (MIT)

   Copyright (c) 2013 Ishibashi Hironori

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to
   deal in the Software without restriction, including without limitation the
   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
   sell copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
   IN THE SOFTWARE.
*/

#include "helper_queue.h"

#include <iostream>
#include <mutex>
#include <condition_variable>
#include "job.h"

namespace Sayuri {
  /********************/
  /* パブリック関数。 */
  /********************/
  // 空きスレッドが仕事を得る。
  Job* HelperQueue::GetJob() {
    std::unique_lock<std::mutex> lock(mutex_);

    // 待つ。
    cond_.wait(lock);

    return job_ptr_;
  }

  // 空きスレッドに仕事を依頼する。
  void HelperQueue::Help(Job& job) {
    job_ptr_ = &job;
    cond_.notify_one();
  }

  // 空きスレッドをキューから開放する。
  void HelperQueue::ReleaseHelper() {
    job_ptr_ = nullptr;
    cond_.notify_all();
  }
}  // namespace Sayuri

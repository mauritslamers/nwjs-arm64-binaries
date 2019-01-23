// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "platform/scheduler/child/webthread_impl_for_worker_scheduler.h"

#include <memory>
#include "base/bind.h"
#include "base/location.h"
#include "base/single_thread_task_runner.h"
#include "base/synchronization/waitable_event.h"
#include "base/time/default_tick_clock.h"
#include "platform/scheduler/base/task_queue.h"
#include "platform/scheduler/child/web_scheduler_impl.h"
#include "platform/scheduler/child/web_task_runner_impl.h"
#include "platform/scheduler/child/worker_scheduler_impl.h"
#include "public/platform/WebTraceLocation.h"

#include "third_party/node-nw/src/node_webkit.h"
#define PLATFORM_HOOK_MAP(type, sym, fn) PLATFORM_EXPORT type fn = nullptr;
#if defined(COMPONENT_BUILD) && defined(WIN32)
#define NW_HOOK_MAP(type, sym, fn) BASE_EXPORT type fn;
#else
#define NW_HOOK_MAP(type, sym, fn) extern type fn;
#endif
#include "content/nw/src/common/node_hooks.h"
#undef NW_HOOK_MAP

namespace blink {
namespace scheduler {

WebThreadImplForWorkerScheduler::WebThreadImplForWorkerScheduler(
    const char* name)
    : WebThreadImplForWorkerScheduler(name, base::Thread::Options()) {}

WebThreadImplForWorkerScheduler::WebThreadImplForWorkerScheduler(
    const char* name,
    base::Thread::Options options)
    : thread_(new base::Thread(name ? name : std::string())) {
  if (g_web_worker_thread_new_fn)
    (*g_web_worker_thread_new_fn)((void*)name, &options);
  bool started = thread_->StartWithOptions(options);
  CHECK(started);
  thread_task_runner_ = thread_->task_runner();
}

void WebThreadImplForWorkerScheduler::Init() {
  base::WaitableEvent completion(
      base::WaitableEvent::ResetPolicy::AUTOMATIC,
      base::WaitableEvent::InitialState::NOT_SIGNALED);
  thread_task_runner_->PostTask(
      FROM_HERE, base::Bind(&WebThreadImplForWorkerScheduler::InitOnThread,
                            base::Unretained(this), &completion));
  completion.Wait();
}

WebThreadImplForWorkerScheduler::~WebThreadImplForWorkerScheduler() {
  // We want to avoid blocking main thread when the thread was already
  // shut down, but calling ShutdownOnThread twice does not cause any problems.
  if (!was_shutdown_on_thread_.IsSet()) {
    base::WaitableEvent completion(
        base::WaitableEvent::ResetPolicy::AUTOMATIC,
        base::WaitableEvent::InitialState::NOT_SIGNALED);
    thread_task_runner_->PostTask(
        FROM_HERE,
        base::Bind(&WebThreadImplForWorkerScheduler::ShutdownOnThread,
                   base::Unretained(this), &completion));
    completion.Wait();
  }
  thread_->Stop();
}

void WebThreadImplForWorkerScheduler::InitOnThread(
    base::WaitableEvent* completion) {
  // TODO(alexclarke): Do we need to unify virtual time for workers and the
  // main thread?
  worker_scheduler_ = CreateWorkerScheduler();
  worker_scheduler_->Init();
  task_queue_ = worker_scheduler_->DefaultTaskQueue();
  idle_task_runner_ = worker_scheduler_->IdleTaskRunner();
  web_scheduler_.reset(new WebSchedulerImpl(
      worker_scheduler_.get(), worker_scheduler_->IdleTaskRunner(),
      worker_scheduler_->DefaultTaskQueue(),
      worker_scheduler_->DefaultTaskQueue(),
      worker_scheduler_->DefaultTaskQueue()));
  base::MessageLoop::current()->AddDestructionObserver(this);
  web_task_runner_ = WebTaskRunnerImpl::Create(task_queue_, base::nullopt);
  completion->Signal();
}

void WebThreadImplForWorkerScheduler::ShutdownOnThread(
    base::WaitableEvent* completion) {
  was_shutdown_on_thread_.Set();

  task_queue_ = nullptr;
  idle_task_runner_ = nullptr;
  web_scheduler_ = nullptr;
  worker_scheduler_ = nullptr;
  web_task_runner_ = nullptr;

  if (completion)
    completion->Signal();
}

std::unique_ptr<WorkerScheduler>
WebThreadImplForWorkerScheduler::CreateWorkerScheduler() {
  return WorkerScheduler::Create();
}

void WebThreadImplForWorkerScheduler::WillDestroyCurrentMessageLoop() {
  ShutdownOnThread(nullptr);
}

blink::PlatformThreadId WebThreadImplForWorkerScheduler::ThreadId() const {
  return thread_->GetThreadId();
}

blink::WebScheduler* WebThreadImplForWorkerScheduler::Scheduler() const {
  return web_scheduler_.get();
}

scoped_refptr<base::SingleThreadTaskRunner>
WebThreadImplForWorkerScheduler::GetTaskRunner() const {
  return task_queue_;
}

SingleThreadIdleTaskRunner* WebThreadImplForWorkerScheduler::GetIdleTaskRunner()
    const {
  return idle_task_runner_.get();
}

blink::WebTaskRunner* WebThreadImplForWorkerScheduler::GetWebTaskRunner() {
  return web_task_runner_.Get();
}

void WebThreadImplForWorkerScheduler::AddTaskObserverInternal(
    base::MessageLoop::TaskObserver* observer) {
  worker_scheduler_->AddTaskObserver(observer);
}

void WebThreadImplForWorkerScheduler::RemoveTaskObserverInternal(
    base::MessageLoop::TaskObserver* observer) {
  worker_scheduler_->RemoveTaskObserver(observer);
}

}  // namespace scheduler
}  // namespace blink
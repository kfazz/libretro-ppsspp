// Copyright (c) 2013- PPSSPP Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0 or later versions.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official git repository and contact information can be found at
// https://github.com/hrydgard/ppsspp and http://www.ppsspp.org/.

#pragma once

#include "ext/native/base/mutex.h"
#include "Core/System.h"
#include "Core/CoreTiming.h"
#include <deque>

template <typename B, typename Event, typename EventType, EventType EVENT_INVALID, EventType EVENT_SYNC, EventType EVENT_FINISH>
struct ThreadEventQueue : public B {
	ThreadEventQueue() : threadEnabled_(false), eventsRunning_(false), eventsHaveRun_(false) {
	}

	void SetThreadEnabled(bool threadEnabled) {
	}

	bool ThreadEnabled() {
		return false;
	}

	void ScheduleEvent(Event ev) {
      events_.push_back(ev);

      RunEventsUntil(0);
	}

	bool HasEvents() {
      return !events_.empty();
	}

	void NotifyDrain() {
	}

	Event GetNextEvent() {
			if (events_.empty())
				return EVENT_INVALID;
			Event ev = events_.front();
			events_.pop_front();
			return ev;
	}

	void RunEventsUntil(u64 globalticks) {
		lock_guard guard(eventsLock_);
		eventsRunning_ = true;
		eventsHaveRun_ = true;
		do {
			while (!HasEvents() && !ShouldExitEventLoop()) {
				eventsWait_.wait(eventsLock_);
			}
			// Quit the loop if the queue is drained and coreState has tripped, or threading is disabled.
			if (!HasEvents()) {
				break;
			}

			for (Event ev = GetNextEvent(); EventType(ev) != EVENT_INVALID; ev = GetNextEvent()) {
				eventsLock_.unlock();
				ProcessEventIfApplicable(ev, globalticks);
				eventsLock_.lock();
			}
		} while (CoreTiming::GetTicks() < globalticks);

		// This will force the waiter to check coreState, even if we didn't actually drain.
		NotifyDrain();
		eventsRunning_ = false;
	}

	void SyncBeginFrame() {
			eventsHaveRun_ = false;
	}

	inline bool ShouldSyncThread(bool force) {
		if (!HasEvents())
			return false;
		if (coreState != CORE_RUNNING && !force)
			return false;

		// Don't run if it's not running, but wait for startup.
		if (!eventsRunning_) {
			if (eventsHaveRun_ || coreState == CORE_ERROR || coreState == CORE_POWERDOWN) {
				return false;
			}
		}

		return true;
	}

	// Force ignores coreState.
	void SyncThread(bool force = false) {
	}

	void FinishEventLoop() {
	}

protected:
	virtual void ProcessEvent(Event ev) = 0;
	virtual bool ShouldExitEventLoop() = 0;

	inline void ProcessEventIfApplicable(Event &ev, u64 &globalticks) {
		switch (EventType(ev)) {
		case EVENT_FINISH:
			// Stop waiting.
			globalticks = 0;
			break;

		case EVENT_SYNC:
			break;

		default:
			ProcessEvent(ev);
		}
	}

private:
	bool threadEnabled_;
	bool eventsRunning_;
	bool eventsHaveRun_;
	std::deque<Event> events_;
	recursive_mutex eventsLock_;
	condition_variable eventsWait_;
	condition_variable eventsDrain_;
};

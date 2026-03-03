/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// Job scheduler - manages periodic tasks
// subsim (C)+(W) Thorsten Jordan. SEE LICENSE

#ifndef JOB_SCHEDULER_H
#define JOB_SCHEDULER_H

#include <list>
#include <utility>

/// Interface for periodic tasks
struct job {
    job() {}
    virtual void run() = 0;
    virtual double get_period() const = 0;
    virtual ~job() {}
};

/// Manages periodic task execution (jobs)
class job_scheduler {
  private:
    std::list<std::pair<double, job *>> jobs; ///< Jobs with accumulated time

  public:
    job_scheduler() = default;
    ~job_scheduler();

    // Non-copyable
    job_scheduler(const job_scheduler &) = delete;
    job_scheduler &operator=(const job_scheduler &) = delete;

    /// Register a new job (scheduler takes ownership)
    void register_job(job *j);

    /// Unregister and delete a job
    void unregister_job(job *j);

    /// Update all jobs and execute those whose period has elapsed
    /// @param delta_t - time elapsed since last update
    void update(double delta_t);

    /// Get number of registered jobs
    size_t job_count() const { return jobs.size(); }

    /// Check if there are any jobs
    bool has_jobs() const { return !jobs.empty(); }
};

#endif

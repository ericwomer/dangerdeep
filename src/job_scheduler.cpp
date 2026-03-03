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

#include "job_scheduler.h"
#include "error.h"

job_scheduler::~job_scheduler() {
    for (auto &job_pair : jobs) {
        delete job_pair.second;
    }
}

void job_scheduler::register_job(job *j) {
    jobs.push_back(std::make_pair(0.0, j));
}

void job_scheduler::unregister_job(job *j) {
    for (auto it = jobs.begin(); it != jobs.end(); ++it) {
        if (it->second == j) {
            delete it->second;
            jobs.erase(it);
            return;
        }
    }
    throw error("[job_scheduler::unregister_job] job not found in list");
}

void job_scheduler::update(double delta_t) {
    for (auto &job_pair : jobs) {
        job_pair.first += delta_t;
        if (job_pair.first >= job_pair.second->get_period()) {
            job_pair.first -= job_pair.second->get_period();
            job_pair.second->run();
        }
    }
}

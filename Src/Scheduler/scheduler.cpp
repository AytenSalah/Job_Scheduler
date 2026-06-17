#include <thread>
#include <chrono>
#include <ctime>
#include <iostream>
#include "Scheduler/Scheduler.h"
#include "Service/JobService.h"
#include "service.h"
using namespace std;

void scheduler_loop(sqlite3 *DB, std::atomic<bool> &running)
{
    while (running)
    {
        long long now = time(nullptr);
        vector<Job> due_jobs get_due_jobs(DB , now);
        for(Job& job : due_jobs)
        {
            cout<<"Job"<< job.name <<endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
}
#include "Job.h"

Job_Type int_to_type(int number)
{
    switch (number)
    {
    case 0:
        return ONE_TIME;
        break;
    case 1:
        return RECURRING;
        break;
    case 2:
        return CRON;
        break;
    default:
        return ONE_TIME;
        break;
    }
}

Status int_to_status(int number)
{
    switch (number) {
    case 0:
        return ACTIVE;
        break;
    case 1:
        return PAUSED;
        break;
    case 2:
        return RUNNING;
        break;
    case 3:
        return COMPLETED;
        break;
    case 4:
        return FAILED;
        break;
    case 5:
        return CANCELED;
        break;
    default:
        return ACTIVE;
        break;
    }
}

int status_to_int(Status status) {
    switch (status) {
    case ACTIVE:
        return 0;
        break;
    case PAUSED:
        return 1;
        break;
    case RUNNING:
        return 2;
        break;
    case COMPLETED:
        return 3;
        break;
    case FAILED:
        return 4;
        break;
    case CANCELED:
        return 5;
        break;
    default:
        return 0;
        break;
    }
}

string type_to_string(Job_Type job_type)
{
    switch (job_type)
    {
    case ONE_TIME:
        return "ONE_TIME";
        break;
    case RECURRING:
        return "RECURRING";
        break;
    case CRON:
        return "CRON";
    default:
        return "UNKNOWN";
        break;
    }
}

string status_to_string(Status status)
{
    switch (status)
    {
    case ACTIVE:
        return "active";
        break;
    case PAUSED:
        return "paused";
        break;
    case RUNNING:
        return "running";
        break;
    case COMPLETED:
        return "completed";
        break;
    case FAILED:
        return "failed";
        break;
    case CANCELED:
        return "canceled";
        break;
    default:
        return "UNKNOWN";
        break;
    }
}

void print_job_info(const Job& job)
{
    cout << "===== Object Information =====\n";

    cout << "ID: " << job.id << "\n";

    cout << "Name: " << job.name << "\n";

    cout << "Type: " << type_to_string(job.type) << "\n";

    cout << "Status: " << status_to_string(job.status) << "\n";

    cout << "Next Run Time: " << job.next_run_time << "\n";

    cout << "Interval Seconds: "
        << (job.interval_seconds.has_value()
            ? to_string(job.interval_seconds.value())
            : "NULL")
        << "\n";

    cout << "Cron Expr: "
        << (job.cron_expr.has_value()
            ? job.cron_expr.value()
            : "NULL")
        << "\n";

    cout << "Payload: " << job.payload << "\n";
    cout << "Retry Count: " << job.retry_count << "\n";
    cout << "Max Retries: " << job.max_retries << "\n";
    cout << "=============================\n";
}

void print_all_jobs(vector<Job> jobs)
{
    cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << endl;
    for (Job job : jobs)
    {
        print_job_info(job);
    }
    cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << endl;
}

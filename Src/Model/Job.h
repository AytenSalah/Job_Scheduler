#pragma once
#include <iostream>
#include <vector>
#include <optional>
#include "sqlite3.h"
#include "crow/crow_all.h"
using namespace std;

enum Status { ACTIVE, PAUSED, RUNNING, COMPLETED, FAILED, CANCELED };
enum Job_Type { ONE_TIME, RECURRING, CRON };

struct Job
{
    int id;
    string name;
    Job_Type type;
    Status status;
    long long next_run_time;

    optional<int> interval_seconds;
    optional<string> cron_expr;

    string payload;

    int retry_count = 0;
    int max_retries = 3;
};

Job_Type int_to_type(int number);
Status int_to_status(int number);
int status_to_int(Status status);
string type_to_string(Job_Type job_type);
string status_to_string(Status status);

void print_job_info(const Job& job);
void print_all_jobs(vector<Job> jobs);
Job create_job_from_json(const crow::json::rvalue& body);
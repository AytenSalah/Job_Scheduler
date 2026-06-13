#pragma once
#include <iostream>
#include <vector>
#include <optional>
#include "sqlite3.h"
#include "Job.h"

using namespace std;

int create_DB(const char* filename, sqlite3** DB);
int create_table(sqlite3* DB);

int insert_job(sqlite3* DB, const Job& job);
vector<Job> select_all_jobs(sqlite3* DB);
bool get_job_by_id(sqlite3* DB, Job& job, int id);
bool update_job_status(sqlite3* DB, int id, Status newStatus, Job& job);
bool delete_job(sqlite3* DB, int id);
bool update_job(sqlite3* DB, Job job, int id);
bool pause_job(sqlite3* DB, int id);
bool resume_job(sqlite3* DB, int id);
bool update_next_run_time(sqlite3* DB, int id, long long new_next_run_time);
bool increment_retry_count(sqlite3* DB, int id);
bool reset_retry_count(sqlite3* DB, int id);
#pragma once
#include <iostream>
#include <vector>
#include "sqlite3.h"
#include "crow_all.h"
#include "database.h"
#include "Job.h"
#include "service.h"

using namespace std;

// routes handlers
void root_route(const crow::request& req, crow::response& res);

void create_task_route(sqlite3* DB, const crow::request& req, crow::response& res);

crow::response delete_task_route(sqlite3* DB, int id);

void get_all_jobs_route(sqlite3* DB, crow::response& res);

std::string get_job_by_id_route(sqlite3* DB, int id);

std::string pause_job_route(sqlite3* DB, int id);

std::string resume_job_route(sqlite3* DB, int id);

// register
void register_routes(crow::SimpleApp& app, sqlite3* DB);
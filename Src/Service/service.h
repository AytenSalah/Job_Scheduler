#pragma once
#include "crow/crow_all.h"
#include <string>
#include <optional>
#include <ctime>
#include "Job.h"

//AC7: Invalid schedule input is rejected with a clear error response.
bool validate_job_input(const crow::json::rvalue& body, std::string& out_error);

//AC2: Creating a valid recurring, cron-like, or one_time job stores the correct next run time.
long long compute_next_run_time(int type, long long now,
                                 std::optional<int> interval_seconds,
                                 std::optional<std::string> cron_expr,
                                 long long client_next_run_time);

Job create_job(sqlite3* DB, const crow::json::rvalue& body, std::string& out_error);

bool is_valid_cron(const std::string& cron_expr, std::string& out_error);
long long compute_cron_next_run(const std::string& cron_expr, long long now);
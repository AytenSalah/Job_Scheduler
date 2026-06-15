#include "database/database.h"

int create_DB(const char* filename, sqlite3** DB)
{
    int rc = sqlite3_open(filename, DB);
    if (rc == SQLITE_OK)
    {
        cout << "Database opened successfuly" << endl;
    }
    else
    {
        cout << "failed to be opened : " << sqlite3_errmsg(*DB) << endl;
    }
    return rc;
}

int create_table(sqlite3* DB)
{
    char* error_message = nullptr;

    const char* sql =
        "CREATE TABLE IF NOT EXISTS jobs ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL, "
        "type INTEGER NOT NULL, "
        "status INTEGER NOT NULL, "
        "next_run_time INTEGER NOT NULL, "
        "interval_seconds INTEGER, "
        "cron_expr TEXT, "
        "payload TEXT NOT NULL, "
        "retry_count INTEGER NOT NULL, "
        "max_retries INTEGER NOT NULL"
        ");";

    int rc2 = sqlite3_exec(DB, sql, nullptr, nullptr, &error_message);

    if (rc2 != SQLITE_OK)
    {
        cout << "Failed to create table: " << error_message << endl;
        sqlite3_free(error_message);
        return rc2;
    }

    cout << "Table created successfully\n";

    return SQLITE_OK;
}

int insert_job(sqlite3* DB, const Job& job)
{
    sqlite3_stmt* stmt;
    const char* sql =
        "INSERT INTO jobs (name, type, status, next_run_time, interval_seconds, cron_expr, payload, retry_count, max_retries) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";

    int rc = sqlite3_prepare_v2(DB, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK)
    {
        cout << "Prepare failed: " << sqlite3_errmsg(DB) << endl;
        return rc;
    }

    sqlite3_bind_text(stmt, 1, job.name.c_str(), -1, reinterpret_cast<void(*)(void*)>(-1));
    sqlite3_bind_int(stmt, 2, job.type);
    sqlite3_bind_int(stmt, 3, job.status);
    sqlite3_bind_int(stmt, 4, job.next_run_time);

    if (job.interval_seconds.has_value()) {
        sqlite3_bind_int(stmt, 5, job.interval_seconds.value());
    }
    else {
        sqlite3_bind_null(stmt, 5);
    }

    if (job.cron_expr.has_value()) {
        sqlite3_bind_text(stmt, 6, job.cron_expr.value().c_str(), -1, reinterpret_cast<void(*)(void*)>(-1));
    }
    else {
        sqlite3_bind_null(stmt, 6);
    }

    sqlite3_bind_text(stmt, 7, job.payload.c_str(), -1, reinterpret_cast<void(*)(void*)>(-1));
    sqlite3_bind_int(stmt, 8, job.retry_count);
    sqlite3_bind_int(stmt, 9, job.max_retries);

    int result = sqlite3_step(stmt);

    if (result != SQLITE_DONE)
    {
        cout << "database failed to insert the job data...." << endl;
        sqlite3_finalize(stmt);
        return -1;
    }

    cout << "job data inserted correctly in the database..." << endl;

    sqlite3_finalize(stmt);

    return (int)sqlite3_last_insert_rowid(DB);
}

vector<Job> select_all_jobs(sqlite3* DB)
{
    sqlite3_stmt* stmt;
    vector<Job> jobs;
    const char* sql = "SELECT * FROM jobs;";
    sqlite3_prepare_v2(DB, sql, -1, &stmt, NULL);
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        Job newJob;
        newJob.id = sqlite3_column_int(stmt, 0);
        newJob.name = (char*)sqlite3_column_text(stmt, 1);
        newJob.type = int_to_type(sqlite3_column_int(stmt, 2));
        newJob.status = int_to_status(sqlite3_column_int(stmt, 3));
        newJob.next_run_time = sqlite3_column_int64(stmt, 4);
        if (sqlite3_column_type(stmt, 5) == SQLITE_NULL) {
            newJob.interval_seconds = nullopt;
        }
        else {
            newJob.interval_seconds = sqlite3_column_int(stmt, 5);
        }
        optional<string> cron_expr;
        if (sqlite3_column_type(stmt, 6) == SQLITE_NULL) {
            newJob.cron_expr = nullopt;
        }
        else {
            newJob.cron_expr = string((char*)sqlite3_column_text(stmt, 6));
        }
        newJob.payload = (char*)sqlite3_column_text(stmt, 7);
        newJob.retry_count = sqlite3_column_int(stmt, 8);
        newJob.max_retries = sqlite3_column_int(stmt, 9);

        jobs.push_back(newJob);
    }
    cout << "object info retreived successfully" << endl;
    sqlite3_finalize(stmt);
    return jobs;
}

bool get_job_by_id(sqlite3* DB, Job& job, int id)
{
    sqlite3_stmt* stmt;
    const char* sql = "SELECT * FROM jobs WHERE id = ?";

    if (sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        cout << "Prepare failed: " << sqlite3_errmsg(DB) << endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        job.id = sqlite3_column_int(stmt, 0);

        const unsigned char* name = sqlite3_column_text(stmt, 1);
        job.name = name ? (char*)name : "";

        job.type = int_to_type(sqlite3_column_int(stmt, 2));
        job.status = int_to_status(sqlite3_column_int(stmt, 3));
        job.next_run_time = sqlite3_column_int64(stmt, 4);

        if (sqlite3_column_type(stmt, 5) == SQLITE_NULL) {
            job.interval_seconds = nullopt;
        }
        else {
            job.interval_seconds = sqlite3_column_int(stmt, 5);
        }
        optional<string> cron_expr;
        if (sqlite3_column_type(stmt, 6) == SQLITE_NULL) {
            job.cron_expr = nullopt;
        }
        else {
            job.cron_expr = string((char*)sqlite3_column_text(stmt, 6));
        }

        const unsigned char* payload = sqlite3_column_text(stmt, 7);
        job.payload = payload ? (char*)payload : "";

        job.retry_count = sqlite3_column_int(stmt, 8);
        job.max_retries = sqlite3_column_int(stmt, 9);

        sqlite3_finalize(stmt);
        return true;
    }

    sqlite3_finalize(stmt);

    cout << "Job with id " << id << " not found\n";
    return false;
}

bool update_job_status(sqlite3* DB, int id, Status newStatus, Job& job)
{
    const char* sql = "UPDATE jobs SET status = ? WHERE id = ?";

    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        cout << "Prepare failed: " << sqlite3_errmsg(DB) << endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, static_cast<int>(newStatus));
    sqlite3_bind_int(stmt, 2, id);

    int rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE)
    {
        cout << "Update failed: " << sqlite3_errmsg(DB) << endl;
        sqlite3_finalize(stmt);
        return false;
    }
    else
    {
        cout << "Job status updated successfully\n";
        get_job_by_id(DB, job, id);
        sqlite3_finalize(stmt);
        return true;
    }
}

bool delete_job(sqlite3* DB, int id)
{
    sqlite3_stmt* stmt;
    const char* sql = "DELETE FROM jobs WHERE id = ?";
    if (sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        cout << "Prepare failed: " << sqlite3_errmsg(DB) << endl;
        return false;
    }
    sqlite3_bind_int(stmt, 1, id);
    int rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        cout << "Delete failed for id: " << id << endl;
        return false;
    }

    if (sqlite3_changes(DB) == 0)
    {
        cout << "No job found with id: " << id << endl;
        return false;
    }

    cout << "Job deleted successfully\n";
    return true;
}

bool update_job(sqlite3* DB, Job job, int id)
{
    sqlite3_stmt* stmt;

    const char* sql =
        "UPDATE jobs SET "
        "name = ?, "
        "type = ?, "
        "status = ?, "
        "next_run_time = ?, "
        "interval_seconds = ?, "
        "cron_expr = ?, "
        "payload = ?, "
        "retry_count = ?, "
        "max_retries = ? "
        "WHERE id = ?";

    if (sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        cout << "Failed to prepare update\n";
        return false;
    }

    sqlite3_bind_text(stmt, 1, job.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, job.type);
    sqlite3_bind_int(stmt, 3, job.status);
    sqlite3_bind_int64(stmt, 4, job.next_run_time);

    if (job.interval_seconds.has_value())
        sqlite3_bind_int(stmt, 5, job.interval_seconds.value());
    else
        sqlite3_bind_null(stmt, 5);

    if (job.cron_expr.has_value())
        sqlite3_bind_text(stmt, 6, job.cron_expr.value().c_str(), -1, SQLITE_TRANSIENT);
    else
        sqlite3_bind_null(stmt, 6);

    sqlite3_bind_text(stmt, 7, job.payload.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 8, job.retry_count);
    sqlite3_bind_int(stmt, 9, job.max_retries);
    sqlite3_bind_int(stmt, 10, id);

    int rc = sqlite3_step(stmt);

    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        cout << "Update failed\n";
        return false;
    }
    if (sqlite3_changes(DB) == 0)
    {
        cout << "No job found with id\n";
        return false;
    }

    cout << "Job updated successfully\n";
    return true;
}

bool pause_job(sqlite3* DB, int id)
{
    sqlite3_stmt* stmt;
    const char* sql = "UPDATE jobs SET status = ? WHERE id = ?;";
    if (sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        cout << "Failed to pause the job\n";
        return false;
    }
    sqlite3_bind_int(stmt, 1, PAUSED);
    sqlite3_bind_int(stmt, 2, id);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE)
    {
        cout << "failed to pause the job.." << endl;
        return false;
    }
    if (sqlite3_changes(DB) == 0)
    {
        cout << "No job with id : " << id << endl;
        return false;
    }
    cout << "job paused successfully..." << endl;
    return true;
}

bool resume_job(sqlite3* DB, int id)
{
    sqlite3_stmt* stmt;
    const char* sql = "UPDATE jobs SET status = ? WHERE id = ?;";
    if (sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        cout << "Failed to resume the job\n";
        return false;
    }
    sqlite3_bind_int(stmt, 1, ACTIVE);
    sqlite3_bind_int(stmt, 2, id);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE)
    {
        cout << "failed to resume the job.." << endl;
        return false;
    }
    if (sqlite3_changes(DB) == 0)
    {
        cout << "No job with id : " << id << endl;
        return false;
    }
    cout << "job resumed successfully..." << endl;
    return true;
}

bool update_next_run_time(sqlite3* DB, int id, long long new_next_run_time)
{
    sqlite3_stmt* stmt;
    const char* sql = "UPDATE jobs SET next_run_time = ? WHERE id = ?;";
    if (sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        cout << "Failed to update the next run time\n";
        return false;
    }

    sqlite3_bind_int64(stmt, 1, new_next_run_time);
    sqlite3_bind_int(stmt, 2, id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE)
    {
        cout << "Failed to update the next run time..." << endl;
        return false;
    }

    if (sqlite3_changes(DB) == 0)
    {
        cout << "No jobs has the id " << id << endl;
        return false;
    }
    cout << "Next run time updated successfully..." << endl;
    return true;
}

bool increment_retry_count(sqlite3* DB, int id)
{
    sqlite3_stmt* stmt;
    const char* sql =
        "UPDATE jobs "
        "SET retry_count = retry_count + 1 "
        "WHERE id = ? AND retry_count < max_retries;";
    if (sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        cout << "Failed to update the next run time\n";
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (result != SQLITE_DONE)
    {
        cout << "Failed to increment the retry count..." << endl;
        return false;
    }

    if (sqlite3_changes(DB) == 0)
    {
        cout << "Failed to increment the retry count..." << endl;
        return false;
    }
    cout << "rety count increment successfully..." << endl;
    return true;
}

bool reset_retry_count(sqlite3* DB, int id)
{
    sqlite3_stmt* stmt;
    const char* sql = "UPDATE jobs SET retry_count = 0 WHERE id = ?;";
    if (sqlite3_prepare_v2(DB, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        cout << "Failed to reset the retry count..." << endl;
        return false;
    }
    sqlite3_bind_int(stmt, 1, id);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (result != SQLITE_DONE)
    {
        cout << "Failed to reset the retry count...";
        return false;
    }
    if (sqlite3_changes(DB) == 0)
    {
        cout << "can not find the job with id " << id << endl;
        return false;
    }
    return true;
}
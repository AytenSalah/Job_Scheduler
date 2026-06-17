
#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include <optional>
#include <chrono>
#include "Controller/Controller.h"
#include <sqlite3.h>
#include "crow/crow_all.h"
#include <asio.hpp>
#include "Scheduler/Scheduler.h"  // add this
#include <thread>                  // add this
#include <atomic>  

using namespace std;
using namespace asio;
/*
//////////////////////////////////////////////////////////////////////
// Database layer: 
enum Status { ACTIVE, PAUSED, RUNNING, COMPLETED, FAILED, CANCELED }; // between [0 , 5]
enum Job_Type { ONE_TIME, RECURRING, CRON }; // between [0 , 2]
struct Job
{
	int id;    
	string name;
	Job_Type type;
	Status status;
	long long next_run_time;

	// interval
	optional<int> interval_seconds; // null or value

	// cron
	optional<string> cron_expr;

	// common
	string payload; // task to be executed. (sending email) .....

	int retry_count = 0;
	int max_retries = 3;
};

int create_DB(const char * filename , sqlite3 **DB)
{
	int rc = sqlite3_open(filename, DB);
	if (rc == SQLITE_OK)
	{
		cout << "Database opened successfuly" << endl;
	}
	else 
	{
		cout << "failed to be opened : "<< sqlite3_errmsg(*DB) << endl; 
	}
	return rc;
}

int create_table(sqlite3 * DB)
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

/*
int insert_job(sqlite3* DB, const Job &job) 
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

	sqlite3_bind_text(stmt, 1, job.name.c_str(), -1 , reinterpret_cast<void(*)(void*)>(-1));
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
		sqlite3_bind_text(stmt, 6, job.cron_expr.value().c_str() , -1 , reinterpret_cast<void(*)(void*)>(-1));
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
	}
	else 
	{
		cout << "job data inserted correctly in the database..." << endl;
	}
	return result; 

}
*/
/*
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
*/
/*
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
	case 0: // ACTIVE, PAUSED, RUNNING, COMPLETED, FAILED, CANCELED
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
	case ACTIVE :
		return 0;
		break;
	case PAUSED :
		return 1;
		break;
	case RUNNING :
		return 2;
		break;
	case COMPLETED:
		return 3;
		break;
	case CANCELED:
		return 4;
		break;
	default :
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

// queue => jobs = select_all_jobs(DB);
// operations
vector<Job> select_all_jobs(sqlite3 * DB) // api
{
	sqlite3_stmt* stmt; 
	vector<Job> jobs;
	const char* sql = "SELECT * FROM jobs;";
	sqlite3_prepare_v2(DB, sql, -1, &stmt, NULL); // prepare sql query.
	while (sqlite3_step(stmt) == SQLITE_ROW) // step for execution.
	{
		Job newJob;
		newJob.id = sqlite3_column_int(stmt, 0);
		newJob.name = (char *)sqlite3_column_text(stmt, 1);
		newJob.type = int_to_type(sqlite3_column_int(stmt , 2));
		newJob.status = int_to_status(sqlite3_column_int(stmt , 3));
		newJob.next_run_time = sqlite3_column_int64(stmt , 4);
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
	sqlite3_finalize(stmt); // memory leak.
	return jobs;
}
//////////////////////////////////////////////////////////////////////
// id auto increment ++
bool get_job_by_id(sqlite3* DB, Job& job, int id) 
{
	sqlite3_stmt* stmt;
	const char* sql = "SELECT * FROM jobs WHERE id = ?";

	if (sqlite3_prepare_v2(DB, sql, -1, &stmt, nullptr) != SQLITE_OK)
	{
		cout << "Prepare failed: " << sqlite3_errmsg(DB) << endl;
		return false;
	}

	sqlite3_bind_int(stmt, 1, id); // set the ? mark.

	if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		job.id = sqlite3_column_int(stmt, 0); // index is zero. (id)

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

void print_job_info(const Job& job)
{
	cout << "===== Object Information =====\n";

	cout << "ID: " << job.id << "\n";

	cout << "Name: " << job.name << "\n";

	cout << "Type: " << type_to_string(job.type) << "\n";

	cout << "Status: " << status_to_string(job.status) << "\n";

	cout << "Next Run Time: " << job.next_run_time << "\n";

	// optional fields
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

bool update_job_status(sqlite3* DB, int id, Status newStatus , Job & job)
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

	// optional interval
	if (job.interval_seconds.has_value())
		sqlite3_bind_int(stmt, 5, job.interval_seconds.value());
	else
		sqlite3_bind_null(stmt, 5);

	// optional cron
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
	// check number of rows affected...
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
	sqlite3_bind_int(stmt , 1 , PAUSED); // 
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

bool resume_job(sqlite3 * DB , int id) 
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

// i will do these function after meeting (update_next_run_time (VERY IMPORTANT) , increment_retry_count)
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
bool reset_retry_count(sqlite3 * DB, int id) // set retries = 0;
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
Job create_job_from_json(const crow::json::rvalue& body)
{
	Job job;
	job.name = body["name"].s();
	job.status = int_to_status(body["status"].i()); // here we need to handle enums operations.
	job.next_run_time = body["next_run_time"].i();
	job.interval_seconds;
	if (!body.has("interval_seconds") || body["interval_seconds"].t() == crow::json::type::Null)
	{
		job.interval_seconds = nullopt;
	}
	else
	{
		job.interval_seconds = static_cast<long long>(body["interval_seconds"].i());
	}
	job.cron_expr;
	if (!body.has("cron_expr") || body["cron_expr"].t() == crow::json::type::Null) {
		job.cron_expr = nullopt;
	}
	else {
		job.cron_expr = body["cron_expr"].s();
	}
	job.type = int_to_type(body["type"].i());
	job.payload = body["payload"].s();
	job.retry_count = body["retry_count"].i();
	job.max_retries = body["max_retries"].i();

	return job;
}
*/
int main()
{

	const char* dir = "jobs.db";
	sqlite3* DB = NULL;
	sqlite3_config(SQLITE_CONFIG_SERIALIZED);
	create_DB(dir, &DB);
	create_table(DB);
	
	std::atomic<bool> running(true);
    std::thread scheduler_thread(scheduler_loop, DB, std::ref(running));
	
	crow::SimpleApp app;
	register_routes(app, DB);
	app.port(3000).multithreaded().run();
	
	running = false;
    scheduler_thread.join();

	return 0 ;


	/*
	Job job2;

	job2.name = "GenerateReport";
	job2.type = RECURRING;
	job2.status = RUNNING;
	job2.next_run_time = 1765000000;

	// interval
	job2.interval_seconds = 3600;

	// cron
	job2.cron_expr = "0 *";
	/*
	job2.payload = "generate daily report";
	job2.retry_count = 1;
	job2.max_retries = 5;
	Job job;

	job.name = "SendEmail";
	job.type = ONE_TIME;
	job.status = ACTIVE;
	job.next_run_time = 1760000000;

	job.interval_seconds = nullopt;
	job.cron_expr = nullopt;

	job.payload = "sending email";
	job.retry_count = 0;
	job.max_retries = 3;
	*/

	// insertion code segment
	//insert_job(DB, job);
	//insert_job(DB, job2);


	// print all jobs in DB
	/*
	vector<Job> jobs = select_all_jobs(DB);
	for (Job job : jobs) {

		cout << job.name << endl;
	}
	cout << "*************************************************" << endl;

	// update and retireve operations...
	Job job_queried;
	bool flag_get_job = get_job_by_id(DB, job_queried , 1);
	if (flag_get_job)
	{
		print_job_info(job_queried);
	}
	else {
		cout << "Job with this id not exist in DB" << endl;
	}

	bool flag_update = update_job_status(DB, 1, RUNNING , job_queried);
	if (flag_update) {
		print_job_info(job_queried);
	}
	else {
		cout << "Job with this id not exist in DB" << endl;
	}

	delete_job(DB, 2); // job with id = 2 will be removed from our database...

	cout << "*************************************" << endl;
	cout << "*************************************" << endl;
	vector<Job> jobs1 = select_all_jobs(DB);
	for (Job job : jobs1) {

		cout << job.name << endl;
	}
	*/


	// update information of job number 3...
	/*
	==== = Object Information ==== =
		ID: 3
		Name : SendEmail
		Type : ONE_TIME
		Status : active
		Next Run Time : 1760000000
		Interval Seconds : NULL
		Cron Expr : NULL
		Payload : sending email
		Retry Count : 0
		Max Retries : 3
	*/
	/*
	Job job6;

	job6.name = "my new task";
	job6.type = ONE_TIME;
	job6.status = CANCELED;
	job6.next_run_time = 1760000000;

	job6.interval_seconds = std::nullopt;
	job6.cron_expr = std::nullopt;

	job6.payload = "palying vooley ball";
	job6.retry_count = 0;
	job6.max_retries = 9;
	update_job(DB, job6, 3);

	cout << "*************************************" << endl;
	cout << "*************************************" << endl;
	vector<Job> jobs3 = select_all_jobs(DB);
	for (Job job : jobs3) {

		cout << job.name << endl;
	}
	*/
	/*
	// pause a job...
	pause_job(DB, 1);
	Job job_ex;
	get_job_by_id(DB, job_ex, 1);
	print_job_info(job_ex);

	cout << "*************************************" << endl;
	cout << "*************************************" << endl;

	// resume a job again (PAUSED => ACTIVE)...
	resume_job(DB, 1);
	get_job_by_id(DB, job_ex, 1);
	print_job_info(job_ex);

	// update next run time...
	update_next_run_time(DB, 1, 96);
	get_job_by_id(DB, job_ex, 1);
	print_job_info(job_ex);

	// increment retry count..
	increment_retry_count(DB, 1);
	get_job_by_id(DB, job_ex, 1);
	print_job_info(job_ex);
	//increment counter again...
	increment_retry_count(DB, 1);
	get_job_by_id(DB, job_ex, 1);
	print_job_info(job_ex);
	// increment again
	increment_retry_count(DB, 1);
	get_job_by_id(DB, job_ex, 1);
	print_job_info(job_ex);

	cout << "//////////////////////////////////////////////" << endl;
	// reset rety count...
	reset_retry_count(DB, 1);
	get_job_by_id(DB, job_ex, 1);
	print_job_info(job_ex);

	cout << "//////////////////////////////////////////////" << endl;
	// reset rety count...
	reset_retry_count(DB, 5);
	get_job_by_id(DB, job_ex, 5);
	print_job_info(job_ex);
	*/
	// 
	//ROUTING IN OUR PROJECT....
   /*
	crow::SimpleApp app;
	// main page route......
	CROW_ROUTE(app, "/")([](const crow::request& req, crow::response& res) {
		res.write("server is listening to our requests now...");
		res.end();
		});

	// post request for create a task....
	CROW_ROUTE(app, "/CreateTask").methods("POST"_method)([DB](const crow::request& req, crow::response& res) {
		auto body = crow::json::load(req.body);
		Job job = create_job_from_json(body);
		job.id = insert_job(DB, job);


		res.write("Object saved in Database correctly..");
		res.end();
		});
	// post request for deleting a task (Tested)
	CROW_ROUTE(app, "/DeleteTask/<int>").methods("DELETE"_method)([DB](int id) {
		bool flag = delete_job(DB, id);
		if (flag) {
			cout << "successfuly job task with id : " << id << " deleted" << endl;
			return crow::response("successfuly job task with id : " + std::to_string(id) + " deleted");
		}
		else {
			cout << "Failed to delete job with id : " << id << endl;
			return "Failed to delete job with id : " + std::to_string(id);
		}

		});

	CROW_ROUTE(app, "/Jobs").methods("GET"_method)([DB](crow::response& res) {
		cout << "List of all jobs : " << endl;
		print_all_jobs(select_all_jobs(DB));
		res.write("All jobs retrieved successfully");
		});

	CROW_ROUTE(app, "/job/<int>").methods("GET"_method)([DB](int id) {
		Job job;
		bool flag = get_job_by_id(DB, job, id);
		if (flag) {
			print_job_info(job);
			return "job with id " + std::to_string(id) + " retrieved successfully";
		}
		else {
			cout << "job with id : " << id << " Not exist in our database" << endl;
			return "job with id " + std::to_string(id) + "not exist in Database";
		}
		});

	CROW_ROUTE(app, "/job/pause/<int>").methods("POST"_method)([DB](int id){
		bool flag = pause_job(DB, id);
		if (flag) {
			cout << "job with id : " << id << "paused successfuly" << endl;
			return "job with id : " + std::to_string(id) + "paused successfuly";
		}
		else {
			cout << "Failed to pause the job with id : " << id << endl;
			return "Failed to pause the job with id : " + std::to_string(id);
		}
	});
	CROW_ROUTE(app, "/job/resume/<int>").methods("POST"_method)([DB](int id) {
		bool flag = resume_job(DB, id);
		if (flag) {

			cout << "job with id : " << id << "resumed successfuly" << endl;
			return "job with id : " + std::to_string(id) + "resumed successfuly";
		}
		else {
			cout << "Failed to resume the job with id : " << id << endl;
			return "Failed to resume the job with id : " + std::to_string(id);
		}
		});
		*/

}

#include "Controller/Controller.h"

// ================= ROUTES IMPLEMENTATION =================

void root_route(const crow::request& req, crow::response& res)
{
    res.write("server is listening to our requests now...");
    res.end();
}

void create_task_route(sqlite3* DB, const crow::request& req, crow::response& res)
{
    auto body = crow::json::load(req.body);
    Job job = create_job_from_json(body);
    job.id = insert_job(DB, job);

    res.write("Object saved in Database correctly..");
    res.end();
}

crow::response delete_task_route(sqlite3* DB, int id)
{
    bool flag = delete_job(DB, id);
    if (flag) {
        cout << "successfuly job task with id : " << id << " deleted" << endl;
        return crow::response("successfuly job task with id : " + std::to_string(id) + " deleted");
    }
    else {
        cout << "Failed to delete job with id : " << id << endl;
        return "Failed to delete job with id : " + std::to_string(id);
    }
}

void get_all_jobs_route(sqlite3* DB, crow::response& res)
{
    cout << "List of all jobs : " << endl;
    print_all_jobs(select_all_jobs(DB));
    res.write("All jobs retrieved successfully");
}

std::string get_job_by_id_route(sqlite3* DB, int id)
{
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
}

std::string pause_job_route(sqlite3* DB, int id)
{
    bool flag = pause_job(DB, id);
    if (flag) {
        cout << "job with id : " << id << "paused successfuly" << endl;
        return "job with id : " + std::to_string(id) + "paused successfuly";
    }
    else {
        cout << "Failed to pause the job with id : " << id << endl;
        return "Failed to pause the job with id : " + std::to_string(id);
    }
}

std::string resume_job_route(sqlite3* DB, int id)
{
    bool flag = resume_job(DB, id);
    if (flag) {
        cout << "job with id : " << id << "resumed successfuly" << endl;
        return "job with id : " + std::to_string(id) + "resumed successfuly";
    }
    else {
        cout << "Failed to resume the job with id : " << id << endl;
        return "Failed to resume the job with id : " + std::to_string(id);
    }
}

// ================= REGISTER ROUTES =================

void register_routes(crow::SimpleApp& app, sqlite3* DB)
{
    CROW_ROUTE(app, "/")(root_route);

    CROW_ROUTE(app, "/CreateTask").methods("POST"_method)
        ([DB](const crow::request& req, crow::response& res) {
        create_task_route(DB, req, res);
            });

    CROW_ROUTE(app, "/DeleteTask/<int>").methods("DELETE"_method)
        ([DB](int id) {
        return delete_task_route(DB, id);
            });

    CROW_ROUTE(app, "/Jobs").methods("GET"_method)
        ([DB](crow::response& res) {
        get_all_jobs_route(DB, res);
            });

    CROW_ROUTE(app, "/job/<int>").methods("GET"_method)
        ([DB](int id) {
        return get_job_by_id_route(DB, id);
            });

    CROW_ROUTE(app, "/job/pause/<int>").methods("POST"_method)
        ([DB](int id) {
        return pause_job_route(DB, id);
            });

    CROW_ROUTE(app, "/job/resume/<int>").methods("POST"_method)
        ([DB](int id) {
        return resume_job_route(DB, id);
            });
}
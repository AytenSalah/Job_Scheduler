#include "service.h"
#include "database.h"
#include <ctime>
#include "croncpp.h"

//AC7: Invalid schedule input is rejected with a clear error response.
bool validate_job_input(const crow::json::rvalue &body, std::string &out_error)

{
    
    if( !body.has("name") ||
        body["name"].t() != crow::json::type::String ||
        body["name"].s() == ""  )
        {
            out_error = "The Name field is required and must be a non-empty String";
            return false ; 
        }
    
    if(!body.has("type") ||
       body["type"].t()!=crow::json::type::Number ||
       body["type"].i() < 0 ||
       body["type"].i() > 2 )
       {
        out_error ="The Type field is required and  must be 0 (ONE_TIME), 1 (RECURRING), or 2 (CRON)" ;
        return false;
       }

    if( !body.has("payload") ||
        body["payload"].t() != crow::json::type::String ||
        body["payload"].s() == ""  )
        {
            out_error = "The Payload field is required and must be a non-empty String";
            return false ; 
        }

    int type = body["type"].i();
    if(type == 0)
    {
        if(!body.has("next_run_time") ||
           body["next_run_time"].t() != crow::json::type::Number ||
           body["next_run_time"].i() <= 0 )
        {
        out_error = "ONE_TIME jobs require a positive 'next_run_time' value";
        return false;
        }
    }
    if(type == 1)
    {
        if(!body.has("interval_seconds") ||
           body["interval_seconds"].t() != crow::json::type::Number ||
           body["interval_seconds"].i() <= 0 )
        {
        out_error = "RECURRING jobs require a positive 'interval_seconds' value";
        return false;
        }
    }

     if(type == 2)
    {
        if(!body.has("cron_expr") ||
           body["cron_expr"].t() != crow::json::type::String ||
           body["cron_expr"].s() == "")
        {
        out_error = "The Cron expression is required and must be a non-empty String";
        return false;
        }

        std::string cron_err;
        if(!is_valid_cron(body["cron_expr"].s(), cron_err))
        {
            out_error = "Invalid cron expression " + cron_err;
            return false;
        }
    }

    return true;
}

bool is_valid_cron(const std::string &cron_expr, std::string &out_error)
{
    try
    {
        std::string full_expr = "0" + cron_expr ;
        cron::make_cron(full_expr);
        retrun true;
    }
    catch(const cron::bad_cronexpr& e)
    {
        out_error = e.what();
        return false;
    }

    return true;
    
}

long long compute_cron_next_run(const std::string &cron_expr, long long now)
{
    std::string full_expr = "0" + cron_expr;
    cron::cronexpr cex = cron::make_cron(full_expr);

    time_t now_t = (time_t)now;
    time_t next_t = cron::cron_next(cex, now_t);

    return (long long)next_t;
}



long long compute_next_run_time(int type, long long now, std::optional<int> interval_seconds,std::optional<std::string> cron_expr, long long client_next_run_time)
{
    if(type == 0) // One_time
     return client_next_run_time;
    if(type == 1) // Recurring 
     return now + interval_seconds.value();
    if(type == 2) // Cron_Expression
     return compute_cron_next_run(cron_expr.value() , now);

    return -1;
}

Job create_job(sqlite3* DB, const crow::json::rvalue& body, std::string& out_error)
{
    if (!validate_job_input(body, out_error))
    {
        Job job;
        job.id = -1;
        return job;
    }

    Job job;
    job.name = body["name"].s();
    job.type = int_to_type(body["type"].i());
    job.payload = body["payload"].s();

    
    if (!body.has("interval_seconds") || body["interval_seconds"].t() == crow::json::type::Null)
        job.interval_seconds = nullopt;
    else
        job.interval_seconds = body["interval_seconds"].i();

    if (!body.has("cron_expr") || body["cron_expr"].t() == crow::json::type::Null)
        job.cron_expr = nullopt;
    else
        job.cron_expr = body["cron_expr"].s();

    
    long long client_next_run_time = 0;
    if (body.has("next_run_time") && body["next_run_time"].t() == crow::json::type::Number)
        client_next_run_time = body["next_run_time"].i();


    job.status = ACTIVE;
    job.retry_count = 0;
    job.max_retries = (body.has("max_retries") && body["max_retries"].t() == crow::json::type::Number)
                       ? body["max_retries"].i()
                       : 3;

    int type = body["type"].i();
    job.next_run_time = compute_next_run_time(type, time(nullptr), job.interval_seconds, job.cron_expr, client_next_run_time);
    job.id = insert_job(DB, job);
    return job;
}


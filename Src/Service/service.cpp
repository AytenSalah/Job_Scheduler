#include "service.h"
#include "database.h"
#include "ctime"

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
    }

    return true;
}

long long compute_next_run_time(int type, long long now, std::optional<int> interval_seconds, long long client_next_run_time)
{
    if(type == 0)
     return client_next_run_time;
    if(type == 1)
     return now + interval_seconds.value();
    if(type == 2)
     return -1;

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
    job.next_run_time = compute_next_run_time(type, time(nullptr), job.interval_seconds, client_next_run_time);

    job.id = insert_job(DB, job);
    return job;
}


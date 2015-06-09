#include <MetricPollster.h>

MetricPollster::MetricPollster()
{

}
        
MetricPollster::~MetricPollster() {}

int MetricPollster::initialize(const char *databaseFile)
{
    database = string(databaseFile);
    
    sqlite3 *db;
    if (sqlite3_open(database.c_str(), &db)) {
        LOG("Cannot open database '%s'", database.c_str());
        return POLLSTER_CANTOPEN;
    }
    sqlite3_close(db);

    return POLLSTER_OK;
}

Measure MetricPollster::create_measure(string name)
{
    Measure measure;
    measure.metric = name;
    return measure;
}

Datapoint MetricPollster::create_datapoint(int timestamp, long value)
{
    Datapoint datapoint;
    datapoint.timestamp = timestamp;
    datapoint.value = value;
    return datapoint;
}

void MetricPollster::get_datapoints(string& name, int start, vector<Measure> &_return)
{
    LOG("Getting datapoints for domain %s...", name.c_str());

    sqlite3 *db;
    if (sqlite3_open_v2(database.c_str(), &db, SQLITE_OPEN_READONLY, NULL)) {
        LOG("Unable to get datapoints, cannot open database '%s'", database.c_str());
        return;
    }
    
    sqlite3_stmt *stmt;
    const char *zTail;

    int rc = sqlite3_prepare(db,
            "select * from stats where name=? and timestamp >= ?;", 
            -1, &stmt, &zTail);
    
    if (rc != SQLITE_OK) {
        LOG("Unable to prepare statement, SQL error code: %d", rc);
        sqlite3_close(db);
        return;
    }

    rc = sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        LOG("Unable to bind domain name, SQL error code: %d", rc);
        sqlite3_finalize(stmt);
        sqlite3_close(db);   
        return;
    }
    
    rc = sqlite3_bind_int(stmt, 2, start);
    if (rc != SQLITE_OK) {
        LOG("Unable to bind start timestamp, SQL error code: %d", rc);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        string metric = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
        long timestamp = sqlite3_column_int(stmt, 3);
        long value = sqlite3_column_int64(stmt, 4);
        string dn = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
        string dv = string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6)));

        Measure measure = create_measure(metric);
        measure.dimensions[dn] = dv;
        measure.datapoints.push_back(create_datapoint(timestamp, value));
        _return.push_back(measure);
    } 

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    LOG("%d datapoints returned for domain %s", _return.size(), name.c_str());
} 

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
    sqlite3 *db;
    if (sqlite3_open(database.c_str(), &db)) {
        LOG("Unable to get datapoints, cannot open database '%s'", database.c_str());
        return;
    }
    
    sqlite3_stmt *stmt;
    const char *zTail;

    int rc = sqlite3_prepare(db,
            "select * from domain_stats where uuid=? and timestamp >= ?;", 
            -1, &stmt, &zTail);
    
    if (rc != SQLITE_OK) {
        LOG("Unable to prepare statement, SQL error code: %d", rc);
        return;
    }

    rc = sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        LOG("Unable to bind domain name, SQL error code: %d", rc);
        return;
    }
    
    rc = sqlite3_bind_int(stmt, 2, start);
    if (rc != SQLITE_OK) {
        LOG("Unable to bind start timestamp, SQL error code: %d", rc);
        return;
    }
    
    Measure cpu_time = create_measure("cpu_time"); 
    Measure used_mem = create_measure("used_mem");
    Measure vcpu_time = create_measure("vcpu_time");
    Measure vcpu_number = create_measure("vcpu_number"); 
    Measure disk_rd_requests = create_measure("disk_rd_requests");
    Measure disk_rd_bytes = create_measure("disk_rd_bytes");
    Measure disk_wr_requests = create_measure("disk_wr_requests");
    Measure disk_wr_bytes = create_measure("disk_wr_bytes");
    Measure if_rx_bytes = create_measure("if_rx_bytes");
    Measure if_rx_packets = create_measure("if_rx_packets"); 
    Measure if_rx_errors = create_measure("if_rx_errors");
    Measure if_rx_drops = create_measure("if_rx_drops");
    Measure if_tx_bytes = create_measure("if_tx_bytes");
    Measure if_tx_packets = create_measure("if_tx_packets");
    Measure if_tx_errors = create_measure("if_tx_errors");
    Measure if_tx_drops = create_measure("if_tx_drops");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        long timestamp = sqlite3_column_int(stmt, 2); 
        cpu_time.datapoints.push_back(create_datapoint(timestamp, sqlite3_column_int64(stmt, 3)));
        used_mem.datapoints.push_back(create_datapoint(timestamp, sqlite3_column_int64(stmt, 4)));
        vcpu_time.datapoints.push_back(create_datapoint(timestamp, sqlite3_column_int64(stmt, 5)));
        vcpu_number.datapoints.push_back(create_datapoint(timestamp, sqlite3_column_int64(stmt, 6))); 
        disk_rd_requests.datapoints.push_back(create_datapoint(timestamp, sqlite3_column_int64(stmt, 7)));
        disk_rd_bytes.datapoints.push_back(create_datapoint(timestamp, sqlite3_column_int64(stmt, 8)));
        disk_wr_requests.datapoints.push_back(create_datapoint(timestamp, sqlite3_column_int64(stmt, 9)));
        disk_wr_bytes.datapoints.push_back(create_datapoint(timestamp, sqlite3_column_int64(stmt, 10)));
        if_rx_bytes.datapoints.push_back(create_datapoint(timestamp, sqlite3_column_int64(stmt, 11)));
        if_rx_packets.datapoints.push_back(create_datapoint(timestamp, sqlite3_column_int64(stmt, 12))); 
        if_rx_errors.datapoints.push_back(create_datapoint(timestamp, sqlite3_column_int64(stmt, 13)));
        if_rx_drops.datapoints.push_back(create_datapoint(timestamp, sqlite3_column_int64(stmt, 14)));
        if_tx_bytes.datapoints.push_back(create_datapoint(timestamp, sqlite3_column_int64(stmt, 15)));
        if_tx_packets.datapoints.push_back(create_datapoint(timestamp, sqlite3_column_int64(stmt, 16)));
        if_tx_errors.datapoints.push_back(create_datapoint(timestamp, sqlite3_column_int64(stmt, 17)));
        if_tx_drops.datapoints.push_back(create_datapoint(timestamp, sqlite3_column_int64(stmt, 18)));
    } 

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    _return.push_back(cpu_time);
    _return.push_back(used_mem);
    _return.push_back(vcpu_time);
    _return.push_back(vcpu_number); 
    _return.push_back(disk_rd_requests);
    _return.push_back(disk_rd_bytes);
    _return.push_back(disk_wr_requests);
    _return.push_back(disk_wr_bytes);
    _return.push_back(if_rx_bytes);
    _return.push_back(if_rx_packets); 
    _return.push_back(if_rx_errors);
    _return.push_back(if_rx_drops);
    _return.push_back(if_tx_bytes);
    _return.push_back(if_tx_packets);
    _return.push_back(if_tx_errors);
    _return.push_back(if_tx_drops);
} 

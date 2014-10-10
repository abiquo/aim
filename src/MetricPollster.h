/**
 * Abiquo community edition
 * cloud management application for hybrid clouds
 * Copyright (C) 2008-2010 - Abiquo Holdings S.L.
 *
 * This application is free software; you can redistribute it and/or
 * modify it under the terms of the GNU LESSER GENERAL PUBLIC
 * LICENSE as published by the Free Software Foundation under
 * version 3 of the License
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * LESSER GENERAL PUBLIC LICENSE v.3 for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef METRIC_POLLSTER_H
#define METRIC_POLLSTER_H

#include <Debug.h>

#include <vector>
#include <string>
#include <ctime>

#include <sqlite3.h>

#include <aim_types.h>

using namespace std;

/* beginning of return codes */
#define POLLSTER_OK        0
#define POLLSTER_CANTOPEN  1
/* end of return codes */

class MetricPollster
{
    private:
        string database;

        Measure create_measure(string name);
        Datapoint create_datapoint(int timestamp, long value);

    public:
        MetricPollster();
        ~MetricPollster();

        int initialize(const char* databaseFile);
        void get_datapoints(string &name, int start, vector<Measure> &_return);
};

#endif

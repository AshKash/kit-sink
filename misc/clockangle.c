#include <stdio.h>
#include <stdlib.h>
#include <math.h>

float get_angle_mins(float minutes)
{
    // 60 minutes == 360 degrees. 1 minute == 6 degrees
    return (int)(minutes * 6) % 360;
}

float get_angle_hours(float hours)
{
    // 12 hours == 360 degrees. 1 hour = 30 degrees
    return (int)(hours * 30) % 360;
}

int main(int argc, char *argv[])
{
    float hours = atoi(argv[1]);
    float minutes = atoi(argv[2]);

    float total = hours + minutes / 60.0;
    float hour_angle = get_angle_hours(total);
    float mins_angle = get_angle_mins(minutes);

    float diff = fabs(hour_angle - mins_angle);

    printf("total: %f hour_angle: %f, min_angle: %f, angle: %f\n", 
            total, hour_angle, mins_angle, diff);
    
}


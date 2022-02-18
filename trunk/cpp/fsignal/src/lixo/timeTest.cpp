#include <iostream>
#include "TimeStampWrapper.h"
#include <ctime>

using namespace std;
using namespace fsignal;

int getTimeInSeconds(TimeStampWrapper tsw) {
    tm* now = (tm*)malloc(sizeof(struct tm));
    TimeWrapper tw = tsw.getTimeWrapper();
    DateWrapper dw = tsw.getDateWrapper();
    now->tm_sec = tw.getSeconds();
    now->tm_min = tw.getMinutes();
    now->tm_hour = tw.getHours();
    cout << "Hour: " << now->tm_hour << endl;
    now->tm_mday = dw.getDay();
    now->tm_mon = dw.getMonth();
    now->tm_year = dw.getYear() - 1900;
    time_t nowsecs = mktime(now);
    free(now);
    return (int)nowsecs;
}

int main() {
	time_t nows=time(NULL);
    TimeStampWrapper tsw=TimeStampWrapper(&nows);
    int timeStamped =getTimeInSeconds(tsw);
    cout << "Time now: " << timeStamped << " or " << nows << endl;
    cout << "Difference: " << timeStamped-nows << endl;

	return 0;
}

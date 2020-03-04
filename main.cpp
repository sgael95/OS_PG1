#include <iostream>
#include <random>
#include <ctime>
#include <cmath>
#include <list>
#include <fstream>


using namespace std;

class Process
{
    public:
        //virtual ~Process();
        int id;
        double service_time;
        double arrival_time;
        double remaining_time;
        double completion_time;
        double wait_time;
        double rTime;
        Process(int a, double atime, double stime){
            id = a;
            arrival_time = atime;
            service_time = stime;
            remaining_time = stime;
        }
        void setRTime(){
            rTime = (wait_time + service_time) / service_time;
        }

        bool operator <(const Process & processObj) const
        {
            return remaining_time < processObj.remaining_time;
        }
    protected:
    private:
};

class Event
{
    public:
        Event(double t, string eType, int id){
        time = t;
        eventType = eType;
        this->id = id;
    }
        Event();
        double time;
        string eventType;
        int id;
        bool operator <(const Event & eventObj) const
        {
            return time < eventObj.time;
        }
    protected:
    private:
};


void generateList(double (&rList)[20000], double avRival, double (&slist)[20000], double aServiceTime);
void firstCome(list<Event> &eQueue, list<Process> &readyQueue, bool &cpu_idle, double &clock, double (&srate)[20000], double &arrival_rate);
void schedule_event(string type, double time, list<Event> &elist, int id, double &clock);
void process_arrival(Event &e, bool &idle, double (&slist)[20000], list<Event> &elist, list<Process> &rQueue, double &clock);
void process_departure(Event &e, bool &idle, list<Event> &elist, list<Process> &rQueue, double &clock);
void shortestRemaining(list<Event> &eQueue, list<Process> &readyQueue, bool &cpu_idle, double &clock, double (&srate)[20000], double &arrival_rate);
void s_process_arrival(Event &e, bool &idle, double (&slist)[20000], list<Event> &elist, list<Process> &rQueue, double &clock, double &tmpclock, double &temp);
void s_process_departure(Event &e, bool &idle, list<Event> &elist, list<Process> &rQueue, double &clock);
void highestRemaining(list<Event> &eQueue, list<Process> &readyQueue, bool &cpu_idle, double &clock, double (&srate)[20000], double &arrival_rate);
void h_process_arrival(Event &e, bool &idle, double (&slist)[20000], list<Event> &elist, list<Process> &rQueue, double &clock);
void h_process_departure(Event &e, bool &idle, list<Event> &elist, list<Process> &rQueue, double &clock);
void sortQueue(list<Process> &rQueue, double &clock);
void metrics(double (&tq)[20000], int (&avgRQueue)[20000], double &endTime, double &arrival_rate);

int main()
{
    srand(time(NULL));

    bool cpu_idle = true;
    const int num = 20000;
    int  scheduler = 3;
    double arrival_rate = 12;
    double service_time = 0.06;
    double clock = 0;
    double rlist[num];
    double srate[num];

    list<Event> eQueue;
    list <Process> readyQueue;

    list <Process> :: iterator it;

    generateList(rlist, arrival_rate, srate, service_time);

    for(int i=0; i<num; i++){
        Event a = Event(rlist[i], "ARR", i);
        eQueue.push_back(a);
    }

    eQueue.sort();

    switch (scheduler){
    case 1:
        firstCome(eQueue, readyQueue, cpu_idle, clock, srate, arrival_rate);
        break;
    case 2:
        shortestRemaining(eQueue, readyQueue, cpu_idle, clock, srate, arrival_rate);
        break;
    case 3:
        highestRemaining(eQueue, readyQueue, cpu_idle, clock, srate, arrival_rate);
        break;
    case 4:
        break;
    default:
        cout << "Not a scheduler." << endl;
        cout << "Scheduler must be 1 - 4." << endl;
    }

    return 0;
}

void generateList(double (&rList)[20000], double avRival, double (&slist)[20000], double aServiceTime){
    double r, a;
    double time = 0.0;
    int length = (sizeof(rList) / sizeof(rList[0]));

    for (int i=0; i < length; i++){
        r = ((double) rand() / RAND_MAX);
        rList[i] = r;
    }

    for(int i=0; i<length; i++){
        a = rList[i];
        time += ((-1.0/avRival) * (log(1.0-a)));
        rList[i] = time;
        slist[i] = ((log(1.0-a) * (-1.0 / aServiceTime)));
    }
}

void highestRemaining(list<Event> &eQueue, list<Process> &readyQueue, bool &cpu_idle, double &clock, double (&srate)[20000], double &arrival_rate){
    int count=0;
    double tq[20000];
    int avgRQueue[20000];
    double endTime = 0.0;
    list<Event>::iterator et;
    list<Process>::iterator pt;
    for(et= eQueue.begin(); et != eQueue.end(); et++){
        Event e = Event(et->time, et->eventType, et->id);
        if(e.eventType == "ARR"){
            h_process_arrival(e, cpu_idle, srate, eQueue, readyQueue, clock);
            eQueue.pop_front();
        } else if (e.eventType == "DEP"){
            pt = readyQueue.begin();
            if(pt->id < 20000){
                 if(count < 20000 ){
                    tq[count] = pt->wait_time + 0.06;
                    avgRQueue[count] = readyQueue.size();
                }
                if(count == 19999){
                    endTime = e.time;
                }
                count++;
            }
            h_process_departure(e, cpu_idle, eQueue, readyQueue, clock);
            eQueue.pop_front();
        } else
            cout << "Type not specified !" << endl;
    }
    metrics(tq, avgRQueue, endTime, arrival_rate);
}

void h_process_arrival(Event &e, bool &idle, double (&slist)[20000], list<Event> &elist, list<Process> &rQueue, double &clock){
    Process a = Process(e.id, e.time, slist[e.id]);
    clock = e.time;
    if(idle){
        idle = false;
        schedule_event("DEP", (e.time + slist[e.id]), elist, e.id, clock);
        rQueue.push_back(a);
    } else {
        rQueue.push_back(a);
    }
}

void h_process_departure(Event &e, bool &idle, list<Event> &elist, list<Process> &rQueue, double &clock){
    rQueue.pop_front();
    clock = e.time;
    sortQueue(rQueue, clock);
    list<Process>::iterator it = rQueue.begin();
    if (rQueue.empty()){
        idle = true;
    } else {
        schedule_event("DEP", (clock + it->service_time), elist, it->id, clock);
    }
}

void sortQueue(list<Process> &rQueue, double &clock){
    list<Process>::iterator it;
    double top = 0.0;
    for(it = rQueue.begin(); it != rQueue.end(); it++){
        it->wait_time = (clock - it->arrival_time);
        it->setRTime();
    }
    list<Process>::iterator it2 = rQueue.begin();
    top = it2->rTime;
    for(it = rQueue.begin(); it !=rQueue.end(); it++){
            if(it->rTime > top){
                Process a = Process(it->id, it->arrival_time, it->service_time);
                rQueue.push_front(a);
                top = it->rTime;
                rQueue.erase(it);
            }
    }
}

void firstCome(list<Event> &eQueue, list<Process> &readyQueue, bool &cpu_idle, double &clock, double (&srate)[20000], double &arrival_rate){
    int count=0;
    double tq[20000];
    int avgRQueue[20000];
    double endTime = 0.0;
    double wait = 0.0;
    list<Event>::iterator et;
    list<Process>::iterator pt;
    for(et= eQueue.begin(); et != eQueue.end(); et++){
        Event e = Event(et->time, et->eventType, et->id);
        if(e.eventType == "ARR"){
            process_arrival(e, cpu_idle, srate, eQueue, readyQueue, clock);
            eQueue.pop_front();
        } else if (e.eventType == "DEP"){
            pt = readyQueue.begin();
            wait = clock - pt->arrival_time;
            if(count < 20000 ){
                tq[count] = wait + 0.06;
                avgRQueue[count] = readyQueue.size();
            }
            if(count == 9999){
                endTime = e.time;
            }
            count++;
            process_departure(e, cpu_idle, eQueue, readyQueue, clock);
            eQueue.pop_front();
        } else
            cout << "Type not specified !" << endl;
    }
    metrics(tq, avgRQueue, endTime, arrival_rate);
}

void metrics(double (&tq)[20000], int (&avgRQueue)[20000], double &endTime, double &arrival_rate){
    double turnAroundTime = 0.0;
    double throughput = 0.0;
    double cpu_utilization = 0.0;
    int avgProcesses = 0.0;

    for(int i = 0; i < 20000; i++){
        turnAroundTime += tq[i];
        avgProcesses += avgRQueue[i];
    }

    turnAroundTime = turnAroundTime/20000;
    avgProcesses = avgProcesses/20000;
    cpu_utilization = arrival_rate * 0.06;

    if(cpu_utilization > 1.0)
        cpu_utilization = 1.0;
    throughput = 20000 / endTime;

    cout << "The average turnaround time:\t" << turnAroundTime << endl;
    cout << "The total throughput:\t" << throughput << endl;
    cout << "The CPU utilization:\t" << cpu_utilization << endl;
    cout << "The average number of processes:\t" << avgProcesses << endl;
}

void shortestRemaining(list<Event> &eQueue, list<Process> &readyQueue, bool &cpu_idle, double &clock, double (&srate)[20000], double &arrival_rate){
    int count=0;
    double tq[20000];
    int avgRQueue[20000];
    double endTime = 0.0;

    list<Event>::iterator et;
    double temp_clock = 0.0;
    double temp = 0.0;
    list<Event>::iterator et2 = eQueue.begin();
    list<Process>::iterator pt;
    for(et= eQueue.begin(); et != eQueue.end(); et++){
        Event e = Event(et->time, et->eventType, et->id);
        if(e.eventType == "ARR"){
            s_process_arrival(e, cpu_idle, srate, eQueue, readyQueue, clock, temp_clock, temp);
            et2++;
            list<Process>::iterator rt = readyQueue.begin();
            temp = clock + rt->remaining_time;
            if(temp < et2->time){
                Event d = Event(temp, "DEP", (rt->id * -1));
                eQueue.insert(et2, d);
                eQueue.pop_front();
            }
        } else if (e.eventType == "DEP"){
            pt = readyQueue.begin();
            double wait = e.time - pt->arrival_time;
            if(pt->id < 20000){
                if(count < 20000 ){
                    tq[count] = wait + 0.06;
                    avgRQueue[count] = readyQueue.size();
                }
                if(count == 19999){
                    endTime = e.time;
                }
                count++;
            }
            s_process_departure(e, cpu_idle, eQueue, readyQueue, clock);
            eQueue.pop_front();
            temp_clock = clock;
        } else
            cout << "Type not specified !" << endl;
    }
    metrics(tq, avgRQueue, endTime, arrival_rate);
}

void s_process_arrival(Event &e, bool &idle, double (&slist)[20000], list<Event> &elist, list<Process> &rQueue, double &clock, double &tmpclock, double &temp){
    Process a = Process(e.id, e.time, slist[e.id]);
    clock = e.time;
    rQueue.push_back(a);
    double subtime=0.0;

    list<Process>::iterator rt = rQueue.begin();
    temp = clock + rt->remaining_time;

    if(idle){
        idle = false;
    } else {
        subtime = clock - tmpclock;
        list<Process>::iterator i = rQueue.begin();
        i->remaining_time = i->remaining_time - subtime;
        temp = clock + rt->remaining_time;
    }
    tmpclock = clock;
    rQueue.sort();
}
void s_process_departure(Event &e, bool &idle, list<Event> &elist, list<Process> &rQueue, double &clock){
    rQueue.pop_front();
    clock = e.time;
    if (rQueue.empty()){
        idle = true;
    }
}

void schedule_event(string type, double time, list<Event> &elist, int id, double &clock){
    Event e = Event(time, type, (id * -1));
    list<Event>::iterator it = elist.begin();
    while((e.time > it->time) && it != elist.end()){
        it++;
    }
    elist.insert(it, e);
}

void process_arrival(Event &e, bool &idle, double (&slist)[20000], list<Event> &elist, list<Process> &rQueue, double &clock){
    Process a = Process(e.id, e.time, slist[e.id]);
    clock = e.time;
    if(idle){
        idle = false;
        schedule_event("DEP", (e.time + slist[e.id]), elist, e.id, clock);
        rQueue.push_back(a);
    } else {
        rQueue.push_back(a);
    }
}

void process_departure(Event &e, bool &idle, list<Event> &elist, list<Process> &rQueue, double &clock){
    rQueue.pop_front();
    clock = e.time;
    list<Process>::iterator it = rQueue.begin();
    if (rQueue.empty()){
        idle = true;
    } else {
        schedule_event("DEP", (clock + it->service_time), elist, it->id, clock);
    }
}

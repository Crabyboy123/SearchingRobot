#include <iostream>
#include "ev3dev.h"
#include <unistd.h>
#include <thread>
#include <chrono>

using namespace std;
using namespace ev3dev;

class SearchingEV3{

    motor left;
    motor right;
    motor small_m;
    color_sensor color_s;


public:
    SearchingEV3() : left(OUTPUT_B), right(OUTPUT_C), small_m(OUTPUT_A), color_s(INPUT_4){}
    void startSearching();

private:


};

int main()
{
    motor m1(OUTPUT_B);
    motor m2(OUTPUT_C);
    m1.set_duty_cycle_sp(100);
    //m2.set_duty_cycle_sp(100);
    //m1.set_time_sp(10000).run_timed();
    //m2.set_time_sp(10000).run_timed();
    motor m3(OUTPUT_A);
    m3.set_duty_cycle_sp(30);
    //color_sensor s(INPUT_4);
    m3.set_time_sp(400);
    m3.run_timed();
    while(m3.state().count("running"))
        this_thread::sleep_for(chrono::milliseconds(10));
    m3.set_duty_cycle_sp(-30);
    m3.set_time_sp(400);
    m3.run_timed();
    //mode_set comm = m3.commands();
    /*for(auto it = comm.begin(); it != comm.end() ; ++it)
        cout << *it << endl;*/
    //m3.run_forever();
    return 0;
}


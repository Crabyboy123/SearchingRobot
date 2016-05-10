#include <iostream>
#include "ev3dev.h"
#include <unistd.h>
#include <thread>
#include <chrono>

using namespace std;
using namespace ev3dev;
#define LEFT 1
#define RIGHT -1
#define FORWARD 1
#define BACKWARD -1
#define OPEN 1
#define CLOSE -1

#define ONE_ANGLE round(1520/180)
enum Color{ NONE, BLACK, BLUE, GREEN, YELLOW, RED, WHITE, BROWN };
class SearchingEV3{

    motor left;
    motor right;
    motor arm_m;
    color_sensor color_s;


public:
    SearchingEV3() : left(OUTPUT_B), right(OUTPUT_C), arm_m(OUTPUT_A), color_s(INPUT_1){}
    void startSearching();

    bool detectTheColor(Color color){
        return color_s.color() == color;
    }

    /**
     * @brief moveTheArm, function controls the movement of the arm
     * @param int way, can be CLOSE or OPEN
     */
    void moveTheArm(int way){
        arm_m.set_duty_cycle_sp(way * 30);
        arm_m.set_time_sp(400);
        arm_m.run_timed();
        while(arm_m.state().count("running"))
            this_thread::sleep_for(chrono::milliseconds(10));

    }

    /**
     * @brief turn, function to turn on itself
     * @param side LEFT/RIGHT
     * @param angle <0,360)
     */
    void turn(int side, int angle){
        left.set_duty_cycle_sp(side*(-100));
        right.set_duty_cycle_sp(side*100);
        left.set_time_sp(ONE_ANGLE * angle);
        right.set_time_sp(ONE_ANGLE * angle);
        left.run_timed();
        right.run_timed();
        while(right.state().count("running"))
            this_thread::sleep_for(chrono::milliseconds(10));
    }

    /**
     * @brief move, function to move itself
     * @param direction FORWARD/BACKWARD
     */
    void move(int direction){
        left.set_duty_cycle_sp(direction * 100);
        right.set_duty_cycle_sp(direction * 100);
        left.run_forever();
        right.run_forever();
    }

};

int main()
{
    /*motor m1(OUTPUT_B);
    motor m2(OUTPUT_C);
    m1.set_duty_cycle_sp(100);*/
    //m2.set_duty_cycle_sp(100);
    //m1.set_time_sp(10000).run_timed();
    //m2.set_time_sp(10000).run_timed();
    //motor m3(OUTPUT_A);
    //m3.set_duty_cycle_sp(30);
    //color_sensor s(INPUT_1);
    //m3.set_time_sp(400);
    /*m3.run_timed();
    while(m3.state().count("running"))
        this_thread::sleep_for(chrono::milliseconds(10));
    m3.set_duty_cycle_sp(-30);
    m3.set_time_sp(400);
    m3.run_timed();*/
    //mode_set comm = m3.commands();
    /*for(auto it = comm.begin(); it != comm.end() ; ++it)
        cout << *it << endl;*/
    //m3.run_forever();
    SearchingEV3 robot;
    robot.turn(LEFT,180);
    return 0;
}


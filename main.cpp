#include <iostream>
#include "ev3dev.h"
#include <unistd.h>
#include <thread>
#include <chrono>
#include <vector>

using namespace std;
using namespace ev3dev;

#define LEFT 1
#define RIGHT -1
#define FORWARD 1
#define BACKWARD -1
#define OPEN 1
#define CLOSE -1

#define AREA_LENGTH 5

//2180
//2670
//3000

#define ONE_ANGLE round(3000/180)

int interrupt = 0;

/**
  * Sruct repesents the command for stack
  */
typedef struct Command{
    int argument1;
    int argument2;
} command;

enum Color{ NONE, BLACK, BLUE, GREEN, YELLOW, RED, WHITE, BROWN };
class SearchingEV3{

    motor left;
    motor right;
    motor arm_m;
    touch_sensor button;
    color_sensor color_s;
    vector<command> stack;
    int orientation;
    int posX, posY;

public:
    SearchingEV3() : left(OUTPUT_B), right(OUTPUT_C), arm_m(OUTPUT_A), color_s(INPUT_1), button(INPUT_4), orientation(0), posX(0), posY(0){}
    void startSearching();

    bool detectTheColor(Color color){
        return color_s.color() == color;
    }

    int getTouchValue(){
        return button.value();
    }

    void push_command(int argument1, int argument2 = 0);
    /**
     * @brief goHome, function moves robot to starting coordinates
     */
    void goHome();

    /**
     * @brief moveTheArm, function controls the movement of the arm
     * @param int way, can be CLOSE or OPEN
     */
    void moveTheArm(int way);

    /**
     * @brief turn, function to turn on itself
     * @param side LEFT/RIGHT
     * @param angle <0,360>
     */
    void turn(int side, int angle);

    /**
     * @brief move, function to move itself
     * @param direction FORWARD/BACKWARD
     */
    void move(int direction);

    /**
     * @brief SearchingEV3::backToOrigin, function moves robot to the position where it found the object
     */
    void backToOrigin();


    /**
     * @brief SearchingEV3::goToPoint, function moves robot to proper coordinates
     * @param x, represents x axis
     * @param y, represents y axis
     */
    void goToPoint(int x, int y);


    /**
     * @brief SearchingEV3::search, function searches for the object
     */
    void search();

    /**
     * @brief findAndBringThread, function maintains the process of analyzing color of the object and sending it to proper place
     */
    void findAndBring();

    /**
     * @brief shutDown, function shuts down the robot
     */
    void shutDown();

};

void SearchingEV3::push_command(int argument1, int argument2){
    command cmd;
    cmd.argument1 = argument1;
    cmd.argument2 = argument2;

    stack.push_back(cmd);
}

void SearchingEV3::goHome(){
    if(posY > 0){
        if(orientation == 0){
            this->turn(LEFT, 90);
            this->turn(LEFT, 90);
        }
        while(posY>0){
            this->move(FORWARD);
            posY--;
        }
    }
    if(posX>0){
        if(orientation == 0){
            this->turn(LEFT, 90);
        }
        else{
            this->turn(RIGHT, 90);
        }
        while(posX > 0){
            this->move(FORWARD);
            posX--;
        }
        this->turn(RIGHT, 90);
    }
    sleep(2);
    posX = 0;
    posY = 0;
}

void SearchingEV3::moveTheArm(int way){
    arm_m.reset();
    arm_m.set_duty_cycle_sp(way * 100);
    arm_m.set_time_sp(700);
    arm_m.run_timed();
    this_thread::sleep_for(chrono::milliseconds(900));
}

void SearchingEV3::turn(int side, int angle){
    left.set_duty_cycle_sp(side*(-50));
    right.set_duty_cycle_sp(side*50);
     int time = ONE_ANGLE * angle + 1000;


    left.set_time_sp(ONE_ANGLE * angle);
    right.set_time_sp(ONE_ANGLE * angle);
    if(side == LEFT){
        left.run_timed();
        right.run_timed();
    }
    else{
        right.run_timed();
        left.run_timed();
    }
    this_thread::sleep_for(chrono::milliseconds(time));
    orientation += side*angle;
    if(orientation < 0) orientation += 360;
    orientation = orientation % 360;
}

void SearchingEV3::move(int direction){
    left.reset();
    right.reset();
    left.set_duty_cycle_sp(direction * 80);
    right.set_duty_cycle_sp(direction * 80);

    left.set_time_sp(400);
    right.set_time_sp(400);
    left.run_timed();
    right.run_timed();

    this_thread::sleep_for(chrono::milliseconds(400));

}

void SearchingEV3::backToOrigin(){
    while(!stack.empty()){
        command cmd = stack.back();
        if(cmd.argument2 != 0)
            turn(cmd.argument1, cmd.argument2);
        else
            move(cmd.argument1);
        stack.pop_back();
    }
}

void SearchingEV3::goToPoint(int x, int y){


    bool up = true;
    if (y >= posY){
        if (orientation == 180){

               this->turn(LEFT,90);
               push_command(RIGHT, 90);
               this->turn(LEFT, 90);
               push_command(RIGHT, 90);
        }
        else if (orientation == 90){
            this->turn(RIGHT, 90);
            push_command(LEFT, 90);
        }
        else if (orientation == 270){
            this->turn(LEFT, 90);
            push_command(RIGHT, 90);
        }
        for (int i = 0; i < y - posY; i++){
            this->move(FORWARD);
            push_command(BACKWARD);
        }
    }
    else{
        up = false;
        if (orientation == 0){
            this->turn(LEFT,90);
            push_command(RIGHT, 90);
            this->turn(LEFT,90);
            push_command(RIGHT, 90);
        }
        else if (orientation == 90){
            this->turn(LEFT, 90);
            push_command(RIGHT, 90);
        }
        else if (orientation == 270){
            this->turn(RIGHT, 90);
            push_command(LEFT, 90);
        }
        for (int i = 0; i < posY - y; i++){
            this->move(FORWARD);
            push_command(BACKWARD);
        }
    }

    if (x >= posX){
        if (up){
            this->turn(RIGHT, 90);
            push_command(LEFT, 90);
        }
        else{
            this->turn(LEFT, 90);
            push_command(RIGHT, 90);
        }
        for (int i = 0; i < x - posX; i++){
            this->move(FORWARD);
            push_command(BACKWARD);
        }
    }
    else{
        if (up){
            this->turn(LEFT, 90);
            push_command(RIGHT, 90);
        }
        else{
            this->turn(RIGHT, 90);
            push_command(LEFT, 90);
        }
        for (int i = 0; i < posX - x; i++){
            this->move(FORWARD);
            push_command(BACKWARD);
        }
    }
}

void SearchingEV3::search(){
    cout << "STARTING MOVING" << endl;
	while (1){
		if (interrupt == 0){
            for (; posX < AREA_LENGTH; posX++){
                if (posX > 0 && posX % 2 != 0 && orientation != -180){
                    this->turn(RIGHT, 90);
                    this->move(FORWARD);
                    this->turn(RIGHT, 90);
				}
                else if (posX > 0 && posX % 2 == 0 && orientation != 0){
                    this->turn(LEFT, 90);
                    this->move(FORWARD);
                    this->turn(LEFT, 90);
				}
                if(orientation == 0){
                    for (; posY < AREA_LENGTH; posY++){
                        if (interrupt != 0){
                            break;
                        }
                        this->move(FORWARD);
                    }
                    if (interrupt != 0){
                        break;
                    }
                }
                else{
                        for (; posY >= 0; posY--){
                            if (interrupt != 0){
                                break;
                            }
                            this->move(FORWARD);
                        }
                        if (interrupt != 0){
                            break;
                        }
                    }
			}

			if (interrupt == 0){
                this->goHome();
			}
		}
        std::cout << interrupt << std::endl;
	}
}

void SearchingEV3::findAndBring(){
    cout << "STARTING FINDING" << endl;
	while (1){
        if (this->detectTheColor(RED)){
			interrupt = 1;
            this->moveTheArm(CLOSE);
            this->goToPoint(AREA_LENGTH, AREA_LENGTH);
            this->moveTheArm(OPEN);
            this->backToOrigin();
			interrupt = 0;
		}
        else if (this->detectTheColor(GREEN)){
			interrupt = 1;
            this->moveTheArm(CLOSE);
            this->goToPoint(AREA_LENGTH, -1);
            this->moveTheArm(OPEN);
            this->backToOrigin();
			interrupt = 0;
		}
	}
}

void SearchingEV3::shutDown(){
    while(1){
        if((this->getTouchValue())){
            terminate();
        }
    }
}
/**
 * @brief shutDownThread, thread maintaining the ShutDown of the robot
 * @param robot
 */
void shutDownThread(SearchingEV3 *robot){
    robot->shutDown();
}

/**
 * @brief searchThread, thread maintaining the Searching
 * @param robot
 */
void searchThread(SearchingEV3 *robot){
    robot->search();
}
/**
 * @brief findThread, thread maintaining the Finding and Bringing the object
 * @param robot
 */
void findThread(SearchingEV3 *robot){
    robot->findAndBring();
}

int main()
{

    SearchingEV3 robot;
	//search thread, needs to be interrupted by colorfound
    std::thread search (searchThread, &robot);
    std::thread find (findThread, &robot);
    std::thread shutDown (shutDownThread, &robot);
    search.join();
    find.join();
    shutDown.join();

    return 0;
}


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

#define AREA_LENGTH 20

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
        arm_m.reset();
        arm_m.set_duty_cycle_sp(way * 100);
        cout << "Rotation: " << arm_m.duty_cycle_sp() << endl;
        arm_m.set_time_sp(1000);
        arm_m.run_timed();
        this_thread::sleep_for(chrono::milliseconds(1600));

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

		left.set_time_sp(1000);
		right.set_time_sp(1000);
		left.run_timed();
		right.run_timed();

        /*left.run_forever();
        right.run_forever();*/
    }

	/**
	* @brief backToOrigin, function to move the robot 
	* back to its original position based on the objects color
	* @param color from enum Color
	*/
	void backToOrigin(Color color){
		for (int i = 0; i < AREA_LENGTH + 1; i++){
			this.move(BACKWARD);
		}
		this.turn(LEFT, 90);

		if (color == RED){
			for (int i = 0; i < AREA_LENGTH + 1; i++){
				this.move(BACKWARD);
			}
		}
		else{
			this.move(FORWARD);
		}
	}

	void goToPoint(int x, int y, int position_x, int position_y, int orientation){

		bool up = true;

		if (y >= position_y){
			if (orientation == -180){
				this.turn(LEFT, 180);
			}
			else if (orientation == 90 || orientation == -270){
				this.turn(RIGHT, 90);
			}
			else if (orientation == -90 || orientation == 270){
				this.turn(LEFT, 90);
			}

			for (int i = 0; i < y - position_y; i++){
				this.move(FORWARD);
			}
		}
		else{
			up = false;
			if (orientation == 0){
				this.turn(LEFT, 180);
			}
			else if (orientation == 90 || orientation == -270){
				this.turn(LEFT, 90);
			}
			else if (orientation == -90 || orientation == 270){
				this.turn(RIGHT, 90);
			}

			for (int i = 0; i < position_y - y; i++){
				this.move(FORWARD);
			}
		}


		if (x >= position_x){
			if (up){
				this.turn(RIGHT, 90);
			}
			else{
				this.turn(LEFT, 90);
			}

			for (int i = 0; i < x - position_x; i++){
				this.move(FORWARD);
			}
		}
		else{
			if (up){
				this.turn(LEFT, 90);
			}
			else{
				this.turn(RIGHT, 90);
			}

			for (int i = 0; i < position_x - x; i++){
				this.move(FORWARD);
			}
		}


	}

};

int posX, poxY;
int orientation = 0;
// orientation : 0 when y++ (up), -180 when y-- (down), {90,-270} x-- (left), {-90, 270} x++ (right) 

int interrupt = 0;

void searchThread(){
	while (1){
		if (interrupt == 0){
			//search in square area of the size AREA_LENGTH*AREA_LENGTH
			for (int x = 0; x < AREA_LENGTH; x++){
				posX = x;
				if (x > 0 && x % 2 != 0){
					robot.turn(RIGHT, 90);
					robot.move(FORWARD);
					robot.turn(RIGHT, 90);
					orientation -= 180;
				}
				else if (x > 0 && x % 2 == 0){
					robot.turn(LEFT, 90);
					robot.move(FORWARD);
					robot.turn(LEFT, 90);
					orientation += 180;
				}

				for (int y = 0; y < AREA_LENGTH; y++){
					if (interrupt != 0){
						break;
					}
					posY = y;
					robot.move(FORWARD);
				}
				if (interrupt != 0){
					break;
				}
			}

			if (interrupt == 0){
				// Go back to origin
				if (AREA_LENGTH % 2 != 0){
					robot.turn(RIGHT, 180);
					orientation -= 180;
					for (int i = 0; i < AREA_LENGTH; i++){
						robot.move(FORWARD);
						posY -= 1;
						if (interrupt != 0){
							break;
						}
					}
				}
				if (interrupt == 0){
					robot.turn(RIGHT, 90);
					orientation -= 90;
					for (int i = 0; i < AREA_LENGTH; i++){
						robot.move(FORWARD);
						posX -= 1;
						if (interrupt != 0){
							break;
						}
					}
					if (interrupt == 0){
						robot.turn(RIGHT);
						orientation -= 90;
					}
				}
			}
		}
		std::cout << interupt << std::endl;
	}
}

void findAndBringThread(){
	while (1){
		if (robot.detectTheColor(RED)){
			interrupt = 1;
			robot.moveTheArm(CLOSE);
			robot.goToPoint(AREA_LENGTH, AREA_LENGTH, posX, posY, orientation);
			robot.moveTheArm(OPEN);
			robot.backToOrigin(RED);
			orientation = 0;
			posX = 0;
			posY = 0;
			interrupt = 0;
		}
		else if (robot.detectTheColor(GREEN)){
			interrupt = 1;
			robot.moveTheArm(CLOSE);
			robot.goToPoint(AREA_LENGTH, -1, posX, posY, orientation);
			robot.moveTheArm(OPEN);
			robot.backToOrigin(GREEN);
			orientation = 0;
			posX = 0;
			posY = 0;
			interrupt = 0;
		}
	}
}

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
    //robot.turn(LEFT,180);
    /*while(1){
        if(robot.detectTheColor(GREEN)){
            robot.moveTheArm(CLOSE);
            cout << "COLOR FOUND " << endl;
            break;
        }
    }
    cout << "ENDS" << endl;*/

	//search thread, needs to be interrupted by colorfound
	
	std::thread search (searchThread);
	std::thread find (findAndBringThread);

	search.join();
	find.join();

    return 0;
}


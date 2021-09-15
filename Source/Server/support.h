#ifndef SUPPORT_H
#define SUPPORT_H

#include<algorithm>
#include<thread>
#include<chrono>
#include<vector>

#include<ncurses.h>
#include<wiringPi.h>

const int mapWid=127;
const int mapHei =52;
const int CLOCK_A= 3;
const int CLOCK_B=29;
const int SEND_A = 0;
const int SEND_B =27;
const int RECV_A = 2;
const int RECV_B =28;
const int GREEN  = 1;
const int RED    = 2;
const int QUIT   = 6;

class Utils
{
public:
    static bool bit(unsigned int x,int index)
    {
        return (x>>index)&1;
    }
};

enum ServerResult{NONE=0,AWIN,BWIN};
enum Direction{UP=1,RIGHT,DOWN,LEFT};
enum ClientMove{STAY=0,W,D,S,A,ATTACK};

class GameManager;

class StateManager
{
protected:
    int x,y;
    Direction dir;
public:
    StateManager()=default;
    void setPosition(int x,int y,Direction dir);
    void applyMove(ClientMove move);
    void goOn(int step);

    void Initialization();
    friend class GameManager;
};

class MessageManager
{
protected:
    bool Signal;
    unsigned int send_data,send_next;
    unsigned int send_index,receive_index;
    unsigned int receive_dataA,receive_doneA;
    unsigned int receive_dataB,receive_doneB;
public:
    MessageManager()=default;
    void runMessage(int times);

    void Initialization();
    friend class GameManager;
};

class GameManager
{
protected:
    StateManager playerA;
    StateManager playerB;
    MessageManager message;
    std::vector<StateManager> bulletA;
    std::vector<StateManager> bulletB;
public:
    GameManager()=default;
    void printWall();
    void printState();
    void updateState();
    void printPlayer(const StateManager &player,bool yes);
    void printBullet(const StateManager &bullet,bool yes);

    void Initialization();
    void Game();
};

#endif
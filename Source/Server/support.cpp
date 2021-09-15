#include"support.h"

extern GameManager mainGame;

void GameManager::printWall()
{
    attron(COLOR_PAIR(GREEN));
    for(int i=1;i<=mapHei;i+=1)
        for(int j=1;j<=mapWid;j+=1)
            mvprintw(i,j," "),refresh();
    attroff(COLOR_PAIR(GREEN));

    for(int i=1;i<=mapWid;i+=1)
    {
        mvprintw(1     ,i,"-"),refresh();
        mvprintw(mapHei,i,"-"),refresh();
    }

    for(int i=1;i<=mapHei;i+=1)
    {
        mvprintw(i,1     ,"|"),refresh();
        mvprintw(i,mapWid,"|"),refresh();
    }
    attroff(COLOR_PAIR(GREEN));
}

void GameManager::printPlayer(const StateManager &player,bool yes)
{
    int startX=player.x-1;
    int startY=player.y-2;

    attron(COLOR_PAIR(GREEN));
    for(int i=1;i<=3;i+=1)
    for(int j=1;j<=5;j+=1)
    mvprintw(startX+i-1,startY+j-1,yes?"*":" "),refresh();
    attroff(COLOR_PAIR(GREEN));

    attron(COLOR_PAIR(RED));
    switch(player.dir)
    {
        case Direction::UP:
        mvprintw(player.x-1,player.y,yes?"*":" "),refresh();
        break;
        case Direction::DOWN:
        mvprintw(player.x+1,player.y,yes?"*":" "),refresh();
        break;
        case Direction::LEFT:
        mvprintw(player.x,player.y-1,yes?"*":" "),refresh();
        mvprintw(player.x,player.y-2,yes?"*":" "),refresh();
        break;
        case Direction::RIGHT:
        mvprintw(player.x,player.y+1,yes?"*":" "),refresh();
        mvprintw(player.x,player.y+2,yes?"*":" "),refresh();
        break;
    }
    mvprintw(player.x,player.y,yes?"*":" "),refresh();
    attroff(COLOR_PAIR(RED));
}

void GameManager::printBullet(const StateManager &bullet,bool yes)
{
    attron(COLOR_PAIR(RED));
    mvprintw(bullet.x,bullet.y,yes?"*":" "),refresh();
    attroff(COLOR_PAIR(RED));
}

void GameManager::printState()
{
    printPlayer(playerA,true);
    printPlayer(playerB,true);
    for(auto i=bulletA.begin();i!=bulletA.end();i++) printBullet(*i,true);
    for(auto i=bulletB.begin();i!=bulletB.end();i++) printBullet(*i,true);
}

void GameManager::updateState()
{
    printPlayer(playerA,false);
    printPlayer(playerB,false);
    for(auto i=bulletA.begin();i!=bulletA.end();i++) printBullet(*i,false);
    for(auto i=bulletB.begin();i!=bulletB.end();i++) printBullet(*i,false);

    if(message.receive_doneA==ClientMove::ATTACK)
    {
        StateManager bullet;
        bullet.Initialization();
        bullet.setPosition(playerA.x,playerA.y,playerA.dir);
        bulletA.push_back(bullet);
    }else playerA.applyMove((ClientMove)message.receive_doneA);
    playerA.x=std::max(3,std::min(mapHei-2,playerA.x));
    playerA.y=std::max(4,std::min(mapWid-3,playerA.y));
    message.receive_doneA=0;

    if(message.receive_doneB==ClientMove::ATTACK)
    {
        StateManager bullet;
        bullet.Initialization();
        bullet.setPosition(playerB.x,playerB.y,playerB.dir);
        bulletB.push_back(bullet);
    }else playerB.applyMove((ClientMove)message.receive_doneB);
    playerB.x=std::max(3,std::min(mapHei-2,playerB.x));
    playerB.y=std::max(4,std::min(mapWid-3,playerB.y));
    message.receive_doneB=0;

    for(auto i=bulletA.begin();i!=bulletA.end();i++) i->goOn(1);
    for(auto i=bulletB.begin();i!=bulletB.end();i++) i->goOn(1);

    for(auto i=bulletA.begin();i!=bulletA.end();)
    {
        if(i->x<=1||i->x>=mapHei||i->y<=1||i->y>=mapWid) i=bulletA.erase(i);
        else if(abs(i->x-playerB.x)<=1&&abs(i->y-playerB.y)<=2)
        {
            i=bulletA.erase(i);if(!message.send_next) message.send_next=ServerResult::AWIN;
        }
        else i++;
    }
    for(auto i=bulletB.begin();i!=bulletB.end();)
    {
        if(i->x<=1||i->x>=mapHei||i->y<=1||i->y>=mapWid) i=bulletB.erase(i);
        else if(abs(i->x-playerA.x)<=1&&abs(i->y-playerA.y)<=2)
        {
            i=bulletB.erase(i);if(!message.send_next) message.send_next=ServerResult::BWIN;
        }
        else i++;
    }
}

void GameManager::Initialization()
{
    message.Initialization();

    initscr(),raw(),noecho();

    start_color(),curs_set(0);
    init_pair(GREEN,COLOR_GREEN,COLOR_BLACK);
    init_pair(RED  ,COLOR_RED  ,COLOR_BLACK);

    wiringPiSetup();
    pinMode(CLOCK_A,OUTPUT);
    pinMode(CLOCK_B,OUTPUT);
    pinMode(SEND_A ,OUTPUT);
    pinMode(SEND_B ,OUTPUT);
    pinMode(RECV_A ,INPUT );
    pinMode(RECV_B ,INPUT );
    
    digitalWrite(CLOCK_A,HIGH);
    digitalWrite(CLOCK_B,HIGH);

    printWall();
    int mid=mapWid>>1;
    attron(COLOR_PAIR(RED));
    mvprintw(mid,mapWid-4,"Welcome"),refresh();
    attroff(COLOR_PAIR(RED));

    playerA.Initialization();
    playerB.Initialization();
    playerA.setPosition(10,10,Direction::DOWN);
    playerB.setPosition(30,30,Direction::UP);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void GameManager::Game()
{
    while(true)
    {
        message.runMessage(12);
        if(message.receive_doneA==QUIT) break;
        updateState();
        printState();
    }
    endwin();
}

void StateManager::applyMove(ClientMove move)
{
    if((ClientMove)dir==move)
        goOn(1);
    else if(ClientMove::W<=move&&move<=ClientMove::A)
        dir=(Direction)move;
}

void StateManager::goOn(int step)
{
    switch(dir)
    {
        case Direction::UP:
            x-=step;break;
        case Direction::DOWN:
            x+=step;break;
        case Direction::LEFT:
            y-=step<<1;break;
        case Direction::RIGHT:
            y+=step<<1;break;
    }
}

void MessageManager::runMessage(int times)
{
    for(int i=1;i<=times;i+=1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        Signal=!Signal;
        digitalWrite(CLOCK_A,Signal?HIGH:LOW);
        digitalWrite(CLOCK_B,Signal?HIGH:LOW);

        if(Signal)
        {
            switch(send_data)
            {
                case ServerResult::AWIN:
                digitalWrite(SEND_A,Utils::bit(1,send_index)?HIGH:LOW);
                digitalWrite(SEND_B,Utils::bit(2,send_index)?HIGH:LOW);
                break;
                case ServerResult::BWIN:
                digitalWrite(SEND_A,Utils::bit(2,send_index)?HIGH:LOW);
                digitalWrite(SEND_B,Utils::bit(1,send_index)?HIGH:LOW);
                break;
                default:
                digitalWrite(SEND_A,Utils::bit(0,send_index)?HIGH:LOW);
                digitalWrite(SEND_B,Utils::bit(0,send_index)?HIGH:LOW);
                break;
            }

            bool readA=digitalRead(RECV_A);
            bool readB=digitalRead(RECV_B);
            
            if(readA) receive_dataA=receive_dataA| (1<<receive_index);
            else      receive_dataA=receive_dataA&~(1<<receive_index);

            if(readB) receive_dataB=receive_dataB| (1<<receive_index);
            else      receive_dataB=receive_dataB&~(1<<receive_index);

            send_index=(send_index+1)%4;
            receive_index=(receive_index+1)%4;
            if(!send_index) send_data=send_next,send_next=0;
            if(!receive_index&&!receive_doneA) receive_doneA=receive_dataA;
            if(!receive_index&&!receive_doneB) receive_doneB=receive_dataB;
        }
    }
}

void StateManager::setPosition(int x,int y,Direction dir)
{
    this->x=x,this->y=y;this->dir=dir;
}

void StateManager::Initialization()
{
    x=mapWid>>1,y=mapHei>>1;dir=Direction::UP;
}

void MessageManager::Initialization()
{
    send_next=0,send_data=0;
    send_index=0,receive_index=3;
    receive_doneA=0,receive_dataA=0;
    receive_doneB=0,receive_dataB=0;
    Signal=false;
}
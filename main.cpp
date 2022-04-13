#include "debug.hpp"
#include <iostream>
#include <sqlite3.h>
#include <string>
#include <vector>
#include <cstring>
#include <list>
#include <gpiod.h>
#include <unistd.h>
#include <thread>
#include <chrono>

#pragma region QueryBuffer
class QueryBuffer
{
    private:
        
    public:
        std::vector<std::string> buffer;
        int c_delay;
        QueryBuffer();
        QueryBuffer(std::vector<std::string> buf, int c_delay);

        ~QueryBuffer();
};

QueryBuffer::QueryBuffer(std::vector<std::string> buf,
                            int c_delay = 0)
{
    this->buffer = buf;
    this->c_delay = c_delay;
    D_OUT("QueryBuffer létrehozva")
}

QueryBuffer::~QueryBuffer()
{
}

#pragma endregion //End QueryBuffer

#pragma region DBConnect
class DBConnect
{
    private:
        sqlite3 *DB;
        sqlite3_stmt *dbStep;
        char sql[60] = {};
        char const * data;
    public:
        DBConnect();
        DBConnect(std::string url);
        void connectTO(std::string url);

        std::vector<std::string> DBSelect(int num, std::string table);

        ~DBConnect();
};

DBConnect::DBConnect()
{

}
DBConnect::DBConnect(std::string url)
{
    connectTO(url);
}

void DBConnect::connectTO(std::string url)
{
    url += "redonykeys";
    D_OUT("Database url: " << url)
    int status = sqlite3_open_v2(url.c_str(),&DB,SQLITE_OPEN_READONLY,NULL);
    D_OUT("Database status: " << status)
    if ( status != SQLITE_OK)
    {
        std::cerr << "Database connection fail" << std::endl;
        exit(1);
    }
    else
    {
        D_OUT("Database connection succesfull")
    }
}

std::vector<std::string> DBConnect::DBSelect(int num, std::string table)
{
    D_OUT("DBSelect args: " << num << " " << table)
    sprintf(sql,"SELECT * FROM %s WHERE id = %d", (char*)table.c_str(), num);
    D_OUT(sql)

    if (sqlite3_prepare_v2(DB, sql, -1, &dbStep,0) != SQLITE_OK)
    {
        std::cerr << "Error in select data" << std::endl;
    }
    else D_OUT("Database prepare succesfull")
    
    
    
    if (sqlite3_step(dbStep) != SQLITE_ROW)
    {
        std::cerr << "Error in stepping data" << std::endl;
    }

    
    
    std::vector<std::string> ret;
    for (int i = 1; i < sqlite3_column_count(dbStep); i++)
    {
        if (sqlite3_column_type(dbStep,i) != SQLITE_NULL)
        {
            D_OUT("elozo")
            this->data = (const char*)sqlite3_column_text(dbStep, i);
            D_OUT(this->data)
            D_OUT("kov sor")
        
            ret.push_back(this->data);
            D_OUT("Data: " << this->data << " push to array")
        }
        
    }
    
    //https://github.com/RobertoChapa/C-SQLite_Demo/blob/main/C%2B%2B_SQLite_Demo.cpp
    //https://www.sqlite.org/c3ref/bind_blob.html
    sqlite3_reset(dbStep);
    
    memset(sql, 0, sizeof(sql));

    return ret;
}

DBConnect::~DBConnect()
{
    int fin = sqlite3_finalize(dbStep);
    int cdbc = sqlite3_close_v2(DB);
    if ( cdbc != SQLITE_OK)
    {
        std::cerr << "Error in closing database : fin code: "<< fin <<" code: " << cdbc << std::endl;
    }
    else
    {
        D_OUT("Closing database succes")
    }
}

#pragma endregion // end DBConnect

#pragma region Loader
class Loader
{
private:
    DBConnect *db;
    std::string entry;
    std::string control;
    std::vector<std::string> command;
public:
    Loader(DBConnect *datab);
    void LoadCommand(std::list<QueryBuffer> *qb, int usd, int wnd, int percent);
    ~Loader();
};

Loader::Loader(DBConnect *datab)
{
    this->db = datab;
    this->entry = this->db->DBSelect(1,"entry")[0];
    D_OUT("entry betöltve: "<< this->entry)
}

void Loader::LoadCommand(std::list<QueryBuffer> *qb, int usd, int wnd, int percent = 100)
{
    if (usd < 1 || usd > 3)
    {
        std::cout << "Hibás Comand" << std::endl;
    }
    else if (wnd != 11 &&
                wnd != 12 &&
                wnd != 21 &&
                wnd != 31 &&
                wnd != 32 &&
                wnd != 33 &&
                wnd != 34)
    {
        std::cout << "Hibás redőnyszám" << std::endl;
    }
    else if (percent < 0 || percent > 100)
    {
        std::cout << "Hibás százalék paraméter" << std::endl;
    }
    else
    {
        std::vector<std::string> allcontainer;
        std::vector<std::string> com = this->db->DBSelect(usd,"kapcs");
        //D_OUT("com változó: " << com[0] << " " << com[1])
        std::string tavir = this->db->DBSelect(wnd,"tavir")[0];
        D_OUT("tavir változó: " << tavir)
        for (size_t i = 0; i < com.size(); i++)
        {
            allcontainer.push_back(this->entry+tavir+com[i]);
            D_OUT("for ciklus : " << i << " , vectorba töltve")
        }
        
        qb->push_back(QueryBuffer(allcontainer));
        
        
    }
}

Loader::~Loader()
{

}
#pragma endregion // End Loader

#pragma region Player
class Player
{
private:
    struct gpiod_chip *chip;
    struct gpiod_line *gpioline;
    void PlaySignalLine(std::string str, int plength = 350,
                                            int replay = 4,
                                                int delayafter = 0);
public:
    Player(unsigned int gpiopin);
    void PlaySignal(QueryBuffer *qb);
    ~Player();
};

Player::Player(unsigned int gpiopin)
{
    chip = gpiod_chip_open_by_name("gpiochip0");
    if (!chip)
    {
        D_OUT("Open chip failed")
        exit(1);
    }

    gpioline = gpiod_chip_get_line(chip, gpiopin);
    if (!gpioline) {
        D_OUT("Get line failed");
        exit(2);
    }

    int ret = gpiod_line_request_output(gpioline, "antenna", 0);
    if (ret < 0) {
        D_OUT("Request line as output failed\n");
        exit(3);
    }

}

void Player::PlaySignal(QueryBuffer *qb)
{
    int l = qb->buffer.size();
    for (int i = 0; i < l; i++)
    {
        std::string str = qb->buffer[i];
        PlaySignalLine(str);
        if (i == 2)
        {
            usleep(qb->c_delay);
            PlaySignalLine(str);
        }
    }
    
}

void Player::PlaySignalLine(std::string str, int plength, int replay, int delayafter)
{
    int ret;
    int l = str.length();
    for (int a = 0; a < replay; a++)
    {
        for (int i = 0; i < l; i++)
        {
            switch (str[i])
            {
            case '1':
                ret = gpiod_line_set_value(gpioline, true);
                if (ret < 0) {
                    D_OUT("Set line output failed\n");
                    exit(4);
                }
                
                std::this_thread::sleep_for(std::chrono::microseconds(340));
                break;

            case '0':
                ret = gpiod_line_set_value(gpioline, false);
                if (ret < 0) {
                    D_OUT("Set line output failed\n");
                    exit(4);
                }
                
                std::this_thread::sleep_for(std::chrono::microseconds(340));
                break;
            
            default:
                break;
            }
        }
        ret = gpiod_line_set_value(gpioline, false);
        if (ret < 0) {
                    D_OUT("Set line output failed\n");
                    exit(4);
        }
        

        std::this_thread::sleep_for(std::chrono::microseconds(10000));
    }
    
}



Player::~Player()
{
    gpiod_line_release(gpioline);
    D_OUT("GPIO line release")
    gpiod_chip_close(chip);
    D_OUT("GPIO chip close")
}

#pragma endregion // Player End

int main(int argc, char* argv[])
{
    if (argc < 3) exit(0);
    DBConnect d("");
    Loader load(&d);
    std::list<QueryBuffer> queryBuffer;
    Player p = Player(21);

    int vec = atoi(argv[1]);

    for (int i = 2; i < argc; i++)
    {
        load.LoadCommand(&queryBuffer,vec, atoi(argv[i]));
    }
    //std::cout << queryBuffer.size() << std::endl;
    while (!queryBuffer.empty())
    {
        p.PlaySignal(&queryBuffer.front());
        queryBuffer.pop_front();
    }
    
}

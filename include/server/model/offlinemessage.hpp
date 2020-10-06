#ifndef OFFLINEMESSAGE_H
#define OFFLINEMESSAGE_H

#include<iostream>
#include<string>
#include<vector>

using namespace std;

class OfflineMessage
{
    public:
        void insert(int userid,string meg);
        vector<string> query(int userid);
        void remove(int userid);
    private:

};


#endif
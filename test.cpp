#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <iostream>
#include <curl/curl.h>
#include <string>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace std;
using namespace rapidjson;

class RPCAgent {

    static char *rpc_usr;
    static char *rpc_pwd;
    static char *rpc_url;
    static char *userpwd;
    static CURL *curl;
    static string fb_buffer;
    
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
        size_t fb_size = size * nmemb;
        fb_buffer.clear();
        fb_buffer.append((const char *)contents, size*nmemb);
        return fb_size;
    }
    
public:
    static string send_raw_req(const string &data) {
        struct curl_slist *headers = curl_slist_append(NULL, "content-type: text/plain;");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, rpc_url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.size());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        
        curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd);
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, RPCAgent::WriteCallback);
        curl_easy_perform(curl);
        
        string retVal(fb_buffer);
        fb_buffer.clear();
        return retVal;
    
    }
    
    static void init(const string &usr, const string &pwd, const string &url) {
        
        curl = curl_easy_init();
        
        //TODO: handle exception
        if(curl==NULL)
            cout<<"curl init failed!"<<endl;
        
        rpc_usr = new char[usr.size()+1];
        rpc_pwd = new char[pwd.size()+1];
        rpc_url = new char[url.size()+1];
        strcpy(rpc_usr, usr.c_str());
        strcpy(rpc_pwd, pwd.c_str());
        strcpy(rpc_url, url.c_str());
        userpwd = new char[usr.size() + pwd.size() + 1];
        strcpy(userpwd, rpc_usr);
        strcat(userpwd, ":");
        strcat(userpwd, rpc_pwd);
        
    }
    
    static void clean() {
        curl_easy_cleanup(curl);
        delete[] rpc_usr;
        delete[] rpc_pwd;
        delete[] rpc_url;
        delete[] userpwd;
    }
    
    
    
};

char *RPCAgent::rpc_usr;
char *RPCAgent::rpc_pwd;
char *RPCAgent::rpc_url;
char *RPCAgent::userpwd;
string RPCAgent::fb_buffer;
CURL *RPCAgent::curl;

int main()
{

    const string req =
        "{\"jsonrpc\": \"1.0\", \"id\":\"curltest\", \"method\": \"getblockhash\", \"params\": [200000] }";
    
    RPCAgent::init("Ulysseys",
                   "YourSuperGreatPasswordNumber_DO_NOT_USE_THIS_OR_YOU_WILL_GET_ROBBED",
                   "http://127.0.0.1:8332/");

    string ans = RPCAgent::send_raw_req(req);
    
    const char *json=ans.c_str();
    Document d;
    d.Parse(json);
    Value& s = d["result"];
    cout<<s.GetString()<<endl;


    return 0;
}
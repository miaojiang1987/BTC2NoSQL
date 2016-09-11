#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <cassert>
#include <iostream>
#include <sstream>
#include <curl/curl.h>
#include <string>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

using namespace std;
using namespace rapidjson;

class RPCAgent {

    static char *rpc_usr;
    static char *rpc_pwd;
    static char *rpc_url;
    static char *userpwd;
    static CURL *curl;
    
    static string feedback_buffer;
    
    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
        size_t fb_size = size * nmemb;
        //feedback_buffer.clear();
        //cout<<"size="<<fb_size<<endl;
        feedback_buffer.append((const char *)contents, size*nmemb);
        return fb_size;
    }
    

    static void send_raw_req(const string &data) {
        struct curl_slist *headers = curl_slist_append(NULL, "content-type: text/plain;");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, rpc_url);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.size());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        
        curl_easy_setopt(curl, CURLOPT_USERPWD, userpwd);
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, RPCAgent::WriteCallback);
        curl_easy_perform(curl);
    }

public:
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

    static string getblockhash(unsigned int height) {
        stringstream ss_req;
        ss_req<<"{\"jsonrpc\": \"1.0\", \"id\":\"getblockhash\", \"method\": \"getblockhash\", \"params\": ["
              <<height<<"]}";
        feedback_buffer.clear();
        send_raw_req(ss_req.str());
        Document json;
        json.Parse(feedback_buffer.c_str());
        Value& blockhash = json["result"];
        return blockhash.GetString();
    }
    
    static string getblock(unsigned int height) {
        
        string blockhash = getblockhash(height);
        stringstream ss_req;
        ss_req<<"{\"jsonrpc\": \"1.0\", \"id\":\"getblock\", \"method\": \"getblock\", \"params\": [\""
              <<blockhash<<"\"]}";
        feedback_buffer.clear();
        send_raw_req(ss_req.str());
        
        Document json;
        json.Parse(feedback_buffer.c_str());
        assert(json["result"].IsObject());
        Value v = json["result"].GetObject();
        
        StringBuffer bf;
        PrettyWriter<StringBuffer> writer(bf);
        v.Accept(writer);
        return bf.GetString();
    }
    
    
    static string gettx(const string &txID) {
        
        stringstream ss_req;
        ss_req<<"{\"jsonrpc\": \"1.0\", \"id\":\"gettx\", \"method\": \"getrawtransaction\", \"params\": [\""
              <<txID<<"\","<<1<<"]}";
        feedback_buffer.clear();
        send_raw_req(ss_req.str());
        
        Document json;
        json.Parse(feedback_buffer.c_str());
        assert(json["result"].IsObject());
        Value v = json["result"].GetObject();
        v.RemoveMember("hex");
        
        StringBuffer bf;
        PrettyWriter<StringBuffer> writer(bf);
        v.Accept(writer);
        return bf.GetString();
    }
    
};

char *RPCAgent::rpc_usr;
char *RPCAgent::rpc_pwd;
char *RPCAgent::rpc_url;
char *RPCAgent::userpwd;
string RPCAgent::feedback_buffer;
CURL *RPCAgent::curl;

int main()
{
    RPCAgent::init("Ulysseys",
                   "YourSuperGreatPasswordNumber_DO_NOT_USE_THIS_OR_YOU_WILL_GET_ROBBED",
                   "http://127.0.0.1:8332/");

    string ans = RPCAgent::gettx("bcb887acb2c01b6c5c8b92c22a368135d207f07a26eff170fe730b1cd40d2547");
    cout<<ans<<endl;

    return 0;
}

#include <stdlib.h>
#include <stdio.h>
#include <cstring>
#include <cassert>
#include <iostream>
#include <sstream>
#include <curl/curl.h>
#include <string>
#include <bson.h>
#include <mongoc.h>
#include <vector>

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
    
    static Value getblock(unsigned int height) {
        
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
        
        return v;
    }
    
    static Value gettx(const string &txID) {
        
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

        return v;
    }
    
};

char *RPCAgent::rpc_usr;
char *RPCAgent::rpc_pwd;
char *RPCAgent::rpc_url;
char *RPCAgent::userpwd;
string RPCAgent::feedback_buffer;
CURL *RPCAgent::curl;

class MongoAgent {
    mongoc_client_t *client;
    mongoc_collection_t *collection;
    bson_error_t error;
    bson_t cmd_reply;
    const string db_name;
    const string clt_name;
    const string key_name;
    
    string runCmd(const string &cmd) {
        bson_t *command = bson_new_from_json((const uint8_t *)cmd.c_str(), -1, &error);
        string retVal;
        if (mongoc_collection_command_simple(collection, command, NULL, &cmd_reply, &error)) {
            char *str = bson_as_json (&cmd_reply, NULL);
            retVal.insert(0, str);
            bson_free (str);
        } else {
            fprintf (stderr, "Failed to run command: %s\n", error.message);
        }
        bson_destroy(command);
        return retVal;
    }
    
public:
    MongoAgent(const string &url, const string &db, const string &clt, const string &key):
    db_name(db), clt_name(clt), key_name(key)  {
        mongoc_init ();
        client = mongoc_client_new (url.c_str());
        collection = mongoc_client_get_collection(client, db.c_str(), clt.c_str());
        const string CMD_CREATE_INDEX = "{\"createIndexes\":\"" + clt + "\", \"indexes\":[{\"key\":{\"" + key_name
            + "\": 1},\"name\":\"" + key_name + "\",\"unique\":true}]}";
        runCmd(CMD_CREATE_INDEX);
    }
    
    void insert(const string &doc) {
        bson_t *new_doc = bson_new_from_json ((const uint8_t *)doc.c_str(), -1, &error);
        if (!mongoc_collection_insert (collection, MONGOC_INSERT_NONE, new_doc, NULL, &error)) {
            fprintf (stderr, "%s\n", error.message);
        }
        bson_destroy(new_doc);
    }
    
    void bulk_insert(const vector<string> &docs) {
        mongoc_bulk_operation_t *bulk = mongoc_collection_create_bulk_operation(collection, true, NULL);
        bson_t reply;
        for (const string &doc: docs) {
            bson_t *new_doc = bson_new_from_json ((const uint8_t *)doc.c_str(), -1, &error);
            mongoc_bulk_operation_insert(bulk, new_doc);
            bson_destroy(new_doc);
        }
        if (!mongoc_bulk_operation_execute(bulk, &reply, &error)) {
            fprintf (stderr, "Error: %s\n", error.message);
        }
        mongoc_bulk_operation_destroy(bulk);
        bson_destroy (&reply);
    }
    
    virtual ~MongoAgent() {
        mongoc_collection_destroy (collection);
        mongoc_client_destroy (client);
        mongoc_cleanup ();
    }
};


string json_dumps(const Value &v) {
    StringBuffer bf;
    PrettyWriter<StringBuffer> writer(bf);
    v.Accept(writer);
    return bf.GetString();
}

vector<string> get_tx_list(const Value &block) {
    assert(block["tx"].IsArray());
    vector<string> retVal;
    for (SizeType i = 0; i < block["tx"].Size(); i++) // Uses SizeType instead of size_t
        retVal.push_back(block["tx"][i].GetString());
    return retVal;
}


int main()
{
    RPCAgent::init("Ulysseys",
                   "YourSuperGreatPasswordNumber_DO_NOT_USE_THIS_OR_YOU_WILL_GET_ROBBED",
                   "http://127.0.0.1:8332/");
    MongoAgent mAgent1("mongodb://localhost:27017/", "mydb", "mycoll1", "height");
    MongoAgent mAgent2("mongodb://localhost:27017/", "mydb", "mycoll2", "txid");
    
    
    for(int i=100;i<200;i++) {
        Value block = RPCAgent::getblock(i);
        vector<string> list = get_tx_list(block);
        mAgent1.insert(json_dumps(block));
        vector<string> tx;
        for(string &l: list)
            tx.push_back(json_dumps(RPCAgent::gettx(l)));
        mAgent2.bulk_insert(tx);
    }
    RPCAgent::clean();

    return 0;
}



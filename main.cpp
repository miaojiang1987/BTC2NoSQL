
#include <bson.h>
#include <mongoc.h>
#include <stdio.h>

#include <iostream>
#include <string>
#include <vector>


//g++ -o main main.cpp -l curl $(pkg-config --cflags --libs libmongoc-1.0) -std=c++11
using namespace std;

class MongoAgent {
    mongoc_client_t *client;
    mongoc_collection_t *collection;
    bson_error_t error;
    bson_t cmd_reply;
    const char *CMD_CREATE_INDEX = "{\"createIndexes\":\"mycoll\", \"indexes\":[{\"key\":{\"i\": 1},\"name\":\"i\",\"unique\":true}]}";
    
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
    MongoAgent(const string &url, const string &db, const string &clt) {
        mongoc_init ();
        client = mongoc_client_new (url.c_str());
        collection = mongoc_client_get_collection(client, db.c_str(), clt.c_str());
        runCmd(CMD_CREATE_INDEX);
        
    }
    
    void insert(const string &doc) {
        bson_t *new_doc = bson_new_from_json ((const uint8_t *)doc.c_str(), -1, &error);
        if (!mongoc_collection_insert (collection, MONGOC_INSERT_NONE, new_doc, NULL, &error)) {
            fprintf (stderr, "%s\n", error.message);
        }
        bson_destroy(new_doc);
    }
    
    void bulk_insert(const vector<bson_t *> &docs) {
        mongoc_bulk_operation_t *bulk = mongoc_collection_create_bulk_operation(collection, true, NULL);
        bson_t reply;
        for (bson_t *doc: docs) {
            mongoc_bulk_operation_insert(bulk, doc);
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



int main (int argc, char *argv[])
{
    MongoAgent mAgent("mongodb://localhost:27017/", "mydb", "mycoll");
    vector<bson_t *> docs;
    for (int i = 0; i < 10; i++) {
        bson_t *doc = BCON_NEW ("i", BCON_INT32 (i));
        docs.push_back(doc);
    }
    mAgent.bulk_insert(docs);
    for(bson_t *doc: docs)
        bson_destroy(doc);

    return 0;
    
}





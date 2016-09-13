
#include <bson.h>
#include <mongoc.h>
#include <stdio.h>


//g++ -o main main.cpp -l curl $(pkg-config --cflags --libs libmongoc-1.0) -std=c++11

void bulk_test()
{
    mongoc_client_t *client;
    mongoc_collection_t *collection;
    
    mongoc_init ();
    
    client = mongoc_client_new ("mongodb://localhost:27017/");
    collection = mongoc_client_get_collection (client, "mydb", "mycoll");
    
    mongoc_bulk_operation_t *bulk;
    bson_error_t error;
    bson_t *doc;
    bson_t reply;
    char *str;
    bool ret;
    int i;
    
    bulk = mongoc_collection_create_bulk_operation (collection, true, NULL);
    
    for (i = 0; i < 10000; i++) {
        doc = BCON_NEW ("i", BCON_INT32 (i));
        mongoc_bulk_operation_insert (bulk, doc);
        bson_destroy (doc);
    }
    
    ret = mongoc_bulk_operation_execute (bulk, &reply, &error);
    
    str = bson_as_json (&reply, NULL);
    printf ("%s\n", str);
    bson_free (str);
    
    if (!ret) {
        fprintf (stderr, "Error: %s\n", error.message);
    }
    
    bson_destroy (&reply);
    mongoc_bulk_operation_destroy (bulk);
    
    mongoc_collection_destroy (collection);
    mongoc_client_destroy (client);
    
    mongoc_cleanup ();
    
}

int main (int argc, char *argv[])
{
    mongoc_client_t *client;
    mongoc_collection_t *collection;
    //mongoc_cursor_t *cursor;
    bson_error_t error;
    //bson_oid_t oid;
    //bson_t *doc;
    
    mongoc_init ();
    client = mongoc_client_new ("mongodb://localhost:27017/");
    collection = mongoc_client_get_collection (client, "mydb", "mycoll");
    
    
    //doc = bson_new ();
    //bson_oid_init (&oid, NULL);
    //BSON_APPEND_OID (doc, "_id", &oid);
    //BSON_APPEND_UTF8 (doc, "aa?", "hoho!");
    
    
    bson_t *new_doc = bson_new_from_json ((const uint8_t *)"{\"aa???\":\"wwwwwwwwww\"}", -1, &error);
    if (!mongoc_collection_insert (collection, MONGOC_INSERT_NONE, new_doc, NULL, &error)) {
        fprintf (stderr, "%s\n", error.message);
    }
    
    //bson_destroy (doc);
    mongoc_collection_destroy (collection);
    mongoc_client_destroy (client);
    mongoc_cleanup ();
    bulk_test();
    return 0;
}





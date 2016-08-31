#include <stdlib.h>
#include <cstring>
#include <iostream>
#include <curl/curl.h>
#include <string>
using namespace std;

string readBuffer;

size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    readBuffer.append((const char *)contents, size*nmemb);
    return realsize;
}


int main()
{
    CURL *curl = curl_easy_init();
    struct curl_slist *headers = NULL;
    
    
    if (curl) {
        
        const char *data =
        "{\"jsonrpc\": \"1.0\", \"id\":\"curltest\", \"method\": \"getinfo\", \"params\": [] }";
        
        headers = curl_slist_append(headers, "content-type: text/plain;");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        
        curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:8332/");
        
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long) strlen(data));
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        
        curl_easy_setopt(curl, CURLOPT_USERPWD,
                         "Ulysseys:YourSuperGreatPasswordNumber_DO_NOT_USE_THIS_OR_YOU_WILL_GET_ROBBED");
        
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_TRY);
        readBuffer.clear();
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        int res = curl_easy_perform(curl);
        cout<<"readBuffer"<<endl;
        cout<<readBuffer<<endl;
    }
    
    return 0;
}
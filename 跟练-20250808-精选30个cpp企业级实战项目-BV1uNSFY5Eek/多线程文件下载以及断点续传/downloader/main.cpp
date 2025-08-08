
#include <iostream>
#include <curl/curl.h>
#include <cstdint>

std::uint32_t write_Function(void * ptr, std::uint32_t size, std::uint32_t member, void * user_data)
{
    std::cout << "write_Function" << std::endl;
    return size * member;
}

int download(const char * url, const char * filename)
{
    CURL *curl_1 = curl_easy_init();
    curl_easy_setopt(curl_1, CURLOPT_URL, url);
    curl_easy_setopt(curl_1, CURLOPT_WRITEFUNCTION, write_Function);

    CURLcode result_curl_1 = curl_easy_perform(curl_1);
    if (result_curl_1 != CURLE_OK)
    {
        std::printf("res: %d\n", result_curl_1);
    }
    curl_easy_cleanup(curl_1);
    return 0;
}

int main(int argc, char * argv[])
{
    return download(argv[0], argv[1]);
}

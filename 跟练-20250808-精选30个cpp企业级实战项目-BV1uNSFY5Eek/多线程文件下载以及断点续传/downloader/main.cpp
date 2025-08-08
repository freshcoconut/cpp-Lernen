#include <iostream>
#include <curl/curl.h>
#include <cstdint>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

struct fileInfo{
    char * file_ptr;
    int offset;
};

std::uint32_t write_Function(void * ptr, std::uint32_t size, std::uint32_t member, void * user_data)
{
    struct fileInfo * info = static_cast<struct fileInfo *>(user_data);
    std::uint32_t result = size * member;
    //std::cout << "write_Function: " << result << std::endl;
    char * file_ptr = static_cast<char *>(user_data);
    std::memcpy(info->file_ptr + info->offset, ptr, result);
    info->offset += result;
    return result;
}

std::int64_t get_Download_File_Size(const char * url)
{
    std::int64_t downloadFileLength = 0;
    CURL *curl_1 = curl_easy_init();
    curl_easy_setopt(curl_1, CURLOPT_URL, url);
    curl_easy_setopt(curl_1, CURLOPT_HEADER, 1);
    curl_easy_setopt(curl_1, CURLOPT_NOBODY, 1);

    CURLcode result_curl_1 = curl_easy_perform(curl_1);
    if (result_curl_1 == CURLE_OK)
    {
        std::cout << "The downloadFileLength succeeds!" << std::endl;
        curl_easy_getinfo(curl_1, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &downloadFileLength);
    }
    else
    {
        std::cout << "The downloadFileLength meets error!" << std::endl;
        downloadFileLength = -1;
    }
    curl_easy_cleanup(curl_1);

    return downloadFileLength;
}

int download(const char * url, const char * filename)
{
    //writing
    int fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        return -1;
    }

    std::uint64_t downloadFileLength = get_Download_File_Size(url);
    std::cout << "downloadFileLength: " << downloadFileLength << std::endl;

    if (lseek(fd, downloadFileLength - 1, SEEK_SET) == -1)
    {
        perror("lseek");
        close(fd);
        return -1;
    }
    if (write(fd, "", 1) == 1)
    {
        perror("write");
        close(fd);
        return -1;
    }

    char * file_ptr = static_cast<char *>(mmap(NULL, downloadFileLength, PROT_READ, PROT_WRITE, fd, 0));
    if (file_ptr == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        return -1;
    }

    struct fileInfo * info = static_cast<struct fileInfo *>(malloc(sizeof(struct fileInfo)));
    if (info == nullptr)
    {
        munmap(file_ptr, downloadFileLength);
        close(fd);
        return -1;
    }
    info->file_ptr = file_ptr;
    info->offset = 0;

    CURL *curl_1 = curl_easy_init();
    curl_easy_setopt(curl_1, CURLOPT_URL, url);
    curl_easy_setopt(curl_1, CURLOPT_WRITEFUNCTION, write_Function);
    curl_easy_setopt(curl_1, CURLOPT_WRITEDATA, info);

    CURLcode result_curl_1 = curl_easy_perform(curl_1);
    if (result_curl_1 != CURLE_OK)
    {
        std::printf("res: %d\n", result_curl_1);
    }
    curl_easy_cleanup(curl_1);

    munmap(file_ptr, downloadFileLength);
    close(fd);

    return 0;
}

int main(int argc, char * argv[])
{
    if (argc != 3)
    {
        std::cout << "argc error!" << std::endl;
        return -1;
    }
    return download(argv[1], argv[2]);
}

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string.h>
#include "pugixml.hpp"
#include <vector>
#include <fstream>


class Functions {
    public:
        static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
        static std::string trimUrl(const std::string& url);
        static std::vector<std::string> extractCodes(const std::string &text);
        static std::string extract_code(const std::string& text);
        static void split_conditions(const std::string& requirement, std::vector<std::vector<std::string>>& or_groups);
        static void curlRequest(std::string scrapingUrl);
        static std::vector<std::string> courseInformationScrape(pugi::xml_document& doc);
        static void courseInformationProcess(std::string coursesUrl, std::vector<std::string>& data);
        static void prerequisiteProcess(std::ofstream& prereqTable, std::string row, std::string currentCourse);
        static void crossListingProcess(std::ofstream& crossListingTable, std::string row, std::string currentCourse);
    };



#endif
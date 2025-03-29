#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <string.h>
#include "pugixml.hpp"
#include <vector>
#include <fstream>


class Functions {
    public:
        static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
        static std::string getMajorFromUrl(const std::string& url);
        static std::string getDepartmentFromUrl(const std::string& url);
        static std::vector<std::string> extractCodes(const std::string &text);
        static std::string extract_code(const std::string& text);
        static void split_conditions(const std::string& requirement, std::vector<std::vector<std::string>>& or_groups);
        static void curlRequest(std::string scrapingUrl);
        static std::vector<std::string> courseInformationScrape(pugi::xml_document& doc);
        static void degreeInformationProcessAndScrape(pugi::xml_document& doc, int id, std::string url, int &group);
        static void courseInformationProcess(std::string coursesUrl, std::vector<std::string>& data);
        static void prerequisiteProcess(std::ofstream& prereqTable, std::string row, std::string currentCourse);
        static void crossListingProcess(std::ofstream& crossListingTable, std::string row, std::string currentCourse);
        static void removeNewlines(std::string& input);
        static void extract_text(const pugi::xml_node& node, std::string& result);
        static void replaceNbsp(std::string& text, std::string repl);
        static void processMajorRules(pugi::xml_node& node_2, std::string& majorRules);
        static void processCourseSuperscripts(pugi::xml_node& node_2, std::vector<std::string>& courseSuperscripts);
        static void processCourseTables(pugi::xml_node &node_2, std::vector<std::string>& courseSuperscripts, std::vector<std::string> &data, const int &majorID, int &credits, int &group);
        static void processCourseRow(pugi::xml_node& node_4, int& year, int& season, int& group, int& hours, const int& majorID, std::vector<std::string>& data, std::vector<std::string>& courseSuperscripts);

    };



#endif
#include "functions.h"
#include <iostream>
#include <regex>
#include <curl/curl.h>




size_t Functions::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::ofstream* file = static_cast<std::ofstream*>(userp);
    file->write(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

std::string Functions::trimUrl(const std::string& url) {
    size_t lastSlash = url.find_last_of('/');
    size_t secondLastSlash = url.find_last_of('/', lastSlash - 1);

    if (secondLastSlash == std::string::npos || lastSlash == std::string::npos) {
        return "invalid"; // Handle unexpected format
    }

    std::string trimmed = url.substr(secondLastSlash + 1, lastSlash - secondLastSlash - 1);
    return trimmed;
}

std::vector<std::string> Functions::extractCodes(const std::string &text) {
    std::regex pattern(R"([A-Z]{4}\d{3})"); // Matches four capital letters followed by three digits
    std::vector<std::string> matches;
    std::sregex_iterator it(text.begin(), text.end(), pattern);
    std::sregex_iterator end;

    while (it != end) {
        matches.push_back(it->str());
        ++it;
    }
    return matches;
}

std::string Functions::extract_code(const std::string& text) {
    std::regex pattern(R"([A-Z]{4}\d{3})");
    std::smatch match;

    if (std::regex_search(text, match, pattern)) {
        return match.str(0); // Return the first match
    }
    return ""; // Return empty string if no match
}

void Functions::split_conditions(const std::string& requirement, std::vector<std::vector<std::string>>& or_groups) {
    std::regex and_split(R"(\s+and\s+)", std::regex::icase);
    std::regex or_split(R"(\s+or\s+|,\s*)", std::regex::icase);

    std::vector<std::string> and_parts;
    std::sregex_token_iterator and_it(requirement.begin(), requirement.end(), and_split, -1);
    std::sregex_token_iterator end;

    // First, split by "and"
    while (and_it != end) {
        and_parts.push_back(*and_it++);
    }

    // Then, split each AND part by "or" or ","
    for (const auto& and_part : and_parts) {
        std::vector<std::string> or_part_group;
        std::sregex_token_iterator or_it(and_part.begin(), and_part.end(), or_split, -1);

        while (or_it != end) {
            or_part_group.push_back(*or_it++);
        }

        if (!or_part_group.empty()) {
            or_groups.push_back(or_part_group);
        }
    }
}

void Functions::curlRequest(std::string scrapingUrl){
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL" << std::endl;
    }

    std::ofstream file("raw.html", std::ios::out | std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open raw.html" << std::endl;
    }

    curl_easy_setopt(curl, CURLOPT_URL, scrapingUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Functions::WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // Follow redirects

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
    }

    curl_easy_cleanup(curl);
    file.close();
    std::cout << "HTML saved to raw.html" << std::endl;
    std::string command = "tidy -q -utf8 -asxml -o cleaned.html raw.html 2>/dev/null";
    system(command.c_str());
    std::cout << "Cleaned HTML saved to cleaned.html" << std::endl;
}

std::vector<std::string> Functions::courseInformationScrape(pugi::xml_document& doc){
    std::vector<std::string> data;
    for (pugi::xpath_node xpath_node : doc.select_nodes("//div[@class='courseblock']")) {
        pugi::xml_node div = xpath_node.node();  // Access the xml_node from xpath_node
        data.push_back(div.child("h2").text().get());
        data.push_back(div.child("p").child("span").child("strong").text().get());
        data.push_back(div.child("p").text().get());
        std::string fulltext=div.child("p").child("strong").text().get();
        pugi::xml_node temp = div.child("p").child("strong").next_sibling();
        while(temp){
            fulltext+=temp.text().get();
            temp=temp.next_sibling();
        }
        data.push_back(fulltext);

    
    }
    return data;
}

void Functions::courseInformationProcess(std::string coursesUrl, std::vector<std::string>& data){
    std::string trimmedUrl = Functions::trimUrl(coursesUrl);
    std::string filename = trimmedUrl + "-courses.txt";
    std::ofstream courseTable(trimmedUrl + "CourseTable.csv");
    std::ofstream prereqTable(trimmedUrl + "PrereqTable.csv");
    std::ofstream crossListingTable(trimmedUrl + "CrossListingTable.csv");
    courseTable<<"Course Code, Course Name, Field, Credits, Lecture, Lab, Description, Difficulty"<<std::endl;
    prereqTable<<"Course Code, Course Code, Grade Req, Group, Text, Concurrent Enrollment"<<std::endl;
    crossListingTable<<"Course Code, Course Code"<<std::endl;
    std::string currentCourse="";
    for(std::string row: data){
        while(row.find("&nbsp;")!=std::string::npos){
            row.replace(row.find("&nbsp;"), 6,"");
        }
        size_t pos;
        while ((pos = row.find("\n")) != std::string::npos) {
            row.replace(pos, 1, " ");  // Replace the newline with a space
        }
        if(row.length()<8 || row.substr(0,5)=="Prere" || row.substr(0,5)=="Cross"){
            //prereqTable<<row<<std::endl;
           if(row.find("Prerequisite")!=std::string::npos){
                Functions::prerequisiteProcess(prereqTable, row, currentCourse);
           }
           if(row.find("Cross Listing")!=std::string::npos){
                Functions::crossListingProcess(crossListingTable, row, currentCourse);
           }
        }
        else if(isdigit(row[4]) && isdigit(row[5]) && isdigit(row[6])){
            courseTable<<row.substr(0,7)<<","<<row.substr(8)<<","<<row.substr(0,4)<<",";
            currentCourse=row.substr(0,7);
        }
        else if(row.substr(0,6)=="Credit"){
            if(row.substr(0,7)=="Credits"){
                courseTable<<row.substr(8,1)<<",";
            }
            else{
                courseTable<<row.substr(7,1)<<",";
            }
            if(row.find("Lecture Hour")!=std::string::npos){
                courseTable<<row.substr(row.find("Lecture Hour")-2,1)<<",";
            }
            else{
                courseTable<<0<<",";
            }
            if(row.find("Lab Hour")!=std::string::npos){
                courseTable<<row.substr(row.find("Lab Hour")-2,1)<<",";
            }else{
                courseTable<<0<<",";
            }
        }
        else if(row.length()>3){
            courseTable<<row.substr(1)<<","<<0<<std::endl;
        }
    }
    courseTable.close();
    prereqTable.close();
    crossListingTable.close();
}

void Functions::prerequisiteProcess(std::ofstream& prereqTable, std::string row, std::string currentCourse){
    size_t prerequisitesPos = row.find("Prerequisite:");
    if(prerequisitesPos!=std::string::npos){
        row=row.substr(prerequisitesPos+13);
    }
    prerequisitesPos = row.find("Prerequisites:");
    if(prerequisitesPos!=std::string::npos){
        row=row.substr(prerequisitesPos+14);
    }
    size_t crossListingPos = row.find("Cross Listing");
    if (crossListingPos != std::string::npos) {
        row = row.substr(0, crossListingPos); 
    } 
    std::vector<std::string> splitStrings;
    size_t start = 0;
    size_t end = row.find(';');
    while (end != std::string::npos) {
        splitStrings.push_back(row.substr(start, end - start));
        start = end + 1;
        end = row.find(';', start);
        
    }
    splitStrings.push_back(row.substr(start));
    std::string group="A";
    for(std::string requirement: splitStrings){
        const std::string phrase = "also taught at";
        if (requirement.size() >= phrase.size() && requirement.find(phrase) != std::string::npos) {
            requirement.erase(requirement.find(phrase));
        }             
        
        if(requirement!=" "){
            std::regex pattern(R"((Grade|grade) of ([A-Z]) or better\s*)");
            std::smatch match;
            std::string gradeReq="";

            if (std::regex_search(requirement, match, pattern)) {
                gradeReq = match[2].str(); // Store the letter grade
                requirement = std::regex_replace(requirement, pattern, ""); // Remove the phrase
            }
            std::vector<std::vector<std::string>> or_groups;
            Functions::split_conditions(requirement, or_groups);
            
            for(std::vector<std::string> a: or_groups){
                if(group=="Z"){
                    group="A";
                }else{
                    group[0]++;
                }
                for(std::string b: a){
                    std::string course = Functions::extract_code(b);
                    if(course!=""){
                        if(b.find("concurrent enrollment")!=std::string::npos){
                            prereqTable<<currentCourse<<","<<course<<","<<gradeReq<<","<<group<<","<<","<<"True"<<std::endl;
                        }else{
                            prereqTable<<currentCourse<<","<<course<<","<<gradeReq<<","<<group<<","<<","<<"False"<<std::endl;
                        }
                        
                    }else{
                        if(b.find("concurrent enrollment")!=std::string::npos){
                            prereqTable<<currentCourse<<","<<","<<","<<group<<","<<"concurrent enrollment"<<","<<"N/A"<<std::endl;
                        }else{
                            prereqTable<<currentCourse<<","<<","<<","<<group<<","<<b<<","<<"N/A"<<std::endl;
                        }           
                    }
                }
            }
        }
    }
}

void Functions::crossListingProcess(std::ofstream& crossListingTable, std::string row, std::string currentCourse){
    if(row.find("Cross Listing")!=std::string::npos){
        row=row.substr(row.find("Cross Listing")+13);
        std::cout<<row<<std::endl;

        std::vector<std::string> equivalents = Functions::extractCodes(row);
        for(std::string a: equivalents){
            if(currentCourse!=a){
                crossListingTable<<currentCourse<<","<<a<<std::endl;
            }
        }
    }
}


void Functions::removeNewlines(std::string& input) {
    for (char& c : input) {
        if (c == '\n' || c == '\r') {
            c = ' ';
        }
    }
}
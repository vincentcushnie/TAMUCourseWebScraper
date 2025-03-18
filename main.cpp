
#include <iostream>
#include <cstdlib>
#include "pugixml.hpp"
#include <fstream>
#include <curl/curl.h>
#include <vector>
#include <regex>
#include <string.h>
#include "functions.h"
#include <sstream> 



void ScrapeDegree(std::string degreeUrl, int id){
    Functions::curlRequest(degreeUrl);
    pugi::xml_document doc;
    if (!doc.load_file("cleaned.html")) {
        std::cerr << "Failed to load cleaned.html" << std::endl;
    }
    Functions::degreeInformationProcessAndScrape(doc, id, degreeUrl);
    
}


void ScrapeCourses(std::string coursesUrl){
    Functions::curlRequest(coursesUrl);
    pugi::xml_document doc;
    if (!doc.load_file("cleaned.html")) {
        std::cerr << "Failed to load cleaned.html" << std::endl;
    }
    std::vector<std::string> data=Functions::courseInformationScrape(doc);
    Functions::courseInformationProcess(coursesUrl, data);
}


int main() {

    std::string majorTable = "majorTable.csv";
    std::ofstream majorFile(majorTable);
    majorFile<<"majorName,majorCode,department,credits,difficulty,rules"<<std::endl;
    majorFile.close();
    std::string majorToCoursesTable = "majorToCoursesTable.csv";
    std::ofstream majorToCoursesFile(majorToCoursesTable);
    majorToCoursesFile<<"majorCode,courseCode,group,timeCode,rules,hours"<<std::endl;
    majorToCoursesFile.close();
    std::string coursesTable = "coursesTable.csv";
    std::ofstream coursesFile(coursesTable);
    coursesFile<<"courseCode,courseName,field,credits,lecture,lab,description,difficulty"<<std::endl;
    coursesFile.close();
    std::string coursesCrossListingTable = "coursesCrossListingTable.csv";
    std::ofstream coursesCrossListingFile(coursesCrossListingTable);
    coursesCrossListingFile<<"Course Code, Course Code"<<std::endl;
    coursesCrossListingFile.close();
    std::string coursesPrerequisitesTable = "coursesPrerequisitesTable.csv";
    std::ofstream coursesPrerequisitesFile(coursesPrerequisitesTable);
    coursesPrerequisitesFile<<"Course Code, Course Code, Grade Req, Group, Text, Concurrent Enrollment"<<std::endl;
    coursesPrerequisitesFile.close();



    std::vector <std::string> majors;
    std::vector <std::string> courses;
    majors.push_back("https://catalog.tamu.edu/undergraduate/arts-and-sciences/mathematics/applied-mathematics-bs-computational-science-emphasis/#programrequirementstext");
    // majors.push_back("https://catalog.tamu.edu/undergraduate/engineering/computer-science/computer-engineering-bs/#programrequirementstext");
    // majors.push_back("https://catalog.tamu.edu/undergraduate/performance-visualization-fine-arts/visualization-bs/#programrequirementstext");
    // majors.push_back("https://catalog.tamu.edu/undergraduate/public-health/bs-internship-track/#programrequirementstext");
    // majors.push_back("https://catalog.tamu.edu/undergraduate/agriculture-life-sciences/agricultural-economics/agribusiness-bs/#programrequirementstext");
    // majors.push_back("https://catalog.tamu.edu/undergraduate/engineering/petroleum/bs/#programrequirementstext");
    
    courses.push_back("https://catalog.tamu.edu/undergraduate/course-descriptions/math/");//math
    courses.push_back("https://catalog.tamu.edu/undergraduate/course-descriptions/engl/");//english
    courses.push_back("https://catalog.tamu.edu/undergraduate/course-descriptions/econ/");//economics
    courses.push_back("https://catalog.tamu.edu/undergraduate/course-descriptions/stat/");//statistics
    courses.push_back("https://catalog.tamu.edu/undergraduate/engineering/computer-science/#coursestext"); //computer science
    courses.push_back("https://catalog.tamu.edu/undergraduate/course-descriptions/comm/"); //communications
    courses.push_back("https://catalog.tamu.edu/undergraduate/course-descriptions/ocng/"); //oceanography
    courses.push_back("https://catalog.tamu.edu/undergraduate/course-descriptions/phys/"); //physics
    courses.push_back("https://catalog.tamu.edu/undergraduate/course-descriptions/isen/"); //physics
    // courses.push_back("https://catalog.tamu.edu/undergraduate/agriculture-life-sciences/poultry-science/#coursestext");
    // courses.push_back("https://catalog.tamu.edu/undergraduate/business/finance/#coursestext");

    for(int i=0; i<majors.size(); ++i){
        ScrapeDegree(majors[i], i+1);
    }
    for(std::string a: courses){
        ScrapeCourses(a);
    }
    return 0;
}


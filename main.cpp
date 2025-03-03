
#include <iostream>
#include <cstdlib>
#include "pugixml.hpp"
#include <fstream>
#include <curl/curl.h>
#include <vector>
#include <string.h>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::ofstream* file = static_cast<std::ofstream*>(userp);
    file->write(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

std::string trimUrl(const std::string& url) {
    size_t lastSlash = url.find_last_of('/');
    size_t secondLastSlash = url.find_last_of('/', lastSlash - 1);

    if (secondLastSlash == std::string::npos || lastSlash == std::string::npos) {
        return "invalid"; // Handle unexpected format
    }

    std::string trimmed = url.substr(secondLastSlash + 1, lastSlash - secondLastSlash - 1);
    return trimmed;
}

void ScrapeDegree(std::string degreeUrl){
    std::cout<<"degreeURL: "<<degreeUrl<<std::endl;
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL" << std::endl;
    }

    std::ofstream file("raw.html", std::ios::out | std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open raw.html" << std::endl;
    }

    curl_easy_setopt(curl, CURLOPT_URL, degreeUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
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
    pugi::xml_document doc;
    if (!doc.load_file("cleaned.html")) {
        std::cerr << "Failed to load cleaned.html" << std::endl;
    }


    std::vector<std::string> data;
    for(pugi::xml_node node_1 : doc.child("html").child("body").child("section").child("div").children("div")){
        if (std::string(node_1.attribute("id").value())=="col-content"){
            for(pugi::xml_node node_2 : node_1.child("main").children("div")){
                if(std::string(node_2.attribute("id").value())=="programrequirementstextcontainer"){
                    for(pugi::xml_node node_3: node_2.child("table").children("tr")){
                        std::string nodeClass = std::string(node_3.attribute("class").value());
                        if(nodeClass=="plangridyear"){
                            data.push_back("year");
                        }else if(nodeClass=="plangridterm"){
                            data.push_back("season");
                        }else if(nodeClass=="odd" || nodeClass=="even"){
                            std::string codecol="empty";
                            std::string titlecol="empty";
                            std::string hourscol="empty";
                            for(pugi::xml_node node_4: node_3.children("td")){
                                if(std::string(node_4.attribute("class").value())=="codecol"){
                                    if(node_4.child("a")){
                                        codecol=node_4.child("a").text().get();
                                    }
                                }
                                if(std::string(node_4.attribute("class").value())=="titlecol"){
                                    titlecol=node_4.text().get();
                                }
                                if(std::string(node_4.attribute("class").value())=="hourscol"){
                                   if(node_4.text().get()!=""){
                                    hourscol=node_4.text().get();
                                    hourscol.erase(0,1);
                                    if(hourscol==""){
                                        hourscol=="empty";
                                    }
                                   }
                                }
                                if(codecol=="empty"||titlecol=="empty"||hourscol=="empty"){
                                    data.push_back("not standard");
                                }else{
                                    data.push_back("titlecol: "+titlecol+" codecol: "+codecol+" hourscol: "+hourscol);
                                }
                    
                            }
                        }else if(nodeClass=="even"){
                            data.push_back("even");
                        }else if(nodeClass=="plangridtotal lastrow odd"||nodeClass=="plangridtotal lastrow even"){
                            data.push_back("total hours");
                        }else if(nodeClass=="plangridsum even"||nodeClass=="plangridsum odd"){
                            data.push_back("semester hours");
                        }
                        else{
                            data.push_back("no case");
                        }
                    }
                }
            }
        }
    }

    auto title = doc.child("html").child("head").child("title");
    std::string filename = trimUrl(degreeUrl) + "-degree.txt";
    std::ofstream courseFile(filename);
    for(std::string row: data){
        if(row.find("&nbsp;",0)!=std::string::npos){
            row.replace(row.find("&nbsp;",0), 6," ");
        }
        courseFile<<row<<std::endl;
    }
    courseFile.close();
}



void prerequisiteProcess(std::ofstream& prereqTable, std::string row){
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
        prereqTable<<row.substr(start, end-start)<<std::endl;
        start = end + 1;
        end = row.find(';', start);
        
    }
    splitStrings.push_back(row.substr(start));
    prereqTable<<row.substr(start)<<std::endl;


}
void crossListingProcess(std::ofstream& crossListingTable, std::string row){

}





void ScrapeCourses(std::string coursesUrl){
    std::cout<<"coursesUrl: "<<coursesUrl<<std::endl;
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize CURL" << std::endl;
    }

    std::ofstream file("raw.html", std::ios::out | std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open raw.html" << std::endl;
    }

    curl_easy_setopt(curl, CURLOPT_URL, coursesUrl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
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
    pugi::xml_document doc;
    if (!doc.load_file("cleaned.html")) {
        std::cerr << "Failed to load cleaned.html" << std::endl;
    }


    std::vector<std::string> data;
    for (pugi::xpath_node xpath_node : doc.select_nodes("//div[@class='courseblock']")) {
        pugi::xml_node div = xpath_node.node();  // Access the xml_node from xpath_node
        data.push_back(div.child("h2").text().get());
        data.push_back(div.child("p").child("span").child("strong").text().get());
        data.push_back(div.child("p").text().get());
        //data.push_back(div.child("p").child("strong").text().get());
        std::string fulltext=div.child("p").child("strong").text().get();
        pugi::xml_node temp = div.child("p").child("strong").next_sibling();
        while(temp){
            fulltext+=temp.text().get();
            temp=temp.next_sibling();
        }
        //std::cout<<fulltext<<std::endl;
        data.push_back(fulltext);

    
    }

    auto title = doc.child("html").child("head").child("title");
    std::string trimmedUrl = trimUrl(coursesUrl);
    std::string filename = trimmedUrl + "-courses.txt";
    std::ofstream courseTable(trimmedUrl + "CourseTable.csv");
    std::ofstream prereqTable(trimmedUrl + "PrereqTable.csv");
    std::ofstream crossListingTable(trimmedUrl + "CrossListingTable.csv");
    courseTable<<"Course Code, Course Name, Field, Credits, Lecture, Lab, Description, Difficulty"<<std::endl;
    prereqTable<<"Course Code, Course Code"<<std::endl;
    crossListingTable<<"Course Code, Course Code"<<std::endl;
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
                prerequisiteProcess(prereqTable, row);
           }
           if(row.find("Cross Listing")!=std::string::npos){
                crossListingProcess(crossListingTable, row);
           }
        }
        else if(isdigit(row[4]) && isdigit(row[5]) && isdigit(row[6])){
            courseTable<<row.substr(0,7)<<","<<row.substr(8)<<","<<row.substr(0,4)<<",";
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


int main() {
    //ScrapeDegree("https://catalog.tamu.edu/undergraduate/arts-and-sciences/mathematics/applied-mathematics-bs-computational-science-emphasis/#programrequirementstext");
    ScrapeCourses("https://catalog.tamu.edu/undergraduate/course-descriptions/math/");
    ScrapeCourses("https://catalog.tamu.edu/undergraduate/engineering/computer-science/#coursestext");
    return 0;
}

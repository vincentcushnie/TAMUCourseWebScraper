
#include <iostream>
#include <cstdlib>
#include "pugixml.hpp"
#include <fstream>
#include <curl/curl.h>
#include <vector>
#include <regex>
#include <string.h>
#include "functions.h"


void ScrapeDegree(std::string degreeUrl){
    Functions::curlRequest(degreeUrl);
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
    std::string filename = Functions::trimUrl(degreeUrl) + "-degree.txt";
    std::ofstream courseFile(filename);
    for(std::string row: data){
        if(row.find("&nbsp;",0)!=std::string::npos){
            row.replace(row.find("&nbsp;",0), 6," ");
        }
        courseFile<<row<<std::endl;
    }
    courseFile.close();
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
    //ScrapeDegree("https://catalog.tamu.edu/undergraduate/arts-and-sciences/mathematics/applied-mathematics-bs-computational-science-emphasis/#programrequirementstext");
    //ScrapeCourses("https://catalog.tamu.edu/undergraduate/course-descriptions/math/");
    //ScrapeCourses("https://catalog.tamu.edu/undergraduate/engineering/computer-science/#coursestext");
    ScrapeCourses("https://catalog.tamu.edu/undergraduate/agriculture-life-sciences/poultry-science/#coursestext");
    ScrapeCourses("https://catalog.tamu.edu/undergraduate/business/finance/#coursestext");
    return 0;
}

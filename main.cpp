
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
                    for(pugi::xml_node node_3: node_2.children("table")){
                        int group=0;
                        int hours;
                        for(pugi::xml_node node_4: node_3.children("tr")){
                            std::string nodeClass = std::string(node_4.attribute("class").value());
                            std::cout<<nodeClass<<std::endl;
                            if(nodeClass=="plangridyear"){
                                data.push_back("year");
                            }else if(nodeClass=="plangridterm"){
                                data.push_back("season");
                            }else if(nodeClass=="odd" || nodeClass=="even"){
                                std::string codecol="";
                                std::string codecol2="";
                                std::string titlecol="";
                                std::string titlecol2="";
                                std::string hourscol="";
                                std::string a="";
                                std::string b="";
                                std::string c="";
                                std::string d="";
                                std::string e="";
                                std::string aa="";
                                std::string bb="";
                                std::string aaa="";
                                for(pugi::xml_node node_5: node_4.children("td")){
                                        
                                    if(std::string(node_5.attribute("class").value())=="codecol"){
                                            
                                            c=node_5.child("div").child("span").child("a").text().get();
                                            b=node_5.child("div").child("a").text().get();
                                            a=node_5.child("a").text().get();
                                            d=node_5.child("span").child("a").text().get();
                                            e=node_5.child("span").text().get();
                                            //for courses div-span-a or div-a or a
                                            //for non courses span-a span
                                    }
                                    if(std::string(node_5.attribute("class").value())=="titlecol"){
                                        aa=node_5.text().get();
                                        bb=node_5.child("div").text().get();
                                    }
                                    if(std::string(node_5.attribute("class").value())=="hourscol"){
                                        aaa=node_5.text().get();
                                    }
                        
                                }
                                if(aaa==""){ 
                                }else{
                                    hours=std::stoi(aaa);
                                    group++;  
                                }
                                if(aa=="" && bb=="" && c==""){
                                    if(d=="" && e==""){
                                        std::cout<<"something wrong"<<std::endl;
                                    }
                                    if(d!=""){
                                        codecol=d;
                                    }
                                    if(e!=""){
                                        codecol=e;
                                    }
                                }else{
                                    if(c!=""){
                                        codecol=c;
                                        titlecol=aa;
                                    }else{
                                        codecol=a;
                                        codecol2=b;
                                        titlecol=aa;
                                        titlecol2=bb;
                                    }
                                }
                                hourscol=hours;
                                Functions::removeNewlines(titlecol);
                                Functions::removeNewlines(titlecol2);
                                Functions::removeNewlines(codecol);
                                Functions::removeNewlines(codecol2);
                                
                                data.push_back("major,"+codecol+","+titlecol+","+std::to_string(group)+","+"timecode");
                                if(codecol2!=""){
                                    data.push_back("major,"+codecol2+","+titlecol2+","+std::to_string(group)+","+"timecode");
                                }
                                
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
    }

    auto title = doc.child("html").child("head").child("title");
    std::string filename = Functions::trimUrl(degreeUrl) + "-degree.txt";
    std::ofstream courseFile(filename);
    courseFile<<"Major Code, Course Code, Group, Time Code"<<std::endl;
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
    ScrapeDegree("https://catalog.tamu.edu/undergraduate/arts-and-sciences/mathematics/applied-mathematics-bs-computational-science-emphasis/#programrequirementstext");
    //ScrapeDegree("https://catalog.tamu.edu/undergraduate/engineering/petroleum/bs/#programrequirementstext");
    //ScrapeCourses("https://catalog.tamu.edu/undergraduate/course-descriptions/math/");
    //ScrapeCourses("https://catalog.tamu.edu/undergraduate/engineering/computer-science/#coursestext");
    // ScrapeCourses("https://catalog.tamu.edu/undergraduate/agriculture-life-sciences/poultry-science/#coursestext");
    // ScrapeCourses("https://catalog.tamu.edu/undergraduate/business/finance/#coursestext");
    return 0;
}



// Functions::curlRequest(degreeUrl);
// pugi::xml_document doc;
// if (!doc.load_file("cleaned.html")) {
//     std::cerr << "Failed to load cleaned.html" << std::endl;
// }
// std::vector<std::string> data;
// for(pugi::xml_node node_1 : doc.child("html").child("body").child("section").child("div").children("div")){
//     if (std::string(node_1.attribute("id").value())=="col-content"){
//         for(pugi::xml_node node_2 : node_1.child("main").children("div")){
//             if(std::string(node_2.attribute("id").value())=="programrequirementstextcontainer"){
//                 for(pugi::xml_node node_3: node_2.child("table").children("tr")){
//                     std::string nodeClass = std::string(node_3.attribute("class").value());
//                     if(nodeClass=="plangridyear"){
//                         data.push_back("year");
//                     }else if(nodeClass=="plangridterm"){
//                         data.push_back("season");
//                     }else if(nodeClass=="odd" || nodeClass=="even"){
//                         std::string codecol="empty";
//                         std::string titlecol="empty";
//                         std::string hourscol="empty";
//                         for(pugi::xml_node node_4: node_3.children("td")){
//                             std::cout<<node_4.text().get()<<std::endl;
//                             if(std::string(node_4.attribute("class").value())=="codecol"){
//                                 if(node_4.child("a")){
//                                     codecol=node_4.child("a").text().get();
//                                 }
//                             }
//                             if(std::string(node_4.attribute("class").value())=="titlecol"){
//                                 titlecol=node_4.text().get();
//                             }
//                             if(std::string(node_4.attribute("class").value())=="hourscol"){
//                                if(node_4.text().get()!=""){
//                                 hourscol=node_4.text().get();
//                                 hourscol.erase(0,1);
//                                 if(hourscol==""){
//                                     hourscol=="empty";
//                                 }
//                                }
//                             }
//                             if(codecol=="empty"||titlecol=="empty"||hourscol=="empty"){
//                                 data.push_back("not standard" + codecol+titlecol+hourscol);
//                             }else{
//                                 data.push_back("titlecol: "+titlecol+" codecol: "+codecol+" hourscol: "+hourscol);
//                             }
                
//                         }
//                     }else if(nodeClass=="even"){
//                         data.push_back("even");
//                     }else if(nodeClass=="plangridtotal lastrow odd"||nodeClass=="plangridtotal lastrow even"){
//                         data.push_back("total hours");
//                     }else if(nodeClass=="plangridsum even"||nodeClass=="plangridsum odd"){
//                         data.push_back("semester hours");
//                     }
//                     else{
//                         data.push_back("no case");
//                     }
//                 }
//             }
//         }
//     }
// }

// auto title = doc.child("html").child("head").child("title");
// std::string filename = Functions::trimUrl(degreeUrl) + "-degree.txt";
// std::ofstream courseFile(filename);
// for(std::string row: data){
//     if(row.find("&nbsp;",0)!=std::string::npos){
//         row.replace(row.find("&nbsp;",0), 6," ");
//     }
//     courseFile<<row<<std::endl;
// }
// courseFile.close();
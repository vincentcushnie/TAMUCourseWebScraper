#include "functions.h"
#include <iostream>
#include <regex>
#include <curl/curl.h>




size_t Functions::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::ofstream* file = static_cast<std::ofstream*>(userp);
    file->write(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

std::string Functions::getMajorFromUrl(const std::string& url) {
    size_t lastSlash = url.find_last_of('/');
    size_t secondLastSlash = url.find_last_of('/', lastSlash - 1);

    if (secondLastSlash == std::string::npos || lastSlash == std::string::npos) {
        return "invalid"; // Handle unexpected format
    }

    std::string trimmed = url.substr(secondLastSlash + 1, lastSlash - secondLastSlash - 1);
    return trimmed;
}

std::string Functions::getDepartmentFromUrl(const std::string& url) {
    size_t lastSlash = url.find_last_of('/');
     lastSlash = url.find_last_of('/', lastSlash - 1);
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
    //std::cout << "HTML saved to raw.html" << std::endl;
    std::string command = "tidy -q -utf8 -asxml -o cleaned.html raw.html 2>/dev/null";
    system(command.c_str());
    //std::cout << "Cleaned HTML saved to cleaned.html" << std::endl;
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
    std::string trimmedUrl = Functions::getMajorFromUrl(coursesUrl);
    std::ofstream courseTable("coursesTable.csv", std::ios::app);
    std::ofstream prereqTable("coursesPrerequisitesTable.csv", std::ios::app);
    std::ofstream crossListingTable("coursesCrossListingTable.csv", std::ios::app);
    
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
            courseTable<<"\""<<row.substr(0,7)<<"\",\""<<row.substr(8)<<"\",\""<<row.substr(0,4)<<"\",";
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
            courseTable<<"\""<<row.substr(1)<<"\","<<0<<std::endl;
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

void Functions::extract_text(const pugi::xml_node& node, std::string& result) {
    for (auto child : node.children()) {
        if (child.type() == pugi::node_pcdata) {
            result += child.value();
        } else if (child.type() == pugi::node_element) {
            extract_text(child, result); // Recursively extract text from elements
        }
    }
}

void Functions::replaceNbsp(std::string& text, std::string repl) {
    const std::string target = "&nbsp;";
    const std::string replacement = repl;
    size_t pos = 0;

    while ((pos = text.find(target, pos)) != std::string::npos) {
        text.replace(pos, target.length(), replacement);
    }
}




void Functions::degreeInformationProcessAndScrape(pugi::xml_document& doc, int id, std::string degreeUrl){
    std::vector<std::string> data;
    std::vector<std::string> courseSuperscripts;
    int majorID=id;
    auto title = doc.child("html").child("head").child("title");
    std::string majorName=std::string(title.text().get());
    majorName=majorName.substr(0, majorName.find('<')-1);
    std::string majorCode="N/A";
    std::string department=Functions::getDepartmentFromUrl(degreeUrl);
    int credits=0;
    int difficulty=0;
    std::string majorRules="";
    for(pugi::xml_node node_1 : doc.child("html").child("body").child("section").child("div").children("div")){
        if (std::string(node_1.attribute("id").value())=="col-content"){
            for(pugi::xml_node node_2 : node_1.child("main").children("div")){
                if(std::string(node_2.attribute("id").value())=="programrequirementstextcontainer"){
                    Functions::processMajorRules(node_2, majorRules);
                    Functions::processCourseSuperscripts(node_2, courseSuperscripts);
                    Functions::processCourseTables(node_2, courseSuperscripts, data, id, credits);
                }
            }
        }
    }
    std::ofstream majorFile("majorTable.csv", std::ios::app);
    majorFile<< "\""<<majorName <<"\",\""<< majorCode<<"\",\""<<department<<"\","<<std::to_string(credits)<<","<<std::to_string(difficulty)<<",\""<<majorRules<<"\""<<std::endl;
    majorFile.close();
    std::ofstream courseFile("majorToCoursesTable.csv", std::ios::app);
    for(std::string row: data){
        if(row.find("&nbsp;",0)!=std::string::npos){
            row.replace(row.find("&nbsp;",0), 6,"");
        }
       // std::cout<<"row: "<<row<<std::endl;
        courseFile<<row<<std::endl;
    }
    courseFile.close();
}


void Functions::processMajorRules(pugi::xml_node& node_2, std::string& majorRules){
    for(pugi::xml_node node_3: node_2.children("p")){
        std::string majorRule="";
        Functions::extract_text(node_3, majorRule);
        Functions::removeNewlines(majorRule);
        Functions::replaceNbsp(majorRule, "");
        majorRules+=majorRule;
        majorRules+=",";
    }
}

void Functions::processCourseSuperscripts(pugi::xml_node& node_2, std::vector<std::string>& courseSuperscripts){
    for(pugi::xml_node node_3: node_2.children("dl")){
        for(pugi::xml_node node_4 : node_3.children("dd")){
            
            std::string rule="";
            Functions::extract_text(node_4.child("p"), rule);
            Functions::removeNewlines(rule);
            Functions::replaceNbsp(rule, "");
            courseSuperscripts.push_back(rule);
        }
    }
}

void Functions::processCourseTables(pugi::xml_node& node_2, std::vector<std::string>& courseSuperscripts, std::vector<std::string>& data, const int& majorID, int& credits){
    int year=0;
    int season=1;
    for(pugi::xml_node node_3: node_2.children("table")){
        if(std::string(node_3.attribute("class").value())=="sc_plangrid"){
            int group=0;
            int hours;
            for(pugi::xml_node node_4: node_3.children("tr")){
                std::string nodeClass = std::string(node_4.attribute("class").value());
                if(nodeClass=="plangridyear"){
                    year++;
                }else if(nodeClass=="plangridterm"){
                    if(season==0){
                        season=1;
                    }else{
                        season=0;
                    }
                }else if(nodeClass=="odd" || nodeClass=="even"){
                    Functions::processCourseRow(node_4, year, season, group, hours, majorID, data, courseSuperscripts);
                    
                }else if(nodeClass=="plangridtotal lastrow odd"||nodeClass=="plangridtotal lastrow even"){
                    for(pugi::xml_node node_5: node_4.children("td")){
                        if(std::string(node_5.attribute("class").value())=="hourscol"){
                            if(credits==0){
                                credits=std::stoi(node_5.text().get());
                            }else{
                                credits+=std::stoi(node_5.text().get());
                            }
                        }
                    }
                }else if(nodeClass=="plangridsum even"||nodeClass=="plangridsum odd"){
                }
                else{
                    data.push_back("no case");
                }
            }
        }
    }
}

void Functions::processCourseRow(pugi::xml_node& node_4, int& year, int& season, int& group, int& hours, const int& majorID, std::vector<std::string>& data, std::vector<std::string>& courseSuperscripts){
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
    std::string superscripts="";
    std::string superscriptsSecond="";
    for(pugi::xml_node node_5: node_4.children("td")){
            
        if(std::string(node_5.attribute("class").value())=="codecol"){                
                c=node_5.child("div").child("span").child("a").text().get();
                b=node_5.child("div").child("a").text().get();
                a=node_5.child("a").text().get();
                if(a==""){
                    a=node_5.child("div").child("div").child("a").text().get();
                }
                d=node_5.child("span").child("a").text().get();
                e=node_5.child("span").text().get();
                superscripts=node_5.child("sup").text().get();
        }
        if(std::string(node_5.attribute("class").value())=="titlecol"){
            aa=node_5.text().get();
            bb=node_5.child("div").text().get();
            if(superscripts==""){
                superscripts=node_5.child("sup").text().get();
            }
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
    if(codecol=="" && codecol2!=""){
        codecol=codecol2;
        codecol2="";
    }
    hourscol=hours;
    Functions::removeNewlines(titlecol);
    Functions::removeNewlines(titlecol2);
    Functions::removeNewlines(codecol);
    Functions::removeNewlines(codecol2);
    std::stringstream ss(superscripts);
    std::string token;
    std::string rules="";
    while (std::getline(ss, token, ',')) {
        rules+=courseSuperscripts[stoi(token)-1];
    }
    std::stringstream s2(superscriptsSecond);
    std::string token2;
    std::string rules2="";
    while (std::getline(s2, token2, ',')) {
        std::cout<<"rules: "<<std::endl;
        rules2+=courseSuperscripts[stoi(token2)-1];
    }
    if(codecol.find("from the following")==std::string::npos && codecol.find("of the following") && codecol2.find("from the following")==std::string::npos && codecol2.find("of the following")){
        data.push_back(std::to_string(majorID)+",\""+codecol+"\",\""+titlecol+"\","+std::to_string(group)+","+std::to_string(year*10+season)+",\""+rules+"\","+std::to_string(hours));
    if(codecol2!=""){
        data.push_back(std::to_string(majorID)+",\""+codecol2+"\",\""+titlecol2+"\","+std::to_string(group)+","+std::to_string(year*10+season)+",\""+rules2+"\","+std::to_string(hours));
    }
    }
    
}
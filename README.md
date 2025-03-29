This is a very simple c++ web scraper that is gong to be used to scrape the data for course craft project.
It uses cURL to get the webpage html, tidy to conver the raw html to xml compatible, and then pugixml to take the information and put it into a .csv file.
NOTE: THIS WEBSCRAPER IS STILL VERY MUCH A WORK IN PROGRESS.

Setup:
Ensure that curl, pugixml, and tidy are installed
pugixml- https://pugixml.org/docs/quickstart.html
tidy - (wsl ubuntu) sudo apt update
sudo apt install tidy
curl - (wsl ubutnu) sudo apt update
sudo apt install curl

Compile with: $ g++ main.cpp Functions.cpp -lpugixml -lcurl -ltidy
Run with: ./a.out

After execution, you should have the following files:
raw.html: holds the response from the curl call
cleaned.html: holds the tidy xml formatted version of the raw.html
courses.csv: holds the parsed information

TODO:

Degrees:
Consider superscript notes
Extract degree general data

Courses:
Correct credits to work with classes that say something like "1 to 3"
Known incorrect courses:
-Math 170 concurrency clause

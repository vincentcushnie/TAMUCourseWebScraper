This is a very simple c++ web scraper that is gong to be used to scrape the data for course craft project.
It uses cURL to get the webpage html, tidy to conver the raw html to xml compatible, and then pugixml to take the information and put it into a .txt file.

NOTE: THIS WEBSCRAPER IS STILL VERY MUCH A WORK IN PROGRESS.

Setup:
Ensure that curl, pugixml, and tidy are installed

pugixml- (wsl ubuntu)
```sh
sudo apt update
sudo apt-get install libpugixml-dev
```

tidy - (wsl ubuntu) 
```sh
sudo apt update
sudo apt-get install libtidy-devsudo apt install tidy
```
curl - (wsl ubutnu)
```sh
sudo apt update
sudo apt sudo apt-get install libcurl4-openssl-devinstall curl
```

Compile with:
```sh
g++ main.cpp -lpugixml -lcurl -ltidy
```
Run with:
```sh
./a.out
```

After execution, you should have the following files:

    raw.html: holds the response from the curl call
    cleaned.html: holds the tidy xml formatted version of the raw.html
    courses.txt: holds the parsed information


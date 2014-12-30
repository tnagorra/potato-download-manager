#Potato Download Manager

##Description
A multipart downloader that downloads potatoes from the internet. It supports download acceleration and resume capabilites.

##Build Instructions
###Dependencies
Make sure to install the dependencies first.

    sudo apt-get install libssl-dev libboost-program-options-dev libboost-thread-dev

###Compile
Once you have installed all the dependencies you can compile the application. You will need git to clone the repository and clang-3.5 to compile the source code.
```bash
#Clone the github repository
git clone --recursive https://github.com/tnagorra/potato-download-manager

#Build it
cd potato-download-manager
make

#Run the application
./bin/aggregator
```

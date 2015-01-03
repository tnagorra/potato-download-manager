#Potato Download Manager

##Description
A multipart downloader that downloads potatoes from the internet. It supports download acceleration and resume capabilites.

##Build Instructions
###Dependencies
Make sure to install the dependencies first.

    sudo apt-get install libssl-dev libboost-program-options-dev libboost-thread-dev libboost-filesystem-dev

###Compile
Once you have installed all the dependencies you can compile the application.
```bash
#Clone the github repository
git clone --recursive https://github.com/tnagorra/potato-download-manager

#Build it
cd potato-download-manager
make

#Run the application
./bin/aggregator
```

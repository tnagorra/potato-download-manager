#Potato Download Manager

##Description
A multipart downloader that downloads potatoes from the internet.

##Build Instructions
###Dependencies
Make sure to install the dependencies first.

    sudo apt-get install libssl-dev libboost-program-options-dev libboost-thread-dev

###Compile
Once you have installed all the dependencies you need to compile the application before you can run it.

```bash
#Clone the github repository
git clone --recursive https://github.com/tnagorra/potato-download-manager

#Build it
cd potato-download-manager
make

#Run the application
./bin/aggregator
```

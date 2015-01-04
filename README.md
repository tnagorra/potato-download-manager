#Potato Download Manager

##Description
A multipart downloader that downloads potatoes from the internet. It supports download acceleration and resume capabilites.

##Building from git
Check out the latest sources with:
    git clone --recursive https://github.com/tnagorra/potato-download-manager
Make sure to install the dependencies first.
```bash
#Install dependencies
sudo apt-get install libssl-dev libboost-program-options-dev libboost-thread-dev libboost-filesystem-dev -y

#Build it
cd potato-download-manager
make

#Run the application
./bin/aggregator
```
#License
It is distributed under GNU GENERAL PUBLIC LICENSE. A copy of the license is available in the distributed LICENSE file.

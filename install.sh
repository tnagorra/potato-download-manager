#! /bin/sh

# Install dependencies
sudo apt-get install libssl-dev libboost-program-options-dev \
libboost-thread-dev libboost-filesystem-dev -y

# Make
make aggregator

# Configure
mkdir $HOME/potato
cp bin/aggregator $HOME/potato/podoman
cp global.ini $HOME/potato/
cp README.md $HOME/potato/
cp LICENSE.txt $HOME/potato/
mkdir $HOME/potato/incomplet
mkdir $HOME/potato/potatoes

# Clean
# make clean

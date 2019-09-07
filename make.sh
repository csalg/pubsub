mkdir build
cd build

git clone https://github.com/nlohmann/json
cd json 
mkdir build
cd build
cmake ..
make
sudo make install
cd ../..
sudo rm -r json

cmake ..
make
sudo make install
mv pubsub ..
cd ..


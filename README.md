Build:
mkdir -p bin
cp -n dict/BoggleWords.dict bin/
g++ -o BoggleMain -Iinclude/ src/*

Run:
cd bin
./BoggleMain

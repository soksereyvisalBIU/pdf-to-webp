install header ===
install "mSYS2 MINGW64"

command ===
cd root
g++ solution.cpp -o compress_pdf -I/mingw64/include -L/mingw64/lib -lpoppler-cpp `pkg-config --cflags --libs opencv4` -ltesseract
./compress_pdf


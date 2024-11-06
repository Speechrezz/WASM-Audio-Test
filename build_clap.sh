cd src
xxd -i index.html > index.html.h
cd ..
mkdir -p build/clap && cd build/clap
cmake cmake ../../
cd ../../
CMAKE_EXPORT_COMPILE_COMMANDS=1 cmake --build build/clap
cp build/clap/compile_commands.json .
rm -rf /Users/joe/Library/Audio/Plug-Ins/CLAP/Synthle.clap
cp -r build/clap/Synthle.clap /Users/joe/Library/Audio/Plug-Ins/CLAP/Synthle.clap

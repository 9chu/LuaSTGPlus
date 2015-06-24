cd 3rdParty/luajit
mkdir _build
cd _build
cmake .. -G "Visual Studio 12" -DLUAJIT_ENABLE_LUA52COMPAT=OFF
msbuild luajit.sln /t:Rebuild /p:Configuration=Release

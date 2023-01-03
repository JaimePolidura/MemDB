#pragma once

#ifdef _WIN32
    #include "FileUtilsWindows.cpp"
#elif __linux__
    #include "FileUtilsLinux.cpp"
#endif
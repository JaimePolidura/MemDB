#pragma once

//Data structures
#include <vector>
#include <queue>
#include <map>
#include <unordered_set>
#include <set>

//Concurrency
#include <thread>
#include <mutex>
#include <future>
#include <atomic>
#include <condition_variable>

//Utils
#include <numeric>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <fstream>
#include <string>
#include <iostream>
#include <chrono>
#include <cstdint>
#include <stdint.h>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <exception>
#include <cstdio>
#include <tgmath.h>
#include <optional>
#include <filesystem>
#include <math.h>
#include <regex>
#include <cstring>

//Deps
#include <boost/beast.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <nlohmann/json.hpp>
#include <boost/asio.hpp>
#include <etcd/Client.hpp>
#include <etcd/Watcher.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <openssl/md5.h>

//OS
//#ifdef _WIN32
//    #include <Windows.h>
//#else
//    #include <sys/stat.h>
//    #include <sys/types.h>
//    #include <unistd.h>
//    #include <fcntl.h>
//#endif

#include "utils.h"

#include <algorithm>
#include <sstream>
#include <string>

std::vector<std::string> string_split(const std::string &str, char delimiter) {
    std::vector<std::string> res;
    std::stringstream ss(str);
    std::string word;
    while (!ss.eof()) {
        std::getline(ss, word, delimiter);
        res.emplace_back(word);
    }

    return res;
}

void unique_spaces(std::string &str) {
    auto res_it = std::unique(str.begin(), str.end(), [](char lhs, char rhs) { return lhs == rhs && lhs == ' '; });

    str.erase(res_it, str.end());
}

void get_device_info_from_buf(std::vector<DeviceInfo>& ARGS_OUT devices_list, const std::string& ARGS_IN buf) {
    std::vector<std::string> devices_list_tmp = string_split(buf, '\n');
    devices_list_tmp.pop_back();  // pop null

#ifdef ADB_DEBUG
    ADB_LOGI("devices_list_tmp.size: %d\n", devices_list_tmp.size());
#endif

    for (auto& device : devices_list_tmp) {
        ADB_LOGI("devices_list_tmp it: %s\n", device.c_str());
        if (device.find('\t') != std::string::npos) {
            device.replace(device.find('\t'), 1, 1, ' ');
        }
        std::vector<std::string> devices_info = string_split(device, ' ');
#ifdef ADB_DEBUG
        ADB_LOGI("devices_info.size: %d\n", devices_info.size());
        for (auto& info : devices_info) {
            ADB_LOGI("devices_info it: %s\n", info.c_str());
        }
#endif
        if (devices_info.size() > 3) {
            DeviceInfo info;
            info.serial = devices_info[0];
            info.state = devices_info[1];
            info.product = string_split(devices_info[2], ':')[1];
            info.model = string_split(devices_info[3], ':')[1];
            info.device = string_split(devices_info[4], ':')[1];
            info.transport_id = std::atoi(string_split(devices_info[5], ':')[1].c_str());

            devices_list.push_back(info);
        } else {
            DeviceInfo info;
            info.serial = devices_info[0];
            info.state = devices_info[1];
            devices_list.push_back(info);
        }
    }
}

void get_lines_from_buf(std::vector<std::string>& ARGS_OUT lines, const std::string& ARGS_IN buf) {
    std::vector<std::string> lines_tmp = string_split(buf, '\n');
    lines_tmp.pop_back();  // pop null

#ifdef ADB_DEBUG
    ADB_LOGI("lines_tmp.size: %d\n", lines_tmp.size());
#endif

    for (auto& line : lines_tmp) {
        lines.push_back(line);
    }
}
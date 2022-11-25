#pragma once

#include <strings.h>
#include <string>
#include <cstdlib>
#include <vector>
#include <toml.hpp>

using namespace std;

class CConfig {
private:
    CConfig() {
    }

    toml::value data;
    static CConfig *m_instance;

public:

    static CConfig *GetInstance() {
        if (m_instance == nullptr) {
            m_instance = new CConfig();
        }
        return m_instance;
    }

    bool Load(string filePath);

    const string GetString(string p_itemname) {

        return nullptr;
    }

    int GetIntDefault(string p_itemname, const int def) {
        auto v = toml::find<std::int32_t>(data, p_itemname);
        return v;
    }

    vector<int> GetIntArray(string key) {
        const auto numbers = toml::find(data, key);
        return toml::get<std::vector<int>>(numbers);
    }
};



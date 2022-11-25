//
// Created by arthur on 2022/10/26.
//

#include "../include/ngx_c_conf.h"
#include <toml.hpp>

CConfig *CConfig::m_instance = nullptr;

bool CConfig::Load(string filePath) {

    data = toml::parse(filePath);

    return true;
}

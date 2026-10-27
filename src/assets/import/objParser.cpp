// MIT License - Copyright (c) 2026 AnselZ
// SPDX-License-Identifier: MIT

// ===============================
// AUTHOR       : Angel Ortiz (angelo12 AT vt DOT edu)
// CREATE DATE  : 2018-07-14
// ===============================

// Headers
#include "assets/import/objParser.h"
#include "math/Vector.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <cerrno>
#include <cstring>
#include <array>
#include <stdexcept>

bool OBJ::buildMeshFromFile(Mesh& mesh, const std::string& path)
{
    std::ifstream file(path);

    if (!file.is_open())
    {

        return false;
    }

    loadFileData(mesh, file);

    return mesh.IsValid();
}

bool OBJ::fileExists(std::string& path)
{
    std::ifstream file(path.c_str());
    return file.good();
}

// Main OBJ parsing function
void OBJ::loadFileData(Mesh& mesh, std::ifstream& file)
{
    mesh = {};
    std::string line;
    while (std::getline(file, line))
    {
        std::istringstream stream(line);
        std::string key;
        stream >> key;
        if (key.empty() || key[0] == '#')
        {
            continue;
        }
        if (key == "v" || key == "vn" || key == "vt")
        {
            Vector3 value;
            if (!(stream >> value.x >> value.y) || (key != "vt" && !(stream >> value.z)) ||
                !std::isfinite(value.x) || !std::isfinite(value.y) || !std::isfinite(value.z))
            {
                throw std::runtime_error("Invalid OBJ vertex");
            }
            if (key == "v")
            {
                mesh.vertices.push_back(value);
            }
            else if (key == "vn")
            {
                mesh.normals.push_back(value);
            }
            else
            {
                mesh.uv.push_back(value);
            }
        }
        else if (key == "f")
        {
            std::vector<Vector3i> corners;
            std::string token;
            const auto index = [](const std::string& text, std::size_t count)
            {
                if (text.empty())
                {
                    return -1;
                }
                std::size_t used = 0;
                const int parsed = std::stoi(text, &used);
                const long long resolved = parsed > 0 ? static_cast<long long>(parsed) - 1 : static_cast<long long>(count) + parsed;
                if (used != text.size() || parsed == 0 || resolved < 0 || resolved >= static_cast<long long>(count))
                {
                    throw std::runtime_error("Invalid OBJ index");
                }
                return static_cast<int>(resolved);
            };
            while (stream >> token)
            {
                if (token[0] == '#')
                {
                    break;
                }
                std::array<std::string, 3> parts;
                std::istringstream fields(token);
                for (int i = 0; i < 3; ++i)
                {
                    std::getline(fields, parts[i], '/');
                }
                if (token.empty() || parts[0].empty() || std::count(token.begin(), token.end(), '/') > 2)
                {
                    throw std::runtime_error("Invalid OBJ face");
                }
                corners.push_back({index(parts[0], mesh.vertices.size()), index(parts[1], mesh.uv.size()), index(parts[2], mesh.normals.size())});
            }
            if (corners.size() < 3)
            {
                throw std::runtime_error("Invalid OBJ face");
            }
            for (std::size_t i = 1; i + 1 < corners.size(); ++i)
            {
                const Vector3i face[3]{corners[0], corners[i], corners[i + 1]};
                Vector3i positions, uvs, normals;

                for (int c = 0; c < 3; ++c)
                {
                    positions[c] = face[c].x;
                    uvs[c] = face[c].y;
                    normals[c] = face[c].z;
                }
                mesh.vertexIndices.push_back(positions);
                mesh.uvIndices.push_back(uvs);
                mesh.normalsIndices.push_back(normals);
            }
        }
    }
    for (std::size_t f = 0; f < mesh.vertexIndices.size(); ++f)
    {
        const auto positions = mesh.vertexIndices[f];
        const Vector3 n = Vector3::Cross(mesh.vertices[positions.y] - mesh.vertices[positions.x], mesh.vertices[positions.z] - mesh.vertices[positions.x]).Normalized();
        for (int c = 0; c < 3; ++c)
        {
            auto& uv = mesh.uvIndices[f][c];
            auto& normal = mesh.normalsIndices[f][c];
            if (uv < 0)
            {
                uv = static_cast<int>(mesh.uv.size());
                mesh.uv.push_back({});
            }
            if (normal < 0)
            {
                normal = static_cast<int>(mesh.normals.size());
                mesh.normals.push_back(n);
            }
        }
    }
    mesh.numVertices = static_cast<int>(mesh.vertices.size());
    mesh.numFaces = static_cast<int>(mesh.vertexIndices.size());
}
// Creates stringstream of string and subdivides it by the delimeter and returns
// a vector of the individual components of the string
std::vector<std::string> OBJ::splitStr(std::string& str, char delim)
{
    std::stringstream ss(str);
    std::string token;
    std::vector<std::string> splitString;
    while (std::getline(ss, token, delim))
    {
        if (token == "")
        {
            // If token is empty just write 0 it will result in a -1 index
            // Since that index number is nonsensical you can catch it pretty easily later
            splitString.push_back("0");
        }
        else
        {
            splitString.push_back(token);
        }
    }
    return splitString;
}

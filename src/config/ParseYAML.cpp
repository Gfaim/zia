#include "config/ParseYAML.hpp"

#include <iostream>
#include <memory>

namespace zia::config {

static Node YAMLToNode(const YAML::Node &node);

static Node DetermineNodeType(const YAML::Node &node)
{
    int nb;
    double dbl;

    if ((std::istringstream(node.as<std::string>()) >> nb >> std::ws).eof())
        return Node(node.as<int>());
    if ((std::istringstream(node.as<std::string>()) >> dbl >> std::ws).eof())
        return Node(node.as<double>());
    if (node.as<std::string>() == "false" || node.as<std::string>() == "true")
        return Node(node.as<bool>());
    return Node(node.as<std::string>());
}

static Node *ParseMapYAML(const YAML::const_iterator &it)
{
    if (it->second.Type() == YAML::NodeType::Null)
        return (new Node(ziapi::config::Null{}));
    if (it->second.Type() == YAML::NodeType::Undefined)
        return (new Node(ziapi::config::Undefined{}));
    if (it->second.Type() == YAML::NodeType::Scalar)
        return (new Node(DetermineNodeType(it->second)));
    if (it->second.Type() == YAML::NodeType::Sequence) {
        ziapi::config::Array val;
        for (std::size_t i = 0; i < it->second.size(); ++i)
            val.push_back(std::make_shared<Node>(Node(YAMLToNode(it->second[i]))));
        return (new Node({val}));
    }
    if (it->second.Type() == YAML::NodeType::Map) {
        ziapi::config::Dict val;
        for (YAML::const_iterator ite = it->second.begin(); ite != it->second.end(); ++ite)
            val.insert(ziapi::config::Dict::value_type(ite->first.as<std::string>(), ParseMapYAML(ite)));
        return (new Node({val}));
    }
    return (new Node(ziapi::config::Undefined{}));
}

static Node YAMLToNode(const YAML::Node &node)
{
    if (node.Type() == YAML::NodeType::Null)
        return Node(ziapi::config::Null{});
    if (node.Type() == YAML::NodeType::Undefined)
        return Node(ziapi::config::Undefined{});
    if (node.Type() == YAML::NodeType::Scalar)
        return DetermineNodeType(node);
    if (node.Type() == YAML::NodeType::Sequence) {
        ziapi::config::Array val;
        for (std::size_t i = 0; i < node.size(); ++i) val.push_back(std::make_shared<Node>(Node(YAMLToNode(node[i]))));
        return Node({val});
    }
    if (node.Type() == YAML::NodeType::Map) {
        ziapi::config::Dict val;
        for (YAML::const_iterator it = node.begin(); it != node.end(); ++it)
            val.insert(ziapi::config::Dict::value_type(it->first.as<std::string>(), ParseMapYAML(it)));
        return Node({val});
    }
    return Node(ziapi::config::Undefined{});
}

Node ParseYAML(const std::string &content)
{
    YAML::Node node = YAML::Load(content);

    return (YAMLToNode(node));
}

Node ParseYAMLFromFile(const std::filesystem::path &file)
{
    YAML::Node node = YAML::LoadFile(file.c_str());

    return YAMLToNode(node);
}

}  // namespace zia::config

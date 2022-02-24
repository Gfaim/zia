#include <gtest/gtest.h>

#include "config/ParseYAML.hpp"

using namespace zia::config;

TEST(yaml_parser_file, test_file)
{
    Node node = ParseYAMLFromFile("./tests/monster.yml");

    EXPECT_EQ(node.AsArray()[0]->AsDict().at("powers")->AsArray()[0]->AsDict().at("damage")->AsInt(), 10);
    EXPECT_EQ(node.AsArray()[0]->AsDict().at("powers")->AsArray()[0]->AsDict().at("name")->AsString(), "Club");
    EXPECT_EQ(node.AsArray()[1]->AsDict().at("position")->AsArray()[1]->AsInt(), 0);
}

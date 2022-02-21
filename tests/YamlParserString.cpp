#include <gtest/gtest.h>

#include "config/ParseYAML.hpp"

using namespace zia::config;

TEST(yaml_parser, empty_string)
{
    Node node = ParseYAML("");

    EXPECT_EQ(node.IsUndefined(), false);
    EXPECT_EQ(node.IsNull(), true);
}

TEST(yaml_parser, null_simple)
{
    Node node = ParseYAML("null");

    EXPECT_EQ(node.IsNull(), true);
    EXPECT_EQ(node, false);
}

TEST(yaml_parser, simple_variable)
{
    Node node = ParseYAML("isOk: false");

    EXPECT_EQ(node["isOk"].AsBool(), false);
}

TEST(yaml_parser, medium_double_cast)
{
    Node node = ParseYAML("isOk: 32.2");

    EXPECT_THROW({ node["isOk"].AsInt(); }, std::bad_variant_access);
    EXPECT_EQ(node["isOk"].AsDouble(), 32.2);
}

TEST(yaml_parser, string_with_quotes)
{
    Node node = ParseYAML("str: \"abcdefg\"");

    EXPECT_EQ(node["str"].AsString(), std::string("abcdefg"));
}

TEST(yaml_parser, string_without_quotes)
{
    Node node = ParseYAML("str: lolololol");

    EXPECT_EQ(node["str"].AsString(), std::string("lolololol"));
}

TEST(yaml_parser, operator_key)
{
    Node node = ParseYAML("str: ouah");

    EXPECT_EQ(node.AsString(), std::string("ouah"));
}

TEST(yaml_parser, operator_index)
{
    Node node = ParseYAML("array: [10, 14, 164]");

    EXPECT_EQ(node["array"][1].AsInt(), 14);
}

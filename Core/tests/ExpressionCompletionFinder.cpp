/*
 * GDevelop Core
 * Copyright 2008-2016 Florian Rival (Florian.Rival@gmail.com). All rights
 * reserved. This project is released under the MIT License.
 */
#include "GDCore/IDE/Events/ExpressionCompletionFinder.h"
#include "DummyPlatform.h"
#include "GDCore/Events/Parsers/ExpressionParser2.h"
#include "GDCore/Extensions/Platform.h"
#include "GDCore/Extensions/PlatformExtension.h"
#include "GDCore/Project/Layout.h"
#include "GDCore/Project/Project.h"
#include "catch.hpp"

TEST_CASE("ExpressionCompletionFinder", "[common][events]") {
  gd::Project project;
  gd::Platform platform;
  SetupProjectWithDummyPlatform(project, platform);
  auto& layout1 = project.InsertNewLayout("Layout1", 0);

  gd::ExpressionParser2 parser(platform, project, layout1);

  auto getCompletionsFor = [&](
      const gd::String& type, const gd::String& expression, size_t location) {
    auto node = parser.ParseExpression(type, expression);
    REQUIRE(node != nullptr);
    return gd::ExpressionCompletionFinder::GetCompletionDescriptionsFor(
        *node, location);
  };

  const std::vector<gd::ExpressionCompletionDescription>
      expectedEmptyCompletions;

  SECTION("Identifier") {
    SECTION("Object or expression completions when type is string") {
      std::vector<gd::ExpressionCompletionDescription> expectedCompletions{
          gd::ExpressionCompletionDescription::ForObject("My"),
          gd::ExpressionCompletionDescription::ForExpression("string", "My")};
      REQUIRE(getCompletionsFor("string", "My", 0) == expectedCompletions);
      REQUIRE(getCompletionsFor("string", "My", 1) == expectedCompletions);
      REQUIRE(getCompletionsFor("string", "My", 2) == expectedEmptyCompletions);
    }
    SECTION("Object or expression completions when type is number") {
      std::vector<gd::ExpressionCompletionDescription> expectedCompletions{
          gd::ExpressionCompletionDescription::ForObject("My"),
          gd::ExpressionCompletionDescription::ForExpression("number", "My")};
      REQUIRE(getCompletionsFor("number", "My", 0) == expectedCompletions);
      REQUIRE(getCompletionsFor("number", "My", 1) == expectedCompletions);
      REQUIRE(getCompletionsFor("number", "My", 2) == expectedEmptyCompletions);
    }
    SECTION("Object when type is an object") {
      std::vector<gd::ExpressionCompletionDescription> expectedCompletions{
          gd::ExpressionCompletionDescription::ForObject("My")};
      REQUIRE(getCompletionsFor("object", "My", 0) == expectedCompletions);
      REQUIRE(getCompletionsFor("object", "My", 1) == expectedCompletions);
      REQUIRE(getCompletionsFor("object", "My", 2) == expectedEmptyCompletions);

      // Also test alternate types also considered as objects (but that can result
      // in different code generation):
      REQUIRE(getCompletionsFor("objectPtr", "My", 0) == expectedCompletions);
      REQUIRE(getCompletionsFor("objectPtr", "My", 1) == expectedCompletions);
      REQUIRE(getCompletionsFor("objectPtr", "My", 2) == expectedEmptyCompletions);
    }
  }
  SECTION("Operator (number)") {
    std::vector<gd::ExpressionCompletionDescription> expectedCompletions{
        gd::ExpressionCompletionDescription::ForObject(""),
        gd::ExpressionCompletionDescription::ForExpression("number", "")};
    REQUIRE(getCompletionsFor("number", "1 + ", 1) == expectedCompletions);
    REQUIRE(getCompletionsFor("number", "1 + ", 2) == expectedCompletions);
    REQUIRE(getCompletionsFor("number", "1 + ", 3) == expectedCompletions);
  }
  SECTION("Operator (string)") {
    std::vector<gd::ExpressionCompletionDescription> expectedCompletions{
        gd::ExpressionCompletionDescription::ForObject(""),
        gd::ExpressionCompletionDescription::ForExpression("string", "")};
    REQUIRE(getCompletionsFor("string", "\"a\" + ", 3) == expectedCompletions);
    REQUIRE(getCompletionsFor("string", "\"a\" + ", 4) == expectedCompletions);
    REQUIRE(getCompletionsFor("string", "\"a\" + ", 5) == expectedCompletions);
  }

  SECTION("Free function") {
    SECTION("Test 1") {
      std::vector<gd::ExpressionCompletionDescription> expectedCompletions{
          gd::ExpressionCompletionDescription::ForExpression("string",
                                                             "Function")};
      REQUIRE(getCompletionsFor("string", "Function(", 0) ==
              expectedCompletions);
      REQUIRE(getCompletionsFor("string", "Function(", 1) ==
              expectedCompletions);
      REQUIRE(getCompletionsFor("string", "Function(", 7) ==
              expectedCompletions);
      REQUIRE(getCompletionsFor("string", "Function(", 8) ==
              expectedCompletions);
      REQUIRE(getCompletionsFor("string", "Function(", 9) ==
              expectedEmptyCompletions);
    }
    SECTION("Unknown function, test with arguments") {
      REQUIRE(getCompletionsFor("string", "Function(1", 9) ==
              expectedEmptyCompletions);
      REQUIRE(getCompletionsFor("string", "Function(\"", 9) ==
              expectedEmptyCompletions);

      std::vector<gd::ExpressionCompletionDescription> expectedCompletions{
          gd::ExpressionCompletionDescription::ForObject("a"),
          gd::ExpressionCompletionDescription::ForExpression("unknown", "a")};
      REQUIRE(getCompletionsFor("string", "Function(a", 9) ==
              expectedCompletions);
    }
    SECTION("Function with a Variable as argument") {
      std::vector<gd::ExpressionCompletionDescription> expectedCompletions{
          gd::ExpressionCompletionDescription::ForVariable("myVar")};
      REQUIRE(getCompletionsFor("number",
                                "MyExtension::GetVariableAsNumber(myVar",
                                33) == expectedCompletions);
    }
  }

  SECTION("Partial object or behavior function") {
    SECTION("Test 1") {
      std::vector<gd::ExpressionCompletionDescription> expectedCompletions{
          gd::ExpressionCompletionDescription::ForBehavior(
              "Func", "MyObject"),
          gd::ExpressionCompletionDescription::ForExpression(
              "string", "Func", "MyObject")};
      REQUIRE(getCompletionsFor("string", "MyObject.Func", 0) ==
              expectedCompletions);
      REQUIRE(getCompletionsFor("string", "MyObject.Func", 5) ==
              expectedCompletions);
      REQUIRE(getCompletionsFor("string", "MyObject.Func", 10) ==
              expectedCompletions);
      REQUIRE(getCompletionsFor("string", "MyObject.Func", 12) ==
              expectedCompletions);
      REQUIRE(getCompletionsFor("string", "MyObject.Func", 13) ==
              expectedEmptyCompletions);
    }
  }

  SECTION("Object function") {
    SECTION("Test 1") {
      std::vector<gd::ExpressionCompletionDescription> expectedCompletions{
          gd::ExpressionCompletionDescription::ForExpression(
              "string", "Func", "MyObject")};
      REQUIRE(getCompletionsFor("string", "MyObject.Func(", 0) ==
              expectedCompletions);
      REQUIRE(getCompletionsFor("string", "MyObject.Func(", 5) ==
              expectedCompletions);
      REQUIRE(getCompletionsFor("string", "MyObject.Func(", 10) ==
              expectedCompletions);
      REQUIRE(getCompletionsFor("string", "MyObject.Func(", 12) ==
              expectedCompletions);
      REQUIRE(getCompletionsFor("string", "MyObject.Func(", 13) ==
              expectedCompletions);
    }
  }

  SECTION("Partial behavior function") {
    SECTION("Test 1") {
      std::vector<gd::ExpressionCompletionDescription> expectedCompletions{
          gd::ExpressionCompletionDescription::ForExpression(
              "string", "Func", "MyObject", "MyBehavior")};
      REQUIRE(getCompletionsFor("string", "MyObject.MyBehavior::Func", 0) ==
              expectedCompletions);
      REQUIRE(getCompletionsFor("string", "MyObject.MyBehavior::Func", 5) ==
              expectedCompletions);
      REQUIRE(getCompletionsFor("string", "MyObject.MyBehavior::Func", 10) ==
              expectedCompletions);
      REQUIRE(getCompletionsFor("string", "MyObject.MyBehavior::Func", 12) ==
              expectedCompletions);
      REQUIRE(getCompletionsFor("string", "MyObject.MyBehavior::Func", 13) ==
              expectedCompletions);
    }
  }

  SECTION("Behavior function") {
    SECTION("Test 1") {
      std::vector<gd::ExpressionCompletionDescription> expectedCompletions{
          gd::ExpressionCompletionDescription::ForExpression(
              "string", "Func", "MyObject", "MyBehavior")};
      REQUIRE(getCompletionsFor("string", "MyObject.MyBehavior::Func(", 0) ==
              expectedCompletions);
      REQUIRE(getCompletionsFor("string", "MyObject.MyBehavior::Func(", 5) ==
              expectedCompletions);
      REQUIRE(getCompletionsFor("string", "MyObject.MyBehavior::Func(", 10) ==
              expectedCompletions);
      REQUIRE(getCompletionsFor("string", "MyObject.MyBehavior::Func(", 24) ==
              expectedCompletions);
      REQUIRE(getCompletionsFor("string", "MyObject.MyBehavior::Func(", 25) ==
              expectedCompletions);
    }
  }
}

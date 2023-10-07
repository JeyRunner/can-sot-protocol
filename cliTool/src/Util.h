#pragma once

#include "iostream"
#include "ftxui/dom/elements.hpp"
#include "yaml-cpp/yaml.h"

using namespace std;


void printEl(ftxui::Element element) {
  using namespace ftxui;

  // Limit the size of the document to 80 char.
  //document = document | size(WIDTH, LESS_THAN, 80);

  auto screen = Screen::Create(Dimension::Fit(element));
  Render(screen, element);

  std::cout << screen.ToString() << '\0' << std::endl;
}

void printEl_asWindow(string title, ftxui::Element element) {
  using namespace ftxui;
  auto w = window(text(title), element);
  printEl(w);
}


ftxui::Element operator ""_T(const char* str, std::size_t) {
  return ftxui::text(string{str});
}


static string firstCharToUpper(string input) {
    string o = input;
    o.at(0) = toupper(o.c_str()[0]);
    return o;
}



/**
 * will print the error and throw.
 * @param errorMsg
 */
void errorAndExit(string errorMsg, string errorType = "GenericError") {
  using namespace ftxui;
  printEl(hbox({">> Error: "_T | bold,
                text(errorMsg)}) |
          color(Color::Red));
  throw runtime_error(errorType);
}


template<class T>
T yamlGetElementOrDefault(YAML::Node node, string key, T defaultValue) {
  if (!node[key]) {
    return defaultValue;
  }
  else {
    return node[key].as<T>();
  }
}
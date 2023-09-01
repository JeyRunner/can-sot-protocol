#pragma once

#include "iostream"
#include "ftxui/dom/elements.hpp"

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
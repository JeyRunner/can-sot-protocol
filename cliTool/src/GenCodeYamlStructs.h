#pragma once

#include <yaml-cpp/yaml.h>


using namespace std;


struct RemoteCall {
  struct Arg {
    string name;
    string type;
  };

  string name;
  string direction;
  list<Arg> args;
  list<Arg> return_values_ok;
  string return_value_error = ""; // if is empty: no return error code

  string typeArgsData;
  string typeReturnData;
  string typeErrorEnum;


  static RemoteCall readFromYaml(YAML::Node node) {
    RemoteCall call;

    if (!node.IsMap()) {
      errorAndExit("remote_calls elements have to be objects.", "remote_calls");
    }
    call.name = node["name"].as<string>();
    call.direction = node["direction"].as<string>();

    if (node["args"]) {
      if (!node["args"].IsSequence()) {
        errorAndExit("remote_calls.args needs to be a list.", "remote_calls");
      }
      for (auto el: node["args"]) {
        Arg arg{
                .name = el["name"].as<string>(),
                .type = el["type"].as<string>()
        };
        call.args.push_back(arg);
      }
    }

    if (node["return_values_ok"]) {
      if (!node["return_values_ok"].IsSequence()) {
        errorAndExit("remote_calls.return_values_ok needs to be a list.", "remote_calls");
      }
      for (auto el: node["return_values_ok"]) {
        Arg arg{
                .name = el["name"].as<string>(),
                .type = el["type"].as<string>()
        };
        call.return_values_ok.push_back(arg);
      }
    }

    call.return_value_error = yamlGetElementOrDefault<string>(node, "return_value_error", "");
    return call;
  }
};

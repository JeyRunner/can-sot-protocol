#pragma once

#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"
#include <ryml.hpp>
#include <ryml_std.hpp>
#include <yaml-cpp/yaml.h>
#include <fstream>
#include "CliArgs.h"
#include "Util.h"
#include "YamlObjectTreeVisitor.h"
#include "GenCodeTemplate.h"

using namespace std;
using namespace ftxui;

class GenCode;


class GenObjectTreeCode: public YamlObjectTreeVisitor {
public:
    string code;
    function<string (string valueNodePath)> getIdForValueNode;
    bool forMaster;

    void visitNodeWithChildren(string nodeName, YAML::Node objectTreeYamlNode, Context context) override {
        code += string(context.nodePathDepthLevel+1, '\t') + "struct " + firstCharToUpper(nodeName) + ": Node {\n";
        YamlObjectTreeVisitor::visitNodeWithChildren(nodeName, objectTreeYamlNode, context);
        code += string(context.nodePathDepthLevel+1, '\t') + "} " + nodeName + "; \n";
    }

    void visitValueNode(ValueNode valueNode, Context context) override {
        string fullPath = context.nodePath+valueNode.name;
        cout << "## " << context.nodePath << "@" << valueNode.name << " -> " << getIdForValueNode(fullPath) << endl;
        auto readWritable = valueNode.getAccessAsCppValueNodeAccStr(forMaster);
        if (!readWritable) {
            cout << ">> Error: node '"<< fullPath <<"' has unknown access value, allowed are: r, w, rw" << endl;
        }
        auto type = valueNode.getTypeAsCppType();
        if (!type) {
            cout << ">> Error: node '"<< fullPath <<"' has unknown type value, allowed are: uint8, uint16, float32" << endl;
            cout << ">> !! HANDLING ENUMS IS NOT IMPLEMENTED -> set this to uint8!" << endl;
            type = "TYPE_UINT8";
        }
        code += string(context.nodePathDepthLevel+2, '\t') + insertIntoTemplate(genCodeTemplate_ValueNode, {
            {"READ_WRITABLE", readWritable.value()},
            {"TYPE", type.value()},
            {"ID", getIdForValueNode(fullPath)},
            {"NAME", valueNode.name},
        });
    }
};


class GenCode{
  public:
    CliArgs::GenProtocolCodeCommand cliArgs;
    /// the key is the value node path, value is the id
    map<string, string> valueNodeIds;

  private:
    GenObjectTreeCode genObjectTreeCode;

  public:
    GenCode(CliArgs::GenProtocolCodeCommand cliArgs)
    : cliArgs(cliArgs) {
        // (bind(&GenCode::getIdForValueNode, *this, std::placeholders::_1));
        genObjectTreeCode.getIdForValueNode = [this](auto && PH1) { return getIdForValueNode(std::forward<decltype(PH1)>(PH1)); };
    }


    void genCode(vector<string> cliCmd) {
      string argsString;
      for (auto a : cliCmd) {argsString += a + " ";}

      //printTest();
      printEl(hbox({">> "_T | bold, "will generate def file from spec file ... "_T}));
      string protocolName = getProtocolNameFromFilename(cliArgs.inputDefFileName);
      printEl_asWindow("Parameters", vbox({
          hbox({text(L" inputDefFileName:       "),text(cliArgs.inputDefFileName) | bold}) | color(Color::Green),
          hbox({text(L" outputHeaderFileName:   "),text(cliArgs.outputHeaderFileName) | bold}) | color(Color::Green),
          hbox({text(L" protocol name:          "),text(protocolName) | bold}) | color(Color::Green),
          hbox({text(L" generate for:           "),text(cliArgs.forMaster ? "master" : "client") | bold}) | color(Color::Green),
      }));
      genObjectTreeCode.forMaster = cliArgs.forMaster;

      //ryml::Tree specYaml = ryml::parse_in_place(s.c_str());
      // read value node ids table
      YAML::Node spec = YAML::LoadFile(cliArgs.inputDefFileName);
      for (auto el : spec["object_tree_node_ids"]) {
          valueNodeIds[el.second.as<string>()] = el.first.as<string>();
      }
      cout << YAML::Node(valueNodeIds) << endl;

      // gen code for object tree
      genObjectTreeCode.acceptObjectTree(spec["object_tree"]);
      // @todo handle enums

      // insert all
      string header = insertIntoTemplate(genCodeTemplateHeaderContent, {
          {"GENERATED_CMD", argsString},
          {"PROTOCOL_CLASS_NAME", firstCharToUpper(protocolName)},
          {"OBJECT_TREE", genObjectTreeCode.code},
          {"NODE_ID_TABLE_SIZE", to_string(valueNodeIds.size())},
          {"NODE_ID_TABLE_CONTENT", genOtNodeIDsTableContent()},
          {"CONSTRUCTOR_SETUP_ALL_NODE_VALUES", genConstructorSetup()},
          {"NODES_TO_SEND_ON_INIT_TABLE_SIZE", "0"}, // @todo
          {"NODES_TO_SEND_ON_INIT_TABLE_CONTENT", ""}, // @todo
      });

      printEl(hbox({">> "_T | bold, text("generated code ("+ to_string(header.size())+" characters)")}) | color(Color::Green));
      //cout << header << endl;

      ofstream outputDefFile(cliArgs.outputHeaderFileName);
      outputDefFile << header;
      outputDefFile.flush();
      outputDefFile.close();
    }



    string genOtNodeIDsTableContent() {
        string t;
        for (auto [path, id] : valueNodeIds) {
            t += "\t\t" + insertIntoTemplate(genCodeTemplate_IdTableEntry_noComma, {{"NODE_PATH", path}}) + ",\n";
        }
        return t;
    }

    string genConstructorSetup() {
        string t;
        for (auto [path, id] : valueNodeIds) {
            t += "\t\t" + insertIntoTemplate(genCodeTemplate_ConstructorSetupEntry, {{"NODE_PATH", path}}) + ",\n";
        }
        return t;
    }




    string getIdForValueNode(string valueNodePath) {
        if (!valueNodeIds.contains(valueNodePath)) {
            cout << endl;
            printEl(hbox({">> "_T | bold, text("can't find value node '" + valueNodePath + "' in object_tree_node_ids ")}) | color(Color::Red));
            cout << "  node id table: " << YAML::Node(valueNodeIds) << endl;
            throw runtime_error("can't find value node id");
        }
        return valueNodeIds[valueNodePath];
    }


    string getProtocolNameFromFilename(string filename) {
        auto loc = filename.find(".");
        if (loc == std::string::npos) {
            cout << "filename does not contain a '.', will just use the full filename as protocol name: " << filename << endl;
            return filename;
        }
        auto s = filename.substr(0, loc);
        return firstCharToUpper(s);
    }
};



